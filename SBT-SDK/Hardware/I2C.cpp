//
// Created by darkr on 10.08.2021.
//

#include "I2C.hpp"
#include "Error.hpp"
#include "GPIO.hpp"
#include "UART.hpp"

static void i2cError(const std::string& comment)
{
    softfault("I2C: " + comment);
}

static void i2cErrorNotInit() { i2cError("Not initialized"); }

static void i2cErrorAlreadyInit() { i2cError("Already initialized"); }

static void i2cErrorUnknownMode() { i2cError("How that even happen"); }

static void i2cErrorUnknownInstance()
{
    i2cError("Somehow instance not set to any I2C...");
}

static void i2cHALErrorGuard(HAL_StatusTypeDef halStatus)
{
    if(halStatus != HAL_OK)
        i2cError("HAL function failed with code " + std::to_string(halStatus));
}

// Register a function created from the template as a callback. callbackType
// must be a constant (literal) expression and not a variable as it is passed as
// the template parameter and must be known at compile time.
#define I2C_REGISTER_CALLBACK(hi2c, callbackType)                              \
    i2cHALErrorGuard(HAL_I2C_RegisterCallback(                                 \
        hi2c,                                                                  \
        static_cast<HAL_I2C_CallbackIDTypeDef>(CallbackType::callbackType),    \
        I2CUniversalCallback<CallbackType::callbackType>));

namespace SBT::Hardware {
// Nested unordered map containing callback functions for each callback type for
// each I2C
static std::unordered_map<
    I2C_TypeDef*, std::unordered_map<I2C::CallbackType, std::function<void()>>>
    callbackFunctions;

// Template from which HAL-compatible callback functions will be created, one
// for each callback type.
template <I2C::CallbackType callbackType>
void I2CUniversalCallback(I2C_HandleTypeDef* hi2c)
{
    // Check if any entry with given key exists. Necessary to avoid allocating
    // memory (which is not allowed in an ISR).
    if(callbackFunctions.count(hi2c->Instance)) {
        auto cfInstance = &callbackFunctions.at(hi2c->Instance);
        if(cfInstance->count(callbackType))
            cfInstance->at(callbackType)();
    }
}

void I2C::Initialize(uint32_t ownAddress)
{
    if(initialized)
        i2cErrorAlreadyInit();

    if(instance == Instance::I2C_2 && uart3.IsInitialized())
        i2cError("Cannot initialize I2C2 along with UART3!");

    switch(instance) {
    case Instance::I2C_1:
        // Enable clocks
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_I2C1_CLK_ENABLE();
        // Set GPIO
        GPIO::Enable(GPIOB, GPIO_PIN_6, GPIO::Mode::AlternateOD,
                     GPIO::Pull::PullUp); // SCL
        GPIO::Enable(GPIOB, GPIO_PIN_7, GPIO::Mode::AlternateOD,
                     GPIO::Pull::PullUp); // SDA
        // Enable interrupts with high priority due to silicon limitation
        // (UM1850 p. 259)
        if(mode == OperatingMode::INTERRUPTS || mode == OperatingMode::DMA) {
            HAL_NVIC_SetPriority(I2C1_EV_IRQn, 9, 0);
            HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
            HAL_NVIC_SetPriority(I2C1_ER_IRQn, 9, 0);
            HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
        }

        state.handle.Instance = I2C1;
        break;

    case Instance::I2C_2:
        // Enable clocks
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_I2C2_CLK_ENABLE();
        // Set GPIO
        GPIO::Enable(GPIOB, GPIO_PIN_10, GPIO::Mode::AlternateOD,
                     GPIO::Pull::PullUp); // SCL
        GPIO::Enable(GPIOB, GPIO_PIN_11, GPIO::Mode::AlternateOD,
                     GPIO::Pull::PullUp); // SDA
        // Enable interrupts with high priority due to silicon limitation
        // (UM1850 p. 259)
        if(mode == OperatingMode::INTERRUPTS || mode == OperatingMode::DMA) {
            HAL_NVIC_SetPriority(I2C2_EV_IRQn, 9, 0);
            HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
            HAL_NVIC_SetPriority(I2C2_ER_IRQn, 9, 0);
            HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
        }

        state.handle.Instance = I2C2;
        break;

    case Instance::NONE:
        i2cErrorUnknownInstance();
    }

    auto& handle = state.handle;
    handle.Init.ClockSpeed = speed;
    handle.Init.DutyCycle = I2C_DUTYCYCLE_2;
    handle.Init.AddressingMode = static_cast<uint32_t>(addressingMode);
    handle.Init.OwnAddress1 = ownAddress;
    handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    handle.Init.OwnAddress2 = 0;
    handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    handle.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    // Set up DMA if selected
    if(mode == OperatingMode::DMA) {
        dmaController->InitController();

        // TX channel
        dmaController->CreateChannel(dmaChannelTx);
        dmaController->SetChannelDirection(dmaChannelTx,
                                           DMA::Direction::MemoryToPeriph);
        dmaController->SetChannelMemInc(dmaChannelTx, DMA::MemInc::Enable);

        // RX channel
        dmaController->CreateChannel(dmaChannelRx);
        dmaController->SetChannelDirection(dmaChannelRx,
                                           DMA::Direction::PeriphToMemory);
        dmaController->SetChannelMemInc(dmaChannelRx, DMA::MemInc::Enable);

        // Link DMA and I2C
        handle.hdmatx = dmaController->InitChannel(dmaChannelTx);
        handle.hdmatx->Parent = &handle;
        handle.hdmarx = dmaController->InitChannel(dmaChannelRx);
        handle.hdmarx->Parent = &handle;
    }

    // Set up MspInit and MspDeInit callbacks
    I2C_REGISTER_CALLBACK(&handle, MspInit)
    I2C_REGISTER_CALLBACK(&handle, MspDeInit)

    // Set registers with prepared data
    i2cHALErrorGuard(HAL_I2C_Init(&handle));

    // Set up the remaining callbacks
    I2C_REGISTER_CALLBACK(&handle, MasterTxComplete)
    I2C_REGISTER_CALLBACK(&handle, MasterRxComplete)
    I2C_REGISTER_CALLBACK(&handle, SlaveTxComplete)
    I2C_REGISTER_CALLBACK(&handle, SlaveRxComplete)
    I2C_REGISTER_CALLBACK(&handle, ListenComplete)
    I2C_REGISTER_CALLBACK(&handle, MemTxComplete)
    I2C_REGISTER_CALLBACK(&handle, MemRxComplete)
    I2C_REGISTER_CALLBACK(&handle, Error)
    I2C_REGISTER_CALLBACK(&handle, Abort)

    initialized = true;
}

void I2C::DeInitialize()
{
    if(!initialized)
        i2cErrorNotInit();

    initialized = false;

    // Deinitialize DMA if selected
    if(mode == OperatingMode::DMA) {
        // TX channel
        dmaController->DeleteChannel(dmaChannelTx);

        // RX channel
        dmaController->DeleteChannel(dmaChannelRx);

        // Do not deinitialize the controller as it may be in use by another
        // device
    }

    switch(instance) {
    case Instance::I2C_1:
        // Disable the I2C1 clock only as GPIOB may be in use by another device
        __HAL_RCC_I2C1_CLK_DISABLE();
        // Disable interrupts
        if(mode == OperatingMode::INTERRUPTS || mode == OperatingMode::DMA) {
            HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
            HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
        }
        break;
    case Instance::I2C_2:
        // Disable the I2C2 clock only as GPIOB may be in use by another device
        __HAL_RCC_I2C2_CLK_DISABLE();
        // Disable interrupts
        if(mode == OperatingMode::INTERRUPTS || mode == OperatingMode::DMA) {
            HAL_NVIC_DisableIRQ(I2C2_EV_IRQn);
            HAL_NVIC_DisableIRQ(I2C2_ER_IRQn);
        }
        break;
    case Instance::NONE:
        i2cErrorUnknownInstance();
    }
}

void I2C::RegisterCallback(CallbackType callbackType,
                           std::function<void()> callbackFunction)
{
    callbackFunctions[state.handle.Instance][callbackType] =
        std::move(callbackFunction);
}

void I2C::SendMaster(uint16_t slaveAddress, uint8_t* data, size_t numOfBytes)
{
    if(!initialized)
        i2cErrorNotInit();

    switch(mode) {
    case OperatingMode::INTERRUPTS:
        SendMasterIT(slaveAddress, data, numOfBytes);
        break;
    case OperatingMode::BLOCKING:
        SendMasterRCC(slaveAddress, data, numOfBytes);
        break;
    case OperatingMode::DMA:
        SendMasterDMA(slaveAddress, data, numOfBytes);
        break;
    default:
        i2cErrorUnknownMode();
    }
}

void I2C::SendMasterIT(uint16_t slaveAddress, uint8_t* data, size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.State == HAL_I2C_STATE_READY) {
        // If I2C is not busy, transmit
        i2cHALErrorGuard(HAL_I2C_Master_Transmit_IT(&state.handle, slaveAddress,
                                                    data, numOfBytes));
    }
}

void I2C::SendMasterDMA(uint16_t slaveAddress, uint8_t* data, size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.State == HAL_I2C_STATE_READY) {
        // If I2C is not busy, transmit
        i2cHALErrorGuard(HAL_I2C_Master_Transmit_DMA(
            &state.handle, slaveAddress, data, numOfBytes));
    }
}

void I2C::SendMasterRCC(uint16_t slaveAddress, uint8_t* data, size_t numOfBytes)
{
    i2cHALErrorGuard(HAL_I2C_Master_Transmit(&state.handle, slaveAddress, data,
                                             numOfBytes, timeout));
}

void I2C::SendSlave(uint8_t* data, size_t numOfBytes)
{
    if(!initialized)
        i2cErrorNotInit();

    switch(mode) {
    case OperatingMode::INTERRUPTS:
        SendSlaveIT(data, numOfBytes);
        break;
    case OperatingMode::BLOCKING:
        SendSlaveRCC(data, numOfBytes);
        break;
    case OperatingMode::DMA:
        SendSlaveDMA(data, numOfBytes);
        break;
    default:
        i2cErrorUnknownMode();
    }
}

void I2C::SendSlaveIT(uint8_t* data, size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.State == HAL_I2C_STATE_READY) {
        // If I2C is not busy, transmit
        i2cHALErrorGuard(
            HAL_I2C_Slave_Transmit_IT(&state.handle, data, numOfBytes));
    }
}

void I2C::SendSlaveDMA(uint8_t* data, size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.State == HAL_I2C_STATE_READY) {
        // If I2C is not busy, transmit
        i2cHALErrorGuard(
            HAL_I2C_Slave_Transmit_DMA(&state.handle, data, numOfBytes));
    }
}

void I2C::SendSlaveRCC(uint8_t* data, size_t numOfBytes)
{
    i2cHALErrorGuard(
        HAL_I2C_Slave_Transmit(&state.handle, data, numOfBytes, timeout));
}

void I2C::ReceiveMaster(uint16_t slaveAddress, uint8_t* data, size_t numOfBytes)
{
    if(!initialized)
        i2cErrorNotInit();

    switch(mode) {
    case OperatingMode::INTERRUPTS:
        ReceiveMasterIT(slaveAddress, data, numOfBytes);
        break;
    case OperatingMode::BLOCKING:
        ReceiveMasterRCC(slaveAddress, data, numOfBytes);
        break;
    case OperatingMode::DMA:
        ReceiveMasterDMA(slaveAddress, data, numOfBytes);
        break;
    default:
        i2cErrorUnknownMode();
    }
}

void I2C::ReceiveMasterIT(uint16_t slaveAddress, uint8_t* data,
                          size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.State == HAL_I2C_STATE_READY) {
        // If I2C is not busy, receive
        i2cHALErrorGuard(HAL_I2C_Master_Receive_IT(&state.handle, slaveAddress,
                                                   data, numOfBytes));
    }
}

void I2C::ReceiveMasterDMA(uint16_t slaveAddress, uint8_t* data,
                           size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.State == HAL_I2C_STATE_READY) {
        // If I2C is not busy, receive
        i2cHALErrorGuard(HAL_I2C_Master_Receive_DMA(&state.handle, slaveAddress,
                                                    data, numOfBytes));
    }
}

void I2C::ReceiveMasterRCC(uint16_t slaveAddress, uint8_t* data,
                           size_t numOfBytes)
{
    i2cHALErrorGuard(HAL_I2C_Master_Receive(&state.handle, slaveAddress, data,
                                            numOfBytes, timeout));
}

void I2C::ReceiveSlave(uint8_t* data, size_t numOfBytes)
{
    if(!initialized)
        i2cErrorNotInit();

    switch(mode) {
    case OperatingMode::INTERRUPTS:
        ReceiveSlaveIT(data, numOfBytes);
        break;
    case OperatingMode::BLOCKING:
        ReceiveSlaveRCC(data, numOfBytes);
        break;
    case OperatingMode::DMA:
        ReceiveSlaveDMA(data, numOfBytes);
        break;
    default:
        i2cErrorUnknownMode();
    }
}

void I2C::ReceiveSlaveIT(uint8_t* data, size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.State == HAL_I2C_STATE_READY) {
        // If I2C is not busy, receive
        i2cHALErrorGuard(
            HAL_I2C_Slave_Receive_IT(&state.handle, data, numOfBytes));
    }
}

void I2C::ReceiveSlaveDMA(uint8_t* data, size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.State == HAL_I2C_STATE_READY) {
        // If I2C is not busy, receive
        i2cHALErrorGuard(
            HAL_I2C_Slave_Receive_DMA(&state.handle, data, numOfBytes));
    }
}

void I2C::ReceiveSlaveRCC(uint8_t* data, size_t numOfBytes)
{
    i2cHALErrorGuard(
        HAL_I2C_Slave_Receive(&state.handle, data, numOfBytes, timeout));
}

void I2C::ReadRegister(uint16_t slaveAddress, uint16_t registerAddress,
                       uint8_t registerSize, uint8_t* data, uint16_t dataSize)
{
    if(!initialized)
        i2cErrorNotInit();

    switch(mode) {
    case OperatingMode::INTERRUPTS:
        ReadRegisterIT(slaveAddress, registerAddress, registerSize, data,
                       dataSize);
        break;
    case OperatingMode::BLOCKING:
        ReadRegisterRCC(slaveAddress, registerAddress, registerSize, data,
                        dataSize);
        break;
    case OperatingMode::DMA:
        ReadRegisterDMA(slaveAddress, registerAddress, registerSize, data,
                        dataSize);
        break;
    default:
        i2cErrorUnknownMode();
    }
}

void I2C::ReadRegisterIT(uint16_t slaveAddress, uint16_t registerAddress,
                         uint8_t registerSize, uint8_t* data, uint16_t dataSize)
{
    // Check if there is no transmission
    if(state.handle.State == HAL_I2C_STATE_READY) {
        // If I2C is not busy, read
        i2cHALErrorGuard(HAL_I2C_Mem_Read_IT(&state.handle, slaveAddress,
                                             registerAddress, registerSize,
                                             data, dataSize));
    }
}

void I2C::ReadRegisterDMA(uint16_t slaveAddress, uint16_t registerAddress,
                          uint8_t registerSize, uint8_t* data,
                          uint16_t dataSize)
{
    // Check if there is no transmission
    if(state.handle.State == HAL_I2C_STATE_READY) {
        // If I2C is not busy, read
        i2cHALErrorGuard(HAL_I2C_Mem_Read_DMA(&state.handle, slaveAddress,
                                              registerAddress, registerSize,
                                              data, dataSize));
    }
}

void I2C::ReadRegisterRCC(uint16_t slaveAddress, uint16_t registerAddress,
                          uint8_t registerSize, uint8_t* data,
                          uint16_t dataSize)
{
    i2cHALErrorGuard(HAL_I2C_Mem_Read(&state.handle, slaveAddress,
                                      registerAddress, registerSize, data,
                                      dataSize, timeout));
}

void I2C::WriteRegister(uint16_t slaveAddress, uint16_t registerAddress,
                        uint8_t registerSize, uint8_t* data, uint16_t dataSize)
{
    if(!initialized)
        i2cErrorNotInit();

    switch(mode) {
    case OperatingMode::INTERRUPTS:
        WriteRegisterIT(slaveAddress, registerAddress, registerSize, data,
                        dataSize);
        break;
    case OperatingMode::BLOCKING:
        WriteRegisterRCC(slaveAddress, registerAddress, registerSize, data,
                         dataSize);
        break;
    case OperatingMode::DMA:
        WriteRegisterDMA(slaveAddress, registerAddress, registerSize, data,
                         dataSize);
        break;
    default:
        i2cErrorUnknownMode();
    }
}

void I2C::WriteRegisterIT(uint16_t slaveAddress, uint16_t registerAddress,
                          uint8_t registerSize, uint8_t* data,
                          uint16_t dataSize)
{
    // Check if there is no transmission
    if(state.handle.State == HAL_I2C_STATE_READY) {
        // If I2C is not busy, write
        i2cHALErrorGuard(HAL_I2C_Mem_Write_IT(&state.handle, slaveAddress,
                                              registerAddress, registerSize,
                                              data, dataSize));
    }
}

void I2C::WriteRegisterDMA(uint16_t slaveAddress, uint16_t registerAddress,
                           uint8_t registerSize, uint8_t* data,
                           uint16_t dataSize)
{
    // Check if there is no transmission
    if(state.handle.State == HAL_I2C_STATE_READY) {
        // If I2C is not busy, write
        i2cHALErrorGuard(HAL_I2C_Mem_Write_DMA(&state.handle, slaveAddress,
                                               registerAddress, registerSize,
                                               data, dataSize));
    }
}

void I2C::WriteRegisterRCC(uint16_t slaveAddress, uint16_t registerAddress,
                           uint8_t registerSize, uint8_t* data,
                           uint16_t dataSize)
{
    i2cHALErrorGuard(HAL_I2C_Mem_Write(&state.handle, slaveAddress,
                                       registerAddress, registerSize, data,
                                       dataSize, timeout));
}

void I2C::ChangeModeToBlocking(uint32_t _timeout)
{
    if(initialized)
        i2cErrorAlreadyInit(); // Too late

    mode = OperatingMode::BLOCKING;
    timeout = _timeout;
}

void I2C::ChangeModeToInterrupts()
{
    if(initialized)
        i2cErrorAlreadyInit(); // Too late

    mode = OperatingMode::INTERRUPTS;
}

void I2C::ChangeModeToDMA()
{
    if(initialized)
        i2cErrorAlreadyInit(); // Too late

    mode = OperatingMode::DMA;
}

void I2C::SetAddressingMode(AddressingMode _addressingMode)
{
    if(initialized)
        i2cErrorAlreadyInit(); // Too late

    addressingMode = _addressingMode;
}

void I2C::SetSpeed(uint32_t _speed)
{
    if(initialized)
        i2cErrorAlreadyInit(); // Too late

    speed = _speed;
}

I2C::I2C(I2C_TypeDef* i2cc)
{
    initialized = false;

    if(i2cc == I2C1) {
        instance = Instance::I2C_1;
        dmaController = &dma1;
        dmaChannelTx = DMA::Channel::Channel6;
        dmaChannelRx = DMA::Channel::Channel7;
    }
    else if(i2cc == I2C2) {
        instance = Instance::I2C_2;
        dmaController = &dma1;
        dmaChannelTx = DMA::Channel::Channel4;
        dmaChannelRx = DMA::Channel::Channel5;
    }
    else
        i2cError("Please choose I2C_1 or I2C_2");

    mode = OperatingMode::INTERRUPTS;
    addressingMode = AddressingMode::_7BIT;
    speed = 100'000;

    timeout = 500;
}

bool I2C::IsTxComplete() const
{
    return state.handle.State != HAL_I2C_STATE_BUSY_TX;
}

bool I2C::IsRxComplete() const
{
    return state.handle.State != HAL_I2C_STATE_BUSY_RX;
}

I2C i2c1(I2C1), i2c2(I2C2);
} // namespace SBT::Hardware

// Handlers for I2C transmission

using namespace SBT::Hardware;

void I2C1_EV_IRQHandler() { HAL_I2C_EV_IRQHandler(&i2c1.GetState().handle); }

void I2C2_EV_IRQHandler() { HAL_I2C_EV_IRQHandler(&i2c2.GetState().handle); }

void I2C1_ER_IRQHandler() { HAL_I2C_ER_IRQHandler(&i2c1.GetState().handle); }

void I2C2_ER_IRQHandler() { HAL_I2C_ER_IRQHandler(&i2c2.GetState().handle); }
