
#include "CanReceiver.hpp"
#include "CAN.hpp"
#include "CommCAN.hpp"
#include "Error.hpp"

using namespace SBT::Hardware;
using namespace SBT::System::Comm;

namespace SBT::System::Tasks {

QueueHandle_t CanReceiver::xQueueHandle;
CAN::RxMessage CanReceiver::mess;
uint8_t CanReceiver::failedMessCount = 0;

CanReceiver::CanReceiver() : Task("CanReceiver", 15, 256) {}

void CanReceiver::initialize()
{
    // Create queue
    xQueueHandle = xQueueCreate(20, sizeof(CAN::RxMessage));

    if(xQueueHandle == NULL)
        softfault(__FILE__, __LINE__, "CanReceiver: Could not create xQueue");
}

void CanReceiver::run()
{
    // Get element from queue and send via Hardware CAN
    xQueueReceive(xQueueHandle, &mess, portMAX_DELAY);

    // Calculate our ID from raw extended CAN ID
    mess.CalculateSBTid();

    // Call proper user function
    std::invoke(CAN::filters[mess.GetFilterBankID()], mess);
}

void CanReceiver::AddToQueue(CAN::RxMessage _mess)
{
    // 1 Tick here means 1ms
    if(xQueueSend(xQueueHandle, &_mess, static_cast<TickType_t>(100)) != pdTRUE)
        failedMessCount++;
}

void CanReceiver::AddToQueueFromISR(CAN::RxMessage _mess)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(xQueueSendFromISR(xQueueHandle, &_mess, &xHigherPriorityTaskWoken) !=
       pdTRUE)
        failedMessCount++;

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

} // namespace SBT::System::Tasks
