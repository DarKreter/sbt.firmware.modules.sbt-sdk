#include "Heartbeat.hpp"

#ifndef SBT_CAN_DISABLE

#ifndef SBT_CAN_SENDER_DISABLE
#include "CanSender.hpp"
#endif
#ifndef SBT_CAN_RECEIVER_DISABLE
#include "CanReceiver.hpp"
#endif

#include "CommCAN.hpp"
#endif

#include "GPIO.hpp"
#include "Time.hpp"

namespace SBT::System::Tasks {

using namespace SBT::Hardware;
using namespace SBT::System;

#ifndef SBT_CAN_DISABLE
using namespace SBT::System::Comm;
#endif

Heartbeat::Heartbeat() : PeriodicTask("Heartbeat", 9, 1000, 80) {}

void Heartbeat::initialize()
{
    GPIO::Enable(GPIOC, GPIO_PIN_13, GPIO::Mode::Output, GPIO::Pull::NoPull);
}

void Heartbeat::run()
{
#ifndef SBT_CAN_DISABLE
#ifndef SBT_CAN_SENDER_DISABLE
    // Create payload for can send
    data.upTime = Time::GetUpTime();

#ifndef SBT_CAN_RECEIVER_DISABLE
    data.canRxMessFailCount = CanReceiver::GetFailedMessCount();
#else
    data.canRxMessFailCount = 0;
#endif

    data.canTxMessFailCount = CanSender::GetFailedMessCount();
    Pack_HEARTBEAT(&data, payload);

    // Send heartbeat
    CAN::Send(CAN_ID::Message::HEARTBEAT, payload);
#endif
#endif

    // blink builtin led
    GPIO::Toggle(GPIOC, GPIO_PIN_13);
}

} // namespace SBT::System::Tasks