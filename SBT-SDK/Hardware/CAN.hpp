//
// Created by darkr on 22.05.2021.
//

#ifndef F1XX_PROJECT_TEMPLATE_CAN_HPP
#define F1XX_PROJECT_TEMPLATE_CAN_HPP

#include <FreeRTOS.h>
#include <array>
#include <event_groups.h>
#include <optional>
#include <queue.h>
#include <stm32f1xx_hal.h>

#include <CAN_ID.hpp>

struct Hardware;

class CAN {
    struct State {
        CAN_HandleTypeDef handle;
        QueueHandle_t queueHandle;
    };

    CAN();

public:
    enum class Mode : uint32_t {
        NORMAL = CAN_MODE_NORMAL,
        LOOPBACK = CAN_MODE_LOOPBACK,
        SILENT = CAN_MODE_SILENT,
        SILENT_LOOPBACK = CAN_MODE_SILENT_LOOPBACK
    };

private:
    enum class SWJ : uint32_t {
        _1TQ = CAN_SJW_1TQ,
        _2TQ = CAN_SJW_2TQ,
        _3TQ = CAN_SJW_3TQ,
        _4TQ = CAN_SJW_4TQ,
    } swj;

    enum class BS1 : uint32_t {
        _1TQ = CAN_BS1_1TQ,
        _2TQ = CAN_BS1_2TQ,
        _3TQ = CAN_BS1_3TQ,
        _4TQ = CAN_BS1_4TQ,
        _5TQ = CAN_BS1_5TQ,
        _6TQ = CAN_BS1_6TQ,
        _7TQ = CAN_BS1_7TQ,
        _8TQ = CAN_BS1_8TQ,
        _9TQ = CAN_BS1_9TQ,
        _10TQ = CAN_BS1_10TQ,
        _11TQ = CAN_BS1_11TQ,
        _12TQ = CAN_BS1_12TQ,
        _13TQ = CAN_BS1_13TQ,
        _14TQ = CAN_BS1_14TQ,
        _15TQ = CAN_BS1_15TQ,
        _16TQ = CAN_BS1_16TQ,
    } bs1;

    enum class BS2 : uint32_t {
        _1TQ = CAN_BS2_1TQ,
        _2TQ = CAN_BS2_2TQ,
        _3TQ = CAN_BS2_3TQ,
        _4TQ = CAN_BS2_4TQ,
        _5TQ = CAN_BS2_5TQ,
        _6TQ = CAN_BS2_6TQ,
        _7TQ = CAN_BS2_7TQ,
        _8TQ = CAN_BS2_8TQ
    } bs2;

    uint16_t prescaler; // 1-1024

    /**
     * @brief Set swj, bs1, bs2 and prescaler to values which gives us certain
     * speed based on clock speed
     */
    void CalculateTQ();

    bool initialized;
    State state;
    Mode mode;
    BoxId deviceID;
    uint32_t baudRate;

    class GenericMessage {
    protected:
        CAN_TxHeaderTypeDef header;

        union {
            uint8_t payload[8];

            struct {
                uint16_t : 16; // make this bit field because then it can be
                               // without name
                ParameterId parameterID; // : 16
                int32_t data;
            } integer;

            struct {
                uint16_t : 16; // make this bit field because then it can be
                               // without name
                ParameterId parameterID; // : 16
                float data;
            } floating;
        };

        void ConfigureMessage(BoxId id);

    public:
        GenericMessage() = default;
        GenericMessage(ParameterId id, int32_t parameter);
        GenericMessage(ParameterId id, float parameter);

        [[nodiscard]] ParameterId GetParameterID() const
        {
            return integer.parameterID;
        }
        [[nodiscard]] int32_t GetDataInt() const { return integer.data; }
        [[nodiscard]] float GetDataFloat() const { return floating.data; }
        [[nodiscard]] uint8_t* GetPayload() { return payload; }
    };

public:
    class TxMessage : public GenericMessage {

    public:
        TxMessage() = default;
        TxMessage(ParameterId id, int32_t parameter)
            : GenericMessage(id, parameter){};
        TxMessage(ParameterId id, float parameter)
            : GenericMessage(id, parameter){};

        void SetParameterID(ParameterId id);
        void SetData(int32_t parameter);
        void SetData(float parameter);

        friend CAN;
    };

    class RxMessage : public GenericMessage {
        BoxId deviceID;
        void SetDeviceID(BoxId _deviceID) { deviceID = _deviceID; }

    public:
        RxMessage() = default;

        [[nodiscard]] BoxId GetDeviceID() { return deviceID; }

        friend CAN;
    };

    /**
     * @brief Set speed of CAN BUS (Default: 250KHz)
     * @param _baudRate values from range [1, 1MHz]
     */

    void SetBaudRate(uint32_t _baudRate);

    /**
     * @brief Set mode (Default: NORMAL)
     * @param _mode NORMAL, LOOPBACK, SILENT or SILENT_LOOPBACK
     */
    void SetMode(Mode _mode);

    /**
     * @brief Return state handler for this object
     */
    CAN::State& GetState() { return state; }

    /**
     * @brief Tells if object was already Initialized (by calling Initialize
     * function)
     * @return true if was, false if not
     */
    [[nodiscard]] bool IsInitialized() const { return initialized; }
    /**
     * @brief Initialize function, needs to be called after configuration and
     * before using Send of Receive
     * @param ourBoxID ID with which our box will be sending frames
     * @param acceptedAddresses list of addresses from which we will be
     * receiving frames
     */
    void Initialize(BoxId ourBoxID,
                    const std::initializer_list<BoxId>& acceptedAddresses);

    /**
     * @brief Checks if we can send message
     */
    bool IsAnyTxMailboxFree();
    /**
     * @brief Send raw CAN message
     *
     * @param id CAN ext_id
     * @param data 8-byte long CAN data
     */
    void Send(const uint32_t& id, uint8_t (&data)[8]);
    /**
     * @brief Sending message
     * @param message We need to provide object of that message
     */
    void Send(TxMessage& message);
    /**
     * @brief Sending message
     * @param message We need to provide object of that message
     */
    void Send(TxMessage&& message);
    /**
     * @brief Sending message
     * @param id parameter id
     * @param parameter value we want to send
     */
    void Send(ParameterId id, int32_t parameter);
    /**
     * @brief Sending message
     * @param id parameter id
     * @param parameter value we want to send
     */
    void Send(ParameterId id, float parameter);

    /**
     * @brief Gets message from queue of received messages
     * @return if there was no message it returns std::nullopt
     */
    std::optional<RxMessage> GetMessage();

private:
    /**
     * @brief Interrupts function calls this to add incoming message to queue
     * @param fifoId id to message queue
     */
    static void saveMessageToQueue(uint32_t fifoId);

    friend void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan);
    friend void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef* hcan);
    friend Hardware;
};

#endif // F1XX_PROJECT_TEMPLATE_CAN_HPP
