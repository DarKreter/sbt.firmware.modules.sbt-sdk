//
// Created by hubert25632 on 28.11.2021.
//

#include "ADC.hpp"
#include "Error.hpp"

#define ADC_ERROR(comment)                                                     \
    softfault(__FILE__, __LINE__, std::string("ADC: ") + std::string(comment))

#define ADC_ERROR_CONV_NI       ADC_ERROR("Converter is not implemented")
#define ADC_ERROR_CONV_NOT_INIT ADC_ERROR("Converter is not initialized")
#define ADC_HAL_ERROR_GUARD(function)                                          \
    {                                                                          \
        HAL_StatusTypeDef halStatus = function;                                \
        if(halStatus != HAL_OK)                                                \
            ADC_ERROR(std::string("HAL function failed with code ") +          \
                      std::to_string(halStatus));                              \
    }

namespace SBT::Hardware {
ADC::ADCChannel::ADCChannel(const Channel channel)
    : channel(channel), config(new ADC_ChannelConfTypeDef)
{
}

ADC::ADCChannel::~ADCChannel() { delete config; }

ADC::ADC(ADC_TypeDef* const adc) : handle(new ADC_HandleTypeDef)
{
    handle->Instance = adc;

    // Select DMA controller and channel
    if(handle->Instance == ADC1) {
        dmaController = &dma1;
        dmaChannel = DMA::Channel::Channel1;
    }
    else
        ADC_ERROR_CONV_NI;

    // Set defaults
    handle->Init.DataAlign = static_cast<uint32_t>(DataAlignment::Right);
    handle->Init.ScanConvMode = ADC_SCAN_ENABLE;
    handle->Init.ContinuousConvMode = ENABLE;
    handle->Init.NbrOfConversion = 0;
    handle->Init.DiscontinuousConvMode = DISABLE;
    handle->Init.ExternalTrigConv = ADC_SOFTWARE_START;
}

IRQn_Type ADC::GetConverterIRQ()
{
    if(handle->Instance == ADC1)
        return ADC1_2_IRQn;
    else
        ADC_ERROR_CONV_NI;
    // To silence error
    return static_cast<IRQn_Type>(0);
}

ADC::ADCChannel* ADC::GetChannelObjectNoError(const Channel channel)
{
    for(auto i : channels)
        if(i->channel == channel)
            return i;
    return nullptr;
}

ADC::ADCChannel* ADC::GetChannelObject(const Channel channel)
{
    ADCChannel* chn = GetChannelObjectNoError(channel);
    if(chn == nullptr)
        ADC_ERROR("Channel does not exist");
    return chn;
}

void ADC::SetConverterDataAlignment(const DataAlignment dataAlignment)
{
    handle->Init.DataAlign = static_cast<uint32_t>(dataAlignment);
}

void ADC::InitConverter()
{
    if(inited)
        ADC_ERROR("Converter is already initialized");

    if(handle->Instance == ADC1)
        __HAL_RCC_ADC1_CLK_ENABLE();
    else
        ADC_ERROR_CONV_NI;

    if(HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_ADC) > 14e6)
        ADC_ERROR("Clock frequency is too high (maximum is 14MHz)");

    // Set up DMA
    dmaController->InitController();
    dmaController->CreateChannel(dmaChannel);
    dmaController->SetChannelMemInc(dmaChannel, DMA::MemInc::Enable);
    dmaController->SetChannelPeriphDataAlignment(
        dmaChannel, DMA::PeriphDataAlignment::HalfWord);
    dmaController->SetChannelMemDataAlignment(dmaChannel,
                                              DMA::MemDataAlignment::HalfWord);
    dmaController->SetChannelMode(dmaChannel, DMA::Mode::Circular);
    handle->DMA_Handle = dmaController->InitChannel(dmaChannel);
    handle->DMA_Handle->Parent = handle;

    inited = true;
}

bool ADC::IsConverterInited() const { return inited; }

void ADC::DeInitConverter()
{
    if(!inited)
        ADC_ERROR_CONV_NOT_INIT;

    inited = false;

    // Deinitialize DMA
    dmaController->DeleteChannel(dmaChannel);

    // Do not deinitialize the controller as it may be in use by another device

    if(handle->Instance == ADC1)
        __HAL_RCC_ADC1_CLK_DISABLE();
    else
        ADC_ERROR_CONV_NI;
}

void ADC::CreateChannel(const Channel channel)
{
    ADCChannel* chn = GetChannelObjectNoError(channel);
    if(chn != nullptr)
        ADC_ERROR("Channel already exists");

    chn = new ADCChannel(channel);
    chn->config->Channel = static_cast<uint32_t>(channel);

    // Set default value
    chn->config->SamplingTime = static_cast<uint32_t>(SamplingTime::ST_239_5);

    channels.push_front(chn);

    if(++handle->Init.NbrOfConversion > 16)
        ADC_ERROR("A maximum of 16 channels is supported");
}

bool ADC::DoesChannelExist(Channel channel)
{
    return GetChannelObjectNoError(channel) != nullptr;
}

void ADC::DeleteChannel(Channel channel)
{
    ADCChannel* chn = GetChannelObject(channel);
    --handle->Init.NbrOfConversion;
    channels.remove(chn);
    delete chn;
}

void ADC::SetChannelSamplingTime(const Channel channel,
                                 const SamplingTime samplingTime)
{
    GetChannelObject(channel)->config->SamplingTime =
        static_cast<uint32_t>(samplingTime);
}

void ADC::InitChannels()
{
    if(!inited)
        ADC_ERROR_CONV_NOT_INIT;
    if(handle->Init.NbrOfConversion == 0)
        ADC_ERROR("No channels exist");
    ADC_HAL_ERROR_GUARD(HAL_ADC_Stop_DMA(handle))

    // (Re)allocate value storage
    delete values;
    values = new uint16_t[handle->Init.NbrOfConversion];

    // Configure channels
    {
        unsigned c = 0;
        for(auto i : channels) {
            i->config->Rank = c + 1;
            i->value = &values[c];
            ADC_HAL_ERROR_GUARD(HAL_ADC_ConfigChannel(handle, i->config))
            c++;
        }
    }

    ADC_HAL_ERROR_GUARD(HAL_ADC_Init(handle))
    ADC_HAL_ERROR_GUARD(HAL_ADCEx_Calibration_Start(handle))
    ADC_HAL_ERROR_GUARD(HAL_ADC_Start_DMA(handle,
                                          reinterpret_cast<uint32_t*>(values),
                                          handle->Init.NbrOfConversion))
}

uint16_t ADC::GetChannelValue(const Channel channel)
{
    return *GetChannelObject(channel)->value;
}

ADC_HandleTypeDef* ADC::GetConverterHandle() { return handle; }

ADC adc1(ADC1);
} // namespace SBT::Hardware

// ADC IRQ handlers

using namespace SBT::Hardware;

void ADC1_2_IRQHandler() { HAL_ADC_IRQHandler(adc1.GetConverterHandle()); }
