//
// Created by darkr on 12.03.2022.
//
#include "CommCAN.hpp"
#include <cstring>

namespace SBT::System::Comm {

using namespace SBT::System::Comm::CAN_ID;

void CAN::GenericMessage::CalculateExtID()
{
    extID =
        ((static_cast<uint32_t>(messageID.priority & 0x07) << 26) |
         (static_cast<uint32_t>(static_cast<uint8_t>(sourceID)) << 18) |
         (static_cast<uint32_t>(static_cast<uint16_t>(messageID.paramID) &
                                0x0FFF)
          << 6) |
         (static_cast<uint32_t>(static_cast<uint8_t>(messageID.group) & 0x3F)));
}

void CAN::GenericMessage::CalculateSBTid()
{
    // To check if received ID is in range we keep our enums without gasps, and
    // then only check limit values
    uint16_t temp;
    messageID.priority = (extID >> 26) & 0x07;

    if((temp = (extID >> 18) & 0xFF) <= static_cast<uint16_t>(Source::UNKNOWN))
        sourceID = static_cast<Source>(temp);
    else
        sourceID = Source::UNKNOWN;

    if((temp = ((extID >> 6) & 0x0FFF)) <=
       static_cast<uint16_t>(Param::UNKNOWN))
        messageID.paramID = static_cast<Param>(temp);
    else
        messageID.paramID = Param::UNKNOWN;

    if((temp = (extID & 0x3F)) <= static_cast<uint16_t>(Group::UNKNOWN))
        messageID.group = static_cast<Group>(temp);
    else
        messageID.group = Group::UNKNOWN;
}

CAN::GenericMessage::GenericMessage(Source sID, Message_t mID,
                                    uint8_t (&data)[8])
    : sourceID{sID}, messageID{mID}, extID{}, payload{}
{
    memcpy(payload, data, 8);

    CalculateExtID();
}

void CAN::GenericMessage::SetSourceID(Source _sourceID)
{
    sourceID = _sourceID;
    CalculateExtID();
}
void CAN::GenericMessage::SetMessageID(Message_t _messageID)
{
    messageID = _messageID;
    CalculateExtID();
}

void CAN::TxMessage::SetPayload(uint8_t (&payload)[8])
{
    memcpy(this->payload, payload, 8);
}

} // namespace SBT::System::Comm