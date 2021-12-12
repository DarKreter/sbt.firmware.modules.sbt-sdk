#ifndef HARDWARE_HPP
#define HARDWARE_HPP
#include "ADC.hpp"
#include "CAN.hpp"
#include "DMA.hpp"
#include "I2C.hpp"
#include "SPI.hpp"
#include "UART.hpp"
#include <string>

// Goal of this abstraction is to isolate main code from hardware-specific code.
// Ideally, this code could use a stub and be compiled for Linux without any
// problem
namespace Gpio {
enum class Mode {
    Input,
    Output,
    AlternateInput,
    AlternatePP,
    AlternateOD
};

enum class Pull {
    Pullup = 1,
    PullDown = 2,
    NoPull = 0
};
} // namespace Gpio

struct Hardware {
    static constexpr size_t txBit = 1 << 0;
    static constexpr size_t rxBit = 1 << 1;

    /**
     * @brief Configure GPIO pin
     * @param gpio GPIO Port
     * @param pin Pin number
     * @param direction Mode of operation
     * @param pull Pulling resistor configuration
     */
    static void enableGpio(GPIO_TypeDef* gpio, uint32_t pin,
                           Gpio::Mode direction,
                           Gpio::Pull pull = Gpio::Pull::NoPull);
    static void toggle(GPIO_TypeDef* gpio, uint32_t pin);

    /**
     * @brief Configure system clock
     */
    static void configureClocks(uint32_t ahbFreq = 72'000'000);

    static UART uart1, uart2, uart3;
    static I2C i2c1, i2c2;
    static SPI_t spi1, spi2;
    static CAN can;
    static DMA dma1;
    static ADC adc1;

    static uint32_t GetAHB_Freq()
    {
        return HAL_RCC_GetHCLKFreq();
    } // 100'000 - 72'000'000
    static uint32_t GetAPB1_Freq()
    {
        return HAL_RCC_GetPCLK1Freq();
    } // 100'000 - 36'000'000
    static uint32_t GetAPB2_Freq()
    {
        return HAL_RCC_GetPCLK2Freq();
    } // 100'000 - 72'000'000
};
void softfault(const std::string& fileName, const int& lineNumber,
               const std::string& comment);

#endif // HARDWARE_HPP
