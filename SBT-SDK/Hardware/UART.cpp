//
// Created by darkr on 15.05.2021.
//

#include "UART.hpp"
#include "Error.hpp"
#include "GPIO.hpp"
#include "I2C.hpp"
#include <cstdarg>
#include <cstring>

#define UART_ERROR(comment)                                                    \
    softfault(__FILE__, __LINE__, std::string("UART: ") + std::string(comment))

#define UART_ERROR_NOT_INIT     UART_ERROR("Not initialized")
#define UART_ERROR_ALREADY_INIT UART_ERROR("Already initialized")
#define UART_ERROR_UNKNOWN_MODE UART_ERROR("How that even happen")
#define UART_HAL_ERROR_GUARD(function)                                         \
    {                                                                          \
        HAL_StatusTypeDef halStatus = function;                                \
        if(halStatus != HAL_OK)                                                \
            UART_ERROR(std::string("HAL function failed with code ") +         \
                       std::to_string(halStatus));                             \
    }

// Register a function created from the template as a callback. callbackType
// must be a constant (literal) expression and not a variable as it is passed as
// the template parameter and must be known at compile time.
#define UART_REGISTER_CALLBACK(huart, callbackType)                            \
    UART_HAL_ERROR_GUARD(HAL_UART_RegisterCallback(                            \
        huart,                                                                 \
        static_cast<HAL_UART_CallbackIDTypeDef>(CallbackType::callbackType),   \
        UARTUniversalCallback<CallbackType::callbackType>))

namespace SBT::Hardware {
// Nested unordered map containing callback functions for each callback type for
// each UART
static std::unordered_map<
    USART_TypeDef*,
    std::unordered_map<UART::CallbackType, std::function<void()>>>
    callbackFunctions;

// Template from which HAL-compatible callback functions will be created, one
// for each callback type.
template <UART::CallbackType callbackType>
void UARTUniversalCallback(UART_HandleTypeDef* huart)
{
    // Check if any entry with given key exists. Necessary to avoid allocating
    // memory (which is not allowed in an ISR).
    if(callbackFunctions.count(huart->Instance)) {
        auto cfInstance = &callbackFunctions.at(huart->Instance);
        if(cfInstance->count(callbackType))
            cfInstance->at(callbackType)();
    }
}

void UART::Initialize()
{

    if(initialized)
        UART_ERROR_ALREADY_INIT;

    if(instance == Instance::UART_3 && i2c2.IsInitialized())
        UART_ERROR("Cannot initialize UART3 along with I2C2!");

    switch(instance) {
    case Instance::UART_1:
        // Enable clocks
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_USART1_CLK_ENABLE();
        // Set GPIO
        if(transmissionMode != TransmissionMode::RECEIVE_ONLY)
            GPIO::Enable(GPIOA, GPIO_PIN_9, GPIO::Mode::AlternatePP,
                         GPIO::Pull::NoPull); // TX1
        if(transmissionMode != TransmissionMode::TRANSMIT_ONLY)
            GPIO::Enable(GPIOA, GPIO_PIN_10, GPIO::Mode::AlternateInput,
                         GPIO::Pull::PullUp); // RX1
        // Enable interrupts with low priority
        if(mode == OperatingMode::INTERRUPTS || mode == OperatingMode::DMA) {
            HAL_NVIC_SetPriority(USART1_IRQn, 11, 0);
            HAL_NVIC_EnableIRQ(USART1_IRQn);
        }
        state.handle.Instance = USART1;
        break;
    case Instance::UART_2:
        // Enable clocks
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_USART2_CLK_ENABLE();
        // Set GPIO
        if(transmissionMode != TransmissionMode::RECEIVE_ONLY)
            GPIO::Enable(GPIOA, GPIO_PIN_2, GPIO::Mode::AlternatePP,
                         GPIO::Pull::NoPull); // TX2
        if(transmissionMode != TransmissionMode::TRANSMIT_ONLY)
            GPIO::Enable(GPIOA, GPIO_PIN_3, GPIO::Mode::AlternateInput,
                         GPIO::Pull::PullUp); // RX2
        // Enable interrupts with low priority
        if(mode == OperatingMode::INTERRUPTS || mode == OperatingMode::DMA) {
            HAL_NVIC_SetPriority(USART2_IRQn, 11, 0);
            HAL_NVIC_EnableIRQ(USART2_IRQn);
        }
        state.handle.Instance = USART2;
        break;
    case Instance::UART_3:
        // Enable clocks
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_USART3_CLK_ENABLE();
        // Set GPIO
        if(transmissionMode != TransmissionMode::RECEIVE_ONLY)
            GPIO::Enable(GPIOB, GPIO_PIN_10, GPIO::Mode::AlternatePP,
                         GPIO::Pull::NoPull); // TX3
        if(transmissionMode != TransmissionMode::TRANSMIT_ONLY)
            GPIO::Enable(GPIOB, GPIO_PIN_11, GPIO::Mode::AlternateInput,
                         GPIO::Pull::PullUp); // RX3
        // Enable interrupts with low priority
        if(mode == OperatingMode::INTERRUPTS || mode == OperatingMode::DMA) {
            HAL_NVIC_SetPriority(USART3_IRQn, 11, 0);
            HAL_NVIC_EnableIRQ(USART3_IRQn);
        }
        state.handle.Instance = USART3;
        break;
    case Instance::NONE:
        UART_ERROR("Somehow instance not set to any UART...");
    }

    state.handle.Init.BaudRate = baudRate;
    state.handle.Init.WordLength = static_cast<uint32_t>(wordLength);
    state.handle.Init.Parity = static_cast<uint32_t>(parity);
    state.handle.Init.StopBits = static_cast<uint32_t>(stopBits);
    state.handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    state.handle.Init.OverSampling = UART_OVERSAMPLING_16;
    state.handle.Init.Mode = static_cast<uint32_t>(transmissionMode);

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

        // Link DMA and UART
        state.handle.hdmatx = dmaController->InitChannel(dmaChannelTx);
        state.handle.hdmatx->Parent = &state.handle;
        state.handle.hdmarx = dmaController->InitChannel(dmaChannelRx);
        state.handle.hdmarx->Parent = &state.handle;
    }

    // Set up MspInit and MspDeInit callbacks
    UART_REGISTER_CALLBACK(&state.handle, MspInit)
    UART_REGISTER_CALLBACK(&state.handle, MspDeInit)

    // Set registers with prepared data
    UART_HAL_ERROR_GUARD(HAL_UART_Init(&state.handle))

    // Set up the remaining callbacks
    UART_REGISTER_CALLBACK(&state.handle, TxHalfComplete)
    UART_REGISTER_CALLBACK(&state.handle, TxComplete)
    UART_REGISTER_CALLBACK(&state.handle, RxHalfComplete)
    UART_REGISTER_CALLBACK(&state.handle, RxComplete)
    UART_REGISTER_CALLBACK(&state.handle, Error)
    UART_REGISTER_CALLBACK(&state.handle, AbortComplete)
    UART_REGISTER_CALLBACK(&state.handle, AbortTxComplete)
    UART_REGISTER_CALLBACK(&state.handle, AbortRxComplete)

    initialized = true;
}

void UART::RegisterCallback(CallbackType callbackType,
                            std::function<void()> callbackFunction)
{
    callbackFunctions[state.handle.Instance][callbackType] =
        std::move(callbackFunction);
}

void UART::Send(uint8_t* data, size_t numOfBytes)
{
    if(!initialized)
        UART_ERROR_NOT_INIT;

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
        UART_ERROR_UNKNOWN_MODE;
    }
}

void UART::SendRCC(uint8_t* data, size_t numOfBytes)
{
    UART_HAL_ERROR_GUARD(
        HAL_UART_Transmit(&state.handle, data, numOfBytes, timeout))
}

void UART::SendIT(uint8_t* data, size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.gState == HAL_UART_STATE_READY) {
        // If UART is not busy, transmit
        UART_HAL_ERROR_GUARD(
            HAL_UART_Transmit_IT(&state.handle, data, numOfBytes))
    }
}

void UART::SendDMA(uint8_t* data, size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.gState == HAL_UART_STATE_READY) {
        // If UART is not busy, transmit
        UART_HAL_ERROR_GUARD(
            HAL_UART_Transmit_DMA(&state.handle, data, numOfBytes))
    }
}

void UART::Receive(uint8_t* data, size_t numOfBytes)
{
    if(!initialized)
        UART_ERROR_NOT_INIT;

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
        UART_ERROR_UNKNOWN_MODE;
    }
}

void UART::ReceiveRCC(uint8_t* data, size_t numOfBytes)
{
    UART_HAL_ERROR_GUARD(
        HAL_UART_Receive(&state.handle, data, numOfBytes, timeout))
}

void UART::ReceiveIT(uint8_t* data, size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.RxState == HAL_UART_STATE_READY) {
        // If UART is not busy, receive
        UART_HAL_ERROR_GUARD(
            HAL_UART_Receive_IT(&state.handle, data, numOfBytes))
    }
}

void UART::ReceiveDMA(uint8_t* data, size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.RxState == HAL_UART_STATE_READY) {
        // If UART is not busy, receive
        UART_HAL_ERROR_GUARD(
            HAL_UART_Receive_DMA(&state.handle, data, numOfBytes))
    }
}

bool UART::IsTxComplete() const
{
    return state.handle.gState != HAL_UART_STATE_BUSY_TX;
}

bool UART::IsRxComplete() const
{
    return state.handle.RxState != HAL_UART_STATE_BUSY_RX;
}

void UART::AbortTx()
{
    UART_HAL_ERROR_GUARD(HAL_UART_AbortTransmit_IT(&state.handle))
}

void UART::AbortRx(){
    UART_HAL_ERROR_GUARD(HAL_UART_AbortReceive_IT(&state.handle))}

UART::UART(USART_TypeDef* usart)
{
    initialized = false;
    printfEnabled = false;
    buffer = nullptr;

    if(usart == USART1) {
        instance = Instance::UART_1;
        dmaController = &dma1;
        dmaChannelTx = DMA::Channel::Channel4;
        dmaChannelRx = DMA::Channel::Channel5;
    }
    else if(usart == USART2) {
        instance = Instance::UART_2;
        dmaController = &dma1;
        dmaChannelTx = DMA::Channel::Channel7;
        dmaChannelRx = DMA::Channel::Channel6;
    }
    else if(usart == USART3) {
        instance = Instance::UART_3;
        dmaController = &dma1;
        dmaChannelTx = DMA::Channel::Channel2;
        dmaChannelRx = DMA::Channel::Channel3;
    }
    else
        UART_ERROR("Please choose UART_1, UART_2 or UART_3");

    mode = OperatingMode::INTERRUPTS;
    wordLength = WordLength::_8BITS;
    parity = Parity::NONE;
    stopBits = StopBits::STOP_BITS_1;
    transmissionMode = TransmissionMode::FULL_DUPLEX;
    baudRate = 115200;
    timeout = 500;
}

void UART::SetWordLength(UART::WordLength _wordLength)
{
    if(initialized)
        UART_ERROR_ALREADY_INIT; // Too late

    wordLength = _wordLength;
}

void UART::SetParity(UART::Parity _parity)
{
    if(initialized)
        UART_ERROR_ALREADY_INIT; // Too late

    parity = _parity;
}

void UART::SetStopBits(UART::StopBits _stopBits)
{
    if(initialized)
        UART_ERROR_ALREADY_INIT; // Too late

    stopBits = _stopBits;
}

void UART::SetBaudRate(uint32_t _baudRate)
{
    if(initialized)
        UART_ERROR_ALREADY_INIT; // Too late

    baudRate = _baudRate;
}

void UART::ChangeModeToBlocking(uint32_t Timeout)
{
    if(initialized)
        UART_ERROR_ALREADY_INIT; // Too late

    mode = OperatingMode::BLOCKING;
    timeout = Timeout;
}

void UART::ChangeModeToInterrupts()
{
    if(initialized)
        UART_ERROR_ALREADY_INIT; // Too late

    mode = OperatingMode::INTERRUPTS;
}

void UART::ChangeModeToDMA()
{
    if(initialized)
        UART_ERROR_ALREADY_INIT; // Too late

    mode = OperatingMode::DMA;
}

void UART::printf(const char* fmt, ...)
{
    if(!printfEnabled)
        SetPrintfBufferSize(128);

    // For custom print working
    va_list vaList;
    va_start(vaList, fmt);

    if(0 < vsprintf(reinterpret_cast<char*>(UART::buffer), fmt,
                    vaList)) // build string
        Send(reinterpret_cast<uint8_t*>(UART::buffer),
             strlen(buffer)); // Real send data by UART

    va_end(vaList);
}

void UART::SetPrintfBufferSize(uint16_t bufferSize)
{
    if(printfEnabled)
        return;

    printfEnabled = true;
    buffer = new char[bufferSize];
}

void UART::DisablePrintf()
{
    if(printfEnabled)
        delete[] buffer;
}

void UART::SetTransmissionMode(UART::TransmissionMode _transmissionMode)
{
    if(initialized)
        UART_ERROR_ALREADY_INIT; // Too late

    transmissionMode = _transmissionMode;
}

UART uart1(USART1), uart2(USART2), uart3(USART3);
} // namespace SBT::Hardware

// Handlers for default HAL UART callbacks

using namespace SBT::Hardware;

void USART1_IRQHandler() { HAL_UART_IRQHandler(&uart1.GetState().handle); }

void USART2_IRQHandler() { HAL_UART_IRQHandler(&uart2.GetState().handle); }

void USART3_IRQHandler() { HAL_UART_IRQHandler(&uart3.GetState().handle); }
