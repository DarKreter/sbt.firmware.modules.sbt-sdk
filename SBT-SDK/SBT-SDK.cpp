//
// Created by jazon on 11/1/21.
//

#include "SBT-SDK.hpp"
#include "Error.hpp"
#include "Hardware.hpp"

#include "stm32f1xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"

#ifndef SBT_DEBUG
static IWDG_HandleTypeDef hiwdg;
#endif

extern "C" {
void xPortSysTickHandler();
void vApplicationIdleHook(void)
{
#ifndef SBT_DEBUG
    HAL_IWDG_Refresh(&hiwdg);
#endif
}
}

namespace SBT::System {
void Init()
{
    HAL_Init();
    Hardware::configureClocks();
    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
}

void Start([[maybe_unused]] unsigned watchdogTimeout_ms)
{
    TaskManager::startTasks();

#ifdef SBT_DEBUG
#pragma message "Watchdog is not enabled in debug mode"
#else
    // Configure and start the watchdog

    hiwdg.Instance = IWDG;

    hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
    hiwdg.Init.Reload = 0;

    // Compute the best prescaler and reload values
    for(unsigned i = 0; i < 7; i++) {
        unsigned prescalerExponent = i + 2;
        unsigned reload =
            (watchdogTimeout_ms * LSI_VALUE / (1000 << prescalerExponent));

        // Check whether the new reload value is acceptable
        if(reload > 4096)
            continue;

        // If the new prescaler and reload values give smaller error, use them
        // instead of the old ones
        if(std::abs(static_cast<int>(watchdogTimeout_ms) -
                    static_cast<int>(reload * (1000 << prescalerExponent) /
                                     LSI_VALUE)) <
           std::abs(static_cast<int>(watchdogTimeout_ms) -
                    static_cast<int>(hiwdg.Init.Reload *
                                     (1000 << hiwdg.Init.Prescaler) /
                                     LSI_VALUE))) {
            hiwdg.Init.Prescaler = prescalerExponent;
            hiwdg.Init.Reload = reload;
        }
    }

    if(hiwdg.Init.Reload == 0)
        softfault(__FILE__, __LINE__,
                  "Requested watchdog timeout value could not be achieved");

    // Prescaler and Reload values are directly written to the IWDG registers
    hiwdg.Init.Prescaler -= 2;
    hiwdg.Init.Reload -= 1;

    HAL_IWDG_Init(&hiwdg);
#endif

    // Start FreeRTOS Kernel
    // should never return
    vTaskStartScheduler();
}

void SystickHandler()
{
    HAL_IncTick();
#if(INCLUDE_xTaskGetSchedulerState == 1)
    if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
#endif /* INCLUDE_xTaskGetSchedulerState */
        xPortSysTickHandler();
#if(INCLUDE_xTaskGetSchedulerState == 1)
    }
#endif /* INCLUDE_xTaskGetSchedulerState */
}
} // namespace SBT::System
