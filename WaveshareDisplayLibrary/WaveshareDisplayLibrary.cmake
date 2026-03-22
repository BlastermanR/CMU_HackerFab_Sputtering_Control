set(LIBRARY_NAME WaveshareDisplayLibrary)

add_library(${LIBRARY_NAME} STATIC
    ${CMAKE_CURRENT_LIST_DIR}/src/WaveshareDisplay.cpp
)

target_include_directories(${LIBRARY_NAME} PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(${LIBRARY_NAME} PUBLIC
    pico_stdlib
    hardware_spi
    hardware_pwm
)