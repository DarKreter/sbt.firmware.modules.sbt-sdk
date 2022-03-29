#ifndef HARDWARE_HPP
#define HARDWARE_HPP

#include <cstdint>
#include <stm32f1xx_hal.h>

namespace SBT::Hardware {
/**
 * @brief Configure system clock
 */
void configureClocks(uint32_t ahbFreq = 72'000'000);
uint32_t GetAHB_Freq();
uint32_t GetAPB1_Freq();
uint32_t GetAPB2_Freq();

void StartWatchdog(IWDG_HandleTypeDef&, unsigned);
} // namespace SBT::Hardware

struct [[deprecated("Hardware components have been moved to the SBT::Hardware "
                    "namespace")]] Hardware;

#endif // HARDWARE_HPP
