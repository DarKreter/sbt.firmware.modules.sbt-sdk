//
// Created by darkr on 16.03.2022.
//

#ifndef F1XX_PROJECT_TEMPLATE_COMMCAN_HPP
#define F1XX_PROJECT_TEMPLATE_COMMCAN_HPP

#include <functional>
#include <map>
#include <stm32f1xx_hal.h>

#include "CanID_autogenerated.hpp"

// We need to befriend CanReceiver in CAN class
namespace SBT::System::Tasks {
struct CanReceiver;
}

namespace SBT::System::Comm {
class CAN {

    // class representing generic CAN message
    class GenericMessage {
    protected:
        // Our SubIDs
        CAN_ID::Source sourceID;
        CAN_ID::Message_t messageID;
        // Raw extended ID
        uint32_t extID;
        // Raw frame data
        uint8_t payload[8];

        // Calculating our subIDs basing on extended ID
        void CalculateSBTid();
        // Calculating extended ID basing on our SubIDs
        void CalculateExtID();

    public:
        GenericMessage() = default;
        /**
         * @brief Creating GenericMessage object
         * It also calculates extended ID.
         * @param sID sourceID
         * @param mID messageID
         * @param data payload
         */
        GenericMessage(CAN_ID::Source sID, CAN_ID::Message_t mID,
                       uint8_t (&data)[8]);

        /**
         * @brief Set source ID.
         * It also calculates extended ID.
         * @param _sourceID id to set
         */
        void SetSourceID(CAN_ID::Source _sourceID);
        /**
         * @brief Set message ID.
         * It also calculates extended ID.
         * @param _messageID id to set
         */
        void SetMessageID(CAN_ID::Message_t _messageID);

        /**
         * @brief Getter for extended ID
         * @return extended CAN ID
         */
        [[nodiscard]] uint32_t GetExtID() const { return extID; }
        /**
         * @brief Getter for source ID
         * @return source ID
         */
        [[nodiscard]] CAN_ID::Source GetSourceID() const { return sourceID; }
        /**
         * @brief Getter for message ID.
         * Contains ParamID, GroupID, priority
         * @return message ID
         */
        [[nodiscard]] CAN_ID::Message_t GetMessageID() const
        {
            return messageID;
        }
        /**
         * @brief Getter for payload
         * @return payload
         */
        [[nodiscard]] uint8_t* GetPayload() { return payload; }

        // CanReceiver need to call CalculateSBTid(); which we don't want to
        // show for standard user
        friend Tasks::CanReceiver;
    };

public:
    // Class for storing transmitted by us messages
    class TxMessage : public GenericMessage {

    public:
        TxMessage() = default;
        TxMessage(CAN_ID::Source sID, CAN_ID::Message_t mID, uint8_t (&data)[8])
            : GenericMessage(sID, mID, data){};

        /**
         * @brief Setter for message payload
         * @param payload
         */
        void SetPayload(uint8_t (&payload)[8]);

        friend CAN;
    };

    // Class for storing received messages from bus
    class RxMessage : public GenericMessage {
        // Filter bank id
        uint8_t filterBankID;

    public:
        RxMessage() = default;

        /**
         * @brief Getter for filter bank ID
         * @return filter bank number
         */
        uint8_t GetFilterBankID() { return filterBankID; }

        friend CAN;
    };

    // Class for easy creating filters
    class Filter {
    public:
        /**
         * @brief Each filter bank can operate in two modes: ID list mode and
         * Mask mode
         */
        enum class FilterType {
            ID_FILTER,
            MASK_FILTER
        };

    private:
        uint32_t filterID;
        uint32_t maskID;

        FilterType filterType;

    public:
        static uint8_t filterBankID;

        /**
         * @brief create object and calculate maskID and filterID based on
         * SourceID
         */
        Filter(CAN_ID::Source);
        /**
         * @brief create object and calculate maskID and filterID based on
         * GroupID
         */
        Filter(CAN_ID::Group);
        /**
         * @brief create object and calculate maskID and filterID based on
         * ParamID
         */
        Filter(CAN_ID::Param);
        /**
         * @brief create object and calculate maskID and filterID based on
         * SourceID & ParamID
         */
        Filter(CAN_ID::Source, CAN_ID::Param);
        /**
         * @brief create object and calculate maskID and filterID based on
         * SourceID & GroupID
         */
        Filter(CAN_ID::Source, CAN_ID::Group);
        /**
         * @brief Create raw filter
         * @param id1 in ID_FILTER: first ID to filter
         * in MASK_FILTER: filter ID
         * @param id2 in ID_FILTER: second ID to filter
         * in MASK_FILTER: mask ID
         * @param _filterType can be ID_FILTER or MASK_FILTER
         */
        Filter(uint32_t id1, uint32_t id2,
               FilterType _filterType = FilterType::ID_FILTER);

        /**
         * @brief Getter for MaskID
         * @return MaskID
         */
        uint32_t GetMaskID() const { return maskID; }
        /**
         * @brief Getter fot FilterID
         * @return FilterID
         */
        uint32_t GetFilterID() const { return filterID; }
        /**
         * @brief Getter for filter type
         * @return FilterType
         */
        FilterType GetFilterType() const { return filterType; }
    };

private:
    // Mapping filter bank numbers to user callbacks
    static std::map<uint8_t, std::function<void(RxMessage)>> filters;

    // default sourceID to use when someone calls Send without CAN_ID::Source as
    // parameter
    static CAN_ID::Source defaultSourceID;
    static bool initialized;

public:
    /**
     * @brief Getter for default source ID
     * @return defaultSourceID
     */
    static CAN_ID::Source GetDefaultSourceID();

    /**
     * @brief Initializing driver
     * @param _sID default source ID will be set to this value
     */
    static void Init(CAN_ID::Source _sID);

    /**
     * @brief Function for registering filters
     * @param filter Filter class object
     * @param callback callback which will be called after receiving message
     * that passes filter
     */
    static void AddFilter(const Filter& filter,
                          const std::function<void(RxMessage)>& callback);

    /**
     * @brief Function for registering filters

     * @param callback
     */
    /**
     * @brief Register a non-static class member function as a custom callback
     * @tparam T Class name
     * @param filter Filter class object
     * @param callbackObject A pointer to the object in context of which the
     * callbackFunction will be called
     * @param callbackFunction Callback which will be called after receiving
     * message that passes filter. A pointer to the callback function called in
     * the context of the callbackObject.
     */
    // This template envelopes the call of the member callback function in a
    // lambda and passes it as an independent callback function.
    template <class T>
    static void AddFilter(const Filter& filter, T* callbackObject,
                          void (T::*callbackFunction)(RxMessage))
    {
        AddFilter(filter, [callbackObject, callbackFunction](RxMessage mess) {
            (callbackObject->*callbackFunction)(mess);
        });
    }

    /**
     * @brief Function called in interrupt. Copy new received message to
     * received messages queue
     * @param fifoId HAL fifoID, required to get received message
     */
    static void CopyRxMessToQueue(uint32_t fifoId);

    /**
     * @brief Add message to transmit messages queue.
     * @param message to send
     */
    static void Send(TxMessage& message);
    /**
     * @brief Add message to transmit messages queue.
     * @param message to send
     */
    static void Send(TxMessage&& message);
    /**
     * @brief Add message to transmit messages queue.
     * @param sID Source ID of transmitting message
     * @param mID Message ID of transmitting message
     * @param data Raw payload of transmitting message
     */
    static void Send(CAN_ID::Source sID, CAN_ID::Message_t mID,
                     uint8_t (&data)[8]);
    /**
     * @brief Add message to transmit messages queue.
     * defaultSourceID is used as SourceID
     * @param mID Message ID of transmitting message
     * @param data Raw payload of transmitting message
     */
    static void Send(CAN_ID::Message_t mID, uint8_t (&data)[8]);

private:
    friend SBT::System::Tasks::CanReceiver;
};

} // namespace SBT::System::Comm

#endif // F1XX_PROJECT_TEMPLATE_COMMCAN_HPP
