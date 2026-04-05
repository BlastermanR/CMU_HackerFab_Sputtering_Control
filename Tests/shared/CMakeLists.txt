# Tests for shared inter-core logic and message types
#
# @author Ryan Massie (rmassie)
# @date 3/31/26

# Compile Intercore.cpp from the main project (defines statusReg, queues)
set(INTERCORE_SRC ${PROJECT_ROOT}/src/shared/Intercore.cpp)

add_executable(test_intercore test_intercore.cpp ${INTERCORE_SRC})
target_include_directories(test_intercore PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(test_intercore PRIVATE GTest::gtest_main pico_stubs)

add_executable(test_messages test_messages.cpp)
target_include_directories(test_messages PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(test_messages PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_intercore)
gtest_discover_tests(test_messages)
