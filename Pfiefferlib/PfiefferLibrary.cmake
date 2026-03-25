# Build the USB Serial Library
set(LIBRARY_NAME Pfiefferlib)

# Define Library
add_library(${LIBRARY_NAME} INTERFACE)

# Header Files
target_include_directories(${LIBRARY_NAME} INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/include/Devices
)