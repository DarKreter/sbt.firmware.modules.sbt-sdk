//
// Created by darkr on 15.05.2021.
//

#include <stdexcept>
#include "UART.hpp"
#include "Hardware.hpp"

void UART::Initialize() {
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
            break;
    case Instance::NONE:
        throw std::runtime_error("Somehow instance not set to UART1 or UART2...");
        
    }
    
    state.handle.Instance = instance == Instance::UART_1 ? USART1 : USART2;
    state.handle.Init.BaudRate = baudRate;
    state.handle.Init.WordLength = UART_WORDLENGTH_8B;
    state.handle.Init.Parity = UART_PARITY_NONE;
    state.handle.Init.StopBits = UART_STOPBITS_1;
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
}

void UART::Send(uint8_t *data, size_t numOfBytes) {
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

bool UART::IsTxComplete() {
    return (xEventGroupGetBits(state.txRxState) & UART::State::txBit) == 0;
}

bool UART::IsRxComplete() {
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
}


UART::UART(USART_TypeDef *usart):  mode{Mode::INTERRUPTS}, baudRate{115200}
{

    if (usart == USART1)
        instance = Instance::UART_1;
    else if (usart == USART2)
        instance = Instance::UART_2;
//    else
//        throw std::runtime_error("Please ensure you choose either UART_1 or UART_2");
}
