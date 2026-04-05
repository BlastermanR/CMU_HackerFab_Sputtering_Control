# Build the Alicat MFC Protocol Library
set(LIBRARY_NAME AlicatMFClib)

# Define Library
add_library(${LIBRARY_NAME} INTERFACE)

# Header Files
target_include_directories(${LIBRARY_NAME} INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/include
)