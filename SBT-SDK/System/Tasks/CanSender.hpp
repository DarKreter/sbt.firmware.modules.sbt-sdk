#ifndef CANSENDER_HPP
#define CANSENDER_HPP

#include "FreeRTOS.h"
#include "queue.h"

#include "CommCAN.hpp"
#include "TaskManager.hpp"
#include "semphr.h"

namespace SBT::System::Tasks {
/**
 * @brief This task has one purpose:
 * check if there is new value in queue, which contains transmitting messages
 * and if there is something it transmit it directly to the CAN bus
 * Task is running without any periodicity but getting from queue is based on
 * semaphore. That means if queue is empty task is getting into hibernation mode
 * until something is added. Queue size is 20 elements. Adding to queue is
 * timeouted to 100ms. If this process takes longer time than 100ms message will
 * not be added to queue. (After 100ms there is definitely bigger problem than
 * just full queue). But if the message is lost we increment failedMessCount
 * variable and Heartbeat is accessing this data and send them in heartbeat
 * frame.
 */

struct CanSender : public SBT::System::Task {
    CanSender();
    void initialize() override;
    void run() override;

    static QueueHandle_t xQueueHandle;
    SBT::System::Comm::CAN::TxMessage mess;

    static SemaphoreHandle_t xSemaphore;

    static uint8_t failedMessCount;

public:
    static uint8_t GetFailedMessCount() { return failedMessCount; }

    static void CanTxCompleteCallback();

    static void AddToQueue(SBT::System::Comm::CAN::TxMessage _mess);
    static void AddToQueueFromISR(SBT::System::Comm::CAN::TxMessage _mess);
};

} // namespace SBT::System::Tasks

#endif
