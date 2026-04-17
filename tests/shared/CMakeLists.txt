# Tests for SputterOS modules
#
# @author Ryan Massie (rmassie)
# @date 3/31/26

# ── SputterOS module tests ──────────────────────────────────────────────────

# SputteringCfg — pure header, no linker dependencies beyond gtest.
add_executable(test_sputtering_cfg test_sputtering_cfg.cpp)
target_include_directories(test_sputtering_cfg PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(test_sputtering_cfg PRIVATE GTest::gtest_main)

# SafetyMonitors — wraps PfeifferGauge/Pump which depend on pico stubs.
set(SAFETY_MONITOR_SRCS
    test_safety_monitors.cpp
    ${PROJECT_ROOT}/src/core0/PfeifferGauge.cpp
    ${PROJECT_ROOT}/src/core0/PfeifferPump.cpp
)
add_executable(test_safety_monitors ${SAFETY_MONITOR_SRCS})
target_include_directories(test_safety_monitors PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(test_safety_monitors PRIVATE GTest::gtest_main GTest::gmock pico_stubs)

# SputteringApp + ProcessPhases — needs test globals, device drivers, pico stubs,
# and SputterOS utility sources (NonBlockingStopwatch, TelemetryLogger).
set(SPUTTEROS_UTIL_SRCS
    ${PROJECT_ROOT}/lib/SputterOS/src/utils/NonBlockingStopwatch.cpp
    ${PROJECT_ROOT}/lib/SputterOS/src/utils/MemoryProfiler.cpp
    ${PROJECT_ROOT}/lib/SputterOS/src/utils/logging/TelemetryLogger.cpp
    ${PROJECT_ROOT}/lib/SputterOS/src/utils/logging/LightweightStringBuilder.cpp
    ${PROJECT_ROOT}/lib/SputterOS/src/utils/logging/ErrorLogger.cpp
)
set(APP_SRCS
    test_sputtering_app.cpp
    test_process_phases.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../mocks/test_globals.cpp
    ${PROJECT_ROOT}/src/SputteringApp.cpp
    ${PROJECT_ROOT}/src/logic/ProcessPhases.cpp
    ${PROJECT_ROOT}/src/core0/AlicatMFC.cpp
    ${PROJECT_ROOT}/src/core0/PfeifferGauge.cpp
    ${PROJECT_ROOT}/src/core0/PfeifferPump.cpp
    ${SPUTTEROS_UTIL_SRCS}
)
add_executable(test_sputtering_app ${APP_SRCS})
target_include_directories(test_sputtering_app PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(test_sputtering_app PRIVATE GTest::gtest_main GTest::gmock pico_stubs)

include(GoogleTest)
gtest_discover_tests(test_sputtering_cfg)
gtest_discover_tests(test_safety_monitors)
gtest_discover_tests(test_sputtering_app)
