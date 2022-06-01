//
// Created by jazon on 11/13/21.
//

#include "GPIO.hpp"
#include "Error.hpp"
#include <set>

static void gpioError(const std::string& comment)
{
    softfault("GPIO: " + comment);
}

// Set of enabled pins
static std::set<std::pair<GPIO_TypeDef*, uint32_t>> enabledPins;

namespace SBT::Hardware {

void GPIO::Enable(GPIO_TypeDef* gpioPort, uint32_t gpioPin, GPIO::Mode mode,
                  GPIO::Pull pull)
{
    // Check if the requested pin is already enabled
    if(enabledPins.count({gpioPort, gpioPin}))
        gpioError("Requested pin is already in use");

    GPIO_InitTypeDef initTypeDef;
    initTypeDef.Pin = gpioPin;
    initTypeDef.Mode = static_cast<uint32_t>(mode);
    if(mode == Mode::AnalogInput) {
        auto [adc, channel] = GetAnalogChannel(gpioPort, gpioPin);
        if(!adc->IsConverterInited())
            adc->InitConverter();
        if(!adc->DoesChannelExist(channel))
            adc->CreateChannel(channel);
        adc->InitChannels();
    }
    initTypeDef.Pull = static_cast<uint32_t>(pull);
    initTypeDef.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(gpioPort, &initTypeDef);

    // Register the requested pin as enabled
    enabledPins.insert({gpioPort, gpioPin});
}
void GPIO::Disable(GPIO_TypeDef* gpioPort, uint32_t gpioPin)
{
    if(!enabledPins.count({gpioPort, gpioPin}))
        gpioError("Requested pin is not in use");

    HAL_GPIO_DeInit(gpioPort, gpioPin);

    enabledPins.erase({gpioPort, gpioPin});
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
        adc = &adc1;
    else
        gpioError("No ADC for requested port");

    // Compute ADC channel number from GPIO pin number
    uint32_t chn = 0;
    while(!(gpioPin & 1)) {
        gpioPin >>= 1;
        chn++;
    }

    return {adc, static_cast<ADC::Channel>(chn)};
}

} // namespace SBT::Hardware
