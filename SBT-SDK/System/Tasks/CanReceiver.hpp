#ifndef CANRECEIVER_HPP
#define CANRECEIVER_HPP

#include "FreeRTOS.h"
#include "queue.h"

#include "CommCAN.hpp"
#include "TaskManager.hpp"
/**
 * @brief This task checks if there is new message in receivedMessageQueue. If
 * there is new value it checks from which filter bank is this mess and calls
 * proper user callback. Adding to queue is happening in interrupt so there
 * isn't any timeout for that, if queue is full we lost this message. But if the
 * message is lost we increment failedMessCount variable and Heartbeat is
 * accessing this data and send them in heartbeat frame. It has almost the
 * highest priority. Queue size is 20. To call proper user function we use map
 * from System::Comm:CAN driver. Similarly as CanSender if queue is empty task
 * is getting into hibernation mode and blocks action until something is added.
 */
namespace SBT::System::Tasks {

struct CanReceiver : public SBT::System::Task {
    CanReceiver();
    void initialize() override;
    void run() override;

    static QueueHandle_t xQueueHandle;
    static SBT::System::Comm::CAN::RxMessage mess;

    static uint8_t failedMessCount;

public:
    static uint8_t GetFailedMessCount() { return failedMessCount; }

    static void AddToQueue(SBT::System::Comm::CAN::RxMessage _mess);
    static void AddToQueueFromISR(SBT::System::Comm::CAN::RxMessage _mess);
};

} // namespace SBT::System::Tasks

#endif
