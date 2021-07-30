//
// Created by darkr on 22.05.2021.
//

#include <CAN.hpp>
#include <Hardware.hpp>
#include <stdexcept>


void CAN::TxMessage::ConfigureMessage(uint32_t id)
{
    header.ExtId = id;
    header.IDE = CAN_ID_EXT;
    header.RTR = CAN_RTR_DATA;
    header.DLC = 8;//payload.size();
}

void CAN::TxMessage::SetParameterID(uint16_t id)
{
    integer.parameterID = id;
}

void CAN::TxMessage::SetData(int32_t parameter)
{
    integer.data = parameter;
}

void CAN::TxMessage::SetData(float parameter)
{
    floating.data = parameter;
}

CAN::TxMessage::TxMessage(uint16_t id, int32_t parameter): payload{}
{
    integer.parameterID = id;
    integer.data = parameter;
}

CAN::TxMessage::TxMessage(uint16_t id, float parameter): payload{}
{
    integer.parameterID = id;
    floating.data = parameter;
}

void CAN::Initialize(const uint32_t ourBoxID, const std::initializer_list <uint32_t>& acceptedAddresses)
{
    if(initialized)
        throw std::runtime_error("CAN already initialized!");
    
    deviceID = ourBoxID;
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_CAN1_CLK_ENABLE();
    Hardware::enableGpio(GPIOA, GPIO_PIN_11, Gpio::Mode::AlternateOD, Gpio::Pull::NoPull); // RX
    Hardware::enableGpio(GPIOA, GPIO_PIN_12, Gpio::Mode::AlternateOD, Gpio::Pull::NoPull); // TX
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 5, 5);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 5, 5);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
    HAL_NVIC_SetPriority(CEC_IRQn, 5, 5);
    HAL_NVIC_EnableIRQ(CEC_IRQn);
    
    CAN_HandleTypeDef& handle = state.handle;
    
    handle.Instance = CAN1;
    handle.Init.Mode = static_cast<uint32_t>(mode);
    handle.Init.Prescaler = 1;
    handle.Init.SyncJumpWidth = CAN_SJW_1TQ;
    handle.Init.TimeSeg1 = CAN_BS1_6TQ;
    handle.Init.TimeSeg2 = CAN_BS2_1TQ;
    handle.Init.TimeTriggeredMode = DISABLE;
    handle.Init.AutoBusOff = DISABLE;
    handle.Init.AutoWakeUp = DISABLE;
    handle.Init.AutoRetransmission = ENABLE;
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
        const uint32_t address = *(acceptedAddresses.begin() + idx);
        const uint32_t lowerPortion = (address & 0b1111111111111u) << 3u;
        const uint32_t upperPortion = (address & 0b111110000000000000u) >> 13u;
        filter.FilterMaskIdLow = (0x1FFFFFFFu & 0b1111111111111u) << 3u;;
        filter.FilterMaskIdHigh = (0x1FFFFFFFu & 0b111110000000000000u) >> 13u;;
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
        throw std::runtime_error("CAN not initialized!");
    
    return HAL_CAN_GetTxMailboxesFreeLevel(&state.handle) > 0;
}

std::optional<CAN::RxMessage> CAN::GetMessage() {
    if(!initialized)
        throw std::runtime_error("CAN not initialized!");
    
    RxMessage rxMessage{};
    if(xQueueReceive(GetState().queueHandle, &rxMessage, 0) == pdTRUE){
        return rxMessage;
    }
    return std::nullopt;
}

CAN::CAN()
{
    initialized = false;
    mode = Mode::NORMAL;
}

void CAN::SetMode(CAN::Mode _mode)
{
    if(initialized)
        throw std::runtime_error("CAN already initialized!"); // Too late
    
    mode = _mode;
}

void CAN::Send(CAN::TxMessage &message)
{
    if(!initialized)
        throw std::runtime_error("CAN not initialized!");
    
    message.ConfigureMessage(deviceID);
    [[maybe_unused]] uint32_t usedMailbox;
    if(IsAnyTxMailboxFree()) {
        HAL_CAN_AddTxMessage(&GetState().handle, &message.header, message.GetPayload(), &usedMailbox);
    }
}

void CAN::Send(TxMessage &&message)
{
    Send(message);
}

void CAN::Send(uint16_t id, int32_t parameter)
{
    Send(TxMessage(id, parameter));
}

void CAN::Send(uint16_t id, float parameter)
{
    Send(TxMessage(id, parameter));
}

// Handlers for CAN transmission
void CAN1_RX0_IRQHandler(){
    HAL_CAN_IRQHandler(&Hardware::can.GetState().handle);
}

void saveMessageToQueue(uint32_t fifoId){
    CAN::RxMessage message{};
    CAN_RxHeaderTypeDef header{};
    HAL_CAN_GetRxMessage( &Hardware::can.GetState().handle, fifoId, &header, message.GetPayload() );
    message.GetDeviceID() = header.IDE == CAN_ID_STD ? header.StdId : header.ExtId;
    
    xQueueSendToBackFromISR(Hardware::can.GetState().queueHandle, &message, NULL);
}

void HAL_CAN_RxFifo0MsgPendingCallback([[maybe_unused]] CAN_HandleTypeDef *hcan){
    saveMessageToQueue(CAN_RX_FIFO0);
}

void HAL_CAN_RxFifo1MsgPendingCallback([[maybe_unused]] CAN_HandleTypeDef *hcan){
    saveMessageToQueue(CAN_RX_FIFO1);
}