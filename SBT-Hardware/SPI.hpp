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
public:
    bool initialized;
    
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

public:


private:
    //TODO: changes this to constructor
    void configureStaticVariables(SPI_TypeDef* usart);
    //UART(USART_TypeDef* usart);
    SPI_t() = default; // = delete;
    
    
    void SendRCC(uint8_t* data, size_t numOfBytes);
    void SendIT(uint8_t* data, size_t numOfBytes);
    void SendDMA(uint8_t* data, size_t numOfBytes);
    
    void ReceiveRCC(uint8_t* data, size_t numOfBytes);
    void ReceiveIT(uint8_t* data, size_t numOfBytes);
    void ReceiveDMA(uint8_t* data, size_t numOfBytes);
    
    
    Instance instance;
    uint32_t timeout;
public:
    [[nodiscard]] bool IsInitialized() const {return initialized;}
    
    void ChangeModeToBlocking(uint32_t tmt = 500);
    void ChangeModeToInterrupts();
    
    
    State& GetState() {return state;}
    
    void Initialize();
    
    void Send(uint8_t *data, size_t numOfBytes);
    
    void Receive(uint8_t *data, size_t numOfBytes);
    
    [[nodiscard]] bool IsTxComplete() const;
    [[nodiscard]] bool IsRxComplete() const;
    
//    void AbortTx();
//    void AbortRx();
    
    friend Hardware;
    
};


#endif //F1XX_PROJECT_TEMPLATE_SPI_HPP
