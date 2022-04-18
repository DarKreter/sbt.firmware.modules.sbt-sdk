//
// Created by darkr on 22.05.2021.
//

#ifndef F1XX_PROJECT_TEMPLATE_CAN_HPP
#define F1XX_PROJECT_TEMPLATE_CAN_HPP

#include "stm32f1xx_hal.h"
#include <functional>

namespace SBT::Hardware {
class hCAN {

public:
    // See ST Reference manual at bxCAN chapter
    enum class Mode : uint32_t {
        NORMAL = CAN_MODE_NORMAL,
        LOOPBACK = CAN_MODE_LOOPBACK,
        SILENT = CAN_MODE_SILENT,
        SILENT_LOOPBACK = CAN_MODE_SILENT_LOOPBACK
    };

    enum class CallbackType {
        TxMailbox0Complete = HAL_CAN_TX_MAILBOX0_COMPLETE_CB_ID,
        TxMailbox1Complete = HAL_CAN_TX_MAILBOX1_COMPLETE_CB_ID,
        TxMailbox2Complete = HAL_CAN_TX_MAILBOX2_COMPLETE_CB_ID,
        TxMailbox0Abort = HAL_CAN_TX_MAILBOX0_ABORT_CB_ID,
        TxMailbox1Abort = HAL_CAN_TX_MAILBOX1_ABORT_CB_ID,
        TxMailbox2Abort = HAL_CAN_TX_MAILBOX2_ABORT_CB_ID,
        RxFifo0MsgPending = HAL_CAN_RX_FIFO0_MSG_PENDING_CB_ID,
        RxFifo1MsgPending = HAL_CAN_RX_FIFO1_MSG_PENDING_CB_ID,
        RxFifo0Full = HAL_CAN_RX_FIFO0_FULL_CB_ID,
        RxFifo1Full = HAL_CAN_RX_FIFO1_FULL_CB_ID,
        Sleep = HAL_CAN_SLEEP_CB_ID,
        WakeUpFromRxMsg = HAL_CAN_WAKEUP_FROM_RX_MSG_CB_ID,
        Error = HAL_CAN_ERROR_CB_ID,
        MspInit = HAL_CAN_MSPINIT_CB_ID,
        MspDeInit = HAL_CAN_MSPDEINIT_CB_ID
    };

private:
    // Types required for calculating baudrate
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

    /*
     * default state is NOT_INITIALIZED
     * after function Initialize it changes value to INITIALIZED
     * after Start, its STARTED
     */
    enum class State : uint8_t {
        NOT_INITIALIZED,
        INITIALIZED,
        STARTED
    } state;

    CAN_HandleTypeDef handle;

    uint16_t prescaler; // 1-1024
    Mode mode;
    uint32_t baudRate;

    /**
     * @brief Set swj, bs1, bs2 and prescaler to values which gives us certain
     * speed based on clock speed
     */
    void CalculateTQ();

public:
    hCAN() noexcept;

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
    CAN_HandleTypeDef& GetHandle() { return handle; }
    /**
     * @brief Getter for state status for this driver
     * @return state could be NOT_INITIALIZED (before calling Initialize()),
     * INITIALIZED(after Initialize() and before Start()), STARTED(after
     * Start())
     */
    [[nodiscard]] State GetState() const { return state; }
    /**
     * @brief Initialize function, needs to be called after configuration and
     * before using Start. Change state to INITIALIZED.
     */
    void Initialize();
    /**
     * @brief Add CAN filter. It adds one filter in LIST mode (filter 2 unique
     * IDs). can must be in INITIALIZED state.
     * @param filterBankIndex id of filter bank for which we want to make filter
     * @param id1 first ID to filter
     * @param id2 second ID to filter
     */
    void AddFilter_LIST(uint8_t filterBankIndex, uint32_t id1, uint32_t id2);
    /**
     * @brief Add CAN filter. It adds one filter in MASK mode. can must be in
     * INITIALIZED state.
     * @param filterBankIndex id of filter bank for which we want to make filter
     * @param id id of mask filter
     * @param mask mask of filter
     */
    void AddFilter_MASK(uint8_t filterBankIndex, uint32_t id, uint32_t mask);
    /**
     * @brief Starts CAN. Needs to be called after Initialize. After that Send
     * and GetRxMessage can be used. Change state to STARTED.
     */
    void Start();
    /**
     * @brief Stops CAN. Needs to be called after Start. After that Send
     * and GetRxMessage cannot be used. Change state to INITIALIZED.
     */
    void Stop();

    /**
     * @brief Checks if we can send message.
     * There are 3 TxMailboxes.
     */
    bool IsAnyTxMailboxFree();

    /**
     * @brief Send raw CAN message
     *
     * @param id CAN ext_id
     * @param data 8-byte long CAN data
     */

    void Send(const uint32_t& id, uint8_t(data)[8]);

    /**
     * @brief Get received message from HAL queue
     * @param fifoId Fifo number of the received message to be read.
     * @param extID pointer to CAN extended ID, which will be overwritten
     * @param payload pointer to payload, which will be overwritten
     * @param filterBankIdx pointer to filter bank ID, which will be overwritten
     */
    void GetRxMessage(uint32_t fifoId, uint32_t* extID, uint8_t* payload,
                      uint8_t* filterBankIdx);

    /**
     * @brief Register a custom callback
     * @param callbackType Event which triggers the callback
     * @param callbackFunction Void function taking no arguments. A regular
     * function pointer will be casted to std::function implicitly. For a
     * non-static class member function use the template version of
     * RegisterCallback.
     */
    void RegisterCallback(CallbackType callbackType,
                          std::function<void()> callbackFunction);

    /**
     * @brief Register a non-static class member function as a custom callback
     * @tparam T Class name
     * @param callbackType Event which triggers the callback
     * @param callbackObject A pointer to the object in context of which the
     * callbackFunction will be called
     * @param callbackFunction A pointer to the callback function called in the
     * context of the callbackObject
     * @example // When called from any function
     * @example RegisterCallback(CallbackType::RxFifo0MsgPending,
     * &myTaskTypeObject, &myTask::myCallback);
     * @example
     * @example // When called from a function being a member of the
     * callbackObject
     * @example RegisterCallback(CallbackType::RxFifo0MsgPending, this,
     * &myTask::myCallback);
     */
    // This template envelopes the call of the member callback function in a
    // lambda and passes it as an independent callback function.
    template <class T>
    void RegisterCallback(CallbackType callbackType, T* callbackObject,
                          void (T::*callbackFunction)())
    {
        RegisterCallback(callbackType, [callbackObject, callbackFunction]() {
            (callbackObject->*callbackFunction)();
        });
    }
};

extern hCAN can;
} // namespace SBT::Hardware

#endif // F1XX_PROJECT_TEMPLATE_CAN_HPP
