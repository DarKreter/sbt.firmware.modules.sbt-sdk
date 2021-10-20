//
// Created by darkr on 18.05.2021.
//

#include <stdexcept>
#include "SPI.hpp"
#include "Hardware.hpp"

void SPI_t::Initialize()
{
    if(initialized)
        softfault(__FILE__, __LINE__, "SPI already initialized!");
    
    CalculateMisoMosi();
    CalculateSpeed();
    
    switch (instance) {
        case Instance::SPI_1:
            // Enable clocks
            __HAL_RCC_GPIOA_CLK_ENABLE();
            __HAL_RCC_SPI1_CLK_ENABLE();
            // Set GPIO
            Hardware::enableGpio(GPIOA, GPIO_PIN_5, Gpio::Mode::AlternatePP, Gpio::Pull::NoPull);  // SCK
            if(misoEnabled)
                Hardware::enableGpio(GPIOA, GPIO_PIN_6, Gpio::Mode::AlternateInput, Gpio::Pull::NoPull);  // MISO
            if(mosiEnabled)
                Hardware::enableGpio(GPIOA, GPIO_PIN_7, Gpio::Mode::AlternatePP, Gpio::Pull::NoPull);  // MOSI
            // Enable interrupts with low priority
            if(mode == OperatingMode::INTERRUPTS)
            {
                HAL_NVIC_SetPriority(SPI1_IRQn, 5, 5);
                HAL_NVIC_EnableIRQ(SPI1_IRQn);
            }
            break;
        case Instance::SPI_2:
            // Enable clocks
            __HAL_RCC_GPIOA_CLK_ENABLE();
            __HAL_RCC_SPI2_CLK_ENABLE();
            // Set GPIO
            Hardware::enableGpio(GPIOB, GPIO_PIN_13, Gpio::Mode::AlternatePP, Gpio::Pull::NoPull);  // SCK
            if(misoEnabled)
                Hardware::enableGpio(GPIOB, GPIO_PIN_14, Gpio::Mode::AlternateInput, Gpio::Pull::NoPull);  // MISO
            if(mosiEnabled)
                Hardware::enableGpio(GPIOB, GPIO_PIN_15, Gpio::Mode::AlternatePP, Gpio::Pull::NoPull);  // MOSI
            // Enable interrupts with low priority
            if(mode == OperatingMode::INTERRUPTS)
            {
                HAL_NVIC_SetPriority(SPI2_IRQn, 5, 5);
                HAL_NVIC_EnableIRQ(SPI2_IRQn);
            }
            break;
        case Instance::NONE:
            softfault(__FILE__, __LINE__, "Somehow instance not set to any SPI...");
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
    
    HAL_SPI_Init(&handle);
    
    if(mode == OperatingMode::INTERRUPTS)
    {
    //TODO: Why this must be commented even on INTERRUPT mode??
//        if((deviceType == DeviceType::MASTER && mosiEnabled) || (deviceType == DeviceType::SLAVE && misoEnabled))
//            __HAL_SPI_ENABLE_IT(&handle, SPI_IT_TXE);
//        if((deviceType == DeviceType::MASTER && misoEnabled) || (deviceType == DeviceType::SLAVE && mosiEnabled))
//            __HAL_SPI_ENABLE_IT(&handle, SPI_IT_RXNE);
        __HAL_SPI_ENABLE_IT(&handle, SPI_IT_ERR);
    }
    // Clear bits
    state.txRxState = xEventGroupCreate();
    xEventGroupClearBits(state.txRxState, Hardware::rxBit | Hardware::txBit);
    
    initialized = true;
}

void SPI_t::Send(uint8_t *data, size_t numOfBytes)
{
    if(!initialized)
        softfault(__FILE__, __LINE__, "SPI not initialized!");
    
    switch(mode)
    {
        case OperatingMode::INTERRUPTS:
            SendIT(data,numOfBytes);
            break;
        case OperatingMode::BLOCKING:
            SendRCC(data,numOfBytes);
            break;
        case OperatingMode::DMA:
            //SendDMA(data,numOfBytes);
            //break;
        default:
            softfault(__FILE__, __LINE__, "How that even happen");
    }
    
}

void SPI_t::SendIT(uint8_t *data, size_t numOfBytes)
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

void SPI_t::SendRCC(uint8_t *data, size_t numOfBytes)
{
    HAL_SPI_Transmit(&state.handle, data, numOfBytes, timeout);
}

void SPI_t::Receive(uint8_t *data, size_t numOfBytes)
{
    if(!initialized)
        softfault(__FILE__, __LINE__, "SPI not initialized!");
    
    switch(mode)
    {
        case OperatingMode::INTERRUPTS:
            ReceiveIT(data,numOfBytes);
            break;
        case OperatingMode::BLOCKING:
            ReceiveRCC(data,numOfBytes);
            break;
        case OperatingMode::DMA:
            //ReceiveDMA(data,numOfBytes);
            //break;
        default:
            softfault(__FILE__, __LINE__, "How that even happen");
    }
    
}

void SPI_t::ReceiveIT(uint8_t *data, size_t numOfBytes)
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

void SPI_t::ReceiveRCC(uint8_t *data, size_t numOfBytes)
{
    HAL_SPI_Receive(&state.handle, data, numOfBytes, timeout);
}

bool SPI_t::IsTxComplete() const
{
    return (xEventGroupGetBits(state.txRxState) & Hardware::txBit) == 0;
}

bool SPI_t::IsRxComplete() const
{
    return (xEventGroupGetBits(state.txRxState) & Hardware::rxBit) == 0;
}

void SPI_t::Abort() {
    HAL_SPI_Abort(&state.handle);
    xEventGroupClearBits(state.txRxState, Hardware::txBit | Hardware::rxBit);
}

SPI_t::SPI_t(SPI_TypeDef *spii)
{
    initialized = false;

    if (spii == SPI1)
        instance = Instance::SPI_1;
    else if (spii == SPI2)
        instance = Instance::SPI_2;
    else
        softfault(__FILE__, __LINE__, "Please choose SPI_1 or SPI_2");
    
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
        softfault(__FILE__, __LINE__, "SPI already initialized!");
        
    prescaler = _prescaler;
}

void SPI_t::SetBaudRate(int32_t _baudRate)
{
    if(initialized)
        softfault(__FILE__, __LINE__, "SPI already initialized!");
    
    if(_baudRate < 1 || 18'000'000 < _baudRate)
        softfault(__FILE__, __LINE__, "Baud rate for SPI must be between 1 and 18'000'000");
    
    baudRate = _baudRate;
}

void SPI_t::SetDataSize(DataSize _dataSize)
{
    if(initialized)
        softfault(__FILE__, __LINE__, "SPI already initialized!"); // Too late
        
    dataSize = _dataSize;
}

void SPI_t::SetFirstBit(FirstBit _firstBit)
{
    if(initialized)
        softfault(__FILE__, __LINE__, "SPI already initialized!");// Too late
        
    firstBit = _firstBit;
}

void SPI_t::SetClockPolarity(ClockPolarity _clockPolarity)
{
    if(initialized)
        softfault(__FILE__, __LINE__, "SPI already initialized!");// Too late
        
    clockPolarity = _clockPolarity;
}

void SPI_t::SetClockPhase(ClockPhase _clockPhase)
{
    if(initialized)
        softfault(__FILE__, __LINE__, "SPI already initialized!");// Too late
    
    clockPhase = _clockPhase;
}

void SPI_t::ChangeModeToBlocking(uint32_t _timeout)
{
    if(initialized)
        softfault(__FILE__, __LINE__, "SPI already initialized!");// Too late
    
    mode = OperatingMode::BLOCKING;
    timeout = _timeout;
}

void SPI_t::ChangeModeToInterrupts()
{
    if(initialized)
        softfault(__FILE__, __LINE__, "SPI already initialized!");// Too late
    
    mode = OperatingMode::INTERRUPTS;
}

void SPI_t::SetTransmissionMode(SPI_t::TransmissionMode _transmissionMode)
{
    if(initialized)
        softfault(__FILE__, __LINE__, "SPI already initialized!");// Too late
    
    transmissionMode = _transmissionMode;
}

void SPI_t::SetDeviceType(SPI_t::DeviceType _deviceType)
{
    if(initialized)
        softfault(__FILE__, __LINE__, "SPI already initialized!");// Too late
    
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
    const int32_t clkSpeed = ( instance == Instance::SPI_1 ?  Hardware::GetAPB2_Freq() : Hardware::GetAPB1_Freq());
    
    for(auto [prescalerValue, prescalerSet] : possiblePrescalers )
    {
        if( std::abs( static_cast<int32_t>(baudRate - (clkSpeed / prescalerValue)) ) < lowestAbsoluteError)
        {
            bestPrescalerSet = prescalerSet;
            bestPrescalerValue = prescalerValue;
            lowestAbsoluteError = std::abs( static_cast<int32_t>(baudRate - (clkSpeed / prescalerValue)) );
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
    
    switch(transmissionMode)
    {
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


