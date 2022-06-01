//
// Created by darkr on 10.08.2021.
//

#ifndef F1XX_PROJECT_TEMPLATE_I2C_HPP
#define F1XX_PROJECT_TEMPLATE_I2C_HPP

#include "DMA.hpp"
#include <functional>
#include <stm32f1xx_hal.h>

namespace SBT::Hardware {
class I2C {
    bool initialized;

    /*-------HANDLERS---------*/
    struct State {
        [[deprecated("Use State field in handle instead")]] void* txRxState;
        I2C_HandleTypeDef handle;
    };
    State state;

    /*-------ENUMS---------*/
    enum class Instance {
        NONE,
        I2C_1,
        I2C_2
    };

    enum class OperatingMode {
        BLOCKING,
        INTERRUPTS,
        DMA
    };

public:
    enum class AddressingMode {
        _7BIT = I2C_ADDRESSINGMODE_7BIT,
        _10BIT = I2C_ADDRESSINGMODE_10BIT
    };

    enum class CallbackType {
        MasterTxComplete = HAL_I2C_MASTER_TX_COMPLETE_CB_ID,
        MasterRxComplete = HAL_I2C_MASTER_RX_COMPLETE_CB_ID,
        SlaveTxComplete = HAL_I2C_SLAVE_TX_COMPLETE_CB_ID,
        SlaveRxComplete = HAL_I2C_SLAVE_RX_COMPLETE_CB_ID,
        ListenComplete = HAL_I2C_LISTEN_COMPLETE_CB_ID,
        MemTxComplete = HAL_I2C_MEM_TX_COMPLETE_CB_ID,
        MemRxComplete = HAL_I2C_MEM_RX_COMPLETE_CB_ID,
        Error = HAL_I2C_ERROR_CB_ID,
        Abort = HAL_I2C_ABORT_CB_ID,
        MspInit = HAL_I2C_MSPINIT_CB_ID,
        MspDeInit = HAL_I2C_MSPDEINIT_CB_ID
    };

private:
    Instance instance;
    OperatingMode mode;
    AddressingMode addressingMode;
    uint32_t speed;

    uint32_t timeout;

    DMA* dmaController;
    DMA::Channel dmaChannelTx, dmaChannelRx;

    HAL_StatusTypeDef SendMasterRCC(uint16_t slaveAddress, uint8_t* data,
                                    size_t numOfBytes);
    HAL_StatusTypeDef SendMasterIT(uint16_t slaveAddress, uint8_t* data,
                                   size_t numOfBytes);
    HAL_StatusTypeDef SendMasterDMA(uint16_t slaveAddress, uint8_t* data,
                                    size_t numOfBytes);

    HAL_StatusTypeDef SendSlaveRCC(uint8_t* data, size_t numOfBytes);
    HAL_StatusTypeDef SendSlaveIT(uint8_t* data, size_t numOfBytes);
    HAL_StatusTypeDef SendSlaveDMA(uint8_t* data, size_t numOfBytes);

    HAL_StatusTypeDef ReceiveMasterRCC(uint16_t slaveAddress, uint8_t* data,
                                       size_t numOfBytes);
    HAL_StatusTypeDef ReceiveMasterIT(uint16_t slaveAddress, uint8_t* data,
                                      size_t numOfBytes);
    HAL_StatusTypeDef ReceiveMasterDMA(uint16_t slaveAddress, uint8_t* data,
                                       size_t numOfBytes);

    HAL_StatusTypeDef ReceiveSlaveRCC(uint8_t* data, size_t numOfBytes);
    HAL_StatusTypeDef ReceiveSlaveIT(uint8_t* data, size_t numOfBytes);
    HAL_StatusTypeDef ReceiveSlaveDMA(uint8_t* data, size_t numOfBytes);

    HAL_StatusTypeDef ReadRegisterRCC(uint16_t slaveAddress,
                                      uint16_t registerAddress,
                                      uint8_t registerSize, uint8_t* data,
                                      uint16_t dataSize);
    HAL_StatusTypeDef ReadRegisterIT(uint16_t slaveAddress,
                                     uint16_t registerAddress,
                                     uint8_t registerSize, uint8_t* data,
                                     uint16_t dataSize);
    HAL_StatusTypeDef ReadRegisterDMA(uint16_t slaveAddress,
                                      uint16_t registerAddress,
                                      uint8_t registerSize, uint8_t* data,
                                      uint16_t dataSize);

    HAL_StatusTypeDef WriteRegisterRCC(uint16_t slaveAddress,
                                       uint16_t registerAddress,
                                       uint8_t registerSize, uint8_t* data,
                                       uint16_t dataSize);
    HAL_StatusTypeDef WriteRegisterIT(uint16_t slaveAddress,
                                      uint16_t registerAddress,
                                      uint8_t registerSize, uint8_t* data,
                                      uint16_t dataSize);
    HAL_StatusTypeDef WriteRegisterDMA(uint16_t slaveAddress,
                                       uint16_t registerAddress,
                                       uint8_t registerSize, uint8_t* data,
                                       uint16_t dataSize);

public:
    I2C() = delete;
    I2C(I2C&) = delete;

    explicit I2C(I2C_TypeDef* i2cc);

    /**
     * @brief Set addressing mode (Default: _7BIT)
     * @param _addressingMode _7BIT or _10BIT
     */
    void SetAddressingMode(AddressingMode _addressingMode);

    /**
     * @brief Sets speed (Default: 100 000)
     * @param _speed
     */
    void SetSpeed(uint32_t _speed);
    /**
     * @brief Changes operating mode of I2C to blocking (Default: Interrupts)
     * @param Timeout timeout
     */
    void ChangeModeToBlocking(uint32_t _timeout = 500);

    /**
     * @brief Changes operating mode of I2C to interrupts (Default: Interrupts)
     * @note Before using I2C in interrupt mode see ES096, section 2.14.1
     */
    void ChangeModeToInterrupts();

    /**
     * @brief Changes operating mode of I2C to DMA (Default: Interrupts)
     */
    void ChangeModeToDMA();

    /**
     * @brief Return state handler for this object
     */
    State& GetState() { return state; }

    /**
     * @brief Send data in I2C as a master
     * @param slaveAddress Address of slave
     * @param data Pointer to data to be sent
     * @param numOfBytes Length of data in bytes
     * @warning Note that data is not copied anywhere and needs to be available
     * during entire transmission.
     * @return HAL_Status returned by HAL function. See HAL doc. for possible
     * values.
     */
    HAL_StatusTypeDef SendMaster(uint16_t slaveAddress, uint8_t* data,
                                 size_t numOfBytes);
    /**
     * @brief Send data in I2C as a slave
     * @param data Pointer to data to be sent
     * @param numOfBytes Length of data in bytes
     * @warning Note that data is not copied anywhere and needs to be available
     * during entire transmission.
     * @return HAL_Status returned by HAL function. See HAL doc. for possible
     * values.
     */
    HAL_StatusTypeDef SendSlave(uint8_t* data, size_t numOfBytes);
    /**
     * @brief Receive data in I2C as a master
     * @param slaveAddress Address of slave
     * @param data Pointer to container where data will be written
     * @param numOfBytes Length of data in bytes
     * @return HAL_Status returned by HAL function. See HAL doc. for possible
     * values.
     */
    HAL_StatusTypeDef ReceiveMaster(uint16_t slaveAddress, uint8_t* data,
                                    size_t numOfBytes);
    /**
     * @brief Receive data in I2C as a slave
     * @param data Pointer to container where data will be written
     * @param numOfBytes Length of data in bytes
     * @return HAL_Status returned by HAL function. See HAL doc. for possible
     * values.
     */
    HAL_StatusTypeDef ReceiveSlave(uint8_t* data, size_t numOfBytes);
    /**
     * @brief Read indicated register from slave device
     * @param slaveAddress Address of slave
     * @param registerAddress Address of register from which we will read data
     * @param registerSize size of register
     * @param data pointer to table where read data will be stored
     * @param dataSize sizeof data
     * @return HAL_Status returned by HAL function. See HAL doc. for possible
     * values.
     */
    HAL_StatusTypeDef ReadRegister(uint16_t slaveAddress,
                                   uint16_t registerAddress,
                                   uint8_t registerSize, uint8_t* data,
                                   uint16_t dataSize);
    /**
     * @brief Write data to register as master
     * @param slaveAddress Address of slave
     * @param registerAddress Address of register where we will write data
     * @param registerSize size of register
     * @param data pointer to table where read data will be stored
     * @param dataSize sizeof data
     * @return HAL_Status returned by HAL function. See HAL doc. for possible
     * values.
     */
    HAL_StatusTypeDef WriteRegister(uint16_t slaveAddress,
                                    uint16_t registerAddress,
                                    uint8_t registerSize, uint8_t* data,
                                    uint16_t dataSize);

    /**
     * @brief Tells if object was already Initialized (by calling Initialize
     * function)
     * @return true if was, false if not
     */
    [[nodiscard]] bool IsInitialized() const { return initialized; }

    /**
     * @brief Configure I2C in master mode
     * @param ownAddress Address of a master
     */
    void Initialize(uint32_t ownAddress);

    /**
     * @brief Reinitialize I2C
     * @param ownAddress Address of a master
     */
    void ReInitialize(uint32_t ownAddress);

    /**
     * @brief Deinitialize I2C
     */
    void DeInitialize();

    /**
     * @brief Register a custom callback
     * @param callbackType Event which triggers the callback
     * @param callbackFunction Void function taking no arguments. A regular
     * function pointer will be casted to std::function implicitly. For a
     * non-static class member function use the template version of
     * RegisterCallback.
     */
    void RegisterCallback(CallbackType callbackType,
                          std::function<void()> callbackFunction);

    /**
     * @brief Register a non-static class member function as a custom callback
     * @tparam T Class name
     * @param callbackType Event which triggers the callback
     * @param callbackObject A pointer to the object in context of which the
     * callbackFunction will be called
     * @param callbackFunction A pointer to the callback function called in the
     * context of the callbackObject
     * @example // When called from any function
     * @example RegisterCallback(CallbackType::MasterTxComplete,
     * &myTaskTypeObject, &myTask::myCallback);
     * @example
     * @example // When called from a function being a member of the
     * callbackObject
     * @example RegisterCallback(CallbackType::MasterTxComplete, this,
     * &myTask::myCallback);
     */
    // This template envelopes the call of the member callback function in a
    // lambda and passes it as an independent callback function.
    template <class T>
    void RegisterCallback(CallbackType callbackType, T* callbackObject,
                          void (T::*callbackFunction)())
    {
        RegisterCallback(callbackType, [callbackObject, callbackFunction]() {
            (callbackObject->*callbackFunction)();
        });
    }

    /**
     * @brief Checks if Transmission is completed
     */
    [[nodiscard]] bool IsTxComplete() const;
    /**
     * @brief Checks if receiving is completed
     */
    [[nodiscard]] bool IsRxComplete() const;
};

extern I2C i2c1, i2c2;
} // namespace SBT::Hardware

#endif // F1XX_PROJECT_TEMPLATE_I2C_HPP
