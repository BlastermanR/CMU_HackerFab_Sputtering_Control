# Build the USB Serial Library
set(LIBRARY_NAME USBSerial)

# Sources
file(GLOB ${LIBRARY_NAME}_SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")

# Define Library
add_library(${LIBRARY_NAME} STATIC ${${LIBRARY_NAME}_SOURCES})

# Header Files
target_include_directories(${LIBRARY_NAME} PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/include
)

# Required Libraries
target_link_libraries(${LIBRARY_NAME} PUBLIC
    pico_stdlib
)