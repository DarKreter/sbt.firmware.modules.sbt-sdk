//
// Created by jazon on 11/13/21.
//

#ifndef F1XX_PROJECT_TEMPLATE_LIB_SBT_SDK_F1XX_SBT_SDK_HARDWARE_GPIO_HPP
#define F1XX_PROJECT_TEMPLATE_LIB_SBT_SDK_F1XX_SBT_SDK_HARDWARE_GPIO_HPP

#include "ADC.hpp"
#include <stm32f1xx_hal_gpio.h>

namespace SBT::Hardware {
class GPIO {
public:
    enum class Mode {
        Input = GPIO_MODE_INPUT,
        Output = GPIO_MODE_OUTPUT_PP,
        OutputOD = GPIO_MODE_OUTPUT_OD,
        AlternateInput = GPIO_MODE_AF_INPUT,
        AlternatePP = GPIO_MODE_AF_PP,
        AlternateOD = GPIO_MODE_AF_OD,
        AnalogInput = GPIO_MODE_ANALOG
    };

    enum class Pull {
        PullUp = GPIO_PULLUP,
        PullDown = GPIO_PULLDOWN,
        NoPull = GPIO_NOPULL
    };

    enum class State {
        LOW = GPIO_PIN_RESET,
        HIGH = GPIO_PIN_SET,
        UNDEFINED = -1
    };

    static void Enable(GPIO_TypeDef* gpioPort, uint32_t gpioPin, Mode mode,
                       Pull pull);
    static void Toggle(GPIO_TypeDef* gpioPort, uint32_t gpioPin);
    static void DigitalWrite(GPIO_TypeDef* gpioPort, uint32_t gpioPin,
                             State state);
    static State DigitalRead(GPIO_TypeDef* gpioPort, uint32_t gpioPin);
    static uint16_t AnalogRead(GPIO_TypeDef* gpioPort, uint32_t gpioPin);

private:
    GPIO() = default;
    static std::pair<ADC*, ADC::Channel>
    GetAnalogChannel(GPIO_TypeDef* gpioPort, uint32_t gpioPin);
};
} // namespace SBT::Hardware

#endif // F1XX_PROJECT_TEMPLATE_LIB_SBT_SDK_F1XX_SBT_SDK_HARDWARE_GPIO_HPP
