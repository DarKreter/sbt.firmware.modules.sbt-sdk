set(HEADER_LIST
        FreeRTOS-Kernel/include
        FreeRTOS-Kernel/portable/GCC/ARM_CM3
        FreeRTOS-Kernel/portable/MemMang
        Config
   )

file(GLOB_RECURSE HEADER_FILES FreeRTOS-Kernel/*.h Config/*.h)

set(SRC_LIST
        FreeRTOS-Kernel/portable/MemMang/heap_4.c
        FreeRTOS-Kernel/portable/GCC/ARM_CM3/port.c
        FreeRTOS-Kernel/tasks.c
        FreeRTOS-Kernel/list.c
        FreeRTOS-Kernel/timers.c
        FreeRTOS-Kernel/queue.c
        FreeRTOS-Kernel/event_groups.c
)

add_library(FreeRTOS-Kernel ${SRC_LIST} ${HEADER_FILES})

target_include_directories(FreeRTOS-Kernel PUBLIC ${HEADER_LIST} )
