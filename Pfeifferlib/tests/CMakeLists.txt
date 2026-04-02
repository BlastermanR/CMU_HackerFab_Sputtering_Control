# Unit tests for Pfeifferlib
# Can be built standalone or included from the top-level test build.
#
# Standalone usage:
#   cmake -B build_tests -S Pfeifferlib/tests -G Ninja
#   ninja -C build_tests
#   ctest --test-dir build_tests --output-on-failure
#
# @author Ryan Massie (rmassie)
# @date 3/31/26

cmake_minimum_required(VERSION 3.14)

# ── Standalone bootstrap (skipped when included from parent) ─────────────────
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(Pfeifferlib_Tests CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.15.2
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    enable_testing()

    # Resolve library root relative to this file
    set(LIB_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
    set(COMMON_INCLUDE_DIRS
        ${LIB_ROOT}/include
        ${LIB_ROOT}/include/Devices
    )
endif()

# ── Test targets ─────────────────────────────────────────────────────────────
add_executable(test_pfeiffer_lib test_pfeiffer_lib.cpp)
target_include_directories(test_pfeiffer_lib PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(test_pfeiffer_lib PRIVATE GTest::gtest_main)

add_executable(test_pfeiffer_device test_pfeiffer_device.cpp)
target_include_directories(test_pfeiffer_device PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(test_pfeiffer_device PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_pfeiffer_lib)
gtest_discover_tests(test_pfeiffer_device)
