//
// Created by darkr on 10.08.2021.
//

#ifndef F1XX_PROJECT_TEMPLATE_I2C_HPP
#define F1XX_PROJECT_TEMPLATE_I2C_HPP

#include <stm32f1xx_hal.h>
#include <FreeRTOS.h>
#include <event_groups.h>

struct Hardware;

class I2C
{
    bool initialized;

    /*-------HANDLERS---------*/
    struct State
    {
        EventGroupHandle_t txRxState;
        I2C_HandleTypeDef handle;
    };
    State state;

    /*-------ENUMS---------*/
    enum class Instance
    {
        NONE,
        I2C_1,
        I2C_2
    };

    enum class OperatingMode
    {
        BLOCKING,
        INTERRUPTS,
        DMA
    };

public:

    enum class AddressingMode
    {
        _7BIT = I2C_ADDRESSINGMODE_7BIT,
        _10BIT = I2C_ADDRESSINGMODE_10BIT
    };

private:
    Instance instance;
    OperatingMode mode;
    AddressingMode addressingMode;
    uint32_t speed;

    uint32_t timeout;
    
    
    explicit I2C(I2C_TypeDef* i2cc);
    I2C()  = delete;


    void SendMasterRCC(uint16_t slaveAddress, uint8_t* data, size_t numOfBytes);
    void SendMasterIT(uint16_t slaveAddress, uint8_t* data, size_t numOfBytes);
    void SendMasterDMA(uint16_t slaveAddress, uint8_t* data, size_t numOfBytes);

    void SendSlaveRCC(uint8_t* data, size_t numOfBytes);
    void SendSlaveIT(uint8_t* data, size_t numOfBytes);
    void SendSlaveDMA(uint8_t* data, size_t numOfBytes);

    void ReceiveMasterRCC(uint16_t slaveAddress, uint8_t* data, size_t numOfBytes);
    void ReceiveMasterIT(uint16_t slaveAddress, uint8_t* data, size_t numOfBytes);
    void ReceiveMasterDMA(uint16_t slaveAddress, uint8_t* data, size_t numOfBytes);

    void ReceiveSlaveRCC(uint8_t* data, size_t numOfBytes);
    void ReceiveSlaveIT(uint8_t* data, size_t numOfBytes);
    void ReceiveSlaveDMA(uint8_t* data, size_t numOfBytes);

    void ReadRegisterRCC(uint16_t slaveAddress, uint16_t registerAddress, uint8_t registerSize, uint8_t* data, uint16_t dataSize);
    void ReadRegisterIT(uint16_t slaveAddress, uint16_t registerAddress, uint8_t registerSize, uint8_t* data, uint16_t dataSize);
    void ReadRegisterDMA(uint16_t slaveAddress, uint16_t registerAddress, uint8_t registerSize, uint8_t* data, uint16_t dataSize);

    void WriteRegisterRCC(uint16_t slaveAddress, uint16_t registerAddress, uint8_t registerSize, uint8_t* data, uint16_t dataSize);
    void WriteRegisterIT(uint16_t slaveAddress, uint16_t registerAddress, uint8_t registerSize, uint8_t* data, uint16_t dataSize);
    void WriteRegisterDMA(uint16_t slaveAddress, uint16_t registerAddress, uint8_t registerSize, uint8_t* data, uint16_t dataSize);


public:
    
    void SetAddressingMode(AddressingMode _addressingMode);
    
    void SetSpeed(uint32_t _speed);
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
    * @brief Return state handler for this object
    */
    State& GetState() {return state;}
    
    /**
     * @brief Send data via interrupt mode I2C as a master
     * @param slaveAddress Address of slave
     * @param data Pointer to data to be sent
     * @param numOfBytes Length of data in bytes
     * @warning Note that data is not copied anywhere and needs to be available during entire transmission.
     */
    void SendMaster(uint16_t slaveAddress, uint8_t* data, size_t numOfBytes);
    
    void SendSlave(uint8_t* data, size_t numOfBytes);
    
    /**
     * @brief Receive data via interrupt mode I2C as a master
     * @param id ID of I2C
     * @param slaveAddress Address of slave
     * @param data Pointer to container where data will be written
     * @param numOfBytes Length of data in bytes
     */
    void ReceiveMaster(uint16_t slaveAddress, uint8_t* data, size_t numOfBytes);
    
    void ReceiveSlave(uint8_t* data, size_t numOfBytes);

    void ReadRegister(uint16_t slaveAddress, uint16_t registerAddress, uint8_t registerSize, uint8_t* data, uint16_t dataSize);
    
    void WriteRegister(uint16_t slaveAddress, uint16_t registerAddress, uint8_t registerSize, uint8_t* data, uint16_t dataSize);
    
    
    /**
    * @brief Tells if object was already Initialized (by calling Initialize function)
    * @return true if was, false if not
    */
    [[nodiscard]] bool IsInitialized() const {return initialized;}
    
    /**
     * @brief Configure I2C in master mode
     * @param id ID of I2C. Currently only I2C_1 is supported.
     * @param address Address of a master
     * @param speed Transmission speed in Hz. This value needs to be between 100000Hz and 400000Hz
     */
    void Initialize(uint32_t ownAddress);
    
    /**
    * @brief Checks if Transmission is completed
    */
    [[nodiscard]] bool IsTxComplete() const;
    /**
    * @brief Checks if receiving is completed
    */
    [[nodiscard]] bool IsRxComplete() const;


    friend Hardware;
};


#endif //F1XX_PROJECT_TEMPLATE_I2C_HPP
