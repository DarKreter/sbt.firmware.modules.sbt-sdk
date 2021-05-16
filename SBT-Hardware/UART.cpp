//
// Created by darkr on 15.05.2021.
//

#include <stdexcept>
#include <stdarg.h>
#include <string.h>
#include "UART.hpp"
#include "Hardware.hpp"

void UART::Initialize() {
    
    if(initialized)
        throw std::runtime_error("SPI already initialized!");
    
    switch (instance) {
        case Instance::UART_1:
            // Enable clocks
            __HAL_RCC_GPIOA_CLK_ENABLE();
            __HAL_RCC_USART1_CLK_ENABLE();
            // Set GPIO
            Hardware::enableGpio(GPIOA, GPIO_PIN_9, Gpio::Mode::AlternatePP, Gpio::Pull::NoPull);  // TX1
            Hardware::enableGpio(GPIOA, GPIO_PIN_10, Gpio::Mode::AlternateInput, Gpio::Pull::Pullup);  // RX1
            // Enable interrupts with low priority
            HAL_NVIC_SetPriority(USART1_IRQn, 10, 0);
            HAL_NVIC_EnableIRQ(USART1_IRQn);
            state.handle.Instance = USART1;
            break;
        case Instance::UART_2:
            // Enable clocks
            __HAL_RCC_GPIOA_CLK_ENABLE();
            __HAL_RCC_USART2_CLK_ENABLE();
            // Set GPIO
            Hardware::enableGpio(GPIOA, GPIO_PIN_2, Gpio::Mode::AlternatePP, Gpio::Pull::NoPull);  // TX2
            Hardware::enableGpio(GPIOA, GPIO_PIN_3, Gpio::Mode::AlternateInput, Gpio::Pull::Pullup);  // RX2
            // Enable interrupts with low priority
            HAL_NVIC_SetPriority(USART2_IRQn, 10, 0);
            HAL_NVIC_EnableIRQ(USART2_IRQn);
            state.handle.Instance = USART2;
            break;
        case Instance::UART_3:
            // Enable clocks
            __HAL_RCC_GPIOB_CLK_ENABLE();
            __HAL_RCC_USART3_CLK_ENABLE();
            // Set GPIO
            Hardware::enableGpio(GPIOB, GPIO_PIN_10, Gpio::Mode::AlternatePP, Gpio::Pull::NoPull);  // TX3
            Hardware::enableGpio(GPIOB, GPIO_PIN_11, Gpio::Mode::AlternateInput, Gpio::Pull::Pullup);  // RX3
            // Enable interrupts with low priority
            HAL_NVIC_SetPriority(USART3_IRQn, 10, 0);
            HAL_NVIC_EnableIRQ(USART3_IRQn);
            state.handle.Instance = USART3;
            break;
    case Instance::NONE:
        throw std::runtime_error("Somehow instance not set to any UART...");
        
    }
    
    state.handle.Init.BaudRate = baudRate;
    state.handle.Init.WordLength = static_cast<uint32_t>(wordLength);
    state.handle.Init.Parity = static_cast<uint32_t>(parity);
    state.handle.Init.StopBits = static_cast<uint32_t>(stopBits);
    state.handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    state.handle.Init.OverSampling = UART_OVERSAMPLING_16;
    state.handle.Init.Mode = UART_MODE_TX_RX;
    
    // Set registers with prepared data
    HAL_UART_Init(&state.handle);
    
    // Enable interrupts
    __HAL_UART_ENABLE_IT(&state.handle, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&state.handle, UART_IT_TC);
    
    // Clear bits
    state.txRxState = xEventGroupCreate();
    xEventGroupClearBits(state.txRxState, UART::State::rxBit | UART::State::txBit);
    
    initialized = true;
}

void UART::Send(uint8_t *data, size_t numOfBytes)
{
    if(!initialized)
        throw std::runtime_error("SPI not initialized!");
    
    switch(mode)
    {
        case Mode::INTERRUPTS:
            SendIT(data,numOfBytes);
            break;
        case Mode::BLOCKING:
            SendRCC(data,numOfBytes);
            break;
        case Mode::DMA:
            //SendDMA(data,numOfBytes);
            //break;
        default:
            throw std::runtime_error("How that even happen");
    }
}

void UART::SendRCC(uint8_t *data, size_t numOfBytes)
{
    HAL_UART_Transmit(&state.handle, data, numOfBytes, timeout);
}

void UART::SendIT(uint8_t *data, size_t numOfBytes)
{
    // Check if event group was created
    if(state.txRxState) {
        // Check if there is no transmission
        if((xEventGroupGetBits(state.txRxState) & UART::State::txBit) == 0) {
            // If UART is not busy, transmit and set TX flag to busy
            HAL_UART_Transmit_IT(&state.handle, data, numOfBytes);
            xEventGroupSetBits(state.txRxState, UART::State::txBit);
        }
    }
}

void UART::Receive(uint8_t *data, size_t numOfBytes)
{
    if(!initialized)
        throw std::runtime_error("SPI not initialized!");
    
    switch(mode)
    {
        case Mode::INTERRUPTS:
            ReceiveIT(data,numOfBytes);
            break;
        case Mode::BLOCKING:
            ReceiveRCC(data,numOfBytes);
            break;
        case Mode::DMA:
            //ReceiveDMA(data,numOfBytes);
            //break;
        default:
            throw std::runtime_error("How that even happen");
    }
}

void UART::ReceiveRCC(uint8_t *data, size_t numOfBytes)
{
    HAL_UART_Receive(&state.handle, data, numOfBytes, timeout);
}

void UART::ReceiveIT(uint8_t *data, size_t numOfBytes)
{
    // Check if event group was created
    if(state.txRxState) {
        // Check if there is no transmission
        if((xEventGroupGetBits(state.txRxState) & UART::State::rxBit) == 0) {
            // If UART is not busy, transmit and set RX flag to busy
            HAL_UART_Receive_IT(&state.handle, data, numOfBytes);
            xEventGroupSetBits(state.txRxState, UART::State::rxBit);
        }
    }
}

bool UART::IsTxComplete() const {
    return (xEventGroupGetBits(state.txRxState) & UART::State::txBit) == 0;
}

bool UART::IsRxComplete() const {
    return (xEventGroupGetBits(state.txRxState) & UART::State::rxBit) == 0;
}

void UART::AbortTx() {
    HAL_UART_AbortTransmit_IT(&state.handle);
    xEventGroupClearBits(state.txRxState, UART::State::txBit);
}

void UART::AbortRx() {
    HAL_UART_AbortReceive_IT(&state.handle);
    xEventGroupClearBits(state.txRxState, UART::State::rxBit);
}

// Handlers for default HAL UART callbacks
void USART1_IRQHandler() {
    HAL_UART_IRQHandler(&Hardware::uart1.getState().handle);
}

void USART2_IRQHandler() {
    HAL_UART_IRQHandler(&Hardware::uart2.getState().handle);
}

void USART3_IRQHandler() {
    HAL_UART_IRQHandler(&Hardware::uart3.getState().handle);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if(huart->Instance == USART1){
        if(auto* eventGroup = Hardware::uart1.getState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::uart1.getState().txBit);
        }
    }
    else if(huart->Instance == USART2){
        if(auto* eventGroup = Hardware::uart2.getState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::uart2.getState().txBit);
        }
    }
    else if(huart->Instance == USART3){
        if(auto* eventGroup = Hardware::uart3.getState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::uart3.getState().txBit);
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if(huart->Instance == USART1){
        if(auto* eventGroup = Hardware::uart1.getState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::uart1.getState().rxBit);
        }
    }
    if(huart->Instance == USART2){
        if(auto* eventGroup = Hardware::uart2.getState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::uart2.getState().rxBit);
        }
    }
    if(huart->Instance == USART3){
        if(auto* eventGroup = Hardware::uart3.getState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::uart3.getState().rxBit);
        }
    }
}

void UART::configureStaticVariables(USART_TypeDef *usart)
{
    initialized = false;
    printfEnabled = false;
    buffer = nullptr;
    
    if (usart == USART1)
        instance = Instance::UART_1;
    else if (usart == USART2)
        instance = Instance::UART_2;
    else if (usart == USART3)
        instance = Instance::UART_3;
    else
        throw std::runtime_error("Please choose UART_1, UART_2 or UART_3");
    
    mode = Mode::INTERRUPTS;
    wordLength = WordLength::_8BITS;
    parity = Parity::NONE;
    stopBits = StopBits::STOP_BITS_1;
    baudRate = 115200;
    timeout = 500;
}

void UART::SetWordLength(UART::WordLength wl)
{
    if(initialized)
        throw std::runtime_error("SPI already initialized!"); // Too late
    
    wordLength = wl;
}

void UART::SetParity(UART::Parity p)
{
    if(initialized)
        throw std::runtime_error("SPI already initialized!"); // Too late
    
    parity = p;
}

void UART::SetStopBits(UART::StopBits sb)
{
    if(initialized)
        throw std::runtime_error("SPI already initialized!"); // Too late
    
    stopBits = sb;
}

void UART::SetBaudRate(uint32_t br)
{
    if(initialized)
        throw std::runtime_error("SPI already initialized!"); // Too late
    
    baudRate = br;
}

void UART::ChangeModeToBlocking(uint32_t tmt)
{
    if(initialized)
        throw std::runtime_error("SPI already initialized!"); // Too late
    
    mode = Mode::BLOCKING;
    timeout = tmt;
}

void UART::ChangeModeToInterrupts()
{
    if(initialized)
        throw std::runtime_error("SPI already initialized!"); // Too late
    
    mode = Mode::INTERRUPTS;
}

void UART::printf(const char *fmt, ...)
{
    if(!printfEnabled)
        throw std::runtime_error("Enable printf feature before using it! [ .EnablePrintf(uint32_t) ]");
    
    //For custom print working
    va_list vaList;
    va_start(vaList, fmt);
    
    if (0 < vsprintf(reinterpret_cast<char *>(UART::buffer), fmt, vaList)) // build string
        Send(reinterpret_cast<uint8_t *>(UART::buffer), strlen(buffer)); //Real send data by UART
    
    va_end(vaList);
}

void UART::EnablePrintf(uint8_t bf)
{
    printfEnabled = true;
    buffer = new char[bf];
}

void UART::DisablePrintf()
{
    delete[] buffer;
}
