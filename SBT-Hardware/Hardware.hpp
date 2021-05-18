#ifndef HARDWARE_HPP
#define HARDWARE_HPP
#include <stm32f1xx_hal.h>
#include <FreeRTOS.h>
#include <event_groups.h>
#include <queue.h>
#include <map>
#include <UART.hpp>

// Goal of this abstraction is to isolate main code from hardware-specific code. Ideally, this code could use a stub and
// be compiled for Linux without any problem
namespace Gpio {
    enum class Mode {
        Input,
        Output,
        AlternateInput,
        AlternatePP,
        AlternateOD
    };

    enum class Pull {
        Pullup = 1,
        PullDown = 2,
        NoPull = 0
    };
}


namespace I2C {
    enum class I2C {
        I2C_1,
        I2C_2
    };

    struct State {
        EventGroupHandle_t txRxState;
        I2C_HandleTypeDef handle;

        static constexpr size_t txBit = 1 << 0;
        static constexpr size_t rxBit = 1 << 1;
    };
}

namespace SPI {
    struct State {
        EventGroupHandle_t txRxState;
        SPI_HandleTypeDef handle;

        static constexpr size_t txBit = 1 << 0;
        static constexpr size_t rxBit = 1 << 1;
    };
}

namespace CAN {
    struct State {
        CAN_HandleTypeDef handle;
        QueueHandle_t queueHandle;
    };

    struct RxMessage {
        uint32_t id;
        std::array<uint8_t, 8> payload;
    };

    struct TxMessage {
        explicit TxMessage(uint32_t id);

        CAN_TxHeaderTypeDef header;
        std::array<uint8_t, 8> payload;
    };
}

struct Hardware {
    /**
     * @brief Configure GPIO pin
     * @param gpio GPIO Port
     * @param pin Pin number
     * @param direction Mode of operation
     * @param pull Pulling resistor configuration
     */
    static void enableGpio(GPIO_TypeDef* gpio, uint32_t pin, Gpio::Mode direction, Gpio::Pull pull = Gpio::Pull::NoPull);
    static void toggle(GPIO_TypeDef* gpio, uint32_t pin);

    /**
     * @brief Configure system clock
     */
    static void configureClocks();
    

    /**
     * @brief Configure I2C in master mode
     * @param id ID of I2C. Currently only I2C_1 is supported.
     * @param address Address of a master
     * @param speed Transmission speed in Hz. This value needs to be between 100000Hz and 400000Hz
     */
    static void initializeI2C(I2C::I2C id, uint32_t address, uint32_t speed);

    /**
     * @brief Send data via interrupt mode I2C as a master
     * @param id ID of I2C
     * @param address Address of slave
     * @param data Pointer to data to be sent
     * @param numOfBytes Length of data in bytes
     * @warning Note that data is not copied anywhere and needs to be available during entire transmission.
     */
    static void i2cSendMaster(I2C::I2C id, uint16_t address, uint8_t data[], size_t numOfBytes);

    /**
     * @brief Receive data via interrupt mode I2C as a master
     * @param id ID of I2C
     * @param address Address of slave
     * @param data Pointer to container where data will be written
     * @param numOfBytes Length of data in bytes
     */
    static void i2cReceiveMaster(I2C::I2C id, uint16_t address, uint8_t data[], size_t numOfBytes);
    static I2C::State& getI2CState(I2C::I2C id);

    static void initializeSpi();
    static void spiSend(uint8_t data[], size_t numOfBytes);
    static void spiReceive(uint8_t data[], size_t numOfBytes);
    static SPI::State& getSpiState();

    static void initializeCan(const std::initializer_list <uint32_t> &acceptedAddresses);
    static bool isAnyTxMailboxFree();
    static void sendCanMessage(CAN::TxMessage &message);
    static std::optional<CAN::RxMessage> getCanMessageFromQueue();
    static CAN::State& getCanState();

    static UART uart1, uart2, uart3;
    
    static void InitializeStaticVariables();
private:
    static std::array<I2C::State, 2> i2cStates;
    static std::array<SPI::State, 1> spiState;
    static std::array<CAN::State, 1> canState;
};


#endif //HARDWARE_HPP
