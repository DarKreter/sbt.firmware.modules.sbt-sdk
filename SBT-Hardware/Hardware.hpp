#ifndef HARDWARE_HPP
#define HARDWARE_HPP
#include <UART.hpp>
#include <SPI.hpp>
#include <I2C.hpp>
#include <CAN.hpp>

// Goal of this abstraction is to isolate main code from hardware-specific code. Ideally, this code could use a stub and
// be compiled for Linux without any problem
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
}

struct Hardware
{
    static constexpr size_t txBit = 1 << 0;
    static constexpr size_t rxBit = 1 << 1;
    
    /**
     * @brief Configure GPIO pin
     * @param gpio GPIO Port
     * @param pin Pin number
     * @param direction Mode of operation
     * @param pull Pulling resistor configuration
     */
    static void enableGpio(GPIO_TypeDef* gpio, uint32_t pin, Gpio::Mode direction, Gpio::Pull pull = Gpio::Pull::NoPull);
    static void toggle(GPIO_TypeDef* gpio, uint32_t pin);

    /**
     * @brief Configure system clock
     */
    static void configureClocks();

    static UART uart1, uart2, uart3;
    static I2C i2c1, i2c2;
    static SPI_t spi1, spi2;
    static CAN can;
};


#endif //HARDWARE_HPP