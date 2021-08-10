//
// Created by darkr on 10.08.2021.
//

#ifndef F1XX_PROJECT_TEMPLATE_I2C_HPP
#define F1XX_PROJECT_TEMPLATE_I2C_HPP

#include <stm32f1xx_hal.h>
#include <FreeRTOS.h>
#include <event_groups.h>

namespace I2C {
    enum class I2C {
        I2C_1,
        I2C_2
    };

    struct State {
        EventGroupHandle_t txRxState;
        I2C_HandleTypeDef handle;
    };
}

/**
     * @brief Configure I2C in master mode
     * @param id ID of I2C. Currently only I2C_1 is supported.
     * @param address Address of a master
     * @param speed Transmission speed in Hz. This value needs to be between 100000Hz and 400000Hz
     */
void initializeI2C(I2C::I2C id, uint32_t address, uint32_t speed);

/**
 * @brief Send data via interrupt mode I2C as a master
 * @param id ID of I2C
 * @param address Address of slave
 * @param data Pointer to data to be sent
 * @param numOfBytes Length of data in bytes
 * @warning Note that data is not copied anywhere and needs to be available during entire transmission.
 */
void i2cSendMaster(I2C::I2C id, uint16_t address, uint8_t data[], size_t numOfBytes);
 void i2cSendSlave(I2C::I2C id, uint8_t data[], size_t numOfBytes);

/**
 * @brief Receive data via interrupt mode I2C as a master
 * @param id ID of I2C
 * @param address Address of slave
 * @param data Pointer to container where data will be written
 * @param numOfBytes Length of data in bytes
 */
 void i2cReceiveMaster(I2C::I2C id, uint16_t address, uint8_t data[], size_t numOfBytes);
 void i2cReceiveSlave(I2C::I2C id, uint8_t data[], size_t numOfBytes);
 I2C::State& getI2CState(I2C::I2C id);

 extern std::array<I2C::State, 2> i2cStates;

#endif //F1XX_PROJECT_TEMPLATE_I2C_HPP
