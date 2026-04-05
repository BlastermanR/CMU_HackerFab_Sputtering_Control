# Build the USB Serial Library
set(LIBRARY_NAME Pfeifferlib)

# Define Library
add_library(${LIBRARY_NAME} INTERFACE)

# Header Files
target_include_directories(${LIBRARY_NAME} INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/include/Devices
)