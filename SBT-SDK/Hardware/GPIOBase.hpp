//
// Created by hubert25632 on 06.04.2022.
//

#ifndef F1XX_PROJECT_TEMPLATE_GPIOBASE_HPP
#define F1XX_PROJECT_TEMPLATE_GPIOBASE_HPP

#include <stm32f1xx_hal_gpio.h>

namespace SBT::Hardware {
class GPIOBase {
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
};
} // namespace SBT::Hardware

#endif // F1XX_PROJECT_TEMPLATE_GPIOBASE_HPP
