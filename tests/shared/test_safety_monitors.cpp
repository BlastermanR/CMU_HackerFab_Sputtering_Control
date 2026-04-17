/**
 * @file test_safety_monitors.cpp
 * @brief Unit tests for OverPressureMonitor and PumpHealthMonitor.
 *
 * Tests cover both safe and unsafe paths for OverPressureMonitor using
 * PfeifferGauge::setPressureForTest() (available under PICO_TESTING).
 *
 * @author Ryan Massie (rmassie)
 * @date 4/16/26
 */

#include "SafetyMonitors.h"
#include "MockSerialDevice.h"
#include <gtest/gtest.h>

// ===========================================================================
// OverPressureMonitor
// ===========================================================================

class OverPressureMonitorTest : public ::testing::Test
{
  protected:
    testing::NiceMock<MockSerialDevice> mockSerial;
    PfeifferGauge                       gauge{&mockSerial};
};

TEST_F(OverPressureMonitorTest, SafeWhenPressureBelowThreshold)
{
    gauge.setPressureForTest(5.0);
    OverPressureMonitor monitor(&gauge, 10.0);
    EXPECT_TRUE(monitor.isSafe());
}

TEST_F(OverPressureMonitorTest, SafeAtExactThreshold)
{
    gauge.setPressureForTest(10.0);
    OverPressureMonitor monitor(&gauge, 10.0);
    EXPECT_TRUE(monitor.isSafe()); // <= threshold is safe
}

TEST_F(OverPressureMonitorTest, UnsafeWhenPressureExceedsThreshold)
{
    gauge.setPressureForTest(10.1);
    OverPressureMonitor monitor(&gauge, 10.0);
    EXPECT_FALSE(monitor.isSafe());
}

TEST_F(OverPressureMonitorTest, UnsafeAtHighPressure)
{
    gauge.setPressureForTest(1013.0); // ~1 atm
    OverPressureMonitor monitor(&gauge);
    EXPECT_FALSE(monitor.isSafe());
}

TEST_F(OverPressureMonitorTest, SafeAtZeroPressure)
{
    gauge.setPressureForTest(0.0);
    OverPressureMonitor monitor(&gauge);
    EXPECT_TRUE(monitor.isSafe());
}

TEST_F(OverPressureMonitorTest, CustomThresholdIsHonoured)
{
    gauge.setPressureForTest(0.5);
    OverPressureMonitor tightMonitor(&gauge, 0.001);
    OverPressureMonitor looseMonitor(&gauge, 1.0);

    EXPECT_FALSE(tightMonitor.isSafe());
    EXPECT_TRUE(looseMonitor.isSafe());
}

TEST_F(OverPressureMonitorTest, NameIsCorrect)
{
    OverPressureMonitor monitor(&gauge);
    EXPECT_STREQ(monitor.name(), "OverPressureMonitor");
}

// ===========================================================================
// PumpHealthMonitor
// ===========================================================================

class PumpHealthMonitorTest : public ::testing::Test
{
  protected:
    testing::NiceMock<MockSerialDevice> mockSerial;
    PfeifferPump                        pump{&mockSerial};
};

TEST_F(PumpHealthMonitorTest, SafeWhenNoError)
{
    pump.setErrorForTest(false);
    PumpHealthMonitor monitor(&pump);
    EXPECT_TRUE(monitor.isSafe());
}

TEST_F(PumpHealthMonitorTest, UnsafeWhenPumpHasError)
{
    pump.setErrorForTest(true);
    PumpHealthMonitor monitor(&pump);
    EXPECT_FALSE(monitor.isSafe());
}

TEST_F(PumpHealthMonitorTest, NameIsCorrect)
{
    PumpHealthMonitor monitor(&pump);
    EXPECT_STREQ(monitor.name(), "PumpHealthMonitor");
}
