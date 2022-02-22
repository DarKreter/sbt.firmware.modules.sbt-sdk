//
// Created by hubert25632 on 28.11.2021.
//

#ifndef F1XX_PROJECT_TEMPLATE_ADC_HPP
#define F1XX_PROJECT_TEMPLATE_ADC_HPP

#include "DMA.hpp"
#include <forward_list>
#include <stm32f1xx_hal.h>

// How to use this driver:
// 1. Set ADC parameters using SetConverterXYZ() (optional)
// 2. Initialize an ADC using InitConverter(). You can check if a converter is
// initialized using IsConverterInited()
// 3. Create channel(s) using CreateChannel()
// 4. Set channel(s) parameters using SetChannelXYZ()
// 5. (Re)initialize channel(s) using InitChannels()
// 6. Read the reading value from the channel using GetChannelValue()

namespace SBT::Hardware {
class ADC {
public:
    // Channels 10-15 are available only on STM32F103Rx and STM32F103Vx
    enum class Channel {
        Channel0 = ADC_CHANNEL_0,
        Channel1 = ADC_CHANNEL_1,
        Channel2 = ADC_CHANNEL_2,
        Channel3 = ADC_CHANNEL_3,
        Channel4 = ADC_CHANNEL_4,
        Channel5 = ADC_CHANNEL_5,
        Channel6 = ADC_CHANNEL_6,
        Channel7 = ADC_CHANNEL_7,
        Channel8 = ADC_CHANNEL_8,
        Channel9 = ADC_CHANNEL_9,
        Channel10 = ADC_CHANNEL_10,
        Channel11 = ADC_CHANNEL_11,
        Channel12 = ADC_CHANNEL_12,
        Channel13 = ADC_CHANNEL_13,
        Channel14 = ADC_CHANNEL_14,
        Channel15 = ADC_CHANNEL_15,
        Channel16 = ADC_CHANNEL_16,
        Channel17 = ADC_CHANNEL_17
    };

    // ADC data alignment
    // Default is right
    enum class DataAlignment {
        Right = ADC_DATAALIGN_RIGHT,
        Left = ADC_DATAALIGN_LEFT
    };

    // Channel sampling time in ADC clock cycles
    // Default is 239.5
    enum class SamplingTime {
        ST_1_5 = ADC_SAMPLETIME_1CYCLE_5,
        ST_7_5 = ADC_SAMPLETIME_71CYCLES_5,
        ST_13_5 = ADC_SAMPLETIME_13CYCLES_5,
        ST_28_5 = ADC_SAMPLETIME_28CYCLES_5,
        ST_41_5 = ADC_SAMPLETIME_41CYCLES_5,
        ST_55_5 = ADC_SAMPLETIME_55CYCLES_5,
        ST_71_5 = ADC_SAMPLETIME_71CYCLES_5,
        ST_239_5 = ADC_SAMPLETIME_239CYCLES_5
    };

private:
    class ADCChannel {
        friend ADC;

    private:
        const Channel channel;
        ADC_ChannelConfTypeDef* const config;
        const uint16_t* value = nullptr;

        explicit ADCChannel(Channel);
        ~ADCChannel();

    public:
        ADCChannel() = delete;
        ADCChannel(ADCChannel&) = delete;
    };

    std::forward_list<ADCChannel*> channels;
    ADC_HandleTypeDef* const handle;
    uint16_t* values = nullptr;
    bool inited = false;

    DMA* dmaController;
    DMA::Channel dmaChannel;

    IRQn_Type GetConverterIRQ();
    ADCChannel* GetChannelObjectNoError(Channel);
    ADCChannel* GetChannelObject(Channel);

public:
    ADC() = delete;
    ADC(ADC&) = delete;

    explicit ADC(ADC_TypeDef*);

    // Set converter parameters
    // Must be called before InitConverter()

    void SetConverterDataAlignment(DataAlignment);

    /// Initialize an ADC
    /// Must be called after setting all parameters
    void InitConverter();

    /// Check whether a converter is initialized
    [[nodiscard]] bool IsConverterInited() const;

    /// Deinitialize an ADC
    void DeInitConverter();

    /// Create an ADC channel
    /// Must be called before setting any parameters
    void CreateChannel(Channel);

    /// Check whether an ADC channel exists
    bool DoesChannelExist(Channel);

    /// Delete an ADC channel
    void DeleteChannel(Channel);

    // Set channel parameters
    // Must be called between CreateChannel() and InitChannel()

    void SetChannelSamplingTime(Channel, SamplingTime);

    /// Initialize ADC channels
    /// Must be called after setting all parameters
    void InitChannels();

    /// Get reading from an ADC channel
    /// \return Reading value
    uint16_t GetChannelValue(Channel);

    /// Get an ADC handle
    /// \return ADC handle
    ADC_HandleTypeDef* GetConverterHandle();
};

extern ADC adc1;
} // namespace SBT::Hardware

#endif // F1XX_PROJECT_TEMPLATE_ADC_HPP
