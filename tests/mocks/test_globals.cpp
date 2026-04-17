/**
 * @file test_globals.cpp
 * @brief Defines test-only global device instances for unit testing.
 *
 * These replace the production globals from GlobalDevices.cpp so that
 * SputteringApp and ProcessPhases can be tested on the host without
 * real UART hardware.
 *
 * @author Ryan Massie (rmassie)
 * @date 4/16/26
 */

#include "test_globals.h"

// Mock serial devices (NiceMock suppresses uninteresting-call warnings).
testing::NiceMock<MockSerialDevice> mockAlicatSerial1;
testing::NiceMock<MockSerialDevice> mockAlicatSerial2;
testing::NiceMock<MockSerialDevice> mockGaugeSerial;
testing::NiceMock<MockSerialDevice> mockPumpSerial;

// Production-compatible global device instances backed by mocks.
AlicatMFC   mfc1(&mockAlicatSerial1);
AlicatMFC   mfc2(&mockAlicatSerial2);
PfeifferGauge gauge(&mockGaugeSerial);
PfeifferPump  pump(&mockPumpSerial);
