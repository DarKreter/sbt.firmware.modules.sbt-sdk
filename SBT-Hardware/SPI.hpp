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
    Instance instance;
    OperatingMode mode;
    Prescaler prescaler;
    DataSize dataSize;
    FirstBit firstBit;
    ClockPolarity clockPolarity;
    ClockPhase clockPhase;
    TransmissionMode transmissionMode;
    uint32_t direction;
    uint32_t baudRate;
    
    /**
     * @brief Set prescaler to value which gives us certain speed
     *        based on clock speed
     */
    void CalculateSpeed();
    
    DeviceType deviceType;
    uint32_t timeout;
    
    explicit SPI_t(SPI_TypeDef* spii);
    SPI_t()  = delete;
    
    /**
     * @brief Set mosiEnabled, misoEnabled and direction based on deviceType and transmissionMode
     */
    void CalculateMisoMosi();
    
    void SendRCC(uint8_t* data, size_t numOfBytes);
    void SendIT(uint8_t* data, size_t numOfBytes);
    void SendDMA(uint8_t* data, size_t numOfBytes);
    
    void ReceiveRCC(uint8_t* data, size_t numOfBytes);
    void ReceiveIT(uint8_t* data, size_t numOfBytes);
    void ReceiveDMA(uint8_t* data, size_t numOfBytes);
    
public:
    
    /**
     * @brief Changes operating mode of SPI to blocking (Default: Interrupts)
     * @param Timeout timeout
     */
    void ChangeModeToBlocking(uint32_t _timeout = 500);
    /**
     * @brief Changes operating mode of SPI to interrupts (Default: Interrupts)
     */
    void ChangeModeToInterrupts();
    
    /**
     * @brief Set prescaler (Default: PRESCALER_8)
     * @param _prescaler PRESCALER_2, PRESCALER_4, ..., PRESCALER_256
     */
    void SetPrescaler(Prescaler _prescaler);
    /**
     * @brief Set prescaler (Default: MSB)
     * @param _firstBit MSB or LSB
     */
    void SetFirstBit(FirstBit _firstBit);
    /**
     * @brief Set data size (Default: _8BIT)
     * @param _dataSize _8BIT or _9BIT
     */
    void SetDataSize(DataSize _dataSize);
    /**
     * @brief Set clock polarity (Default: HIGH)
     * @param _clockPolarity HIGH or LOW
     */
    void SetClockPolarity(ClockPolarity _clockPolarity);
    /**
     * @brief Set clock phase (Default: _2EDGE)
     * @param _clockPhase _1EDGE or _2EDGE
     */
    void SetClockPhase(ClockPhase _clockPhase);
    /**
     * @brief Set transmission mode (Default: FULL_DUPLEX)
     * @param _transmissionMode FULL_DUPLEX, HALF_DUPLEX, RECEIVE_ONLY or TRANSMIT_ONLY
     */
    void SetTransmissionMode(TransmissionMode _transmissionMode);
    /**
     * @brief Set device type (Default: MASTER)
     * @param _deviceType MASTER or SLAVE
     */
    void SetDeviceType(DeviceType _deviceType);
    /**
     * @brief Set baudRate (Default: 1'000'000)
     * @param _baudRate (1 - 18'000'000)
     */
    void SetBaudRate(int32_t _baudRate);
    
    /**
     * @brief Return state handler for this object
     */
    State& GetState() {return state;}
    
    /**
     * @brief Tells if object was already Initialized (by calling Initialize function)
     * @return true if was, false if not
     */
    [[nodiscard]] bool IsInitialized() const {return initialized;}
    
    /**
     * @brief Initialize function, needs to be called after configuration and before using Send of Receive
     */
    void Initialize();
    
    /**
     * @brief Sending data. Way of sending is defined by your configuration before Initialize()
     * @param data pointer to data table
     * @param numOfBytes number of bytes to send
     */
    void Send(uint8_t *data, size_t numOfBytes);
    /**
     * @brief Receiving data. Way of receiving is defined by your configuration before Initialize()
     * @param data pointer to data table
     * @param numOfBytes number of bytes expected to receive
     */
    void Receive(uint8_t *data, size_t numOfBytes);
    
    /**
     * @brief Checks if Transmission is completed
     */
    [[nodiscard]] bool IsTxComplete() const;
    /**
     * @brief Checks if receiving is completed
     */
    [[nodiscard]] bool IsRxComplete() const;
    
    /**
     * @brief Abort communication
     */
    void Abort();
    
    friend Hardware;
    
};


#endif //F1XX_PROJECT_TEMPLATE_SPI_HPP
