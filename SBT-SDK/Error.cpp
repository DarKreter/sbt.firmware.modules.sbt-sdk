//
// Created by hubert25632 on 12.02.2022.
//

#include "Error.hpp"
#include <FreeRTOS.h>
#include <task.h>

void softfault([[maybe_unused]] const std::string& fileName,
               [[maybe_unused]] const int& lineNumber,
               [[maybe_unused]] const std::string& comment)
{
    // A software error occurred
    taskDISABLE_INTERRUPTS();
    while(true)
        ;
}

void softfault([[maybe_unused]] const std::string& comment)
{
    // A software error occurred. See call stack to find out where it happened.
    taskDISABLE_INTERRUPTS();
    while(true)
        ;
}

void vAssertCalled([[maybe_unused]] const char* fileName,
                   [[maybe_unused]] const int lineNumber)
{
    // FreeRTOS assertion failed
    taskDISABLE_INTERRUPTS();
    while(true)
        ;
}

void vApplicationMallocFailedHook()
{
    // Memory allocation failed. See call stack to find out where it happened.
    taskDISABLE_INTERRUPTS();
    while(true)
        ;
}

void vApplicationStackOverflowHook([[maybe_unused]] TaskHandle_t xTask,
                                   [[maybe_unused]] char* pcTaskName)
{
    // Task of name 'pcTaskName' caused stack overflow
    taskDISABLE_INTERRUPTS();
    while(true)
        ;
}
