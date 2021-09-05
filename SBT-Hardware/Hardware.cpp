#include <Hardware.hpp>

UART Hardware::uart1(USART1), Hardware::uart2(USART2), Hardware::uart3(USART3);
I2C Hardware::i2c1(I2C1), Hardware::i2c2(I2C2);
SPI_t Hardware::spi1(SPI1), Hardware::spi2(SPI2);
CAN Hardware::can;

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

// You did something wrong
void softfault([[maybe_unused]]const std::string& fileName, [[maybe_unused]]const int& lineNumber, [[maybe_unused]]const std::string& comment)
{
    while (true);
}