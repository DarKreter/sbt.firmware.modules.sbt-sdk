# Append current directory to CMAKE_MODULE_PATH for making device specific cmake modules visible
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

# Target definition
set(CMAKE_SYSTEM_NAME  Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CPU_OPTIONS "-mthumb -mcpu=cortex-m3 -DSTM32F103xB")

#---------------------------------------------------------------------------------------
# Set toolchain paths
#---------------------------------------------------------------------------------------
if(NOT DEFINED CMAKE_C_COMPILER OR NOT DEFINED CMAKE_CXX_COMPILER)
    set(TOOLCHAIN arm-none-eabi)
    if(NOT DEFINED TOOLCHAIN_PREFIX)
        if(CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
            set(TOOLCHAIN_PREFIX "/usr")
        elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL Darwin)
            set(TOOLCHAIN_PREFIX "/usr/local")
        elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
            message(STATUS "Please specify the TOOLCHAIN_PREFIX !\n For example: -DTOOLCHAIN_PREFIX=\"C:/Program Files/GNU Tools ARM Embedded\" ")
        else()
            set(TOOLCHAIN_PREFIX "/usr")
            message(STATUS "No TOOLCHAIN_PREFIX specified, using default: " ${TOOLCHAIN_PREFIX})
        endif()
    endif()

    set(TOOLCHAIN_BIN_DIR ${TOOLCHAIN_PREFIX}/bin)
    set(TOOLCHAIN_INC_DIR ${TOOLCHAIN_PREFIX}/${TOOLCHAIN}/include)
    set(TOOLCHAIN_LIB_DIR ${TOOLCHAIN_PREFIX}/${TOOLCHAIN}/lib)

    # Set system depended extensions
    if(WIN32)
        set(TOOLCHAIN_EXT ".exe" )
    else()
        set(TOOLCHAIN_EXT "" )
    endif()
endif()

# Perform compiler test with static library
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

#---------------------------------------------------------------------------------------
# Set compiler/linker flags
#---------------------------------------------------------------------------------------

# Object build options
# -fno-builtin          Do not use built-in functions provided by GCC.
# -Wall                 Print only standard warnings, for all use Wextra
# -ffunction-sections   Place each function item into its own section in the output file.
# -fdata-sections       Place each data item into its own section in the output file.
# -fomit-frame-pointer  Omit the frame pointer in functions that don’t need one.
# -mabi=aapcs           Defines enums to be a variable sized type.
set(OBJECT_GEN_FLAGS "${CPU_OPTIONS} -fno-builtin -Werror -Wall -Wextra -pedantic -ffunction-sections -fdata-sections -fomit-frame-pointer -mabi=aapcs")

set(CMAKE_C_FLAGS   "${OBJECT_GEN_FLAGS} -std=gnu17" CACHE INTERNAL "C Compiler options")
set(CMAKE_CXX_FLAGS "${OBJECT_GEN_FLAGS} -std=c++17 -fno-use-cxa-atexit" CACHE INTERNAL "C++ Compiler options")
set(CMAKE_ASM_FLAGS "${OBJECT_GEN_FLAGS} -x assembler-with-cpp" CACHE INTERNAL "ASM Compiler options")


# -Wl,--gc-sections     Perform the dead code elimination.
# --specs=nano.specs    Link with newlib-nano.
# --specs=nosys.specs   No syscalls, provide empty implementations for the POSIX system calls.

#set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections -Wl,--export-dynamic --specs=nano.specs --specs=nosys.specs -T${LINKER_SCRIPT} -nostartfiles" CACHE INTERNAL "Linker options")

#---------------------------------------------------------------------------------------
# Set debug/release build configuration Options
#---------------------------------------------------------------------------------------

# Options for DEBUG build
# -Og   Enables optimizations that do not interfere with debugging.
# -g    Produce debugging information in the operating system’s native format.
set(CMAKE_C_FLAGS_DEBUG "-Og -g -DSBT_DEBUG" CACHE INTERNAL "C Compiler options for debug build type")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g -DSBT_DEBUG" CACHE INTERNAL "C++ Compiler options for debug build type")
set(CMAKE_ASM_FLAGS_DEBUG "-g" CACHE INTERNAL "ASM Compiler options for debug build type")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "Linker options for debug build type")

# Options for RELEASE build
# -Os   Optimize for size. -Os enables all -O2 optimizations.
# -flto Runs the standard link-time optimizer.
set(CMAKE_C_FLAGS_RELEASE "-Os -flto -ffat-lto-objects" CACHE INTERNAL "C Compiler options for release build type")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -flto -ffat-lto-objects" CACHE INTERNAL "C++ Compiler options for release build type")
set(CMAKE_ASM_FLAGS_RELEASE "" CACHE INTERNAL "ASM Compiler options for release build type")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto -ffat-lto-objects" CACHE INTERNAL "Linker options for release build type")


#---------------------------------------------------------------------------------------
# Set compilers
#---------------------------------------------------------------------------------------
if(NOT DEFINED CMAKE_C_COMPILER OR NOT DEFINED CMAKE_CXX_COMPILER)
    set(CMAKE_C_COMPILER ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-gcc${TOOLCHAIN_EXT} CACHE INTERNAL "C Compiler")
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-g++${TOOLCHAIN_EXT} CACHE INTERNAL "C++ Compiler")
    set(CMAKE_ASM_COMPILER ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-gcc${TOOLCHAIN_EXT} CACHE INTERNAL "ASM Compiler")
endif()

set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_PREFIX}/${${TOOLCHAIN}} ${CMAKE_PREFIX_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
