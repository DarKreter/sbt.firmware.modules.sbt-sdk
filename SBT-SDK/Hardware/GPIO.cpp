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
    switch(mode) {
    case Mode::Input:
        initTypeDef.Mode = GPIO_MODE_INPUT;
        break;
    case Mode::Output:
        initTypeDef.Mode = GPIO_MODE_OUTPUT_PP;
        break;
    case Mode::AlternateInput:
        initTypeDef.Mode = GPIO_MODE_AF_INPUT;
        break;
    case Mode::AlternatePP:
        initTypeDef.Mode = GPIO_MODE_AF_PP;
        break;
    case Mode::AlternateOD:
        initTypeDef.Mode = GPIO_MODE_AF_OD;
        break;
    case Mode::AnalogInput:
        initTypeDef.Mode = GPIO_MODE_ANALOG;
        auto [adc, channel] = GetAnalogChannel(gpioPort, gpioPin);
        if(!adc->IsConverterInited())
            adc->InitConverter();
        adc->CreateChannel(channel);
        adc->InitChannels();
        break;
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
    switch(state) {
    case State::HIGH:
        HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_SET);
        break;
    case State::LOW:
        HAL_GPIO_WritePin(gpioPort, gpioPin, GPIO_PIN_RESET);
        break;
    case State::UNDEFINED:
        break;
    }
}
GPIO::State GPIO::DigitalRead(GPIO_TypeDef* gpioPort, uint32_t gpioPin)
{
    switch(HAL_GPIO_ReadPin(gpioPort, gpioPin)) {
    case GPIO_PIN_SET:
        return State::HIGH;
    case GPIO_PIN_RESET:
        return State::LOW;
    }

    return State::UNDEFINED;
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
