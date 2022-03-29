//
// Created by jazon on 11/1/21.
//

#include "SBT-SDK.hpp"
#include "CAN.hpp"
#include "CanReceiver.hpp"
#include "CanSender.hpp"
#include "CommCAN.hpp"
#include "Hardware.hpp"
#include "Heartbeat.hpp"

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

    Hardware::can.SetMode(Hardware::hCAN::Mode::NORMAL);
    Hardware::can.SetBaudRate(250'000);

    using namespace Hardware;
    using namespace System::Comm;

    Hardware::can.RegisterCallback(hCAN::CallbackType::RxFifo0MsgPending, []() {
        CAN::CopyRxMessToQueue(CAN_RX_FIFO0);
    });
    //    Hardware::can.RegisterCallback(hCAN::CallbackType::RxFifo1MsgPending,
    //    []() {
    //        CAN::CopyRxMessToQueue(CAN_RX_FIFO1);
    //    });

    Hardware::can.RegisterCallback(hCAN::CallbackType::TxMailbox0Complete,
                                   Tasks::CanSender::CanTxCompleteCallback);
    Hardware::can.RegisterCallback(hCAN::CallbackType::TxMailbox1Complete,
                                   Tasks::CanSender::CanTxCompleteCallback);
    Hardware::can.RegisterCallback(hCAN::CallbackType::TxMailbox2Complete,
                                   Tasks::CanSender::CanTxCompleteCallback);

    Hardware::can.Initialize();

    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
}

void Start([[maybe_unused]] unsigned watchdogTimeout_ms)
{
    // Register system tasks
    TaskManager::registerSystemTask(
        std::make_shared<System::Tasks::Heartbeat>());
    TaskManager::registerSystemTask(
        std::make_shared<System::Tasks::CanSender>());
    TaskManager::registerSystemTask(
        std::make_shared<System::Tasks::CanReceiver>());

    // Register all tasks in FreeRTOS - allocate local stack etc.
    TaskManager::startTasks();

#ifdef SBT_DEBUG
#pragma message "Watchdog is not enabled in debug mode"
#else
    // Configure and start the watchdog
    Hardware::StartWatchdog(hiwdg, watchdogTimeout_ms);
#endif

    // Calls "initialize()" function for all registered tasks
    TaskManager::TasksInit();

    // Start CAN
    Hardware::can.Start();

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
