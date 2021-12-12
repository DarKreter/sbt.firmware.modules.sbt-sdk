set(HEADER_LIST
        STM32CubeF1/Drivers/STM32F1xx_HAL_Driver/Inc
        STM32CubeF1/Drivers/CMSIS/Device/ST/STM32F1xx/Include
        STM32CubeF1/Drivers/CMSIS/Include
        Config
   )

file(GLOB_RECURSE HEADER_FILES STM32CubeF1/*.h Config/*.h)

set(SRC_LIST
        STM32CubeF1/Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/system_stm32f1xx.c
        STM32CubeF1/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c
        STM32CubeF1/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c
        STM32CubeF1/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c
        STM32CubeF1/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_usart.c
        STM32CubeF1/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_i2c.c
        STM32CubeF1/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_spi.c
        STM32CubeF1/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c
        STM32CubeF1/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c
        STM32CubeF1/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c
        STM32CubeF1/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_can.c
        STM32CubeF1/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c
        STM32CubeF1/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc.c
        STM32CubeF1/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc_ex.c
)

add_library(STM32Cube-F1 ${SRC_LIST} ${HEADER_FILES})

target_include_directories(STM32Cube-F1 PUBLIC ${HEADER_LIST} )
