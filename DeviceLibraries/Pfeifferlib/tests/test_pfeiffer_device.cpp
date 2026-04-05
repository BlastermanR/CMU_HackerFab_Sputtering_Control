/**
 * @file test_pfeiffer_device.cpp
 * @brief Unit tests for PfeifferDevice CRTP, MPT200, and TC110DriveUnit parameter logic.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/31/26
 */
#include "Devices/MPT200.h"
#include "Devices/TC110DriveUnit.h"
#include "PfeifferDevice.h"
#include <gtest/gtest.h>

using namespace Pfeiffer;

// ═══════════════════════════════════════════════════════════════════════════
// MPT200 Tests
// ═══════════════════════════════════════════════════════════════════════════

class MPT200Test : public ::testing::Test
{
  protected:
    MPT200 gauge{1}; // address 1
};

TEST_F(MPT200Test, GetParamDefValidParam)
{
    const PfeifferParamDef *def = MPT200::getParamDef(740); // Pressure
    ASSERT_NE(def, nullptr);
    EXPECT_EQ(def->number, 740);
    EXPECT_STREQ(def->name, "Pressure");
}

TEST_F(MPT200Test, GetParamDefInvalidParam)
{
    const PfeifferParamDef *def = MPT200::getParamDef(9999);
    EXPECT_EQ(def, nullptr);
}

TEST_F(MPT200Test, CreateReadCommandPressure)
{
    PfeifferCommand cmd;
    bool            result = gauge.createReadCommand(static_cast<uint16_t>(MPT200Cmd::Pressure), cmd);

    EXPECT_TRUE(result);
    EXPECT_EQ(cmd.address, "001");
    EXPECT_EQ(cmd.action, READ_PARAMETER);
    EXPECT_EQ(cmd.paramNum, "740");
    EXPECT_EQ(cmd.data, QUERY_DATA_STR);
}

TEST_F(MPT200Test, CreateReadCommandRefusesWriteOnly)
{
    PfeifferCommand cmd;
    // PressureSetPoint (741) is WRITE_ONLY
    bool result = gauge.createReadCommand(static_cast<uint16_t>(MPT200Cmd::PressureSetPoint), cmd);

    EXPECT_FALSE(result);
}

TEST_F(MPT200Test, CreateReadCommandUnknownParam)
{
    PfeifferCommand cmd;
    bool            result = gauge.createReadCommand(9999, cmd);

    EXPECT_FALSE(result);
}

TEST_F(MPT200Test, CreateWriteCommandSensorOnOff)
{
    PfeifferCommand cmd;
    // SensorOnOff (41) is READ_WRITE, data type 6 (boolean_new)
    bool result = gauge.createWriteCommand(static_cast<uint16_t>(MPT200Cmd::SensorOnOff), 1.0, cmd);

    EXPECT_TRUE(result);
    EXPECT_EQ(cmd.address, "001");
    EXPECT_EQ(cmd.action, DATA_RESPONSE);
    EXPECT_EQ(cmd.paramNum, "041");
    EXPECT_EQ(cmd.data, "ON     "); // Type 6: boolean_new, 7 chars space-padded
}

TEST_F(MPT200Test, CreateWriteCommandOutOfBounds)
{
    PfeifferCommand cmd;
    // SensorOnOff max is 1, try 5
    bool result = gauge.createWriteCommand(static_cast<uint16_t>(MPT200Cmd::SensorOnOff), 5.0, cmd);

    EXPECT_FALSE(result);
}

TEST_F(MPT200Test, CreateWriteCommandRefusesReadOnly)
{
    PfeifferCommand cmd;
    // Error (303) is READ_ONLY
    bool result = gauge.createWriteCommand(static_cast<uint16_t>(MPT200Cmd::Error), 0.0, cmd);

    EXPECT_FALSE(result);
}

TEST_F(MPT200Test, AddressSetGet)
{
    gauge.setAddress(5);
    EXPECT_EQ(gauge.getAddress(), 5);

    PfeifferCommand cmd;
    gauge.createReadCommand(static_cast<uint16_t>(MPT200Cmd::Pressure), cmd);
    EXPECT_EQ(cmd.address, "005");
}

// ═══════════════════════════════════════════════════════════════════════════
// TC110DriveUnit Tests
// ═══════════════════════════════════════════════════════════════════════════

class TC110Test : public ::testing::Test
{
  protected:
    TC110DriveUnit pump{1}; // address 1
};

TEST_F(TC110Test, GetParamDefPumpStation)
{
    const PfeifferParamDef *def = TC110DriveUnit::getParamDef(10); // PumpgStatn
    ASSERT_NE(def, nullptr);
    EXPECT_EQ(def->number, 10);
    EXPECT_STREQ(def->name, "PumpgStatn");
    EXPECT_EQ(def->access, AccessType::READ_WRITE);
}

TEST_F(TC110Test, GetParamDefStatusReadOnly)
{
    const PfeifferParamDef *def = TC110DriveUnit::getParamDef(309); // ActualSpd
    ASSERT_NE(def, nullptr);
    EXPECT_EQ(def->access, AccessType::READ_ONLY);
}

TEST_F(TC110Test, CreateReadCommandActualSpeed)
{
    PfeifferCommand cmd;
    bool            result = pump.createReadCommand(static_cast<uint16_t>(TC110Cmd::ActualSpd_Hz), cmd);

    EXPECT_TRUE(result);
    EXPECT_EQ(cmd.paramNum, "309");
}

TEST_F(TC110Test, CreateWriteCommandErrorAckn)
{
    PfeifferCommand cmd;
    // ErrorAckn (9) is WRITE_ONLY, range 1-1
    bool result = pump.createWriteCommand(static_cast<uint16_t>(TC110Cmd::ErrorAckn), 1.0, cmd);

    EXPECT_TRUE(result);
    EXPECT_EQ(cmd.paramNum, "009");
}

TEST_F(TC110Test, CreateWriteCommandBoundsCheck)
{
    PfeifferCommand cmd;
    // RUTimeSVal (700): range 1-120
    EXPECT_TRUE(pump.createWriteCommand(static_cast<uint16_t>(TC110Cmd::RUTimeSVal), 60.0, cmd));
    EXPECT_FALSE(pump.createWriteCommand(static_cast<uint16_t>(TC110Cmd::RUTimeSVal), 0.0, cmd));   // below min
    EXPECT_FALSE(pump.createWriteCommand(static_cast<uint16_t>(TC110Cmd::RUTimeSVal), 200.0, cmd)); // above max
}

TEST_F(TC110Test, CreateWriteCommandRefusesReadOnly)
{
    PfeifferCommand cmd;
    // ActualSpd_Hz (309) is READ_ONLY
    bool result = pump.createWriteCommand(static_cast<uint16_t>(TC110Cmd::ActualSpd_Hz), 100.0, cmd);

    EXPECT_FALSE(result);
}

TEST_F(TC110Test, GetParamDefUnknown) { EXPECT_EQ(TC110DriveUnit::getParamDef(5555), nullptr); }

// ═══════════════════════════════════════════════════════════════════════════
// Data Type Encoding Tests (via createWriteCommand)
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(TC110Test, WriteCommandBooleanTrue)
{
    PfeifferCommand cmd;
    // PumpgStatn (10) is type 0 (boolean), value 1 → "111111"
    EXPECT_TRUE(pump.createWriteCommand(static_cast<uint16_t>(TC110Cmd::PumpgStatn), 1.0, cmd));
    EXPECT_EQ(cmd.data, "111111");
}

TEST_F(TC110Test, WriteCommandBooleanFalse)
{
    PfeifferCommand cmd;
    // PumpgStatn (10) is type 0 (boolean), value 0 → "000000"
    EXPECT_TRUE(pump.createWriteCommand(static_cast<uint16_t>(TC110Cmd::PumpgStatn), 0.0, cmd));
    EXPECT_EQ(cmd.data, "000000");
}

TEST_F(TC110Test, WriteCommandShortInt)
{
    PfeifferCommand cmd;
    // GasMode (27) is type 7 (u_short_int), value 2 → "002"
    EXPECT_TRUE(pump.createWriteCommand(static_cast<uint16_t>(TC110Cmd::GasMode), 2.0, cmd));
    EXPECT_EQ(cmd.data, "002");
}

TEST_F(TC110Test, WriteCommandFixedPoint)
{
    PfeifferCommand cmd;
    // SpdSVal (707) is type 2 (u_real), value 50.0 → 50.0 * 100 = 5000 → "005000"
    EXPECT_TRUE(pump.createWriteCommand(static_cast<uint16_t>(TC110Cmd::SpdSVal), 50.0, cmd));
    EXPECT_EQ(cmd.data, "005000");
}

TEST_F(TC110Test, WriteCommandInteger)
{
    PfeifferCommand cmd;
    // RUTimeSVal (700) is type 1 (u_integer), value 60 → "000060"
    EXPECT_TRUE(pump.createWriteCommand(static_cast<uint16_t>(TC110Cmd::RUTimeSVal), 60.0, cmd));
    EXPECT_EQ(cmd.data, "000060");
}

TEST_F(MPT200Test, WriteCommandBooleanNewOff)
{
    PfeifferCommand cmd;
    // SensorOnOff (41) is type 6, value 0 → "OFF    "
    EXPECT_TRUE(gauge.createWriteCommand(static_cast<uint16_t>(MPT200Cmd::SensorOnOff), 0.0, cmd));
    EXPECT_EQ(cmd.data, "OFF    ");
}

// ═══════════════════════════════════════════════════════════════════════════
// parseResponseValue Tests
// ═══════════════════════════════════════════════════════════════════════════

TEST_F(TC110Test, ParseResponseValueInteger)
{
    PfeifferCommand response;
    response.data = "000633";

    double value = 0.0;
    EXPECT_TRUE(pump.parseResponseValue(static_cast<uint16_t>(TC110Cmd::ActualSpd_Hz), response, value));
    EXPECT_DOUBLE_EQ(value, 633.0);
}

TEST_F(TC110Test, ParseResponseValueBoolean)
{
    PfeifferCommand response;
    response.data = "111111";

    double value = 0.0;
    EXPECT_TRUE(pump.parseResponseValue(static_cast<uint16_t>(TC110Cmd::PumpgStatn), response, value));
    EXPECT_DOUBLE_EQ(value, 1.0);
}

TEST_F(TC110Test, ParseResponseValueFixedPoint)
{
    PfeifferCommand response;
    response.data = "001571";

    double value = 0.0;
    EXPECT_TRUE(pump.parseResponseValue(static_cast<uint16_t>(TC110Cmd::DrvCurrent), response, value));
    EXPECT_DOUBLE_EQ(value, 15.71);
}

TEST_F(TC110Test, ParseResponseValueUnknownParam)
{
    PfeifferCommand response;
    response.data = "000000";

    double value = 0.0;
    EXPECT_FALSE(pump.parseResponseValue(9999, response, value));
}

TEST_F(MPT200Test, ParseResponseValueBooleanNew)
{
    PfeifferCommand response;
    response.data = "ON     ";

    double value = 0.0;
    EXPECT_TRUE(gauge.parseResponseValue(static_cast<uint16_t>(MPT200Cmd::SensorOnOff), response, value));
    EXPECT_DOUBLE_EQ(value, 1.0);
}
