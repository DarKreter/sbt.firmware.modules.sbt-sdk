//
// Created by darkr on 15.05.2021.
//

#ifndef F1XX_PROJECT_TEMPLATE_UART_HPP
#define F1XX_PROJECT_TEMPLATE_UART_HPP

#include<FreeRTOS.h>
#include <event_groups.h>
#include <stm32f1xx_hal.h>

#include<array>


struct Hardware;

class UART
{
    /*-------HANDLERS---------*/
    struct State
    {
        EventGroupHandle_t txRxState;
        UART_HandleTypeDef handle;
        
        static constexpr size_t txBit = 1 << 0;
        static constexpr size_t rxBit = 1 << 1;
    };
    
    State state;
    
    /*-------ENUMS---------*/
    enum class Instance
    {
        NONE,
        UART_1,
        UART_2
    };
    
    enum class Mode
    {
        BLOCKING,
        INTERRUPTS,
        DMA
    };
    
    Instance instance;
    Mode mode;
    
public:
    UART(USART_TypeDef* usart);
    
    void SendRCC(uint8_t* data, size_t numOfBytes);
    void SendIT(uint8_t* data, size_t numOfBytes);
    void SendDMA(uint8_t* data, size_t numOfBytes);
    
    void ReceiveRCC(uint8_t* data, size_t numOfBytes);
    void ReceiveIT(uint8_t* data, size_t numOfBytes);
    void ReceiveDMA(uint8_t* data, size_t numOfBytes);
    
public:
    uint32_t baudRate;
    
    UART() = delete;
    
    UART(uint32_t baudRate);
    
    State& getState() {return state;}
    
    void Initialize();
    
    void Send(uint8_t *data, size_t numOfBytes);
    
    void Receive(uint8_t *data, size_t numOfBytes);
    
    bool IsTxComplete();
    
    bool IsRxComplete();
    
    void AbortTx();
    
    void AbortRx();
    
    friend Hardware;
};


#endif //F1XX_PROJECT_TEMPLATE_UART_HPP
