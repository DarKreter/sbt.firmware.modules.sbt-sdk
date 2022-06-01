//
// Created by jazon on 11/13/21.
//

#ifndef F1XX_PROJECT_TEMPLATE_LIB_SBT_SDK_F1XX_SBT_SDK_HARDWARE_GPIO_HPP
#define F1XX_PROJECT_TEMPLATE_LIB_SBT_SDK_F1XX_SBT_SDK_HARDWARE_GPIO_HPP

#include "ADC.hpp"
#include "GPIOBase.hpp"
#include "Pinouts.hpp"

namespace SBT::Hardware {
class GPIO : public GPIOBase {
public:
    static void Enable(GPIO_TypeDef* gpioPort, uint32_t gpioPin, Mode mode,
                       Pull pull);
    static void Disable(GPIO_TypeDef* gpioPort, uint32_t gpioPin);
    static void Toggle(GPIO_TypeDef* gpioPort, uint32_t gpioPin);
    static void DigitalWrite(GPIO_TypeDef* gpioPort, uint32_t gpioPin,
                             State state);
    static State DigitalRead(GPIO_TypeDef* gpioPort, uint32_t gpioPin);
    static uint16_t AnalogRead(GPIO_TypeDef* gpioPort, uint32_t gpioPin);

    // Overloaded methods using BSP structures
    static inline void Enable(BSP::DevicePinConfig devicePinConfig)
    {
        Enable(reinterpret_cast<GPIO_TypeDef*>(devicePinConfig.gpioPortBase),
               devicePinConfig.gpioPin, devicePinConfig.mode,
               devicePinConfig.pull);
    }

    static inline void Disable(BSP::DevicePin devicePin)
    {
        Disable(reinterpret_cast<GPIO_TypeDef*>(devicePin.gpioPortBase),
                devicePin.gpioPin);
    }

    static inline void Toggle(BSP::DevicePin devicePin)
    {
        Toggle(reinterpret_cast<GPIO_TypeDef*>(devicePin.gpioPortBase),
               devicePin.gpioPin);
    }

    static inline void DigitalWrite(BSP::DevicePin devicePin, State state)
    {
        DigitalWrite(reinterpret_cast<GPIO_TypeDef*>(devicePin.gpioPortBase),
                     devicePin.gpioPin, state);
    }

    static inline State DigitalRead(BSP::DevicePin devicePin)
    {
        return DigitalRead(
            reinterpret_cast<GPIO_TypeDef*>(devicePin.gpioPortBase),
            devicePin.gpioPin);
    }

    static inline uint16_t AnalogRead(BSP::DevicePin devicePin)
    {
        return AnalogRead(
            reinterpret_cast<GPIO_TypeDef*>(devicePin.gpioPortBase),
            devicePin.gpioPin);
    }

private:
    GPIO() = default;
    static std::pair<ADC*, ADC::Channel>
    GetAnalogChannel(GPIO_TypeDef* gpioPort, uint32_t gpioPin);
};
} // namespace SBT::Hardware

#endif // F1XX_PROJECT_TEMPLATE_LIB_SBT_SDK_F1XX_SBT_SDK_HARDWARE_GPIO_HPP
