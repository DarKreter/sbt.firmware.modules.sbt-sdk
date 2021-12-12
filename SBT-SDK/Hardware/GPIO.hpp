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
        Input,
        Output,
        AlternateInput,
        AlternatePP,
        AlternateOD,
        AnalogInput
    };

    enum class Pull {
        PullUp = 1,
        PullDown = 2,
        NoPull = 0
    };

    enum class State {
        LOW = 0,
        HIGH = 1,
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
