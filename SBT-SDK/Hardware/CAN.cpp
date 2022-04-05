//
// Created by darkr on 22.05.2021.
//
#include <optional>

#include "CAN.hpp"
#include "Error.hpp"
#include "GPIO.hpp"
#include "Hardware.hpp"

static void canError(const std::string& comment)
{
    softfault("CAN: " + comment);
}

static void canErrorNotInit() { canError("Not initialized"); }

static void canErrorNotStarted() { canError("Not started"); }

static void canErrorAlreadyInit() { canError("Already initialized"); }

static void canErrorAlreadyStarted() { canError("Already started"); }

static void canHALErrorGuard(HAL_StatusTypeDef halStatus)
{
    if(halStatus != HAL_OK)
        canError("HAL function failed with code " + std::to_string(halStatus));
}

#define CAN_ACTIVE_NOTIFICATION(hcan, callbackType)                            \
    if(callbackFunctions.count(callbackType))                                  \
        canHALErrorGuard(HAL_CAN_ActivateNotification(                         \
            hcan, static_cast<HAL_CAN_CallbackIDTypeDef>(callbackType)));

// Register a function created from the template as a callback. callbackType
// must be a constant (literal) expression and not a variable as it is passed as
// the template parameter and must be known at compile time.
#define CAN_REGISTER_CALLBACK(hcan, callbackType)                              \
    canHALErrorGuard(HAL_CAN_RegisterCallback(                                 \
        hcan,                                                                  \
        static_cast<HAL_CAN_CallbackIDTypeDef>(CallbackType::callbackType),    \
        CANUniversalCallback<CallbackType::callbackType>))

namespace SBT::Hardware {

// Unordered map containing callback functions for each callback type
static std::unordered_map<hCAN::CallbackType, std::function<void()>>
    callbackFunctions;

// Template from which HAL-compatible callback functions will be created, one
// for each callback type.
template <hCAN::CallbackType callbackType>
void CANUniversalCallback([[maybe_unused]] CAN_HandleTypeDef* hcan)
{
    // Check if any entry with given key exists. Necessary to avoid allocating
    // memory (which is not allowed in an ISR).
    if(callbackFunctions.count(callbackType))
        callbackFunctions.at(callbackType)();
}

hCAN::hCAN() noexcept
{
    // Set default values
    state = State::NOT_INITIALIZED;
    mode = Mode::NORMAL;
    baudRate = 250'000;
}

void hCAN::Initialize()
{
    if(state != State::NOT_INITIALIZED)
        canErrorAlreadyInit();

    // Calculate swj, bs1, bs2 and prescaler based on baudRate
    CalculateTQ();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_CAN1_CLK_ENABLE();
    GPIO::Enable(GPIOA, GPIO_PIN_11, GPIO::Mode::Input,
                 GPIO::Pull::NoPull); // RX
    GPIO::Enable(GPIOA, GPIO_PIN_12, GPIO::Mode::AlternatePP,
                 GPIO::Pull::NoPull); // TX
    HAL_NVIC_SetPriority(USB_HP_CAN1_TX_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
    HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);

    handle.Instance = CAN1;
    handle.Init.Mode = static_cast<uint32_t>(mode);
    handle.Init.Prescaler = prescaler;
    handle.Init.SyncJumpWidth = static_cast<uint32_t>(swj);
    handle.Init.TimeSeg1 = static_cast<uint32_t>(bs1);
    handle.Init.TimeSeg2 = static_cast<uint32_t>(bs2);
    handle.Init.TimeTriggeredMode = DISABLE;
    handle.Init.AutoBusOff = DISABLE;
    handle.Init.AutoWakeUp = DISABLE;
    handle.Init.AutoRetransmission = DISABLE;
    handle.Init.ReceiveFifoLocked = DISABLE;
    handle.Init.TransmitFifoPriority = DISABLE;

    CAN_REGISTER_CALLBACK(&handle, MspInit);
    CAN_REGISTER_CALLBACK(&handle, MspDeInit);

    canHALErrorGuard(HAL_CAN_Init(&handle));

    // Set up the remaining callbacks
    CAN_REGISTER_CALLBACK(&handle, TxMailbox0Complete);
    CAN_REGISTER_CALLBACK(&handle, TxMailbox1Complete);
    CAN_REGISTER_CALLBACK(&handle, TxMailbox2Complete);
    CAN_REGISTER_CALLBACK(&handle, TxMailbox0Abort);
    CAN_REGISTER_CALLBACK(&handle, TxMailbox1Abort);
    CAN_REGISTER_CALLBACK(&handle, TxMailbox2Abort);
    CAN_REGISTER_CALLBACK(&handle, RxFifo0MsgPending);
    CAN_REGISTER_CALLBACK(&handle, RxFifo1MsgPending);
    CAN_REGISTER_CALLBACK(&handle, RxFifo0Full);
    CAN_REGISTER_CALLBACK(&handle, RxFifo1Full);
    CAN_REGISTER_CALLBACK(&handle, Sleep);
    CAN_REGISTER_CALLBACK(&handle, WakeUpFromRxMsg);
    CAN_REGISTER_CALLBACK(&handle, Error);

    state = State::INITIALIZED;
}

void hCAN::Start()
{
    if(state == State::NOT_INITIALIZED)
        canErrorNotInit();
    else if(state == State::STARTED)
        canErrorAlreadyStarted();

    canHALErrorGuard(HAL_CAN_Start(&handle));

    CAN_ACTIVE_NOTIFICATION(&handle, CallbackType::MspInit)
    CAN_ACTIVE_NOTIFICATION(&handle, CallbackType::MspDeInit)
    CAN_ACTIVE_NOTIFICATION(&handle, CallbackType::TxMailbox0Complete)
    CAN_ACTIVE_NOTIFICATION(&handle, CallbackType::TxMailbox1Complete)
    CAN_ACTIVE_NOTIFICATION(&handle, CallbackType::TxMailbox2Complete)
    CAN_ACTIVE_NOTIFICATION(&handle, CallbackType::TxMailbox0Abort)
    CAN_ACTIVE_NOTIFICATION(&handle, CallbackType::TxMailbox1Abort)
    CAN_ACTIVE_NOTIFICATION(&handle, CallbackType::TxMailbox2Abort)
    CAN_ACTIVE_NOTIFICATION(&handle, CallbackType::RxFifo0MsgPending)
    CAN_ACTIVE_NOTIFICATION(&handle, CallbackType::RxFifo1MsgPending)
    CAN_ACTIVE_NOTIFICATION(&handle, CallbackType::RxFifo0Full)
    CAN_ACTIVE_NOTIFICATION(&handle, CallbackType::RxFifo1Full)
    CAN_ACTIVE_NOTIFICATION(&handle, CallbackType::Sleep)
    CAN_ACTIVE_NOTIFICATION(&handle, CallbackType::WakeUpFromRxMsg)
    CAN_ACTIVE_NOTIFICATION(&handle, CallbackType::Error)

    state = State::STARTED;
}

void hCAN::AddFilter_LIST(uint8_t filterBankIndex, uint32_t id1, uint32_t id2)
{
    // Need to be called after Initialized and before Start
    if(state == State::NOT_INITIALIZED)
        canErrorNotInit();
    else if(state == State::STARTED)
        canErrorAlreadyStarted();

    // Configure filters
    CAN_FilterTypeDef HALfilter;
    HALfilter.FilterScale = CAN_FILTERSCALE_32BIT;
    HALfilter.FilterActivation = CAN_FILTER_ENABLE;

    HALfilter.FilterMode = CAN_FILTERMODE_IDLIST;

    HALfilter.FilterIdHigh = ((id1 << 3) >> 16) & 0xffff;
    HALfilter.FilterIdLow = ((id1 << 3) & 0xffff) | CAN_ID_EXT;
    HALfilter.FilterMaskIdHigh = ((id2 << 3) >> 16) & 0xffff;
    HALfilter.FilterMaskIdLow = ((id2 << 3) & 0xffff) | CAN_ID_EXT;

    HALfilter.FilterBank = filterBankIndex;
    HALfilter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    HAL_CAN_ConfigFilter(&handle, &HALfilter);
}

void hCAN::AddFilter_MASK(uint8_t filterBankIndex, uint32_t id, uint32_t mask)
{
    // Need to be called after Initialized and before Start
    if(state == State::NOT_INITIALIZED)
        canErrorNotInit();
    else if(state == State::STARTED)
        canErrorAlreadyStarted();

    // Configure filters
    CAN_FilterTypeDef HALfilter;
    HALfilter.FilterScale = CAN_FILTERSCALE_32BIT;
    HALfilter.FilterActivation = CAN_FILTER_ENABLE;

    HALfilter.FilterMode = CAN_FILTERMODE_IDMASK;

    HALfilter.FilterIdHigh = id >> 13 & 0xFFFF;
    HALfilter.FilterIdLow = id << 3 & 0xFFF8;
    HALfilter.FilterMaskIdHigh = mask >> 13 & 0xFFFF;
    HALfilter.FilterMaskIdLow = mask << 3 & 0xFFF8;

    HALfilter.FilterBank = filterBankIndex;
    HALfilter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    HAL_CAN_ConfigFilter(&handle, &HALfilter);
}

void hCAN::GetRxMessage(uint32_t fifoId, uint32_t* extID, uint8_t* payload,
                        uint8_t* filterBankIdx)
{
    CAN_RxHeaderTypeDef header;
    HAL_CAN_GetRxMessage(&handle, fifoId, &header, payload);

    (*extID) = header.IDE == CAN_ID_STD ? header.StdId : header.ExtId;

    // Get info from which filter comes that message
    (*filterBankIdx) = header.FilterMatchIndex;
}

bool hCAN::IsAnyTxMailboxFree()
{
    if(state != State::STARTED)
        canErrorNotStarted();

    return HAL_CAN_GetTxMailboxesFreeLevel(&handle) > 0;
}

void hCAN::Send(const uint32_t& id, uint8_t(data)[8])
{
    if(state != State::STARTED)
        canErrorNotStarted();

    [[maybe_unused]] uint32_t usedMailbox;

    CAN_TxHeaderTypeDef header;
    header.ExtId = id;
    header.IDE = CAN_ID_EXT;
    header.RTR = CAN_RTR_DATA;
    header.DLC = 8;

    canHALErrorGuard(
        HAL_CAN_AddTxMessage(&handle, &header, data, &usedMailbox));
}

void hCAN::RegisterCallback(CallbackType callbackType,
                            std::function<void()> callbackFunction)
{
    callbackFunctions[callbackType] = std::move(callbackFunction);
}

void hCAN::CalculateTQ()
{
    // Struct that represents one setup of CAN properties
    struct TQ {
        uint8_t divider;
        hCAN::SWJ _swj;
        hCAN::BS1 _bs1;
        hCAN::BS2 _bs2;

        constexpr TQ(uint8_t a, hCAN::SWJ b, hCAN::BS1 c, hCAN::BS2 d)
            : divider{a}, _swj{b}, _bs1{c}, _bs2{d}
        {
        }
    };

    // Create all possibilities with sampling point around 75%
    constexpr std::array TQ_possibilities = {
        TQ(8, SWJ::_1TQ, BS1::_6TQ, BS2::_1TQ),
        TQ(9, SWJ::_1TQ, BS1::_7TQ, BS2::_1TQ),
        TQ(10, SWJ::_1TQ, BS1::_8TQ, BS2::_1TQ),
        TQ(11, SWJ::_1TQ, BS1::_9TQ, BS2::_1TQ),
        TQ(12, SWJ::_1TQ, BS1::_9TQ, BS2::_2TQ),
        TQ(13, SWJ::_1TQ, BS1::_10TQ, BS2::_2TQ),
        TQ(14, SWJ::_1TQ, BS1::_11TQ, BS2::_2TQ),
        TQ(15, SWJ::_1TQ, BS1::_12TQ, BS2::_2TQ),
        TQ(16, SWJ::_1TQ, BS1::_13TQ, BS2::_2TQ),
        TQ(17, SWJ::_1TQ, BS1::_14TQ, BS2::_2TQ),
        TQ(18, SWJ::_1TQ, BS1::_15TQ, BS2::_2TQ),
        TQ(19, SWJ::_1TQ, BS1::_16TQ, BS2::_2TQ),
        TQ(20, SWJ::_2TQ, BS1::_16TQ, BS2::_2TQ),
        TQ(21, SWJ::_2TQ, BS1::_16TQ, BS2::_3TQ),
        TQ(22, SWJ::_3TQ, BS1::_16TQ, BS2::_3TQ),
        TQ(23, SWJ::_4TQ, BS1::_16TQ, BS2::_3TQ),
        TQ(24, SWJ::_4TQ, BS1::_16TQ, BS2::_4TQ),
        TQ(25, SWJ::_4TQ, BS1::_16TQ, BS2::_5TQ)};

    // Start with first possibility
    TQ bestTQ(8, SWJ::_1TQ, BS1::_6TQ, BS2::_1TQ);
    uint32_t lowestAbsError = baudRate;
    uint32_t _bestPrescaler = 0;

    // lambda expression which calculates best prescaler basing on clock speed
    // and TimeQuanta divider
    static const auto calculatePrescaler =
        [AHB_Freq = Hardware::GetAPB1_Freq(),
         _baudRate = baudRate](uint8_t divider) -> std::optional<uint32_t> {
        uint32_t _prescaler = AHB_Freq / (divider * _baudRate);
        if(_prescaler > 1024)
            return std::nullopt;

        return _prescaler;
    };

    // Calculate BaudRate basing on TimeQuanta divider and prescaler
    static const auto calculateBaudRate =
        [AHB_Freq = Hardware::GetAPB1_Freq()](uint8_t divider,
                                              uint32_t _prescaler) -> uint32_t {
        return AHB_Freq / (divider * _prescaler);
    };

    // iterate by all possibilities and calculate prescaler for each one.
    // One with the lowest absolute error wins.
    for(auto TQ_possibility : TQ_possibilities) {
        if(const auto _prsclr = calculatePrescaler(TQ_possibility.divider)) {
            const auto actualAbsError =
                static_cast<uint32_t>(std::abs(static_cast<int32_t>(
                    calculateBaudRate(TQ_possibility.divider, _prsclr.value()) -
                    baudRate)));
            if(actualAbsError < lowestAbsError) {
                bestTQ = TQ_possibility;
                lowestAbsError = actualAbsError;
                _bestPrescaler = _prsclr.value();
            }
        }
        // If error is equals to zero, stop searching for better option (we
        // already got the best one)
        if(!lowestAbsError)
            break;
    }

    // Set properties
    swj = bestTQ._swj;
    bs1 = bestTQ._bs1;
    bs2 = bestTQ._bs2;
    prescaler = _bestPrescaler;
}

void hCAN::SetBaudRate([[maybe_unused]] uint32_t _baudRate)
{
    if(state != State::NOT_INITIALIZED)
        canErrorAlreadyInit(); // Too late
    else if(_baudRate > 1'000'000)
        canError("CAN BUS speed must be below 1MHz!");
    else if(_baudRate > Hardware::GetAPB1_Freq() / 8)
        canError("Too high baud rate for this clock speed!");

    baudRate = _baudRate;
}

void hCAN::SetMode(hCAN::Mode _mode)
{
    if(state != State::NOT_INITIALIZED)
        canErrorAlreadyInit(); // Too late

    mode = _mode;
}

hCAN can;
} // namespace SBT::Hardware

// Handlers for CAN transmission

using namespace SBT::Hardware;

void CAN1_RX0_IRQHandler() { HAL_CAN_IRQHandler(&can.GetHandle()); }
void CAN1_TX_IRQHandler() { HAL_CAN_IRQHandler(&can.GetHandle()); }
void CAN1_RX1_IRQHandler() { HAL_CAN_IRQHandler(&can.GetHandle()); }
void CAN1_SCE_IRQHandler() { HAL_CAN_IRQHandler(&can.GetHandle()); }