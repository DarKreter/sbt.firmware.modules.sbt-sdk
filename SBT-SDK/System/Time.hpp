#ifndef SBT_SYSTEM_TIME_HPP
#define SBT_SYSTEM_TIME_HPP

#include <cstdint>
#include <stm32f1xx_hal.h>

namespace SBT::System::Time {
inline uint32_t GetUpTime() { return HAL_GetTick(); }
} // namespace SBT::System::Time

#endif