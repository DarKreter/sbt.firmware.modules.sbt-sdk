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
    bool initialized;
    bool printfEnabled;
    
    char* buffer;   //buffer for Print function
    
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
    //TODO: changes this to constructor
    void configureStaticVariables(USART_TypeDef* usart);
    //UART(USART_TypeDef* usart);
    UART();// = delete;
    
    
    void SendRCC(uint8_t* data, size_t numOfBytes);
    void SendIT(uint8_t* data, size_t numOfBytes);
    void SendDMA(uint8_t* data, size_t numOfBytes);
    
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
    [[nodiscard]] bool IsInitialized() const {return initialized;}
    
    void SetPrintfBufferSize(uint16_t bf);
    void DisablePrintf();
    
    void ChangeModeToBlocking(uint32_t tmt = 500);
    void ChangeModeToInterrupts();
    
    void SetTransmissionMode(TransmissionMode tm);
    void SetWordLength(WordLength wl);
    void SetParity(Parity p);
    void SetStopBits(StopBits sb);
    void SetBaudRate(uint32_t br);
    
    State& GetState() {return state;}
    
    void Initialize();
    
    void Send(uint8_t *data, size_t numOfBytes);
    void printf(const char *fmt, ...);
    
    void Receive(uint8_t *data, size_t numOfBytes);
    
    [[nodiscard]] bool IsTxComplete() const;
    [[nodiscard]] bool IsRxComplete() const;
    
    void AbortTx();
    void AbortRx();
    
    friend Hardware;
};


#endif //F1XX_PROJECT_TEMPLATE_UART_HPP
