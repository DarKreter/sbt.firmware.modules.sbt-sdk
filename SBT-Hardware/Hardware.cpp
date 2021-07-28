#include <Hardware.hpp>
#include <event_groups.h>

UART Hardware::uart1(USART1), Hardware::uart2(USART2), Hardware::uart3(USART3);
SPI_t Hardware::spi1(SPI1), Hardware::spi2(SPI2);
CAN Hardware::can;

std::array<I2C::State, 2> Hardware::i2cStates;

void Hardware::enableGpio(GPIO_TypeDef* gpio, uint32_t pin, Gpio::Mode direction, Gpio::Pull pull) {

    GPIO_InitTypeDef initTypeDef;
    initTypeDef.Pin = pin;
    switch (direction) {
        case Gpio::Mode::Input:
            initTypeDef.Mode = GPIO_MODE_INPUT;
            break;
        case Gpio::Mode::Output:
            initTypeDef.Mode = GPIO_MODE_OUTPUT_PP;
            break;
        case Gpio::Mode::AlternateInput:
            initTypeDef.Mode = GPIO_MODE_AF_INPUT;
            break;
        case Gpio::Mode::AlternatePP:
            initTypeDef.Mode = GPIO_MODE_AF_PP;
            break;
        case Gpio::Mode::AlternateOD:
            initTypeDef.Mode = GPIO_MODE_AF_OD;
            break;
    }
    initTypeDef.Pull = static_cast<uint32_t>(pull);
    initTypeDef.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(gpio, &initTypeDef);
}

void Hardware::toggle(GPIO_TypeDef* gpio, uint32_t pin) {
    HAL_GPIO_TogglePin(gpio, pin);
}

void Hardware::configureClocks() {
    RCC_ClkInitTypeDef clkInitStruct;
    clkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
    clkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    HAL_RCC_ClockConfig(&clkInitStruct, FLASH_LATENCY_0);
    // Systick is executed once every 1ms
    SysTick_Config(SystemCoreClock / 1000);

    // Enable clocks for GPIOs
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
}

void Hardware::initializeI2C(I2C::I2C id, uint32_t address, uint32_t speed) {
    switch (id) {
        case I2C::I2C::I2C_1:
            // Enable clocks
            __HAL_RCC_GPIOB_CLK_ENABLE();
            __HAL_RCC_I2C1_CLK_ENABLE();
            // Set GPIO
            enableGpio(GPIOB, GPIO_PIN_6, Gpio::Mode::AlternateOD, Gpio::Pull::Pullup);  // SCL
            enableGpio(GPIOB, GPIO_PIN_7, Gpio::Mode::AlternateOD, Gpio::Pull::Pullup);  // SDA
            // Enable interrupts with high priority due to silicon limitation (UM1850 p. 259)
            HAL_NVIC_SetPriority(I2C1_EV_IRQn, 1, 1);
            HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
            HAL_NVIC_SetPriority(I2C1_ER_IRQn, 1, 1);
            HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
            break;
        case I2C::I2C::I2C_2:
            break;
    }

    I2C::State& state = getI2CState(id);

    state.handle.Instance = I2C1;
    state.handle.Init.ClockSpeed = speed;
    state.handle.Init.DutyCycle = I2C_DUTYCYCLE_2;
    state.handle.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    state.handle.Init.OwnAddress1 = address;
    state.handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    state.handle.Init.OwnAddress2 = 0;
    state.handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    state.handle.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    // Set registers with prepared data
    HAL_I2C_Init(&state.handle);

    // Enable interrupts
    __HAL_I2C_ENABLE_IT(&state.handle, I2C_IT_BUF);
    __HAL_I2C_ENABLE_IT(&state.handle, I2C_IT_EVT);
    __HAL_I2C_ENABLE_IT(&state.handle, I2C_IT_ERR);

    // Clear bits
    state.txRxState = xEventGroupCreate();
    xEventGroupClearBits(state.txRxState, Hardware::rxBit | Hardware::txBit);
}

void Hardware::i2cSendMaster(I2C::I2C id, uint16_t address, uint8_t *data, size_t numOfBytes) {
    I2C::State& state = getI2CState(id);
    // Check if event group was created
    if(state.txRxState) {
        // Check if there is no transmission
        if((xEventGroupGetBits(state.txRxState) & Hardware::txBit) == 0) {
            // If I2C is not busy, transmit and set TX flag to busy
            HAL_I2C_Master_Transmit_IT(&state.handle, address, data, numOfBytes);
            xEventGroupSetBits(state.txRxState, Hardware::txBit);
        }
    }
}

void Hardware::i2cReceiveMaster(I2C::I2C id, uint16_t address, uint8_t *data, size_t numOfBytes) {
    I2C::State& state = getI2CState(id);
    // Check if event group was created
    if(state.txRxState) {
        // Check if there is no transmission
        if((xEventGroupGetBits(state.txRxState) & Hardware::rxBit) == 0) {
            // If I2C is not busy, transmit and set RX flag to busy
            HAL_I2C_Master_Transmit_IT(&state.handle, address, data, numOfBytes);
            xEventGroupSetBits(state.txRxState, Hardware::rxBit);
        }
    }
}

I2C::State &Hardware::getI2CState(I2C::I2C id) {
    switch (id) {
        case I2C::I2C::I2C_1:
            return i2cStates[0];
        case I2C::I2C::I2C_2:
            return i2cStates[1];
    }
    // Give up and stay here
    return getI2CState(id);
}

// Handlers for I2C transmission

void I2C1_EV_IRQHandler(){
    HAL_I2C_EV_IRQHandler(&Hardware::getI2CState(I2C::I2C::I2C_1).handle);
}

void I2C1_ER_IRQHandler(){
    HAL_I2C_ER_IRQHandler(&Hardware::getI2CState(I2C::I2C::I2C_1).handle);
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c){
    if(hi2c->Instance == I2C1){
        if(auto* eventGroup = Hardware::getI2CState(I2C::I2C::I2C_1).txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::txBit);
        }
    }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c){
    if(hi2c->Instance == I2C1){
        if(auto* eventGroup = Hardware::getI2CState(I2C::I2C::I2C_1).txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::rxBit);
        }
    }
}

