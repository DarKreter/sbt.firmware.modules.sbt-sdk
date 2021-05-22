//
// Created by darkr on 18.05.2021.
//

#ifndef F1XX_PROJECT_TEMPLATE_SPI_HPP
#define F1XX_PROJECT_TEMPLATE_SPI_HPP

#include <stm32f1xx_hal.h>
#include <FreeRTOS.h>
#include <event_groups.h>

struct Hardware;

class SPI_t
{
    bool initialized;
    bool misoEnabled;
    bool mosiEnabled;
    
    /*-------HANDLERS---------*/
    struct State
    {
        EventGroupHandle_t txRxState;
        SPI_HandleTypeDef handle;
    };
    State state;
    
    /*-------ENUMS---------*/
    enum class Instance
    {
        NONE,
        SPI_1,
        SPI_2
    };
    
    enum class OperatingMode
    {
        BLOCKING,
        INTERRUPTS,
        DMA
    };
    
public:

    enum class Prescaler
    {
        PRESCALER_2   = SPI_BAUDRATEPRESCALER_2,
        PRESCALER_4   = SPI_BAUDRATEPRESCALER_4,
        PRESCALER_8   = SPI_BAUDRATEPRESCALER_8,
        PRESCALER_16  = SPI_BAUDRATEPRESCALER_16,
        PRESCALER_32  = SPI_BAUDRATEPRESCALER_32,
        PRESCALER_64  = SPI_BAUDRATEPRESCALER_64,
        PRESCALER_128 = SPI_BAUDRATEPRESCALER_128,
        PRESCALER_256 = SPI_BAUDRATEPRESCALER_256
    };
    
    enum class DataSize
    {
        _8BIT = SPI_DATASIZE_8BIT,
        _9BIT = SPI_DATASIZE_16BIT
    };
    
    enum class FirstBit
    {
        MSB = SPI_FIRSTBIT_MSB,
        LSB = SPI_FIRSTBIT_LSB
    };

    enum class ClockPolarity
    {
        HIGH = SPI_POLARITY_HIGH,
        LOW  = SPI_POLARITY_LOW
    };
    
    enum class ClockPhase
    {
        _1EDGE = SPI_PHASE_1EDGE,
        _2EDGE = SPI_PHASE_2EDGE
    };
    
    enum class TransmissionMode
    {
        FULL_DUPLEX,
        HALF_DUPLEX,
        RECEIVE_ONLY,
        TRANSMIT_ONLY
    };
    
    enum class DeviceType
    {
        MASTER = SPI_MODE_MASTER,
        SLAVE  = SPI_MODE_SLAVE
    };
    
private:
    //TODO: changes this to constructor
    void ConfigureStaticVariables(SPI_TypeDef* spii);
    //UART(USART_TypeDef* usart);
    SPI_t() = default; // = delete;
    
    void CalculateMisoMosi();
    
    void SendRCC(uint8_t* data, size_t numOfBytes);
    void SendIT(uint8_t* data, size_t numOfBytes);
    void SendDMA(uint8_t* data, size_t numOfBytes);
    
    void ReceiveRCC(uint8_t* data, size_t numOfBytes);
    void ReceiveIT(uint8_t* data, size_t numOfBytes);
    void ReceiveDMA(uint8_t* data, size_t numOfBytes);
    
    
    Instance instance;
    OperatingMode mode;
    Prescaler prescaler;
    DataSize dataSize;
    FirstBit firstBit;
    ClockPolarity clockPolarity;
    ClockPhase clockPhase;
    TransmissionMode transmissionMode;
    uint32_t direction;
    
    DeviceType deviceType;
    uint32_t timeout;
public:
    [[nodiscard]] bool IsInitialized() const {return initialized;}
    
    void ChangeModeToBlocking(uint32_t tmt = 500);
    void ChangeModeToInterrupts();
    
    
    void SetPrescaler(Prescaler pr);
    void SetFirstBit(FirstBit fb);
    void SetDataSize(DataSize ds);
    void SetClockPolarity(ClockPolarity cp);
    void SetClockPhase(ClockPhase cp);
    void SetTransmissionMode(TransmissionMode tm);
    void SetDeviceType(DeviceType dt);
    
    State& GetState() {return state;}
    
    void Initialize();
    
    void Send(uint8_t *data, size_t numOfBytes);
    
    void Receive(uint8_t *data, size_t numOfBytes);
    
    [[nodiscard]] bool IsTxComplete() const;
    [[nodiscard]] bool IsRxComplete() const;
    
    void Abort();
    
    friend Hardware;
    
};


#endif //F1XX_PROJECT_TEMPLATE_SPI_HPP
