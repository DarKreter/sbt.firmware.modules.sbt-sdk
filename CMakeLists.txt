cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchain-arm-none-eabi.cmake)

# Project name and a few useful settings. Other commands can pick up the results
project(sbt-sdk-f1xx
    VERSION 0.0.1
    DESCRIPTION "Software Development Kit for PWr Solar Boat Team STM32 projects"
    HOMEPAGE_URL "https://gitlab.com/solar-boat-pwr/firmware.modules.sbt-sdk"
    LANGUAGES CXX C
)

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    # Optionally set things like CMAKE_CXX_STANDARD, CMAKE_POSITION_INDEPENDENT_CODE here
    # Let's ensure -std=c++xx instead of -std=g++xx
    # set(CMAKE_CXX_EXTENSIONS OFF)

    # Let's nicely support folders in IDE's
    # set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Testing only available if this is the main app
    # Note this needs to be done in the main CMakeLists
    # since it calls enable_testing, which must be in the
    # main CMakeLists.
    # include(CTest) - for feature use

    # Docs only available if this is the main app
    # find_package(Doxygen)
    # if(Doxygen_FOUND)
	# add_subdirectory(docs)
    # else()
	# message(STATUS "Doxygen not found, not building docs")
    # endif()
endif()

include(cmake/stm32cubef1.cmake)
include(cmake/freertos-kernel.cmake)

add_subdirectory(SBT-SDK)
