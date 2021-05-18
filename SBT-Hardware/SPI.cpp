//
// Created by darkr on 18.05.2021.
//

#include <stdexcept>
#include "SPI.hpp"
#include "Hardware.hpp"

void SPI_t::Initialize()
{
    switch (instance) {
        case Instance::SPI_1:
            // Enable clocks
            __HAL_RCC_GPIOA_CLK_ENABLE();
            __HAL_RCC_SPI1_CLK_ENABLE();
            // Set GPIO
            Hardware::enableGpio(GPIOA, GPIO_PIN_5, Gpio::Mode::AlternatePP, Gpio::Pull::NoPull);  // SCK
            Hardware::enableGpio(GPIOA, GPIO_PIN_6, Gpio::Mode::AlternateInput, Gpio::Pull::NoPull);  // MISO
            Hardware::enableGpio(GPIOA, GPIO_PIN_7, Gpio::Mode::AlternatePP, Gpio::Pull::NoPull);  // MOSI
            // Enable interrupts with low priority
            HAL_NVIC_SetPriority(SPI1_IRQn, 5, 5);
            HAL_NVIC_EnableIRQ(SPI1_IRQn);
            
            break;
        case Instance::SPI_2:
            // Enable clocks
            __HAL_RCC_GPIOA_CLK_ENABLE();
            __HAL_RCC_SPI2_CLK_ENABLE();
            // Set GPIO
            Hardware::enableGpio(GPIOB, GPIO_PIN_13, Gpio::Mode::AlternatePP, Gpio::Pull::NoPull);  // SCK
            Hardware::enableGpio(GPIOB, GPIO_PIN_14, Gpio::Mode::AlternateInput, Gpio::Pull::NoPull);  // MISO
            Hardware::enableGpio(GPIOB, GPIO_PIN_15, Gpio::Mode::AlternatePP, Gpio::Pull::NoPull);  // MOSI
            // Enable interrupts with low priority
            HAL_NVIC_SetPriority(SPI2_IRQn, 5, 5);
            HAL_NVIC_EnableIRQ(SPI2_IRQn);
            break;
        case Instance::NONE:
            throw std::runtime_error("Somehow instance not set to any SPI...");
    }
    
    auto& handle = state.handle;
    handle.Instance = instance == Instance::SPI_1 ? SPI1 : SPI2;
    handle.Init.Mode = SPI_MODE_MASTER;
    handle.Init.Direction = SPI_DIRECTION_2LINES_RXONLY;
    handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    handle.Init.CLKPhase = SPI_PHASE_2EDGE;
    handle.Init.CLKPolarity = SPI_POLARITY_HIGH;
    handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    handle.Init.DataSize = SPI_DATASIZE_8BIT;
    handle.Init.FirstBit = SPI_FIRSTBIT_MSB;
    handle.Init.NSS = SPI_NSS_SOFT;
    handle.Init.TIMode = SPI_TIMODE_DISABLE;
    
    HAL_SPI_Init(&handle);
    
    //__HAL_SPI_ENABLE_IT(&handle, SPI_IT_TXE);
    __HAL_SPI_ENABLE_IT(&handle, SPI_IT_RXNE);
    __HAL_SPI_ENABLE_IT(&handle, SPI_IT_ERR);
    
    // Clear bits
    state.txRxState = xEventGroupCreate();
    xEventGroupClearBits(state.txRxState, Hardware::rxBit | Hardware::txBit);
}

void SPI_t::Send(uint8_t *data, size_t numOfBytes)
{
    // Check if event group was created
    if(state.txRxState) {
        // Check if there is no transmission
        if((xEventGroupGetBits(state.txRxState) & Hardware::txBit) == 0) {
            // If UART is not busy, transmit and set TX flag to busy
            HAL_SPI_Transmit_IT(&state.handle, data, numOfBytes);
            xEventGroupSetBits(state.txRxState, Hardware::txBit);
        }
    }
}

void SPI_t::Receive(uint8_t *data, size_t numOfBytes)
{
    // Check if event group was created
    if(state.txRxState) {
        // Check if there is no transmission
        if((xEventGroupGetBits(state.txRxState) & Hardware::rxBit) == 0) {
            // If UART is not busy, transmit and set RX flag to busy
            HAL_SPI_Receive_IT(&state.handle, data, numOfBytes);
            xEventGroupSetBits(state.txRxState, Hardware::rxBit);
        }
    }
}

bool SPI_t::IsTxComplete() const
{
    return (xEventGroupGetBits(state.txRxState) & Hardware::txBit) == 0;
}

bool SPI_t::IsRxComplete() const
{
    return (xEventGroupGetBits(state.txRxState) & Hardware::rxBit) == 0;
}

void SPI_t::configureStaticVariables(SPI_TypeDef *spii)
{
    initialized = false;
    
    if (spii == SPI1)
        instance = Instance::SPI_1;
    else if (spii == SPI2)
        instance = Instance::SPI_2;
    else
        throw std::runtime_error("Please choose SPI_1 or SPI_2");
    

    timeout = 500;
}


// Handlers for SPI transmission
void SPI1_IRQHandler(){
    HAL_SPI_IRQHandler(&Hardware::spi1.GetState().handle);
}

void SPI2_IRQHandler(){
    HAL_SPI_IRQHandler(&Hardware::spi2.GetState().handle);
}

void HAL_SPI_TxCpltCallback([[maybe_unused]] SPI_HandleTypeDef *hspi){
    if(hspi->Instance == SPI1){
        if(auto* eventGroup = Hardware::spi1.GetState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::txBit);
        }
    }
    else if(hspi->Instance == SPI2){
        if(auto* eventGroup = Hardware::spi2.GetState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::txBit);
        }
    }
    
}

void HAL_SPI_RxCpltCallback([[maybe_unused]] SPI_HandleTypeDef *hspi){
    if(hspi->Instance == SPI1){
        if(auto* eventGroup = Hardware::spi1.GetState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::rxBit);
        }
    }
    else if(hspi->Instance == SPI2){
        if(auto* eventGroup = Hardware::spi2.GetState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::rxBit);
        }
    }
}

void HAL_SPI_TxHalfCpltCallback([[maybe_unused]] SPI_HandleTypeDef *hspi){
    
    if(hspi->Instance == SPI1){
        if(auto* eventGroup = Hardware::spi1.GetState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::rxBit);
        }
    }
    else if(hspi->Instance == SPI2){
        if(auto* eventGroup = Hardware::spi2.GetState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::rxBit);
        }
    }
    
}

void HAL_SPI_RxHalfCpltCallback([[maybe_unused]] SPI_HandleTypeDef *hspi){
    if(hspi->Instance == SPI1){
        if(auto* eventGroup = Hardware::spi1.GetState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::rxBit);
        }
    }
    else if(hspi->Instance == SPI2){
        if(auto* eventGroup = Hardware::spi2.GetState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::rxBit);
        }
    }
    
    
}

void HAL_SPI_ErrorCallback([[maybe_unused]] SPI_HandleTypeDef *hspi){
    if(hspi->Instance == SPI1){
        if(auto* eventGroup = Hardware::spi1.GetState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::rxBit);
        }
    }
    else if(hspi->Instance == SPI2){
        if(auto* eventGroup = Hardware::spi2.GetState().txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::rxBit);
        }
    }
    
    
}


