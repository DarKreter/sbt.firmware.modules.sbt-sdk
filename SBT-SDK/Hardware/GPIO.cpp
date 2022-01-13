//
// Created by jazon on 11/13/21.
//

#include "GPIO.hpp"
#include "Hardware.hpp"

namespace SBT::Hardware {

void GPIO::Enable(GPIO_TypeDef* gpioPort, uint32_t gpioPin, GPIO::Mode mode,
                  GPIO::Pull pull)
{
    GPIO_InitTypeDef initTypeDef;
    initTypeDef.Pin = gpioPin;
    initTypeDef.Mode = static_cast<uint32_t>(mode);
    if(mode == Mode::AnalogInput) {
        auto [adc, channel] = GetAnalogChannel(gpioPort, gpioPin);
        if(!adc->IsConverterInited())
            adc->InitConverter();
        adc->CreateChannel(channel);
        adc->InitChannels();
    }
    initTypeDef.Pull = static_cast<uint32_t>(pull);
    initTypeDef.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(gpioPort, &initTypeDef);
}
void GPIO::Toggle(GPIO_TypeDef* gpioPort, uint32_t gpioPin)
{
    HAL_GPIO_TogglePin(gpioPort, gpioPin);
}
void GPIO::DigitalWrite(GPIO_TypeDef* gpioPort, uint32_t gpioPin,
                        GPIO::State state)
{
    if(state == State::UNDEFINED)
        return;
    HAL_GPIO_WritePin(gpioPort, gpioPin, static_cast<GPIO_PinState>(state));
}
GPIO::State GPIO::DigitalRead(GPIO_TypeDef* gpioPort, uint32_t gpioPin)
{
    auto state = static_cast<GPIO::State>(HAL_GPIO_ReadPin(gpioPort, gpioPin));
    if(!(state == State::LOW || state == State::HIGH))
        return State::UNDEFINED;
    return state;
}

uint16_t GPIO::AnalogRead(GPIO_TypeDef* gpioPort, uint32_t gpioPin)
{
    auto [adc, channel] = GetAnalogChannel(gpioPort, gpioPin);
    return adc->GetChannelValue(channel);
}

std::pair<ADC*, ADC::Channel> GPIO::GetAnalogChannel(GPIO_TypeDef* gpioPort,
                                                     uint32_t gpioPin)
{
    ADC* adc;
    if(gpioPort == GPIOA)
        adc = &::Hardware::adc1;
    else
        softfault(__FILE__, __LINE__, "GPIO: No ADC for requested port");

    // Compute ADC channel number from GPIO pin number
    uint32_t chn = 0;
    while(!(gpioPin & 1)) {
        gpioPin >>= 1;
        chn++;
    }

    return {adc, static_cast<ADC::Channel>(chn)};
}

} // namespace SBT::Hardware
