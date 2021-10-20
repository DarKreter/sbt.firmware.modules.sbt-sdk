//
// Created by darkr on 10.08.2021.
//
#include <I2C.hpp>
#include <Hardware.hpp>
#include <stdexcept>


void I2C::Initialize(uint32_t ownAddress)
{
    if(initialized)
        softfault(__FILE__, __LINE__, "I2C already initialized!");

    if(instance == Instance::I2C_2 && Hardware::uart3.IsInitialized())
        softfault(__FILE__, __LINE__, "Cannot initialize I2C2 along with UART3!");
    
    switch (instance)
    {
        case Instance::I2C_1:
            // Enable clocks
            __HAL_RCC_GPIOB_CLK_ENABLE();
            __HAL_RCC_I2C1_CLK_ENABLE();
            // Set GPIO
            Hardware::enableGpio(GPIOB, GPIO_PIN_6, Gpio::Mode::AlternateOD, Gpio::Pull::Pullup);  // SCL
            Hardware::enableGpio(GPIOB, GPIO_PIN_7, Gpio::Mode::AlternateOD, Gpio::Pull::Pullup);  // SDA
            // Enable interrupts with high priority due to silicon limitation (UM1850 p. 259)
            if(mode == OperatingMode::INTERRUPTS)
            {
                HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
                HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
                HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 0);
                HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
            }

            state.handle.Instance = I2C1;
            break;

        case Instance::I2C_2:
            // Enable clocks
            __HAL_RCC_GPIOB_CLK_ENABLE();
            __HAL_RCC_I2C2_CLK_ENABLE();
            // Set GPIO
            Hardware::enableGpio(GPIOB, GPIO_PIN_10, Gpio::Mode::AlternateOD, Gpio::Pull::Pullup);  // SCL
            Hardware::enableGpio(GPIOB, GPIO_PIN_11, Gpio::Mode::AlternateOD, Gpio::Pull::Pullup);  // SDA
            // Enable interrupts with high priority due to silicon limitation (UM1850 p. 259)
            if(mode == OperatingMode::INTERRUPTS)
            {
                HAL_NVIC_SetPriority(I2C2_EV_IRQn, 0, 0);
                HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
                HAL_NVIC_SetPriority(I2C2_ER_IRQn, 0, 0);
                HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
            }

            state.handle.Instance = I2C2;
            break;

        case Instance::NONE:
            softfault(__FILE__, __LINE__, "Somehow instance not set to any I2C...");
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

    // Set registers with prepared data
    HAL_I2C_Init(&handle);

    // Enable interrupts
    if(mode == OperatingMode::INTERRUPTS)
    {
        __HAL_I2C_ENABLE_IT(&handle, I2C_IT_BUF);
        __HAL_I2C_ENABLE_IT(&handle, I2C_IT_EVT);
        __HAL_I2C_ENABLE_IT(&handle, I2C_IT_ERR);
    }

    // Clear bits
    state.txRxState = xEventGroupCreate();
    xEventGroupClearBits(state.txRxState, Hardware::rxBit | Hardware::txBit);

    initialized = true;
}

void I2C::SendMaster(uint16_t slaveAddress, uint8_t *data, size_t numOfBytes)
{
    if(!initialized)
        softfault(__FILE__, __LINE__, "I2C not initialized!");

    switch(mode)
    {
        case OperatingMode::INTERRUPTS:
            SendMasterIT(slaveAddress, data, numOfBytes);
            break;
        case OperatingMode::BLOCKING:
            SendMasterRCC(slaveAddress, data, numOfBytes);
            break;
        case OperatingMode::DMA:
            //SendMasterDMA(slaveAddress, data, numOfBytes);
            //break;
        default:
            softfault(__FILE__, __LINE__, "How that even happen");
            
    }
}

void I2C::SendMasterIT(uint16_t slaveAddress, uint8_t *data, size_t numOfBytes)
{
    // Check if event group was created
    if(state.txRxState)
        // Check if there is no transmission
        if((xEventGroupGetBits(state.txRxState) & Hardware::txBit) == 0)
        {
            // If I2C is not busy, transmit and set TX flag to busy
            HAL_I2C_Master_Transmit_IT(&state.handle, slaveAddress, data, numOfBytes);
            xEventGroupSetBits(state.txRxState, Hardware::txBit);
        }
}

void I2C::SendMasterRCC(uint16_t slaveAddress, uint8_t *data, size_t numOfBytes)
{
    HAL_I2C_Master_Transmit(&state.handle, slaveAddress, data, numOfBytes, timeout);
}

 void I2C::SendSlave(uint8_t *data, size_t numOfBytes)
{
    if(!initialized)
        softfault(__FILE__, __LINE__, "I2C not initialized!");

    switch(mode)
    {
        case OperatingMode::INTERRUPTS:
            SendSlaveIT(data, numOfBytes);
            break;
        case OperatingMode::BLOCKING:
            SendSlaveRCC(data, numOfBytes);
            break;
        case OperatingMode::DMA:
            //SendSlaveDMA(data, numOfBytes);
            //break;
        default:
            softfault(__FILE__, __LINE__, "How that even happen");
    }
}

void I2C::SendSlaveIT(uint8_t *data, size_t numOfBytes)
{
    // Check if event group was created
    if(state.txRxState)
        // Check if there is no transmission
        if((xEventGroupGetBits(state.txRxState) & Hardware::txBit) == 0)
        {
            // If I2C is not busy, transmit and set TX flag to busy
            HAL_I2C_Slave_Transmit_IT(&state.handle, data, numOfBytes);
            xEventGroupSetBits(state.txRxState, Hardware::txBit);
        }

}

void I2C::SendSlaveRCC(uint8_t *data, size_t numOfBytes)
{
    HAL_I2C_Slave_Transmit(&state.handle, data, numOfBytes, timeout);
}


void I2C::ReceiveMaster(uint16_t slaveAddress, uint8_t *data, size_t numOfBytes)
{
    if(!initialized)
        softfault(__FILE__, __LINE__, "I2C not initialized!");

    switch(mode)
    {
        case OperatingMode::INTERRUPTS:
            ReceiveMasterIT(slaveAddress, data, numOfBytes);
            break;
        case OperatingMode::BLOCKING:
            ReceiveMasterRCC(slaveAddress, data, numOfBytes);
            break;
        case OperatingMode::DMA:
            //ReceiveMasterDMA(slaveAddress, data, numOfBytes);
            //break;
        default:
            softfault(__FILE__, __LINE__, "How that even happen");
    }
}

void I2C::ReceiveMasterIT(uint16_t slaveAddress, uint8_t *data, size_t numOfBytes)
{
    // Check if event group was created
    if(state.txRxState)
        // Check if there is no transmission
        if((xEventGroupGetBits(state.txRxState) & Hardware::rxBit) == 0)
        {
            // If I2C is not busy, transmit and set TX flag to busy
            HAL_I2C_Master_Receive_IT(&state.handle, slaveAddress, data, numOfBytes);
            xEventGroupSetBits(state.txRxState, Hardware::rxBit);
        }
}

void I2C::ReceiveMasterRCC(uint16_t slaveAddress, uint8_t *data, size_t numOfBytes)
{
    HAL_I2C_Master_Receive(&state.handle, slaveAddress, data, numOfBytes, timeout);
}

void I2C::ReceiveSlave(uint8_t *data, size_t numOfBytes)
{
    if(!initialized)
        softfault(__FILE__, __LINE__, "I2C not initialized!");

    switch(mode)
    {
        case OperatingMode::INTERRUPTS:
            ReceiveSlaveIT(data, numOfBytes);
            break;
        case OperatingMode::BLOCKING:
            ReceiveSlaveRCC(data, numOfBytes);
            break;
        case OperatingMode::DMA:
            //ReceiveSlaveDMA(data, numOfBytes);
            //break;
        default:
            softfault(__FILE__, __LINE__, "How that even happen");
    }
}

void I2C::ReceiveSlaveIT(uint8_t *data, size_t numOfBytes)
{
    // Check if event group was created
    if(state.txRxState)
        // Check if there is no transmission
        if((xEventGroupGetBits(state.txRxState) & Hardware::rxBit) == 0)
        {
            // If I2C is not busy, transmit and set TX flag to busy
            HAL_I2C_Slave_Receive_IT(&state.handle, data, numOfBytes);
            xEventGroupSetBits(state.txRxState, Hardware::rxBit);
        }

}

void I2C::ReceiveSlaveRCC(uint8_t *data, size_t numOfBytes)
{
    HAL_I2C_Slave_Receive(&state.handle, data, numOfBytes, timeout);
}

void I2C::ReadRegister(uint16_t slaveAddress, uint16_t registerAddress, uint8_t registerSize, uint8_t* data, uint16_t dataSize)
{
    if(!initialized)
        softfault(__FILE__, __LINE__, "I2C not initialized!");

    switch(mode)
    {
        case OperatingMode::INTERRUPTS:
            ReadRegisterIT(slaveAddress, registerAddress, registerSize, data, dataSize);
            break;
        case OperatingMode::BLOCKING:
            ReadRegisterRCC(slaveAddress, registerAddress, registerSize, data, dataSize);
            break;
        case OperatingMode::DMA:
            //ReceiveSlaveDMA(data, numOfBytes);
            //break;
        default:
            softfault(__FILE__, __LINE__, "How that even happen");
    }
}

void I2C::ReadRegisterIT(uint16_t slaveAddress, uint16_t registerAddress, uint8_t registerSize, uint8_t* data, uint16_t dataSize)
{
    // Check if event group was created
    if(state.txRxState)
        // Check if there is no transmission
        if((xEventGroupGetBits(state.txRxState) & Hardware::rxBit) == 0)
        {
            // If I2C is not busy, transmit and set TX flag to busy
            HAL_I2C_Mem_Read_IT(&state.handle, slaveAddress, registerAddress, registerSize, data, dataSize);
            xEventGroupSetBits(state.txRxState, Hardware::rxBit);
        }
}

void I2C::ReadRegisterRCC(uint16_t slaveAddress, uint16_t registerAddress, uint8_t registerSize, uint8_t* data, uint16_t dataSize)
{
    HAL_I2C_Mem_Read(&state.handle, slaveAddress, registerAddress, registerSize, data, dataSize, timeout);
}

void I2C::WriteRegister(uint16_t slaveAddress, uint16_t registerAddress, uint8_t registerSize, uint8_t* data, uint16_t dataSize)
{
    if(!initialized)
        softfault(__FILE__, __LINE__, "I2C not initialized!");

    switch(mode)
    {
        case OperatingMode::INTERRUPTS:
            WriteRegisterIT(slaveAddress, registerAddress, registerSize, data, dataSize);
            break;
        case OperatingMode::BLOCKING:
            WriteRegisterRCC(slaveAddress, registerAddress, registerSize, data, dataSize);
            break;
        case OperatingMode::DMA:
            //ReceiveSlaveDMA(data, numOfBytes);
            //break;
        default:
            softfault(__FILE__, __LINE__, "How that even happen");
    }
}

void I2C::WriteRegisterIT(uint16_t slaveAddress, uint16_t registerAddress, uint8_t registerSize, uint8_t* data, uint16_t dataSize)
{
    // Check if event group was created
    if(state.txRxState)
        // Check if there is no transmission
        if((xEventGroupGetBits(state.txRxState) & Hardware::txBit) == 0)
        {
            // If I2C is not busy, transmit and set TX flag to busy
            HAL_I2C_Mem_Write_IT(&state.handle, slaveAddress, registerAddress, registerSize, data, dataSize);
            xEventGroupSetBits(state.txRxState, Hardware::txBit);
        }
}

void I2C::WriteRegisterRCC(uint16_t slaveAddress, uint16_t registerAddress, uint8_t registerSize, uint8_t* data, uint16_t dataSize)
{
    HAL_I2C_Mem_Write(&state.handle, slaveAddress, registerAddress, registerSize, data, dataSize, timeout);
}

void I2C::ChangeModeToBlocking(uint32_t _timeout)
{
    if(initialized)
        softfault(__FILE__, __LINE__, "I2C already initialized!");// Too late

    mode = OperatingMode::BLOCKING;
    timeout = _timeout;
}

//void I2C::ChangeModeToInterrupts()
//{
//    if(initialized)
//        softfault(__FILE__, __LINE__, "I2C already initialized!");// Too late
//
//
//    mode = OperatingMode::INTERRUPTS;
//}

void I2C::SetAddressingMode(AddressingMode _addressingMode)
{
    if(initialized)
        softfault(__FILE__, __LINE__, "I2C already initialized!"); // Too late

    addressingMode = _addressingMode;
}

void I2C::SetSpeed(uint32_t _speed)
{
    if(initialized)
        softfault(__FILE__, __LINE__, "I2C already initialized!");// Too late

    speed = _speed;
}

I2C::I2C(I2C_TypeDef *i2cc)
{
    initialized = false;

    if (i2cc == I2C1)
        instance = Instance::I2C_1;
    else if (i2cc == I2C2)
        instance = Instance::I2C_2;
    else
        softfault(__FILE__, __LINE__, "Please choose I2C_1 or I2C_2");

    mode = OperatingMode::BLOCKING;
    addressingMode = AddressingMode::_7BIT;
    speed = 100'000;

    timeout = 500;
}

bool I2C::IsTxComplete() const
{
    return (xEventGroupGetBits(state.txRxState) & Hardware::txBit) == 0;
}

bool I2C::IsRxComplete() const
{
    return (xEventGroupGetBits(state.txRxState) & Hardware::rxBit) == 0;
}


// Handlers for I2C transmission
void I2C1_EV_IRQHandler()
{
    HAL_I2C_EV_IRQHandler(&Hardware::i2c1.GetState().handle);
}

void I2C2_EV_IRQHandler()
{
    HAL_I2C_EV_IRQHandler(&Hardware::i2c2.GetState().handle);
}

void I2C1_ER_IRQHandler()
{
    HAL_I2C_ER_IRQHandler(&Hardware::i2c1.GetState().handle);
}

void I2C2_ER_IRQHandler()
{
    HAL_I2C_ER_IRQHandler(&Hardware::i2c2.GetState().handle);
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c){

    if(hi2c->Instance == I2C1)
    {
        if(auto* eventGroup = Hardware::i2c1.GetState().txRxState)
            xEventGroupClearBitsFromISR(eventGroup, Hardware::txBit);
    }
    else if(hi2c->Instance == I2C2)
    {
        if(auto* eventGroup = Hardware::i2c2.GetState().txRxState)
            xEventGroupClearBitsFromISR(eventGroup, Hardware::txBit);
    }


}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if(hi2c->Instance == I2C1)
    {
        if(auto* eventGroup = Hardware::i2c1.GetState().txRxState)
            xEventGroupClearBitsFromISR(eventGroup, Hardware::rxBit);
    }
    else if(hi2c->Instance == I2C2)
    {
        if(auto* eventGroup = Hardware::i2c2.GetState().txRxState)
            xEventGroupClearBitsFromISR(eventGroup, Hardware::rxBit);
    }
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if(hi2c->Instance == I2C1)
    {
        if(auto* eventGroup = Hardware::i2c1.GetState().txRxState)
            xEventGroupClearBitsFromISR(eventGroup, Hardware::rxBit);
    }
    else if(hi2c->Instance == I2C2)
    {
        if(auto* eventGroup = Hardware::i2c2.GetState().txRxState)
            xEventGroupClearBitsFromISR(eventGroup, Hardware::rxBit);
    }
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if(hi2c->Instance == I2C1)
    {
        if(auto* eventGroup = Hardware::i2c1.GetState().txRxState)
            xEventGroupClearBitsFromISR(eventGroup, Hardware::txBit);
    }
    else if(hi2c->Instance == I2C2)
    {
        if(auto* eventGroup = Hardware::i2c2.GetState().txRxState)
            xEventGroupClearBitsFromISR(eventGroup, Hardware::txBit);
    }
    
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if(hi2c->Instance == I2C1)
    {
        if(auto* eventGroup = Hardware::i2c1.GetState().txRxState)
            xEventGroupClearBitsFromISR(eventGroup, Hardware::rxBit);
    }
    else if(hi2c->Instance == I2C2)
    {
        if(auto* eventGroup = Hardware::i2c2.GetState().txRxState)
            xEventGroupClearBitsFromISR(eventGroup, Hardware::rxBit);
    }
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if(hi2c->Instance == I2C1)
    {
        if(auto* eventGroup = Hardware::i2c1.GetState().txRxState)
            xEventGroupClearBitsFromISR(eventGroup, Hardware::txBit);
    }
    else if(hi2c->Instance == I2C2)
    {
        if(auto* eventGroup = Hardware::i2c2.GetState().txRxState)
            xEventGroupClearBitsFromISR(eventGroup, Hardware::txBit);
    }
    
}