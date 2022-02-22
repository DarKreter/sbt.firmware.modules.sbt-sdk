//
// Created by hubert25632 on 27.11.2021.
//

#ifndef F1XX_PROJECT_TEMPLATE_DMA_HPP
#define F1XX_PROJECT_TEMPLATE_DMA_HPP

#include <forward_list>
#include <stm32f1xx_hal.h>

// How to use this driver:
// 1. Initialize a DMA controller using InitController()
// 2. Create a DMA channel using CreateChannel()
// 3. Set channel parameters using SetChannelXYZ() (optional)
// 4. Initialize the channel using InitChannel(). You can either use the DMA
// channel handle returned by this function to initiate a DMA transfer directly
// or link the channel to a DMA-capable peripheral as follows:
//
// uart1.hdmatx = SBT::Hardware::dma1.InitChannel(DMA::Channel::Channel4);
// uart1.hdmatx->Parent = &uart1;
//
// Where uart1 is an UART_HandleTypeDef type structure representing UART1.

namespace SBT::Hardware {
class DMA {
public:
    enum class Channel {
        Channel1 = 1,
        Channel2,
        Channel3,
        Channel4,
        Channel5,
        Channel6,
        Channel7
    };

    // Channel configuration values. The first ones are default.
    enum class Direction {
        PeriphToMemory = DMA_PERIPH_TO_MEMORY,
        MemoryToPeriph = DMA_MEMORY_TO_PERIPH,
        MemoryToMemory = DMA_MEMORY_TO_MEMORY
    };
    enum class PeriphInc {
        Disable = DMA_PINC_DISABLE,
        Enable = DMA_PINC_ENABLE
    };
    enum class MemInc {
        Disable = DMA_MINC_DISABLE,
        Enable = DMA_MINC_ENABLE
    };
    enum class PeriphDataAlignment {
        Byte = DMA_PDATAALIGN_BYTE,
        HalfWord = DMA_PDATAALIGN_HALFWORD,
        Word = DMA_PDATAALIGN_WORD
    };
    enum class MemDataAlignment {
        Byte = DMA_MDATAALIGN_BYTE,
        HalfWord = DMA_MDATAALIGN_HALFWORD,
        Word = DMA_MDATAALIGN_WORD
    };
    enum class Mode {
        Normal = DMA_NORMAL,
        Circular = DMA_CIRCULAR
    };
    enum class Priority {
        Low = DMA_PRIORITY_LOW,
        Medium = DMA_PRIORITY_MEDIUM,
        High = DMA_PRIORITY_HIGH,
        VeryHigh = DMA_PRIORITY_VERY_HIGH
    };

private:
    // List of channel handles
    std::forward_list<DMA_HandleTypeDef*> channels;
    DMA_TypeDef* const dma;

    DMA_Channel_TypeDef* GetChannelInstance(Channel);
    IRQn_Type GetChannelIRQ(Channel);
    DMA_HandleTypeDef* GetChannelHandleNoError(Channel);

public:
    DMA() = delete;
    DMA(DMA&) = delete;

    explicit DMA(DMA_TypeDef*);

    /// Initialize a DMA controller
    void InitController();

    /// Deinitialize a DMA controller
    void DeInitController();

    /// Create a DMA channel
    /// Must be called before setting any parameters
    void CreateChannel(Channel);

    /// Check whether a DMA channel exists
    bool DoesChannelExist(Channel);

    /// Delete a DMA channel
    void DeleteChannel(Channel);

    // Set channel parameters
    // Must be called between CreateChannel() and InitChannel()

    void SetChannelDirection(Channel, Direction);
    void SetChannelPeriphInc(Channel, PeriphInc);
    void SetChannelMemInc(Channel, MemInc);
    void SetChannelPeriphDataAlignment(Channel, PeriphDataAlignment);
    void SetChannelMemDataAlignment(Channel, MemDataAlignment);
    void SetChannelMode(Channel, Mode);
    void SetChannelPriority(Channel, Priority);

    /// Initialize a DMA channel
    /// Must be called after setting all parameters
    /// \return DMA channel handle
    DMA_HandleTypeDef* InitChannel(Channel);

    /// Deinitialize a DMA channel
    void DeInitChannel(Channel);

    /// Get a DMA channel handle
    /// \return DMA channel handle
    DMA_HandleTypeDef* GetChannelHandle(Channel);
};

extern DMA dma1;
} // namespace SBT::Hardware

#endif // F1XX_PROJECT_TEMPLATE_DMA_HPP
