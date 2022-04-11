//
// Created by jazon on 11/1/21.
//

#include "SBT-SDK.hpp"

#ifndef SBT_CAN_DISABLE
#include "CAN.hpp"
#include "CommCAN.hpp"

#ifndef SBT_CAN_SENDER_DISABLE
#include "CanSender.hpp"
#endif
#ifndef SBT_CAN_RECEIVER_DISABLE
#include "CanReceiver.hpp"
#endif
#endif

#include "Hardware.hpp"

#ifndef SBT_HEARTBEAT_DISABLE
#include "Heartbeat.hpp"
#endif

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

#ifndef SBT_CAN_DISABLE

#ifndef SBT_CAN_MODE
#define SBT_CAN_MODE NORMAL
#endif

    Hardware::can.SetMode(Hardware::hCAN::Mode::SBT_CAN_MODE);

#ifndef SBT_CAN_BAUDRATE
#define SBT_CAN_BAUDRATE 250000
#endif

    Hardware::can.SetBaudRate(SBT_CAN_BAUDRATE);

    using namespace Hardware;
    using namespace System::Comm;

#ifndef SBT_CAN_RECEIVER_DISABLE
    Hardware::can.RegisterCallback(hCAN::CallbackType::RxFifo0MsgPending, []() {
        CAN::CopyRxMessToQueue(CAN_RX_FIFO0);
    });
    //    Hardware::can.RegisterCallback(hCAN::CallbackType::RxFifo1MsgPending,
    //    []() {
    //        CAN::CopyRxMessToQueue(CAN_RX_FIFO1);
    //    });
#endif

#ifndef SBT_CAN_SENDER_DISABLE
    Hardware::can.RegisterCallback(hCAN::CallbackType::TxMailbox0Complete,
                                   Tasks::CanSender::CanTxCompleteCallback);
    Hardware::can.RegisterCallback(hCAN::CallbackType::TxMailbox1Complete,
                                   Tasks::CanSender::CanTxCompleteCallback);
    Hardware::can.RegisterCallback(hCAN::CallbackType::TxMailbox2Complete,
                                   Tasks::CanSender::CanTxCompleteCallback);
#endif

    Hardware::can.Initialize();

#ifndef SBT_CAN_ID
#define SBT_CAN_ID DEFAULT
#endif

    CAN::Init(CAN_ID::Source::SBT_CAN_ID);
#endif

    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
}

void Start([[maybe_unused]] unsigned watchdogTimeout_ms)
{
    // Register system tasks

#ifndef SBT_HEARTBEAT_DISABLE
    TaskManager::registerSystemTask(
        std::make_shared<System::Tasks::Heartbeat>());
#endif

#ifndef SBT_CAN_DISABLE
#ifndef SBT_CAN_SENDER_DISABLE
    TaskManager::registerSystemTask(
        std::make_shared<System::Tasks::CanSender>());
#endif
#ifndef SBT_CAN_RECEIVER_DISABLE
    TaskManager::registerSystemTask(
        std::make_shared<System::Tasks::CanReceiver>());
#endif
#endif

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

#ifndef SBT_CAN_DISABLE
    // Start CAN
    Hardware::can.Start();
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
