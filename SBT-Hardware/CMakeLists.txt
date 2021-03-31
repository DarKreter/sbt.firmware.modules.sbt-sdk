set(HEADER_LIST
        Hardware.hpp
)

set(SRC_LIST
        Hardware.cpp
)

add_library(SBT-Hardware ${SRC_LIST} ${HEADER_LIST})

target_include_directories(SBT-Hardware PUBLIC ./ )

target_link_libraries(SBT-Hardware PRIVATE STM32Cube-F1)
target_link_libraries(SBT-Hardware PRIVATE FreeRTOS-Kernel)

add_dependencies(SBT-Hardware STM32Cube-F1)
add_dependencies(SBT-Hardware FreeRTOS-Kernel)