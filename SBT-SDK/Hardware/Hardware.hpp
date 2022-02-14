#ifndef HARDWARE_HPP
#define HARDWARE_HPP

#include <cstdint>

namespace SBT::Hardware {
/**
 * @brief Configure system clock
 */
void configureClocks(uint32_t ahbFreq = 72'000'000);
uint32_t GetAHB_Freq();
uint32_t GetAPB1_Freq();
uint32_t GetAPB2_Freq();
} // namespace SBT::Hardware

struct [[deprecated("Hardware components have been moved to the SBT::Hardware "
                    "namespace")]] Hardware;

#endif // HARDWARE_HPP
