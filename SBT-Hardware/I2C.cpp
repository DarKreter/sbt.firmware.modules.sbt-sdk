//
// Created by darkr on 10.08.2021.
//
#include <array>
#include <I2C.hpp>
#include <Hardware.hpp>

std::array<I2C::State, 2> i2cStates;


void initializeI2C(I2C::I2C id, uint32_t address, uint32_t speed) {
    switch (id) {
        case I2C::I2C::I2C_1:
            // Enable clocks
            __HAL_RCC_GPIOB_CLK_ENABLE();
            __HAL_RCC_I2C1_CLK_ENABLE();
            // Set GPIO
            Hardware::enableGpio(GPIOB, GPIO_PIN_6, Gpio::Mode::AlternateOD, Gpio::Pull::Pullup);  // SCL
            Hardware::enableGpio(GPIOB, GPIO_PIN_7, Gpio::Mode::AlternateOD, Gpio::Pull::Pullup);  // SDA
            // Enable interrupts with high priority due to silicon limitation (UM1850 p. 259)
            HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
            HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
            HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 0);
            HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
            break;
            case I2C::I2C::I2C_2:
                break;
    }

    I2C::State& state = getI2CState(id);

    state.handle.Instance = I2C1;
    state.handle.Init.ClockSpeed = speed;
    state.handle.Init.DutyCycle = I2C_DUTYCYCLE_2;
    state.handle.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    state.handle.Init.OwnAddress1 = address;
    state.handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    state.handle.Init.OwnAddress2 = 0;
    state.handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    state.handle.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    // Set registers with prepared data
    HAL_I2C_Init(&state.handle);

    // Enable interrupts
    //    __HAL_I2C_ENABLE_IT(&state.handle, I2C_IT_BUF);
    //    __HAL_I2C_ENABLE_IT(&state.handle, I2C_IT_EVT);
    //    __HAL_I2C_ENABLE_IT(&state.handle, I2C_IT_ERR);


    // Clear bits
    state.txRxState = xEventGroupCreate();
    xEventGroupClearBits(state.txRxState, Hardware::rxBit | Hardware::txBit);
}

void i2cSendMaster(I2C::I2C id, uint16_t address, uint8_t *data, size_t numOfBytes) {
    I2C::State& state = getI2CState(id);
    // Check if event group was created
    if(state.txRxState) {
        // Check if there is no transmission
        //        if((xEventGroupGetBits(state.txRxState) & Hardware::txBit) == 0) {
        // If I2C is not busy, transmit and set TX flag to busy
        auto temp = HAL_I2C_Master_Transmit(&state.handle, address, data, numOfBytes, 1000);
        Hardware::uart1.printf("%d\n", (int)temp);
        xEventGroupSetBits(state.txRxState, Hardware::txBit);
        //        }
    }
}

void i2cReceiveMaster(I2C::I2C id, uint16_t address, uint8_t *data, size_t numOfBytes) {
    I2C::State& state = getI2CState(id);
    // Check if event group was created
    if(state.txRxState) {
        // Check if there is no transmission
        //        if((xEventGroupGetBits(state.txRxState) & Hardware::rxBit) == 0) {
        // If I2C is not busy, transmit and set RX flag to busy

        auto temp = HAL_I2C_Master_Receive_IT(&state.handle, address, data, numOfBytes);
        Hardware::uart1.printf("%d", (int)temp);


        xEventGroupSetBits(state.txRxState, Hardware::rxBit);
        //        }
    }
}

void i2cSendSlave(I2C::I2C id, uint8_t *data, size_t numOfBytes) {
    I2C::State& state = getI2CState(id);
    // Check if event group was created
    if(state.txRxState) {
        // Check if there is no transmission
        //        if((xEventGroupGetBits(state.txRxState) & Hardware::txBit) == 0) {
        // If I2C is not busy, transmit and set TX flag to busy
        HAL_I2C_Slave_Transmit_IT(&state.handle, data, numOfBytes);
        xEventGroupSetBits(state.txRxState, Hardware::txBit);
        //        }
    }
}

void i2cReceiveSlave(I2C::I2C id, uint8_t *data, size_t numOfBytes) {
    I2C::State& state = getI2CState(id);
    // Check if event group was created
    if(state.txRxState) {
        // Check if there is no transmission
        //        if((xEventGroupGetBits(state.txRxState) & Hardware::rxBit) == 0) {
        // If I2C is not busy, transmit and set RX flag to busy
        HAL_I2C_Slave_Receive_IT(&state.handle, data, numOfBytes);
        xEventGroupSetBits(state.txRxState, Hardware::rxBit);
        //        }
    }
}

I2C::State &getI2CState(I2C::I2C id) {
    switch (id) {
        case I2C::I2C::I2C_1:
            return i2cStates[0];
            case I2C::I2C::I2C_2:
                return i2cStates[1];
    }
    // Give up and stay here
    return getI2CState(id);
}

// Handlers for I2C transmission
void I2C1_EV_IRQHandler(){
    HAL_I2C_EV_IRQHandler(&getI2CState(I2C::I2C::I2C_1).handle);
}

void I2C1_ER_IRQHandler(){
    HAL_I2C_ER_IRQHandler(&getI2CState(I2C::I2C::I2C_1).handle);
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c){
    if(hi2c->Instance == I2C1){
        if(auto* eventGroup = getI2CState(I2C::I2C::I2C_1).txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::txBit);
        }
    }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c){
    if(hi2c->Instance == I2C1){
        if(auto* eventGroup = getI2CState(I2C::I2C::I2C_1).txRxState) {
            xEventGroupClearBitsFromISR(eventGroup, Hardware::rxBit);
        }
    }
}
