
#include "CanSender.hpp"
#include "CAN.hpp"
#include "CommCAN.hpp"
#include "Error.hpp"

using namespace SBT::Hardware;
using namespace SBT::System::Comm;

namespace SBT::System::Tasks {

QueueHandle_t CanSender::xQueueHandle;
SemaphoreHandle_t CanSender::xSemaphore;
uint8_t CanSender::failedMessCount = 0;

CanSender::CanSender() : Task("CanSender", 12, 40) {}

void CanSender::initialize()
{
    // Create queue
    xQueueHandle = xQueueCreate(20, sizeof(CAN::TxMessage));
    if(xQueueHandle == NULL)
        softfault(__FILE__, __LINE__, "CanSender: Could not create xQueue");

    xSemaphore = xSemaphoreCreateCounting(3, 3);
    if(xSemaphore == NULL)
        softfault(__FILE__, __LINE__, "CanSender: Could not create xSemaphore");
}

void CanSender::run()
{
    // Get element from queue and send via Hardware CAN
    // Wait here until something is in queue
    xQueueReceive(xQueueHandle, &mess, portMAX_DELAY);

    if(xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE)
        SBT::Hardware::can.Send(mess.GetExtID(), mess.GetPayload());
}

void CanSender::CanTxCompleteCallback()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void CanSender::AddToQueue(CAN::TxMessage _mess)
{
    // 1 Tick here means 1ms
    if(xQueueSend(xQueueHandle, &_mess, static_cast<TickType_t>(100)) != pdTRUE)
        failedMessCount++;
}

void CanSender::AddToQueueFromISR(CAN::TxMessage _mess)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(xQueueSendFromISR(xQueueHandle, &_mess, &xHigherPriorityTaskWoken) !=
       pdTRUE)
        failedMessCount++;

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

} // namespace SBT::System::Tasks
