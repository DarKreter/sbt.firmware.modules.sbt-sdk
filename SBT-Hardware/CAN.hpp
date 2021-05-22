//
// Created by darkr on 22.05.2021.
//

#ifndef F1XX_PROJECT_TEMPLATE_CAN_HPP
#define F1XX_PROJECT_TEMPLATE_CAN_HPP

#include <stm32f1xx_hal.h>
#include <FreeRTOS.h>
#include <event_groups.h>
#include <array>
#include <optional>
#include <queue.h>

struct Hardware;

class CAN
{
    struct State
    {
        CAN_HandleTypeDef handle;
        QueueHandle_t queueHandle;
    };
    
    //TODO: changes this to constructor
    void ConfigureStaticVariables();
    CAN() = default;// =delete;

public:
    enum class Mode: uint32_t
    {
        NORMAL = CAN_MODE_NORMAL,
        LOOPBACK = CAN_MODE_LOOPBACK,
        SILENT = CAN_MODE_SILENT,
        SILENT_LOOPBACK = CAN_MODE_SILENT_LOOPBACK
    };
    
private:
    bool initialized;
    State state;
    Mode mode;
    uint32_t deviceID;
    
public:
    
    class TxMessage
    {
        CAN_TxHeaderTypeDef header;
        
        union
        {
            uint8_t payload[8];

            struct
            {
                uint32_t    : 16,
                            parameterID : 16,
                            data : 32;
            }integer;
            
            struct
            {
                uint32_t    : 16,
                            parameterID : 16;
                float data;
            }floating;
            
        };
        
        void ConfigureMessage(uint32_t id);
    public:
    
        TxMessage() = default;
        TxMessage(uint16_t id, uint32_t parameter);
        TxMessage(uint16_t id, float parameter);
        
        void SetParameterID(uint16_t id);
        void SetData(uint32_t parameter);
        void SetData(float parameter);
        
        [[nodiscard]] uint16_t GetParameterID() const {return integer.parameterID;}
        [[nodiscard]] uint32_t GetDataInt() const {return integer.data;}
        [[nodiscard]] uint32_t GetDataFloat() const {return floating.data;}
        [[nodiscard]] uint8_t* GetPayload(){ return payload; }
        
        friend CAN;
    };
    
    
    class RxMessage: public TxMessage
    {
    private:
        //Hiding them
        using TxMessage::SetData;
        using TxMessage::SetParameterID;
        
        uint32_t deviceID;
        
    public:
        [[nodiscard]] uint32_t& GetDeviceID() {return deviceID;}
    };
    
    void SetMode(Mode md);
    
    CAN::State& GetState(){ return state;   }
    
    [[nodiscard]] bool IsInitialized() const {return initialized;}
    void Initialize(uint32_t ourID, const std::initializer_list <uint32_t> &acceptedAddresses);
    
    bool IsAnyTxMailboxFree();
    void Send(TxMessage &message);
    void Send(TxMessage &&message);
    
    std::optional<RxMessage> GetMessage();
    
    
    friend Hardware;
};


#endif //F1XX_PROJECT_TEMPLATE_CAN_HPP
