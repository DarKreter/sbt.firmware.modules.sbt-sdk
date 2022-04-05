#include "Heartbeat.hpp"
#include "CanReceiver.hpp"
#include "CanSender.hpp"
#include "CommCAN.hpp"
#include "GPIO.hpp"
#include "Time.hpp"

namespace SBT::System::Tasks {

using namespace SBT::Hardware;
using namespace SBT::System;
using namespace SBT::System::Comm;

Heartbeat::Heartbeat() : PeriodicTask("Heartbeat", 9, 1000, 68), payload{} {}

void Heartbeat::initialize()
{
    GPIO::Enable(GPIOC, GPIO_PIN_13, GPIO::Mode::Output, GPIO::Pull::NoPull);
}

void Heartbeat::run()
{
    // Create payload for can send
    data.upTime = Time::GetUpTime();
    data.canRxMessFailCount = CanReceiver::GetFailedMessCount();
    data.canTxMessFailCount = CanSender::GetFailedMessCount();
    Pack_HEARTBEAT(&data, payload);

    // Send heartbeat
    CAN::Send(CAN_ID::Message::HEARTBEAT, payload);
    // blink builtin led
    GPIO::Toggle(GPIOC, GPIO_PIN_13);
}

} // namespace SBT::System::Tasks