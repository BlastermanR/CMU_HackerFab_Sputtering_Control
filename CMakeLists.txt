# Generated Cmake Pico project file
cmake_minimum_required(VERSION 3.13)

# Set c++ version
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Set comiple Commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

# Define project name
project(SputteringACS C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# ---------------------------------------------------------------------------
# SputterOS framework (v0.5)
# ---------------------------------------------------------------------------
add_subdirectory(lib/SputterOS)

# ---------------------------------------------------------------------------
# Device and platform libraries
# ---------------------------------------------------------------------------
include(${CMAKE_SOURCE_DIR}/DeviceLibraries/AlicatMFClib/AlicatMFClib.cmake)
include(${CMAKE_SOURCE_DIR}/UARTLibrary/UARTLibrary.cmake)
include(${CMAKE_SOURCE_DIR}/DeviceLibraries/Pfeifferlib/PfeifferLibrary.cmake)
include(${CMAKE_SOURCE_DIR}/SerialDeviceLibrary/SerialDeviceLibrary.cmake)

# ---------------------------------------------------------------------------
# Source collection
#
# Excludes legacy src/core1/ and src/shared/ (replaced by SputterOS).
# ---------------------------------------------------------------------------
file(GLOB SOURCES_CORE0  "${CMAKE_SOURCE_DIR}/src/core0/*.cpp")
file(GLOB SOURCES_HAL    "${CMAKE_SOURCE_DIR}/src/hal/*.cpp")
file(GLOB SOURCES_LOGIC  "${CMAKE_SOURCE_DIR}/src/logic/*.cpp")

set(SOURCES
    ${SOURCES_CORE0}
    ${SOURCES_HAL}
    ${SOURCES_LOGIC}
    "${CMAKE_SOURCE_DIR}/src/SputteringApp.cpp"
)

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------
add_executable(SputteringACS ${SOURCES})

pico_set_program_name(SputteringACS "SputteringACS")
pico_set_program_version(SputteringACS "0.2")

pico_enable_stdio_uart(SputteringACS 0)
pico_enable_stdio_usb(SputteringACS 1)

# ---------------------------------------------------------------------------
# Include directories
# ---------------------------------------------------------------------------
target_include_directories(SputteringACS PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/config
    ${CMAKE_SOURCE_DIR}/include/hal
    ${CMAKE_SOURCE_DIR}/include/safety
    ${CMAKE_SOURCE_DIR}/include/osal
    ${CMAKE_SOURCE_DIR}/include/logic
    ${CMAKE_SOURCE_DIR}/include/core0
    ${CMAKE_SOURCE_DIR}/include/shared
)

# ---------------------------------------------------------------------------
# Link libraries
# ---------------------------------------------------------------------------
target_link_libraries(SputteringACS PRIVATE
    # SputterOS framework
    SputterOS

    # Device drivers
    UARTInterface
    AlicatMFClib
    Pfeifferlib
    SerialDeviceLibrary

    # Pico SDK
    pico_stdlib
    pico_multicore
    hardware_uart
    hardware_pio
    hardware_irq
)

pico_add_extra_outputs(SputteringACS)
