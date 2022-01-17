//
// Created by darkr on 22.05.2021.
//

#include "GPIO.hpp"
#include <CAN.hpp>
#include <Hardware.hpp>
#include <stdexcept>
#include <tuple>

void CAN::GenericMessage::ConfigureMessage(BoxId id)
{
    header.ExtId = static_cast<uint32_t>(id);
    header.IDE = CAN_ID_EXT;
    header.RTR = CAN_RTR_DATA;
    header.DLC = 8; // payload.size();
}

void CAN::TxMessage::SetParameterID(ParameterId id)
{
    integer.parameterID = id;
}

void CAN::TxMessage::SetData(int32_t parameter) { integer.data = parameter; }

void CAN::TxMessage::SetData(float parameter) { floating.data = parameter; }

CAN::GenericMessage::GenericMessage(ParameterId id, int32_t parameter)
    : payload{}
{
    integer.parameterID = id;
    integer.data = parameter;
}

CAN::GenericMessage::GenericMessage(ParameterId id, float parameter) : payload{}
{
    floating.parameterID = id;
    floating.data = parameter;
}

void CAN::Initialize(BoxId ourBoxID,
                     const std::initializer_list<BoxId>& acceptedAddresses)
{
    if(initialized)
        softfault(__FILE__, __LINE__, "CAN already initialized!");

    CalculateTQ();

    deviceID = ourBoxID;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_CAN1_CLK_ENABLE();
    {
        using namespace SBT::Hardware;
        GPIO::Enable(GPIOA, GPIO_PIN_11, GPIO::Mode::Input,
                     GPIO::Pull::NoPull); // RX
        GPIO::Enable(GPIOA, GPIO_PIN_12, GPIO::Mode::AlternatePP,
                     GPIO::Pull::NoPull); // TX
    }
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 5, 5);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 5, 5);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
    HAL_NVIC_SetPriority(CEC_IRQn, 5, 5);
    HAL_NVIC_EnableIRQ(CEC_IRQn);

    CAN_HandleTypeDef& handle = state.handle;

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

    HAL_CAN_Init(&handle);

    // Configure filters

    CAN_FilterTypeDef filter;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    filter.FilterActivation = CAN_FILTER_ENABLE;

    for(uint32_t idx = 0; idx < acceptedAddresses.size(); ++idx) {
        // RM0008 p. 665
        const uint32_t address =
            static_cast<uint32_t>(*(acceptedAddresses.begin() + idx));
        const uint32_t lowerPortion = (address & 0b1111111111111u) << 3u;
        const uint32_t upperPortion = (address & 0b111110000000000000u) >> 13u;
        filter.FilterMaskIdLow = (0x1FFFFFFFu & 0b1111111111111u) << 3u;
        ;
        filter.FilterMaskIdHigh = (0x1FFFFFFFu & 0b111110000000000000u) >> 13u;
        ;
        filter.FilterIdLow = lowerPortion;
        filter.FilterIdHigh = upperPortion;
        filter.FilterBank = idx;
        filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
        HAL_CAN_ConfigFilter(&handle, &filter);
    }
    HAL_CAN_Start(&handle);

    // Create queue for messages
    state.queueHandle = xQueueCreate(20, sizeof(RxMessage));

    HAL_CAN_ActivateNotification(&handle, CAN_IT_RX_FIFO0_MSG_PENDING);
    HAL_CAN_ActivateNotification(&handle, CAN_IT_RX_FIFO1_MSG_PENDING);

    initialized = true;
}

bool CAN::IsAnyTxMailboxFree()
{
    if(!initialized)
        softfault(__FILE__, __LINE__, "CAN not initialized!");

    return HAL_CAN_GetTxMailboxesFreeLevel(&state.handle) > 0;
}

std::optional<CAN::RxMessage> CAN::GetMessage()
{
    if(!initialized)
        softfault(__FILE__, __LINE__, "CAN not initialized!");

    RxMessage rxMessage{};
    if(xQueueReceive(GetState().queueHandle, &rxMessage, 0) == pdTRUE) {
        return rxMessage;
    }
    return std::nullopt;
}

CAN::CAN()
{
    initialized = false;
    mode = Mode::NORMAL;
    baudRate = 250'000;
}

void CAN::CalculateTQ()
{
    struct TQ {
        uint8_t divider;
        CAN::SWJ _swj;
        CAN::BS1 _bs1;
        CAN::BS2 _bs2;

        constexpr TQ(uint8_t a, CAN::SWJ b, CAN::BS1 c, CAN::BS2 d)
            : divider{a}, _swj{b}, _bs1{c}, _bs2{d}
        {
            ;
        }
    };

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

    TQ bestTQ(8, SWJ::_1TQ, BS1::_6TQ, BS2::_1TQ);
    int32_t lowestAbsError = baudRate;
    uint32_t _bestPrescaler = 0;

    static const auto calculatePrescaler =
        [AHB_Freq = Hardware::GetAPB1_Freq(),
         _baudRate = baudRate](uint8_t divider) -> std::optional<uint32_t> {
        uint32_t _prescaler = AHB_Freq / (divider * _baudRate);
        if(_prescaler > 1024)
            return std::nullopt;

        return _prescaler;
    };

    static const auto calculateBaudRate =
        [AHB_Freq = Hardware::GetAPB1_Freq()](uint8_t divider,
                                              uint32_t _prescaler) -> int32_t {
        return AHB_Freq / (divider * _prescaler);
    };

    for(auto TQ_possibility : TQ_possibilities) {
        if(const auto _prsclr = calculatePrescaler(TQ_possibility.divider)) {
            const auto actualAbsError = std::abs(static_cast<int32_t>(
                calculateBaudRate(TQ_possibility.divider, _prsclr.value()) -
                baudRate));
            if(actualAbsError < lowestAbsError) {
                bestTQ = TQ_possibility;
                lowestAbsError = actualAbsError;
                _bestPrescaler = _prsclr.value();
            }
        }
        if(!lowestAbsError)
            break;
    }

    swj = bestTQ._swj;
    bs1 = bestTQ._bs1;
    bs2 = bestTQ._bs2;
    prescaler = _bestPrescaler;
}

void CAN::SetBaudRate([[maybe_unused]] uint32_t _baudRate)
{
    if(initialized)
        softfault(__FILE__, __LINE__, "CAN already initialized!"); // Too late
    else if(_baudRate > 1'000'000)
        softfault(__FILE__, __LINE__, "CAN BUS speed must be below 1MHz!");
    else if(_baudRate > Hardware::GetAPB1_Freq() / 8)
        softfault(__FILE__, __LINE__,
                  "Too high baud rate for this clock speed!");

    baudRate = _baudRate;
}

void CAN::SetMode(CAN::Mode _mode)
{
    if(initialized)
        softfault(__FILE__, __LINE__, "CAN already initialized!"); // Too late

    mode = _mode;
}

void CAN::Send(const uint32_t& id, uint8_t (&data)[8])
{
    if(!initialized)
        softfault(__FILE__, __LINE__, "CAN not initialized!");

    [[maybe_unused]] uint32_t usedMailbox;

    CAN_TxHeaderTypeDef header;
    header.ExtId = id;
    header.IDE = CAN_ID_EXT;
    header.RTR = CAN_RTR_DATA;
    header.DLC = 8;

    while(!IsAnyTxMailboxFree())
        ;

    HAL_CAN_AddTxMessage(&GetState().handle, &header, data, &usedMailbox);
}

void CAN::Send(CAN::TxMessage& message)
{
    if(!initialized)
        softfault(__FILE__, __LINE__, "CAN not initialized!");

    message.ConfigureMessage(deviceID);
    [[maybe_unused]] uint32_t usedMailbox;

    while(!IsAnyTxMailboxFree())
        ;

    HAL_CAN_AddTxMessage(&GetState().handle, &message.header,
                         message.GetPayload(), &usedMailbox);
}

void CAN::Send(TxMessage&& message) { Send(message); }

void CAN::Send(ParameterId id, int32_t parameter)
{
    Send(TxMessage(id, parameter));
}

void CAN::Send(ParameterId id, float parameter)
{
    Send(TxMessage(id, parameter));
}

void CAN::saveMessageToQueue(uint32_t fifoId)
{
    CAN::RxMessage message{};
    CAN_RxHeaderTypeDef header{};
    HAL_CAN_GetRxMessage(&Hardware::can.GetState().handle, fifoId, &header,
                         message.GetPayload());
    uint32_t boxid = header.IDE == CAN_ID_STD ? header.StdId : header.ExtId;
    message.SetDeviceID(static_cast<BoxId>(boxid));

    xQueueSendToBackFromISR(Hardware::can.GetState().queueHandle, &message,
                            NULL);
}

// Handlers for CAN transmission
void CAN1_RX0_IRQHandler()
{
    HAL_CAN_IRQHandler(&Hardware::can.GetState().handle);
}

void HAL_CAN_RxFifo0MsgPendingCallback([[maybe_unused]] CAN_HandleTypeDef* hcan)
{
    Hardware::can.saveMessageToQueue(CAN_RX_FIFO0);
}

void HAL_CAN_RxFifo1MsgPendingCallback([[maybe_unused]] CAN_HandleTypeDef* hcan)
{
    Hardware::can.saveMessageToQueue(CAN_RX_FIFO1);
}