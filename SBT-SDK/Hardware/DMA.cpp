//
// Created by hubert25632 on 27.11.2021.
//

#include "DMA.hpp"
#include "Hardware.hpp"

#define DMA_ERROR(comment)                                                     \
    softfault(__FILE__, __LINE__, std::string("DMA: ") + std::string(comment))

#define DMA_ERROR_CTRL_NI DMA_ERROR("Controller not implemented")

DMA::DMA(DMA_TypeDef* const dma) : dma(dma) {}

DMA_Channel_TypeDef* DMA::GetChannelInstance(const Channel channel)
{
    return reinterpret_cast<DMA_Channel_TypeDef*>(
        reinterpret_cast<uint32_t>(dma) + 0x8 +
        20 * (static_cast<uint32_t>(channel) - 1));
}

IRQn_Type DMA::GetChannelIRQ(const Channel channel)
{
    int irq = 0;
    if(dma == DMA1)
        irq = static_cast<int>(DMA1_Channel1_IRQn);
    else
        DMA_ERROR_CTRL_NI;
    return static_cast<IRQn_Type>(irq + (unsigned)(channel)-1);
}

DMA_HandleTypeDef* DMA::GetChannelHandleNoError(const Channel channel)
{
    DMA_Channel_TypeDef* instance = GetChannelInstance(channel);
    for(auto i : channels)
        if(i->Instance == instance)
            return i;
    return nullptr;
}

void DMA::InitController()
{
    // Enable the DMA clock
    if(dma == DMA1)
        __HAL_RCC_DMA1_CLK_ENABLE();
    else
        DMA_ERROR_CTRL_NI;
}

void DMA::CreateChannel(const Channel channel)
{
    DMA_HandleTypeDef* handle = GetChannelHandleNoError(channel);
    if(handle != nullptr)
        DMA_ERROR("Channel already exists");

    handle = new DMA_HandleTypeDef;
    handle->Instance = GetChannelInstance(channel);

    // Set default values
    handle->Init.Direction = static_cast<uint32_t>(Direction::PeriphToMemory);
    handle->Init.PeriphInc = static_cast<uint32_t>(PeriphInc::Disable);
    handle->Init.MemInc = static_cast<uint32_t>(MemInc::Disable);
    handle->Init.PeriphDataAlignment =
        static_cast<uint32_t>(PeriphDataAlignment::Byte);
    handle->Init.MemDataAlignment =
        static_cast<uint32_t>(MemDataAlignment::Byte);
    handle->Init.Mode = static_cast<uint32_t>(Mode::Normal);
    handle->Init.Priority = static_cast<uint32_t>(Priority::Low);

    channels.push_front(handle);
}

void DMA::SetChannelDirection(const Channel channel, const Direction direction)
{
    GetChannelHandle(channel)->Init.Direction =
        static_cast<uint32_t>(direction);
}

void DMA::SetChannelPeriphInc(const Channel channel, const PeriphInc periphInc)
{
    GetChannelHandle(channel)->Init.PeriphInc =
        static_cast<uint32_t>(periphInc);
}

void DMA::SetChannelMemInc(const Channel channel, const MemInc memInc)
{
    GetChannelHandle(channel)->Init.MemInc = static_cast<uint32_t>(memInc);
}

void DMA::SetChannelPeriphDataAlignment(
    const Channel channel, const PeriphDataAlignment periphDataAlignment)
{
    GetChannelHandle(channel)->Init.PeriphDataAlignment =
        static_cast<uint32_t>(periphDataAlignment);
}

void DMA::SetChannelMemDataAlignment(const Channel channel,
                                     const MemDataAlignment memDataAlignment)
{
    GetChannelHandle(channel)->Init.MemDataAlignment =
        static_cast<uint32_t>(memDataAlignment);
}

void DMA::SetChannelMode(const Channel channel, const Mode mode)
{
    GetChannelHandle(channel)->Init.Mode = static_cast<uint32_t>(mode);
}

void DMA::SetChannelPriority(const Channel channel, const Priority priority)
{
    GetChannelHandle(channel)->Init.Priority = static_cast<uint32_t>(priority);
}

DMA_HandleTypeDef* DMA::InitChannel(const Channel channel)
{
    DMA_HandleTypeDef* handle = GetChannelHandle(channel);
    if(HAL_DMA_Init(handle) != HAL_OK)
        DMA_ERROR("HAL_DMA_Init failed");
    IRQn_Type irq = GetChannelIRQ(channel);
    HAL_NVIC_SetPriority(irq, 8, 0);
    HAL_NVIC_EnableIRQ(irq);
    return handle;
}

DMA_HandleTypeDef* DMA::GetChannelHandle(const Channel channel)
{
    DMA_HandleTypeDef* handle = GetChannelHandleNoError(channel);
    if(handle == nullptr)
        DMA_ERROR("Channel does not exist");
    return handle;
}

// DMA IRQ handlers

void DMA1_Channel1_IRQHandler()
{
    HAL_DMA_IRQHandler(Hardware::dma1.GetChannelHandle(DMA::Channel::Channel1));
}
void DMA1_Channel2_IRQHandler()
{
    HAL_DMA_IRQHandler(Hardware::dma1.GetChannelHandle(DMA::Channel::Channel2));
}
void DMA1_Channel3_IRQHandler()
{
    HAL_DMA_IRQHandler(Hardware::dma1.GetChannelHandle(DMA::Channel::Channel3));
}
void DMA1_Channel4_IRQHandler()
{
    HAL_DMA_IRQHandler(Hardware::dma1.GetChannelHandle(DMA::Channel::Channel4));
}
void DMA1_Channel5_IRQHandler()
{
    HAL_DMA_IRQHandler(Hardware::dma1.GetChannelHandle(DMA::Channel::Channel5));
}
void DMA1_Channel6_IRQHandler()
{
    HAL_DMA_IRQHandler(Hardware::dma1.GetChannelHandle(DMA::Channel::Channel6));
}
void DMA1_Channel7_IRQHandler()
{
    HAL_DMA_IRQHandler(Hardware::dma1.GetChannelHandle(DMA::Channel::Channel7));
}