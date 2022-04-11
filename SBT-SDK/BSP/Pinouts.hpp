//
// Created by hubert25632 on 24.02.2022.
//

#ifndef F1XX_PROJECT_TEMPLATE_PINOUTS_HPP
#define F1XX_PROJECT_TEMPLATE_PINOUTS_HPP

#include "GPIOBase.hpp"
#include "Mainboards.hpp"

namespace SBT::BSP {

using namespace Hardware;

struct DevicePin {
    uint32_t gpioPortBase;
    uint32_t gpioPin;
};

struct DevicePinConfig : public DevicePin {
    GPIOBase::Mode mode;
    GPIOBase::Pull pull;
};

struct CANPinout {
    DevicePinConfig rx;
    DevicePinConfig tx;
};

struct I2CPinout {
    DevicePinConfig scl;
    DevicePinConfig sda;
};

struct SPIPinout {
    DevicePinConfig sck;
    DevicePinConfig miso;
    DevicePinConfig mosi;
};

struct UARTPinout {
    DevicePinConfig tx;
    DevicePinConfig rx;
};

namespace Pinouts {
constexpr CANPinout CAN_1 = {
#ifndef SBT_BSP_REMAP_CAN1
    {{GPIOA_BASE, GPIO_PIN_11}, GPIOBase::Mode::Input, GPIOBase::Pull::NoPull},
    {{GPIOA_BASE, GPIO_PIN_12},
     GPIOBase::Mode::AlternatePP,
     GPIOBase::Pull::NoPull}
#else
    {{GPIOB_BASE, GPIO_PIN_8}, GPIOBase::Mode::Input, GPIOBase::Pull::NoPull},
    {{GPIOB_BASE, GPIO_PIN_9},
     GPIOBase::Mode::AlternatePP,
     GPIOBase::Pull::NoPull}
#endif
};

constexpr I2CPinout I2C_1 = {
#ifndef SBT_BSP_REMAP_I2C1
    {{GPIOB_BASE, GPIO_PIN_6},
     GPIOBase::Mode::AlternateOD,
     GPIOBase::Pull::PullUp},
    {{GPIOB_BASE, GPIO_PIN_7},
     GPIOBase::Mode::AlternateOD,
     GPIOBase::Pull::PullUp}
#else
    {{GPIOB_BASE, GPIO_PIN_8},
     GPIOBase::Mode::AlternateOD,
     GPIOBase::Pull::PullUp},
    {{GPIOB_BASE, GPIO_PIN_9},
     GPIOBase::Mode::AlternateOD,
     GPIOBase::Pull::PullUp}
#endif
};

constexpr I2CPinout I2C_2 = {{{GPIOB_BASE, GPIO_PIN_10},
                              GPIOBase::Mode::AlternateOD,
                              GPIOBase::Pull::PullUp},
                             {{GPIOB_BASE, GPIO_PIN_11},
                              GPIOBase::Mode::AlternateOD,
                              GPIOBase::Pull::PullUp}};

constexpr SPIPinout SPI_1 = {
#ifndef SBT_BSP_REMAP_SPI1
    {{GPIOA_BASE, GPIO_PIN_5},
     GPIOBase::Mode::AlternatePP,
     GPIOBase::Pull::NoPull},
    {{GPIOA_BASE, GPIO_PIN_6},
     GPIOBase::Mode::AlternateInput,
     GPIOBase::Pull::NoPull},
    {{GPIOA_BASE, GPIO_PIN_7},
     GPIOBase::Mode::AlternatePP,
     GPIOBase::Pull::NoPull}
#else
    {{GPIOB_BASE, GPIO_PIN_3},
     GPIOBase::Mode::AlternatePP,
     GPIOBase::Pull::NoPull},
    {{GPIOB_BASE, GPIO_PIN_4},
     GPIOBase::Mode::AlternateInput,
     GPIOBase::Pull::NoPull},
    {{GPIOB_BASE, GPIO_PIN_5},
     GPIOBase::Mode::AlternatePP,
     GPIOBase::Pull::NoPull}
#endif
};

constexpr SPIPinout SPI_2 = {{{GPIOB_BASE, GPIO_PIN_13},
                              GPIOBase::Mode::AlternatePP,
                              GPIOBase::Pull::NoPull},
                             {{GPIOB_BASE, GPIO_PIN_14},
                              GPIOBase::Mode::AlternateInput,
                              GPIOBase::Pull::NoPull},
                             {{GPIOB_BASE, GPIO_PIN_15},
                              GPIOBase::Mode::AlternatePP,
                              GPIOBase::Pull::NoPull}};

constexpr UARTPinout UART_1 = {
#ifndef SBT_BSP_REMAP_UART1
    {{GPIOA_BASE, GPIO_PIN_9},
     GPIOBase::Mode::AlternatePP,
     GPIOBase::Pull::NoPull},
    {{GPIOA_BASE, GPIO_PIN_10},
     GPIOBase::Mode::AlternateInput,
     GPIOBase::Pull::PullUp}
#else
    {{GPIOB_BASE, GPIO_PIN_6},
     GPIOBase::Mode::AlternatePP,
     GPIOBase::Pull::NoPull},
    {{GPIOB_BASE, GPIO_PIN_7},
     GPIOBase::Mode::AlternateInput,
     GPIOBase::Pull::PullUp}
#endif
};

constexpr UARTPinout UART_2 = {{{GPIOA_BASE, GPIO_PIN_2},
                                GPIOBase::Mode::AlternatePP,
                                GPIOBase::Pull::NoPull},
                               {{GPIOA_BASE, GPIO_PIN_3},
                                GPIOBase::Mode::AlternateInput,
                                GPIOBase::Pull::PullUp}};

constexpr UARTPinout UART_3 = {{{GPIOB_BASE, GPIO_PIN_10},
                                GPIOBase::Mode::AlternatePP,
                                GPIOBase::Pull::NoPull},
                               {{GPIOB_BASE, GPIO_PIN_11},
                                GPIOBase::Mode::AlternateInput,
                                GPIOBase::Pull::PullUp}};

// High-power pins

#ifdef SBT_BSP_PORT_BASE_HP1
#ifdef SBT_BSP_PIN_HP1
constexpr DevicePin HP_1{SBT_BSP_PORT_BASE_HP1, SBT_BSP_PIN_HP1};
#endif
#endif

#ifdef SBT_BSP_PORT_BASE_HP2
#ifdef SBT_BSP_PIN_HP2
constexpr DevicePin HP_2{SBT_BSP_PORT_BASE_HP2, SBT_BSP_PIN_HP2};
#endif
#endif

#ifdef SBT_BSP_PORT_BASE_HP3
#ifdef SBT_BSP_PIN_HP3
constexpr DevicePin HP_3{SBT_BSP_PORT_BASE_HP3, SBT_BSP_PIN_HP3};
#endif
#endif

#ifdef SBT_BSP_PORT_BASE_HP4
#ifdef SBT_BSP_PIN_HP4
constexpr DevicePin HP_4{SBT_BSP_PORT_BASE_HP4, SBT_BSP_PIN_HP4};
#endif
#endif

} // namespace Pinouts
} // namespace SBT::BSP

#endif // F1XX_PROJECT_TEMPLATE_PINOUTS_HPP
