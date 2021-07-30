//
// Created by darkr on 15.05.2021.
//

#ifndef F1XX_PROJECT_TEMPLATE_UART_HPP
#define F1XX_PROJECT_TEMPLATE_UART_HPP

#include <FreeRTOS.h>
#include <event_groups.h>
#include <stm32f1xx_hal.h>
#include <array>

struct Hardware;

class UART
{
    /**
     * @brief Set to true after calling Initialize() function
     */
    bool initialized;
    /**
     * @brief Set to true if user uses printf() function
     * If printfEnabled is true then buffor should exist
     */
    bool printfEnabled;
    
    /**
     * @brief buffer for Print function
     */
    char* buffer;
    
    /*-------HANDLERS---------*/
    struct State
    {
        EventGroupHandle_t txRxState;
        UART_HandleTypeDef handle;
    };
    State state;
    
    /*-------ENUMS---------*/
    enum class Instance
    {
        NONE,
        UART_1,
        UART_2,
        UART_3
    };
    
public:
    enum class OperatingMode
    {
        BLOCKING,
        INTERRUPTS,
        DMA
    };
    
    enum class WordLength
    {
        _8BITS = UART_WORDLENGTH_8B,
        _9BITS = UART_WORDLENGTH_9B
    };
    
    enum class Parity
    {
        NONE = UART_PARITY_NONE,
        EVEN = UART_PARITY_EVEN,
        ODD  = UART_PARITY_ODD
    };
    
    enum class StopBits
    {
        STOP_BITS_1 = UART_STOPBITS_1,
        STOP_BITS_2 = UART_STOPBITS_2
    };
    
    enum class TransmissionMode
    {
        FULL_DUPLEX   = UART_MODE_TX_RX,
        RECEIVE_ONLY  = UART_MODE_RX,
        TRANSMIT_ONLY = UART_MODE_TX
    };
    
private:
    //setting default settings
    UART(USART_TypeDef* usart);
    //To avoid creating objects with different constructor than one above
    UART() = delete;
    
    //Send functions
    void SendRCC(uint8_t* data, size_t numOfBytes);
    void SendIT(uint8_t* data, size_t numOfBytes);
    void SendDMA(uint8_t* data, size_t numOfBytes);
    
    //Receive functions
    void ReceiveRCC(uint8_t* data, size_t numOfBytes);
    void ReceiveIT(uint8_t* data, size_t numOfBytes);
    void ReceiveDMA(uint8_t* data, size_t numOfBytes);
    
    
    Instance instance;
    OperatingMode mode;
    WordLength wordLength;
    Parity parity;
    StopBits stopBits;
    TransmissionMode transmissionMode;
    uint32_t baudRate;
    uint32_t timeout;
public:
    /**
     * @brief Tells if object was already Initialized (by calling Initialize function)
     * @return true if was, false if not
     */
    [[nodiscard]] bool IsInitialized() const {return initialized;}
    
    /**
     * @brief Set buffer(for using printf function) to sent size
     * Also use new to allocate memory for buffer
     * @param bufferSize size of buffer
     */
    void SetPrintfBufferSize(uint16_t bufferSize);
    /**
     * @brief delete the buffer table
     */
    void DisablePrintf();
    
    /**
     * @brief Changes operating mode of UART to blocking (Default: Interrupts)
     * @param Timeout timeout
     */
    void ChangeModeToBlocking(uint32_t Timeout = 500);
    /**
     * @brief Changes operating mode of UART to interrupts (Default: Interrupts)
     */
    void ChangeModeToInterrupts();
    
    /**
     * @brief Sets transmission mode (Default: FULL_DUPLEX)
     * @param _transmissionMode FULL_DUPLEX, RECEIVE_ONLY or TRANSMIT_ONLY
     */
    void SetTransmissionMode(TransmissionMode _transmissionMode);
    /**
     * @brief Sets word length (Default: UART_WORDLENGTH_8B)
     * @param _wordLength UART_WORDLENGTH_8B or UART_WORDLENGTH_9B
     */
    void SetWordLength(WordLength _wordLength);
    /**
     * @brief Set parity (Default: NONE)
     * @param _parity NONE, EVEN or ODD
     */
    void SetParity(Parity _parity);
    /**
     * @brief set stop bits (Default: STOP_BITS_1)
     * @param _stopBits STOP_BITS_1 or STOP_BITS_2
     */
    void SetStopBits(StopBits _stopBits);
    /**
     * @brief set baud rate (Default: 115200)
     * @param _baudRate
     */
    void SetBaudRate(uint32_t _baudRate);
    
    /**
     * @brief Return state handler for this object
     */
    State& GetState() {return state;}
    
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
     * @brief Sends string
     * @param fmt string to send
     * @param ... additional parameters
     */
    void printf(const char *fmt, ...);
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
     * @brief Abort Transmission
     */
    void AbortTx();
    
    /**
     * @brief Abort receiving
     */
    void AbortRx();
    
    friend Hardware;
};


#endif //F1XX_PROJECT_TEMPLATE_UART_HPP
