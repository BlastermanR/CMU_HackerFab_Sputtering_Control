# Unit Test Build for SputteringACS
# This is a host-native build — it does NOT use the Pico SDK.
#
# @author Ryan Massie (rmassie)
# @date 3/31/26
#
# Usage:
#   cmake -B build_tests -S tests -G Ninja
#   ninja -C build_tests
#   ctest --test-dir build_tests --output-on-failure

cmake_minimum_required(VERSION 3.14)
project(SputteringACS_Tests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Signal that we are building for unit testing on the host (no Pico hardware)
add_compile_definitions(PICO_TESTING)

# ── Fetch Google Test ────────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)
# Prevent GoogleTest from overriding our compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# ── Common include paths (mirrors the real project layout) ───────────────────
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks          # Pico stubs & GMock mocks
    ${PROJECT_ROOT}/include                     # Project headers
    ${PROJECT_ROOT}/include/core0
    ${PROJECT_ROOT}/include/core1
    ${PROJECT_ROOT}/include/shared
    ${PROJECT_ROOT}/AlicatMFClib/include
    ${PROJECT_ROOT}/Pfeifferlib/include
    ${PROJECT_ROOT}/Pfeifferlib/include/Devices
    ${PROJECT_ROOT}/SerialDeviceLibrary/include
    ${PROJECT_ROOT}/UARTLibrary/include
    ${PROJECT_ROOT}/DisplayLibrary/include
    ${PROJECT_ROOT}/USBSerialLibrary/include
)

# ── Pico SDK stubs (compiled once, linked by tests that need it) ─────────────
add_library(pico_stubs STATIC mocks/pico_stubs.cpp)
target_include_directories(pico_stubs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/mocks)

# ── Test subdirectories ──────────────────────────────────────────────────────
# Sub-library tests live alongside their library source code and support
# standalone builds.  The top-level build pulls them in here.
add_subdirectory(${PROJECT_ROOT}/AlicatMFClib/tests  ${CMAKE_BINARY_DIR}/AlicatMFClib_tests)
add_subdirectory(${PROJECT_ROOT}/Pfeifferlib/tests    ${CMAKE_BINARY_DIR}/Pfeifferlib_tests)
add_subdirectory(shared)
