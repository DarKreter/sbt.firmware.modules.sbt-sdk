//
// Created by hubert25632 on 28.11.2021.
//

#include "ADC.hpp"
#include "DMA.hpp"
#include "Hardware.hpp"

#define ADC_ERROR(comment)                                                     \
    softfault(__FILE__, __LINE__, std::string("ADC: ") + std::string(comment))

#define ADC_ERROR_CONV_NI ADC_ERROR("Converter not implemented")

ADC::ADCChannel::ADCChannel(const Channel channel)
    : channel(channel), config(new ADC_ChannelConfTypeDef)
{
}

ADC::ADC(ADC_TypeDef* const adc) : handle(new ADC_HandleTypeDef)
{
    handle->Instance = adc;

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

    DMA* dmac = nullptr;
    auto dma_channel = static_cast<DMA::Channel>(0);
    if(handle->Instance == ADC1) {
        __HAL_RCC_ADC1_CLK_ENABLE();
        dmac = &Hardware::dma1;
        dma_channel = DMA::Channel::Channel1;
    }
    else
        ADC_ERROR_CONV_NI;
    if(HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_ADC) > 14e6)
        ADC_ERROR("Clock frequency is too high (maximum is 14MHz)");

    // Set up DMA
    dmac->InitController();
    dmac->CreateChannel(dma_channel);
    dmac->SetChannelMemInc(dma_channel, DMA::MemInc::Enable);
    dmac->SetChannelPeriphDataAlignment(dma_channel,
                                        DMA::PeriphDataAlignment::HalfWord);
    dmac->SetChannelMemDataAlignment(dma_channel,
                                     DMA::MemDataAlignment::HalfWord);
    dmac->SetChannelMode(dma_channel, DMA::Mode::Circular);
    handle->DMA_Handle = dmac->InitChannel(dma_channel);
    handle->DMA_Handle->Parent = handle;

    inited = true;
}

bool ADC::IsConverterInited() const { return inited; }

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

void ADC::SetChannelSamplingTime(const Channel channel,
                                 const SamplingTime samplingTime)
{
    GetChannelObject(channel)->config->SamplingTime =
        static_cast<uint32_t>(samplingTime);
}

void ADC::InitChannels()
{
    if(!inited)
        ADC_ERROR("Converter is not initialized");
    if(handle->Init.NbrOfConversion == 0)
        ADC_ERROR("No channels exist");
    if(HAL_ADC_Stop_DMA(handle) != HAL_OK)
        ADC_ERROR("HAL_ADC_Stop_DMA failed");

    // (Re)allocate value storage
    delete values;
    values = new uint16_t[handle->Init.NbrOfConversion];

    // Configure channels
    {
        unsigned c = 0;
        for(auto i : channels) {
            i->config->Rank = c + 1;
            i->value = &values[c];
            if(HAL_ADC_ConfigChannel(handle, i->config) != HAL_OK)
                ADC_ERROR("HAL_ADC_ConfigChannel failed");
            c++;
        }
    }

    if(HAL_ADC_Init(handle) != HAL_OK)
        ADC_ERROR("HAL_ADC_Init failed");

    if(HAL_ADCEx_Calibration_Start(handle) != HAL_OK)
        ADC_ERROR("HAL_ADCEx_Calibration_Start failed");

    if(HAL_ADC_Start_DMA(handle, reinterpret_cast<uint32_t*>(values),
                         handle->Init.NbrOfConversion) != HAL_OK)
        ADC_ERROR("HAL_ADC_Start_DMA failed");
}

uint16_t ADC::GetChannelValue(const Channel channel)
{
    return *GetChannelObject(channel)->value;
}

ADC_HandleTypeDef* ADC::GetConverterHandle() { return handle; }

// ADC IRQ handlers

void ADC1_2_IRQHandler()
{
    HAL_ADC_IRQHandler(Hardware::adc1.GetConverterHandle());
}
