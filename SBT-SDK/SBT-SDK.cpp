//
// Created by jazon on 11/1/21.
//

#include "SBT-SDK.hpp"

#include "stm32f1xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"

extern "C" {
void xPortSysTickHandler();
}

namespace SBT::System {
void Init()
{
    HAL_Init();
    Hardware::configureClocks();
    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
}

void Start()
{
    TaskManager::startTasks();

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

[[maybe_unused]] void softfault([[maybe_unused]] const std::string& fileName,
                                [[maybe_unused]] const int& lineNumber,
                                [[maybe_unused]] const std::string& comment)
{
    while(true)
        ;
}
} // namespace SBT::System

void vAssertCalled(const char* fileName, const int lineNumber)
{
    softfault(fileName, lineNumber, "FreeRTOS assertion failed");
}

void vApplicationMallocFailedHook(void)
{
    // Memory allocation failed. See call stack to find out where it happened.
    while(true)
        ;
}

void vApplicationStackOverflowHook([[maybe_unused]] TaskHandle_t xTask,
                                   [[maybe_unused]] char* pcTaskName)
{
    // Task of name 'pcTaskName' caused stack overflow
    while(true)
        ;
}
