//
// Created by darkr on 16.03.2022.
//

#include "CommCAN.hpp"
#include "CAN.hpp"
#include "CanReceiver.hpp"
#include "CanSender.hpp"
#include "Error.hpp"

#define CAN_ERROR(comment)                                                     \
    softfault(__FILE__, __LINE__,                                              \
              std::string("CommCAN: ") + std::string(comment))

#define CAN_ERROR_NOT_INIT CAN_ERROR("Not initialized")

namespace SBT::System::Comm {

using namespace SBT::System::Comm::CAN_ID;

uint8_t CAN::Filter::filterBankID = 0;

std::map<uint8_t, std::function<void(CAN::RxMessage)>> CAN::filters;
Source CAN::defaultSourceID = Source::DEFAULT;
bool CAN::initialized = false;

CAN::Filter::Filter(Group _gID) : filterType{FilterType::MASK_FILTER}
{
    maskID = 0x1FFFFFFF & 0x3F;
    filterID = 0x1FFFFFFF & static_cast<uint32_t>(_gID);
}

CAN::Filter::Filter(Param _pID) : filterType{FilterType::MASK_FILTER}
{
    maskID = (0x1FFFFFFF & 0x0FFF) << 6;
    filterID = (0x1FFFFFFF & static_cast<uint32_t>(_pID)) << 6;
}

CAN::Filter::Filter(Source _sID) : filterType{FilterType::MASK_FILTER}
{
    maskID = (0x1FFFFFFF & 0xFF) << 18;
    filterID = (0x1FFFFFFF & static_cast<uint32_t>(_sID)) << 18;
}

CAN::Filter::Filter(Source _sID, Param _pID)
    : filterType{FilterType::MASK_FILTER}
{
    maskID = ((0x1FFFFFFF & 0xFF) << 18) | ((0x1FFFFFFF & 0x0FFF) << 6);
    filterID = ((0x1FFFFFFF & static_cast<uint32_t>(_sID)) << 18) |
               ((0x1FFFFFFF & static_cast<uint32_t>(_pID)) << 6);
}

CAN::Filter::Filter(Source _sID, CAN_ID::Group _gID)
    : filterType{FilterType::MASK_FILTER}
{
    maskID = ((0x1FFFFFFF & 0xFF) << 18) | (0x1FFFFFFF & 0x3F);
    filterID = ((0x1FFFFFFF & static_cast<uint32_t>(_sID)) << 18) |
               (0x1FFFFFFF & static_cast<uint32_t>(_gID));
}

CAN::Filter::Filter(uint32_t id1, uint32_t id2, FilterType _filterType)
    : filterType{_filterType}
{
    filterID = id1;
    maskID = id2;
}

Source CAN::GetDefaultSourceID() { return defaultSourceID; }

void CAN::Init(Source _sID)
{
    defaultSourceID = _sID;
    initialized = true;
}

void CAN::AddFilter(const Filter& filter,
                    const std::function<void(RxMessage)>& callback)
{
    if(!initialized)
        CAN_ERROR_NOT_INIT;

    if(Filter::filterBankID >= 14)
        CAN_ERROR("Too many filters. (You have only 14 filter banks)");

    if(filter.GetFilterType() == Filter::FilterType::MASK_FILTER)
        Hardware::can.AddFilter_MASK(Filter::filterBankID, filter.GetFilterID(),
                                     filter.GetMaskID());

    else if(filter.GetFilterType() == Filter::FilterType::ID_FILTER)
        Hardware::can.AddFilter_LIST(Filter::filterBankID, filter.GetFilterID(),
                                     filter.GetMaskID());

    // Add new callback to map
    filters[Filter::filterBankID++] = callback;
}

void CAN::Send(CAN::TxMessage& message)
{
    if(!initialized)
        CAN_ERROR_NOT_INIT;

    // Send is adding message to queue and from there it will be transmitted to
    // the CAN bus
    SBT::System::Tasks::CanSender::AddToQueue(message);
    //    SBT::Hardware::can.Send(message.GetExtID(), message.GetPayload());
}

void CAN::Send(TxMessage&& message) { Send(message); }

void CAN::Send(Source sID, Message_t mID, uint8_t (&data)[8])
{
    Send(TxMessage(sID, mID, data));
}

void CAN::Send(Message_t mID, uint8_t (&data)[8])
{
    Send(TxMessage(defaultSourceID, mID, data));
}

void CAN::CopyRxMessToQueue(uint32_t fifoId)
{
    CAN::RxMessage message{};

    Hardware::can.GetRxMessage(fifoId, &message.extID, message.payload,
                               &message.filterBankID);

    // To avoid calling user function in interrupt, we add this message to queue
    // and system task will get it from here and call proper callback
    Tasks::CanReceiver::AddToQueueFromISR(message);
}

} // namespace SBT::System::Comm
