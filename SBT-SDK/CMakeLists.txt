set(HEADER_LIST
        SBT-SDK.hpp
        Hardware/Hardware.hpp
        System/Task.hpp
        System/TaskManager.hpp
        )

set(SRC_LIST
        SBT-SDK.cpp
        Hardware/Hardware.cpp
        Hardware/UART.cpp
        Hardware/SPI.cpp
        Hardware/CAN.cpp
        Hardware/I2C.cpp
        System/Task.cpp
        System/TaskManager.cpp
        )

add_library(SBT-SDK ${SRC_LIST} ${HEADER_LIST})

target_include_directories(SBT-SDK PUBLIC . Hardware System)

target_link_libraries(SBT-SDK PRIVATE STM32Cube-F1)
target_link_libraries(SBT-SDK PRIVATE FreeRTOS-Kernel)

add_dependencies(SBT-SDK STM32Cube-F1)
add_dependencies(SBT-SDK FreeRTOS-Kernel)
