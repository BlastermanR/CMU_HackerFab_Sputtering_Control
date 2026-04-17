/**
 * @file test_globals.h
 * @brief Test-only global device instances using MockSerialDevice.
 *
 * Replaces the production `GlobalDevices.cpp` definitions for host-side
 * unit tests. Each device is backed by a NiceMock<MockSerialDevice> so
 * UART calls are silently ignored.
 *
 * Include this header in exactly ONE translation unit per test executable
 * that links against sources referencing `mfc1`, `mfc2`, `gauge`, `pump`.
 *
 * @author Ryan Massie (rmassie)
 * @date 4/16/26
 */
#ifndef TEST_GLOBALS_H
#define TEST_GLOBALS_H

#include "MockSerialDevice.h"
#include "AlicatMFC.h"
#include "PfeifferGauge.h"
#include "PfeifferPump.h"

// Global mock serial devices — defined in test_globals.cpp.
extern testing::NiceMock<MockSerialDevice> mockAlicatSerial1;
extern testing::NiceMock<MockSerialDevice> mockAlicatSerial2;
extern testing::NiceMock<MockSerialDevice> mockGaugeSerial;
extern testing::NiceMock<MockSerialDevice> mockPumpSerial;

#endif // TEST_GLOBALS_H
