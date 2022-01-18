//
// Created by darkr on 18.05.2021.
//

#include "GPIO.hpp"
#include "Hardware.hpp"
#include <stdexcept>

#define SPI_ERROR(comment)                                                     \
    softfault(__FILE__, __LINE__, std::string("SPI: ") + std::string(comment))

#define SPI_ERROR_NOT_INIT     SPI_ERROR("Not initialized")
#define SPI_ERROR_ALREADY_INIT SPI_ERROR("Already initialized")
#define SPI_ERROR_UNKNOWN_MODE SPI_ERROR("How that even happen")
#define SPI_HAL_ERROR_GUARD(function)                                          \
    {                                                                          \
        HAL_StatusTypeDef halStatus = function;                                \
        if(halStatus != HAL_OK)                                                \
            SPI_ERROR(std::string("HAL function failed with code ") +          \
                      std::to_string(halStatus));                              \
    }

// Register a function created from the template as a callback. callbackType
// must be a constant (literal) expression and not a variable as it is passed as
// the template parameter and must be known at compile time.
#define SPI_REGISTER_CALLBACK(hspi, callbackType)                              \
    SPI_HAL_ERROR_GUARD(HAL_SPI_RegisterCallback(                              \
        hspi,                                                                  \
        static_cast<HAL_SPI_CallbackIDTypeDef>(CallbackType::callbackType),    \
        SPIUniversalCallback<CallbackType::callbackType>))

// Nested unordered map containing callback functions for each callback type for
// each SPI
static std::unordered_map<
    SPI_TypeDef*,
    std::unordered_map<SPI_t::CallbackType, std::function<void()>>>
    callbackFunctions;

// Template from which HAL-compatible callback functions will be created, one
// for each callback type.
template <SPI_t::CallbackType callbackType>
void SPIUniversalCallback(SPI_HandleTypeDef* hspi)
{
    auto callbackFunction = callbackFunctions[hspi->Instance][callbackType];
    // Check if the function is actually able to be called (not empty)
    if(callbackFunction)
        callbackFunction();
}

void SPI_t::Initialize()
{
    if(initialized)
        SPI_ERROR_ALREADY_INIT;

    CalculateMisoMosi();
    CalculateSpeed();

    {
        using namespace SBT::Hardware;

        switch(instance) {
        case Instance::SPI_1:
            // Enable clocks
            __HAL_RCC_GPIOA_CLK_ENABLE();
            __HAL_RCC_SPI1_CLK_ENABLE();
            // Set GPIO
            GPIO::Enable(GPIOA, GPIO_PIN_5, GPIO::Mode::AlternatePP,
                         GPIO::Pull::NoPull); // SCK
            if(misoEnabled)
                GPIO::Enable(GPIOA, GPIO_PIN_6, GPIO::Mode::AlternateInput,
                             GPIO::Pull::NoPull); // MISO
            if(mosiEnabled)
                GPIO::Enable(GPIOA, GPIO_PIN_7, GPIO::Mode::AlternatePP,
                             GPIO::Pull::NoPull); // MOSI
            // Enable interrupts with low priority
            if(mode == OperatingMode::INTERRUPTS ||
               mode == OperatingMode::DMA) {
                HAL_NVIC_SetPriority(SPI1_IRQn, 5, 5);
                HAL_NVIC_EnableIRQ(SPI1_IRQn);
            }
            break;
        case Instance::SPI_2:
            // Enable clocks
            __HAL_RCC_GPIOA_CLK_ENABLE();
            __HAL_RCC_SPI2_CLK_ENABLE();
            // Set GPIO
            GPIO::Enable(GPIOB, GPIO_PIN_13, GPIO::Mode::AlternatePP,
                         GPIO::Pull::NoPull); // SCK
            if(misoEnabled)
                GPIO::Enable(GPIOB, GPIO_PIN_14, GPIO::Mode::AlternateInput,
                             GPIO::Pull::NoPull); // MISO
            if(mosiEnabled)
                GPIO::Enable(GPIOB, GPIO_PIN_15, GPIO::Mode::AlternatePP,
                             GPIO::Pull::NoPull); // MOSI
            // Enable interrupts with low priority
            if(mode == OperatingMode::INTERRUPTS ||
               mode == OperatingMode::DMA) {
                HAL_NVIC_SetPriority(SPI2_IRQn, 5, 5);
                HAL_NVIC_EnableIRQ(SPI2_IRQn);
            }
            break;
        case Instance::NONE:
            SPI_ERROR("Somehow instance not set to any SPI...");
        }
    }

    auto& handle = state.handle;
    handle.Instance = instance == Instance::SPI_1 ? SPI1 : SPI2;
    handle.Init.Mode = static_cast<uint32_t>(deviceType);
    handle.Init.Direction = static_cast<uint32_t>(direction);
    handle.Init.BaudRatePrescaler = static_cast<uint32_t>(prescaler);
    handle.Init.CLKPhase = static_cast<uint32_t>(clockPhase);
    handle.Init.CLKPolarity = static_cast<uint32_t>(clockPolarity);
    handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    handle.Init.DataSize = static_cast<uint32_t>(dataSize);
    handle.Init.FirstBit = static_cast<uint32_t>(firstBit);
    handle.Init.NSS = SPI_NSS_SOFT;
    handle.Init.TIMode = SPI_TIMODE_DISABLE;

    // Set up DMA if selected
    if(mode == OperatingMode::DMA) {
        dmaController->InitController();

        // Determine data alignment
        DMA::PeriphDataAlignment periphDataAlignment =
            DMA::PeriphDataAlignment::Byte;
        DMA::MemDataAlignment memDataAlignment = DMA::MemDataAlignment::Byte;
        if(dataSize == DataSize::_16BIT) {
            periphDataAlignment = DMA::PeriphDataAlignment::HalfWord;
            memDataAlignment = DMA::MemDataAlignment::HalfWord;
        }

        // TX channel
        dmaController->CreateChannel(dmaChannelTx);
        dmaController->SetChannelDirection(dmaChannelTx,
                                           DMA::Direction::MemoryToPeriph);
        dmaController->SetChannelMemInc(dmaChannelTx, DMA::MemInc::Enable);
        dmaController->SetChannelPeriphDataAlignment(dmaChannelTx,
                                                     periphDataAlignment);
        dmaController->SetChannelMemDataAlignment(dmaChannelTx,
                                                  memDataAlignment);

        // RX channel
        dmaController->CreateChannel(dmaChannelRx);
        dmaController->SetChannelDirection(dmaChannelRx,
                                           DMA::Direction::MemoryToPeriph);
        dmaController->SetChannelMemInc(dmaChannelRx, DMA::MemInc::Enable);
        dmaController->SetChannelPeriphDataAlignment(dmaChannelRx,
                                                     periphDataAlignment);
        dmaController->SetChannelMemDataAlignment(dmaChannelRx,
                                                  memDataAlignment);

        // Link SPI and DMA channel
        handle.hdmatx = dmaController->InitChannel(dmaChannelTx);
        handle.hdmatx->Parent = &handle;
        handle.hdmarx = dmaController->InitChannel(dmaChannelRx);
        handle.hdmarx->Parent = &handle;
    }

    // Set up MspInit and MspDeInit callbacks
    SPI_REGISTER_CALLBACK(&handle, MspInit)
    SPI_REGISTER_CALLBACK(&handle, MspDeInit)

    SPI_HAL_ERROR_GUARD(HAL_SPI_Init(&handle))

    // Set up the remaining callbacks
    SPI_REGISTER_CALLBACK(&handle, TxComplete)
    SPI_REGISTER_CALLBACK(&handle, RxComplete)
    SPI_REGISTER_CALLBACK(&handle, TxRxComplete)
    SPI_REGISTER_CALLBACK(&handle, TxHalfComplete)
    SPI_REGISTER_CALLBACK(&handle, RxHalfComplete)
    SPI_REGISTER_CALLBACK(&handle, TxRxHalfComplete)
    SPI_REGISTER_CALLBACK(&handle, Error)
    SPI_REGISTER_CALLBACK(&handle, Abort)

    initialized = true;
}

void SPI_t::RegisterCallback(CallbackType callbackType,
                             std::function<void()> callbackFunction)
{
    callbackFunctions[state.handle.Instance][callbackType] =
        std::move(callbackFunction);
}

void SPI_t::Send(uint8_t* data, size_t numOfBytes)
{
    if(!initialized)
        SPI_ERROR_NOT_INIT;

    switch(mode) {
    case OperatingMode::INTERRUPTS:
        SendIT(data, numOfBytes);
        break;
    case OperatingMode::BLOCKING:
        SendRCC(data, numOfBytes);
        break;
    case OperatingMode::DMA:
        SendDMA(data, numOfBytes);
        break;
    default:
        SPI_ERROR_UNKNOWN_MODE;
    }
}

void SPI_t::SendIT(uint8_t* data, size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.State == HAL_SPI_STATE_READY) {
        // If SPI is not busy, transmit
        SPI_HAL_ERROR_GUARD(
            HAL_SPI_Transmit_IT(&state.handle, data, numOfBytes))
    }
}

void SPI_t::SendDMA(uint8_t* data, size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.State == HAL_SPI_STATE_READY) {
        // If SPI is not busy, transmit
        SPI_HAL_ERROR_GUARD(
            HAL_SPI_Transmit_DMA(&state.handle, data, numOfBytes))
    }
}

void SPI_t::SendRCC(uint8_t* data, size_t numOfBytes)
{
    SPI_HAL_ERROR_GUARD(
        HAL_SPI_Transmit(&state.handle, data, numOfBytes, timeout))
}

void SPI_t::Receive(uint8_t* data, size_t numOfBytes)
{
    if(!initialized)
        SPI_ERROR_NOT_INIT;

    switch(mode) {
    case OperatingMode::INTERRUPTS:
        ReceiveIT(data, numOfBytes);
        break;
    case OperatingMode::BLOCKING:
        ReceiveRCC(data, numOfBytes);
        break;
    case OperatingMode::DMA:
        ReceiveDMA(data, numOfBytes);
        break;
    default:
        SPI_ERROR_UNKNOWN_MODE;
    }
}

void SPI_t::ReceiveIT(uint8_t* data, size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.State == HAL_SPI_STATE_READY) {
        // If SPI is not busy, receive
        SPI_HAL_ERROR_GUARD(HAL_SPI_Receive_IT(&state.handle, data, numOfBytes))
    }
}

void SPI_t::ReceiveDMA(uint8_t* data, size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.State == HAL_SPI_STATE_READY) {
        // If SPI is not busy, receive
        SPI_HAL_ERROR_GUARD(
            HAL_SPI_Receive_DMA(&state.handle, data, numOfBytes))
    }
}

void SPI_t::ReceiveRCC(uint8_t* data, size_t numOfBytes)
{
    SPI_HAL_ERROR_GUARD(
        HAL_SPI_Receive(&state.handle, data, numOfBytes, timeout))
}

bool SPI_t::IsTxComplete() const
{
    return !(state.handle.State == HAL_SPI_STATE_BUSY_TX ||
             state.handle.State == HAL_SPI_STATE_BUSY_TX_RX);
}

bool SPI_t::IsRxComplete() const
{
    return !(state.handle.State == HAL_SPI_STATE_BUSY_RX ||
             state.handle.State == HAL_SPI_STATE_BUSY_TX_RX);
}

void SPI_t::Abort(){SPI_HAL_ERROR_GUARD(HAL_SPI_Abort(&state.handle))}

SPI_t::SPI_t(SPI_TypeDef* spii)
{
    initialized = false;

    if(spii == SPI1) {
        instance = Instance::SPI_1;
        dmaController = &Hardware::dma1;
        dmaChannelTx = DMA::Channel::Channel3;
        dmaChannelRx = DMA::Channel::Channel2;
    }
    else if(spii == SPI2) {
        instance = Instance::SPI_2;
        dmaController = &Hardware::dma1;
        dmaChannelTx = DMA::Channel::Channel5;
        dmaChannelRx = DMA::Channel::Channel4;
    }
    else
        SPI_ERROR("Please choose SPI_1 or SPI_2");

    mode = OperatingMode::INTERRUPTS;
    dataSize = DataSize::_8BIT;
    firstBit = FirstBit::MSB;
    clockPolarity = ClockPolarity::HIGH;
    clockPhase = ClockPhase::_2EDGE;
    transmissionMode = TransmissionMode::FULL_DUPLEX;
    deviceType = DeviceType::MASTER;

    prescaler = Prescaler::PRESCALER_8;

    baudRate = 1'000'000;
    timeout = 500;
}

void SPI_t::SetPrescaler(SPI_t::Prescaler _prescaler)
{
    if(initialized)
        SPI_ERROR_ALREADY_INIT;

    prescaler = _prescaler;
}

void SPI_t::SetBaudRate(int32_t _baudRate)
{
    if(initialized)
        SPI_ERROR_ALREADY_INIT;

    if(_baudRate < 1 || 18'000'000 < _baudRate)
        SPI_ERROR("Baud rate for SPI must be between 1 and 18'000'000");

    baudRate = _baudRate;
}

void SPI_t::SetDataSize(DataSize _dataSize)
{
    if(initialized)
        SPI_ERROR_ALREADY_INIT; // Too late

    dataSize = _dataSize;
}

void SPI_t::SetFirstBit(FirstBit _firstBit)
{
    if(initialized)
        SPI_ERROR_ALREADY_INIT; // Too late

    firstBit = _firstBit;
}

void SPI_t::SetClockPolarity(ClockPolarity _clockPolarity)
{
    if(initialized)
        SPI_ERROR_ALREADY_INIT; // Too late

    clockPolarity = _clockPolarity;
}

void SPI_t::SetClockPhase(ClockPhase _clockPhase)
{
    if(initialized)
        SPI_ERROR_ALREADY_INIT; // Too late

    clockPhase = _clockPhase;
}

void SPI_t::ChangeModeToBlocking(uint32_t _timeout)
{
    if(initialized)
        SPI_ERROR_ALREADY_INIT; // Too late

    mode = OperatingMode::BLOCKING;
    timeout = _timeout;
}

void SPI_t::ChangeModeToInterrupts()
{
    if(initialized)
        SPI_ERROR_ALREADY_INIT; // Too late

    mode = OperatingMode::INTERRUPTS;
}

void SPI_t::ChangeModeToDMA()
{
    if(initialized)
        SPI_ERROR_ALREADY_INIT; // Too late

    mode = OperatingMode::DMA;
}

void SPI_t::SetTransmissionMode(SPI_t::TransmissionMode _transmissionMode)
{
    if(initialized)
        SPI_ERROR_ALREADY_INIT; // Too late

    transmissionMode = _transmissionMode;
}

void SPI_t::SetDeviceType(SPI_t::DeviceType _deviceType)
{
    if(initialized)
        SPI_ERROR_ALREADY_INIT; // Too late

    deviceType = _deviceType;
}

void SPI_t::CalculateSpeed()
{
    constexpr std::array possiblePrescalers = {
        std::make_pair(2, Prescaler::PRESCALER_2),
        std::make_pair(4, Prescaler::PRESCALER_4),
        std::make_pair(8, Prescaler::PRESCALER_8),
        std::make_pair(16, Prescaler::PRESCALER_16),
        std::make_pair(32, Prescaler::PRESCALER_32),
        std::make_pair(64, Prescaler::PRESCALER_64),
        std::make_pair(128, Prescaler::PRESCALER_128),
        std::make_pair(256, Prescaler::PRESCALER_256)};

    Prescaler bestPrescalerSet = static_cast<Prescaler>(0);
    int32_t bestPrescalerValue = 0;
    int32_t lowestAbsoluteError = baudRate;
    const int32_t clkSpeed =
        (instance == Instance::SPI_1 ? Hardware::GetAPB2_Freq()
                                     : Hardware::GetAPB1_Freq());

    for(auto [prescalerValue, prescalerSet] : possiblePrescalers) {
        if(std::abs(static_cast<int32_t>(
               baudRate - (clkSpeed / prescalerValue))) < lowestAbsoluteError) {
            bestPrescalerSet = prescalerSet;
            bestPrescalerValue = prescalerValue;
            lowestAbsoluteError = std::abs(
                static_cast<int32_t>(baudRate - (clkSpeed / prescalerValue)));
        }
        if(!lowestAbsoluteError)
            break;
    }

    prescaler = bestPrescalerSet;
    baudRate = clkSpeed / bestPrescalerValue;
}

void SPI_t::CalculateMisoMosi()
{
    mosiEnabled = misoEnabled = true;

    switch(transmissionMode) {
    case TransmissionMode::FULL_DUPLEX:
        direction = SPI_DIRECTION_2LINES;
        break;

    case TransmissionMode::HALF_DUPLEX:
        direction = SPI_DIRECTION_1LINE;
        if(deviceType == DeviceType::MASTER)
            misoEnabled = false;
        else
            mosiEnabled = false;

        break;
    case TransmissionMode::RECEIVE_ONLY:
        direction = SPI_DIRECTION_2LINES_RXONLY;
        if(deviceType == DeviceType::MASTER)
            mosiEnabled = false;
        else
            misoEnabled = false;

        break;
    case TransmissionMode::TRANSMIT_ONLY:
        direction = SPI_DIRECTION_2LINES;
        if(deviceType == DeviceType::MASTER)
            misoEnabled = false;
        else
            mosiEnabled = false;

        break;
    }
}

// Handlers for SPI transmission
void SPI1_IRQHandler()
{
    HAL_SPI_IRQHandler(&Hardware::spi1.GetState().handle);
}

void SPI2_IRQHandler()
{
    HAL_SPI_IRQHandler(&Hardware::spi2.GetState().handle);
}
