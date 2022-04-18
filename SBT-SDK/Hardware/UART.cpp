//
// Created by darkr on 15.05.2021.
//

#include "UART.hpp"
#include "Error.hpp"
#include "GPIO.hpp"
#include <cstdarg>
#include <cstring>

static void uartError(const std::string& comment)
{
    softfault("UART: " + comment);
}

static void uartErrorNotInit() { uartError("Not initialized"); }

static void uartErrorAlreadyInit() { uartError("Already initialized"); }

static void uartErrorUnknownMode() { uartError("How that even happen"); }

static void uartErrorUnknownInstance()
{
    uartError("Somehow instance not set to any UART...");
}

static void uartHALErrorGuard(HAL_StatusTypeDef halStatus)
{
    if(halStatus != HAL_OK)
        uartError("HAL function failed with code " + std::to_string(halStatus));
}

// Register a function created from the template as a callback. callbackType
// must be a constant (literal) expression and not a variable as it is passed as
// the template parameter and must be known at compile time.
#define UART_REGISTER_CALLBACK(huart, callbackType)                            \
    uartHALErrorGuard(HAL_UART_RegisterCallback(                               \
        huart,                                                                 \
        static_cast<HAL_UART_CallbackIDTypeDef>(CallbackType::callbackType),   \
        UARTUniversalCallback<CallbackType::callbackType>));

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
        uartErrorAlreadyInit();

    switch(instance) {
    case Instance::UART_1:
        // Enable the clock
        __HAL_RCC_USART1_CLK_ENABLE();
        // Set GPIO
        if(transmissionMode != TransmissionMode::RECEIVE_ONLY)
            GPIO::Enable(BSP::Pinouts::UART_1.tx); // TX1
        if(transmissionMode != TransmissionMode::TRANSMIT_ONLY)
            GPIO::Enable(BSP::Pinouts::UART_1.rx); // RX1
#ifdef SBT_BSP_REMAP_UART1
        __HAL_AFIO_REMAP_USART1_ENABLE();
#endif
        // Enable interrupts with low priority
        if(mode == OperatingMode::INTERRUPTS || mode == OperatingMode::DMA) {
            HAL_NVIC_SetPriority(USART1_IRQn, 11, 0);
            HAL_NVIC_EnableIRQ(USART1_IRQn);
        }
        state.handle.Instance = USART1;
        break;
    case Instance::UART_2:
        // Enable the clock
        __HAL_RCC_USART2_CLK_ENABLE();
        // Set GPIO
        if(transmissionMode != TransmissionMode::RECEIVE_ONLY)
            GPIO::Enable(BSP::Pinouts::UART_2.tx); // TX2
        if(transmissionMode != TransmissionMode::TRANSMIT_ONLY)
            GPIO::Enable(BSP::Pinouts::UART_2.rx); // RX2
        // Enable interrupts with low priority
        if(mode == OperatingMode::INTERRUPTS || mode == OperatingMode::DMA) {
            HAL_NVIC_SetPriority(USART2_IRQn, 11, 0);
            HAL_NVIC_EnableIRQ(USART2_IRQn);
        }
        state.handle.Instance = USART2;
        break;
    case Instance::UART_3:
        // Enable the clock
        __HAL_RCC_USART3_CLK_ENABLE();
        // Set GPIO
        if(transmissionMode != TransmissionMode::RECEIVE_ONLY)
            GPIO::Enable(BSP::Pinouts::UART_3.tx); // TX3
        if(transmissionMode != TransmissionMode::TRANSMIT_ONLY)
            GPIO::Enable(BSP::Pinouts::UART_3.rx); // RX3
        // Enable interrupts with low priority
        if(mode == OperatingMode::INTERRUPTS || mode == OperatingMode::DMA) {
            HAL_NVIC_SetPriority(USART3_IRQn, 11, 0);
            HAL_NVIC_EnableIRQ(USART3_IRQn);
        }
        state.handle.Instance = USART3;
        break;
    case Instance::NONE:
        uartErrorUnknownInstance();
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
    uartHALErrorGuard(HAL_UART_Init(&state.handle));

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

void UART::DeInitialize()
{
    if(!initialized)
        uartErrorNotInit();

    initialized = false;

    // Deinitialize UART using HAL
    uartHALErrorGuard(HAL_UART_DeInit(&state.handle));

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
    case Instance::UART_1:
        // Disable the clock
        __HAL_RCC_USART1_CLK_DISABLE();
        // Disable interrupts
        if(mode == OperatingMode::INTERRUPTS || mode == OperatingMode::DMA)
            HAL_NVIC_DisableIRQ(USART1_IRQn);
        break;
    case Instance::UART_2:
        // Disable the clock
        __HAL_RCC_USART2_CLK_DISABLE();
        // Disable interrupts
        if(mode == OperatingMode::INTERRUPTS || mode == OperatingMode::DMA)
            HAL_NVIC_DisableIRQ(USART2_IRQn);
        break;
    case Instance::UART_3:
        // Disable the clock
        __HAL_RCC_USART3_CLK_DISABLE();
        // Disable interrupts
        if(mode == OperatingMode::INTERRUPTS || mode == OperatingMode::DMA)
            HAL_NVIC_DisableIRQ(USART3_IRQn);
        break;
    case Instance::NONE:
        uartErrorUnknownInstance();
    }
}

void UART::RegisterCallback(CallbackType callbackType,
                            std::function<void()> callbackFunction)
{
    callbackFunctions[state.handle.Instance][callbackType] =
        std::move(callbackFunction);
}

HAL_StatusTypeDef UART::Send(uint8_t* data, size_t numOfBytes)
{
    if(!initialized)
        uartErrorNotInit();

    switch(mode) {
    case OperatingMode::INTERRUPTS:
        return SendIT(data, numOfBytes);
        break;
    case OperatingMode::BLOCKING:
        return SendRCC(data, numOfBytes);
        break;
    case OperatingMode::DMA:
        return SendDMA(data, numOfBytes);
        break;
    default:
        uartErrorUnknownMode();
        return HAL_ERROR;
    }
}

HAL_StatusTypeDef UART::SendRCC(uint8_t* data, size_t numOfBytes)
{
    return HAL_UART_Transmit(&state.handle, data, numOfBytes, timeout);
}

HAL_StatusTypeDef UART::SendIT(uint8_t* data, size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.gState == HAL_UART_STATE_READY) {
        // If UART is not busy, transmit
        return HAL_UART_Transmit_IT(&state.handle, data, numOfBytes);
    }

    return HAL_BUSY;
}

HAL_StatusTypeDef UART::SendDMA(uint8_t* data, size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.gState == HAL_UART_STATE_READY) {
        // If UART is not busy, transmit
        return HAL_UART_Transmit_DMA(&state.handle, data, numOfBytes);
    }
    return HAL_BUSY;
}

HAL_StatusTypeDef UART::Receive(uint8_t* data, size_t numOfBytes)
{
    if(!initialized)
        uartErrorNotInit();

    switch(mode) {
    case OperatingMode::INTERRUPTS:
        return ReceiveIT(data, numOfBytes);
        break;
    case OperatingMode::BLOCKING:
        return ReceiveRCC(data, numOfBytes);
        break;
    case OperatingMode::DMA:
        return ReceiveDMA(data, numOfBytes);
        break;
    default:
        uartErrorUnknownMode();
        return HAL_ERROR;
    }
}

HAL_StatusTypeDef UART::ReceiveRCC(uint8_t* data, size_t numOfBytes)
{
    return HAL_UART_Receive(&state.handle, data, numOfBytes, timeout);
}

HAL_StatusTypeDef UART::ReceiveIT(uint8_t* data, size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.RxState == HAL_UART_STATE_READY) {
        // If UART is not busy, receive
        return HAL_UART_Receive_IT(&state.handle, data, numOfBytes);
    }
    return HAL_BUSY;
}

HAL_StatusTypeDef UART::ReceiveDMA(uint8_t* data, size_t numOfBytes)
{
    // Check if there is no transmission
    if(state.handle.RxState == HAL_UART_STATE_READY) {
        // If UART is not busy, receive
        return HAL_UART_Receive_DMA(&state.handle, data, numOfBytes);
    }
    return HAL_BUSY;
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
    uartHALErrorGuard(HAL_UART_AbortTransmit_IT(&state.handle));
}

void UART::AbortRx()
{
    uartHALErrorGuard(HAL_UART_AbortReceive_IT(&state.handle));
}

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
        uartError("Please choose UART_1, UART_2 or UART_3");

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
        uartErrorAlreadyInit(); // Too late

    wordLength = _wordLength;
}

void UART::SetParity(UART::Parity _parity)
{
    if(initialized)
        uartErrorAlreadyInit(); // Too late

    parity = _parity;
}

void UART::SetStopBits(UART::StopBits _stopBits)
{
    if(initialized)
        uartErrorAlreadyInit(); // Too late

    stopBits = _stopBits;
}

void UART::SetBaudRate(uint32_t _baudRate)
{
    if(initialized)
        uartErrorAlreadyInit(); // Too late

    baudRate = _baudRate;
}

void UART::ChangeModeToBlocking(uint32_t Timeout)
{
    if(initialized)
        uartErrorAlreadyInit(); // Too late

    mode = OperatingMode::BLOCKING;
    timeout = Timeout;
}

void UART::ChangeModeToInterrupts()
{
    if(initialized)
        uartErrorAlreadyInit(); // Too late

    mode = OperatingMode::INTERRUPTS;
}

void UART::ChangeModeToDMA()
{
    if(initialized)
        uartErrorAlreadyInit(); // Too late

    mode = OperatingMode::DMA;
}

HAL_StatusTypeDef UART::printf(const char* fmt, ...)
{
    if(!printfEnabled)
        SetPrintfBufferSize(128);

    // For custom print working
    va_list vaList;
    va_start(vaList, fmt);
    HAL_StatusTypeDef status = HAL_ERROR;
    if(0 < vsprintf(reinterpret_cast<char*>(UART::buffer), fmt,
                    vaList)) // build string
        status = Send(reinterpret_cast<uint8_t*>(UART::buffer),
                      strlen(buffer)); // Real send data by UART

    va_end(vaList);

    return status;
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
        uartErrorAlreadyInit(); // Too late

    transmissionMode = _transmissionMode;
}

UART uart1(USART1), uart2(USART2), uart3(USART3);
} // namespace SBT::Hardware

// Handlers for default HAL UART callbacks

using namespace SBT::Hardware;

void USART1_IRQHandler() { HAL_UART_IRQHandler(&uart1.GetState().handle); }

void USART2_IRQHandler() { HAL_UART_IRQHandler(&uart2.GetState().handle); }

void USART3_IRQHandler() { HAL_UART_IRQHandler(&uart3.GetState().handle); }
