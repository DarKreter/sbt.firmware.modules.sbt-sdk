//
// Created by darkr on 15.05.2021.
//

#include "UART.hpp"
#include "GPIO.hpp"
#include "Hardware.hpp"
#include <stdarg.h>
#include <stdexcept>
#include <string.h>

void UART::Initialize()
{

    if(initialized)
        softfault(__FILE__, __LINE__, "UART already initialized!");

    if(instance == Instance::UART_3 && Hardware::i2c2.IsInitialized())
        softfault(__FILE__, __LINE__,
                  "Cannot initialize UART3 along with I2C2!");

    {
        using namespace SBT::Hardware;

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
            if(mode == OperatingMode::INTERRUPTS) {
                HAL_NVIC_SetPriority(USART1_IRQn, 10, 0);
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
            if(mode == OperatingMode::INTERRUPTS) {
                HAL_NVIC_SetPriority(USART2_IRQn, 10, 0);
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
            if(mode == OperatingMode::INTERRUPTS) {
                HAL_NVIC_SetPriority(USART3_IRQn, 10, 0);
                HAL_NVIC_EnableIRQ(USART3_IRQn);
            }
            state.handle.Instance = USART3;
            break;
        case Instance::NONE:
            softfault(__FILE__, __LINE__,
                      "Somehow instance not set to any UART...");
        }
    }

    state.handle.Init.BaudRate = baudRate;
    state.handle.Init.WordLength = static_cast<uint32_t>(wordLength);
    state.handle.Init.Parity = static_cast<uint32_t>(parity);
    state.handle.Init.StopBits = static_cast<uint32_t>(stopBits);
    state.handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    state.handle.Init.OverSampling = UART_OVERSAMPLING_16;
    state.handle.Init.Mode = static_cast<uint32_t>(transmissionMode);

    // Set registers with prepared data
    HAL_UART_Init(&state.handle);

    // Enable interrupts
    if(mode == OperatingMode::INTERRUPTS) {
        __HAL_UART_ENABLE_IT(&state.handle, UART_IT_RXNE);
        __HAL_UART_ENABLE_IT(&state.handle, UART_IT_TC);
        // Clear bits
        state.txRxState = xEventGroupCreate();
        xEventGroupClearBits(state.txRxState,
                             Hardware::rxBit | Hardware::txBit);
    }
    initialized = true;
}

void UART::Send(uint8_t* data, size_t numOfBytes)
{
    if(!initialized)
        softfault(__FILE__, __LINE__, "UART not initialized!");

    switch(mode) {
    case OperatingMode::INTERRUPTS:
        SendIT(data, numOfBytes);
        break;
    case OperatingMode::BLOCKING:
        SendRCC(data, numOfBytes);
        break;
    case OperatingMode::DMA:
        // SendDMA(data,numOfBytes);
        // break;
    default:
        softfault(__FILE__, __LINE__, "How that even happen");
    }
}

void UART::SendRCC(uint8_t* data, size_t numOfBytes)
{
    HAL_UART_Transmit(&state.handle, data, numOfBytes, timeout);
}

void UART::SendIT(uint8_t* data, size_t numOfBytes)
{
    // Check if event group was created
    if(state.txRxState) {
        // Check if there is no transmission
        if((xEventGroupGetBits(state.txRxState) & Hardware::txBit) == 0) {
            // If UART is not busy, transmit and set TX flag to busy
            HAL_UART_Transmit_IT(&state.handle, data, numOfBytes);
            xEventGroupSetBits(state.txRxState, Hardware::txBit);
        }
    }
}

void UART::Receive(uint8_t* data, size_t numOfBytes)
{
    if(!initialized)
        softfault(__FILE__, __LINE__, "SPI not initialized!");

    switch(mode) {
    case OperatingMode::INTERRUPTS:
        ReceiveIT(data, numOfBytes);
        break;
    case OperatingMode::BLOCKING:
        ReceiveRCC(data, numOfBytes);
        break;
    case OperatingMode::DMA:
        // ReceiveDMA(data,numOfBytes);
        // break;
    default:
        softfault(__FILE__, __LINE__, "How that even happen");
    }
}

void UART::ReceiveRCC(uint8_t* data, size_t numOfBytes)
{
    HAL_UART_Receive(&state.handle, data, numOfBytes, timeout);
}

void UART::ReceiveIT(uint8_t* data, size_t numOfBytes)
{
    // Check if event group was created
    if(state.txRxState) {
        // Check if there is no transmission
        if((xEventGroupGetBits(state.txRxState) & Hardware::rxBit) == 0) {
            // If UART is not busy, transmit and set RX flag to busy
            HAL_UART_Receive_IT(&state.handle, data, numOfBytes);
            xEventGroupSetBits(state.txRxState, Hardware::rxBit);
        }
    }
}

bool UART::IsTxComplete() const
{
    return (xEventGroupGetBits(state.txRxState) & Hardware::txBit) == 0;
}

bool UART::IsRxComplete() const
{
    return (xEventGroupGetBits(state.txRxState) & Hardware::rxBit) == 0;
}

void UART::AbortTx()
{
    HAL_UART_AbortTransmit_IT(&state.handle);
    xEventGroupClearBits(state.txRxState, Hardware::txBit);
}

void UART::AbortRx()
{
    HAL_UART_AbortReceive_IT(&state.handle);
    xEventGroupClearBits(state.txRxState, Hardware::rxBit);
}

// Handlers for default HAL UART callbacks
void USART1_IRQHandler()
{
    HAL_UART_IRQHandler(&Hardware::uart1.GetState().handle);
}

void USART2_IRQHandler()
{
    HAL_UART_IRQHandler(&Hardware::uart2.GetState().handle);
}

void USART3_IRQHandler()
{
    HAL_UART_IRQHandler(&Hardware::uart3.GetState().handle);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
    if(huart->Instance == USART1) {
        if(auto* eventGroup = Hardware::uart1.GetState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::txBit);
        }
    }
    else if(huart->Instance == USART2) {
        if(auto* eventGroup = Hardware::uart2.GetState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::txBit);
        }
    }
    else if(huart->Instance == USART3) {
        if(auto* eventGroup = Hardware::uart3.GetState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::txBit);
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
    if(huart->Instance == USART1) {
        if(auto* eventGroup = Hardware::uart1.GetState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::rxBit);
        }
    }
    if(huart->Instance == USART2) {
        if(auto* eventGroup = Hardware::uart2.GetState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::rxBit);
        }
    }
    if(huart->Instance == USART3) {
        if(auto* eventGroup = Hardware::uart3.GetState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::rxBit);
        }
    }
}

UART::UART(USART_TypeDef* usart)
{
    initialized = false;
    printfEnabled = false;
    buffer = nullptr;

    if(usart == USART1)
        instance = Instance::UART_1;
    else if(usart == USART2)
        instance = Instance::UART_2;
    else if(usart == USART3)
        instance = Instance::UART_3;
    else
        softfault(__FILE__, __LINE__, "Please choose UART_1, UART_2 or UART_3");

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
        softfault(__FILE__, __LINE__, "UART already initialized!"); // Too late

    wordLength = _wordLength;
}

void UART::SetParity(UART::Parity _parity)
{
    if(initialized)
        softfault(__FILE__, __LINE__, "UART already initialized!"); // Too late

    parity = _parity;
}

void UART::SetStopBits(UART::StopBits _stopBits)
{
    if(initialized)
        softfault(__FILE__, __LINE__, "UART already initialized!"); // Too late

    stopBits = _stopBits;
}

void UART::SetBaudRate(uint32_t _baudRate)
{
    if(initialized)
        softfault(__FILE__, __LINE__, "UART already initialized!"); // Too late

    baudRate = _baudRate;
}

void UART::ChangeModeToBlocking(uint32_t Timeout)
{
    if(initialized)
        softfault(__FILE__, __LINE__, "UART already initialized!"); // Too late

    mode = OperatingMode::BLOCKING;
    timeout = Timeout;
}

void UART::ChangeModeToInterrupts()
{
    if(initialized)
        softfault(__FILE__, __LINE__, "UART already initialized!"); // Too late

    mode = OperatingMode::INTERRUPTS;
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
        softfault(__FILE__, __LINE__, "UART already initialized!"); // Too late

    transmissionMode = _transmissionMode;
}
