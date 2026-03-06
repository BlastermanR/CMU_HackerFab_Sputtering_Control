# Build the USB Serial Library
set(LIBRARY_NAME UARTInterface)

# Define Library
add_library(${LIBRARY_NAME} INTERFACE)

# Tell CMake to compile PIO files into headers
pico_generate_pio_header(${LIBRARY_NAME} ${CMAKE_CURRENT_LIST_DIR}/pio/uart_tx.pio)
pico_generate_pio_header(${LIBRARY_NAME} ${CMAKE_CURRENT_LIST_DIR}/pio/uart_rx.pio)

# Header Files
target_include_directories(${LIBRARY_NAME} INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/include
)

# Required Libraries
target_link_libraries(${LIBRARY_NAME} INTERFACE
    pico_stdlib
)