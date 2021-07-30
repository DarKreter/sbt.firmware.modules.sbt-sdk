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
    
    CAN();

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
                            parameterID : 16;
                int32_t data;
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
        TxMessage(uint16_t id, int32_t parameter);
        TxMessage(uint16_t id, float parameter);
        
        void SetParameterID(uint16_t id);
        void SetData(int32_t parameter);
        void SetData(float parameter);
        
        [[nodiscard]] uint16_t GetParameterID() const {return integer.parameterID;}
        [[nodiscard]] int32_t  GetDataInt() const {return integer.data;}
        [[nodiscard]] float    GetDataFloat() const {return floating.data;}
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
    
    /**
     * @brief Set mode (Default: NORMAL)
     * @param _mode NORMAL, LOOPBACK, SILENT or SILENT_LOOPBACK
     */
    void SetMode(Mode _mode);
    
    /**
    * @brief Return state handler for this object
    */
    CAN::State& GetState(){ return state;   }
    
    /**
     * @brief Tells if object was already Initialized (by calling Initialize function)
     * @return true if was, false if not
     */
    [[nodiscard]] bool IsInitialized() const {return initialized;}
    /**
     * @brief Initialize function, needs to be called after configuration and before using Send of Receive
     * @param ourBoxID ID with which our box will be sending frames
     * @param acceptedAddresses list of addresses from which we will be receiving frames
     */
    void Initialize(uint32_t ourBoxID, const std::initializer_list <uint32_t> &acceptedAddresses);
    
    /**
     * @brief Checks if we can send message
     */
    bool IsAnyTxMailboxFree();
    /**
     * @brief Sending message
     * @param message We need to provide object of that message
     */
    void Send(TxMessage &message);
    /**
     * @brief Sending message
     * @param message We need to provide object of that message
     */
    void Send(TxMessage &&message);
    /**
     * @brief Sending message
     * @param id parameter id
     * @param parameter value we want to send
     */
    void Send(uint16_t id, int32_t parameter);
    /**
     * @brief Sending message
     * @param id parameter id
     * @param parameter value we want to send
     */
    void Send(uint16_t id, float parameter);
    
    /**
     * @brief Gets message from queue of received messages
     * @return if there was no message it returns std::nullopt
     */
    std::optional<RxMessage> GetMessage();
    
    
    friend Hardware;
};


#endif //F1XX_PROJECT_TEMPLATE_CAN_HPP
