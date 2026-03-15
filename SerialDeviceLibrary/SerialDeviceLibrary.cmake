# Build the Serial Device Library
set(LIBRARY_NAME SerialDeviceLibrary)

# Define Library
add_library(${LIBRARY_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/src/SerialDeviceBase.cpp
)

# Header Files
target_include_directories(${LIBRARY_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

# Required Libraries
target_link_libraries(${LIBRARY_NAME} PUBLIC
    pico_stdlib
    hardware_uart
    UARTInterface
)
