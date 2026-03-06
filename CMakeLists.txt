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
set(PROJECT SputteringACS)
project(${PROJECT} C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Include Libraries
include(${CMAKE_SOURCE_DIR}/UARTLibrary/UARTLibrary.cmake)
include(${CMAKE_SOURCE_DIR}/USBSerialLibrary/USBSerialLibrary.cmake)

# Use GLOB to gather files
file(GLOB SOURCES "src/*.cpp")

# Add executable.
add_executable(${PROJECT} ${SOURCES})

# Set name (Change if desired)
pico_set_program_name(${PROJECT} "${PROJECT}")

# Set version
pico_set_program_version(${PROJECT} "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${PROJECT} 0)
pico_enable_stdio_usb(${PROJECT} 1)

# Add the link libraries to the executable
target_link_libraries(${PROJECT} PRIVATE
        # Custom Libs
        USBSerial
        UARTInterface

        # Pico Libs
        pico_stdlib
        hardware_uart
        hardware_pio
        hardware_irq
    )

# Add the standard include files to the build
target_include_directories(${PROJECT} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

pico_add_extra_outputs(${PROJECT})

