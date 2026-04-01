/**
 * @file test_pfieffer_device.cpp
 * @brief Unit tests for PfiefferDevice CRTP, MPT200, and TC110DriveUnit parameter logic.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/31/26
 */
#include <gtest/gtest.h>
#include "PfiefferDevice.h"
#include "Devices/MPT200.h"
#include "Devices/TC110DriveUnit.h"

using namespace Pfieffer;

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
    const PfiefferParamDef *def = MPT200::getParamDef(740); // Pressure
    ASSERT_NE(def, nullptr);
    EXPECT_EQ(def->number, 740);
    EXPECT_STREQ(def->name, "Pressure");
}

TEST_F(MPT200Test, GetParamDefInvalidParam)
{
    const PfiefferParamDef *def = MPT200::getParamDef(9999);
    EXPECT_EQ(def, nullptr);
}

TEST_F(MPT200Test, CreateReadCommandPressure)
{
    PfiefferCommand cmd;
    bool result = gauge.createReadCommand(static_cast<uint16_t>(MPT200Cmd::Pressure), cmd);

    EXPECT_TRUE(result);
    EXPECT_EQ(cmd.address, "001");
    EXPECT_EQ(cmd.action, READ_PARAMETER);
    EXPECT_EQ(cmd.paramNum, "740");
    EXPECT_EQ(cmd.data, QUERY_DATA_STR);
}

TEST_F(MPT200Test, CreateReadCommandRefusesWriteOnly)
{
    PfiefferCommand cmd;
    // PressureSetPoint (741) is WRITE_ONLY
    bool result = gauge.createReadCommand(static_cast<uint16_t>(MPT200Cmd::PressureSetPoint), cmd);

    EXPECT_FALSE(result);
}

TEST_F(MPT200Test, CreateReadCommandUnknownParam)
{
    PfiefferCommand cmd;
    bool result = gauge.createReadCommand(9999, cmd);

    EXPECT_FALSE(result);
}

TEST_F(MPT200Test, CreateWriteCommandSensorOnOff)
{
    PfiefferCommand cmd;
    // SensorOnOff (41) is READ_WRITE, range 0-1
    bool result = gauge.createWriteCommand(static_cast<uint16_t>(MPT200Cmd::SensorOnOff), 1.0, cmd);

    EXPECT_TRUE(result);
    EXPECT_EQ(cmd.address, "001");
    EXPECT_EQ(cmd.action, DATA_RESPONSE);
    EXPECT_EQ(cmd.paramNum, "041");
    EXPECT_EQ(cmd.data, "000001");
}

TEST_F(MPT200Test, CreateWriteCommandOutOfBounds)
{
    PfiefferCommand cmd;
    // SensorOnOff max is 1, try 5
    bool result = gauge.createWriteCommand(static_cast<uint16_t>(MPT200Cmd::SensorOnOff), 5.0, cmd);

    EXPECT_FALSE(result);
}

TEST_F(MPT200Test, CreateWriteCommandRefusesReadOnly)
{
    PfiefferCommand cmd;
    // Error (303) is READ_ONLY
    bool result = gauge.createWriteCommand(static_cast<uint16_t>(MPT200Cmd::Error), 0.0, cmd);

    EXPECT_FALSE(result);
}

TEST_F(MPT200Test, AddressSetGet)
{
    gauge.setAddress(5);
    EXPECT_EQ(gauge.getAddress(), 5);

    PfiefferCommand cmd;
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
    const PfiefferParamDef *def = TC110DriveUnit::getParamDef(10); // PumpgStatn
    ASSERT_NE(def, nullptr);
    EXPECT_EQ(def->number, 10);
    EXPECT_STREQ(def->name, "PumpgStatn");
    EXPECT_EQ(def->access, AccessType::READ_WRITE);
}

TEST_F(TC110Test, GetParamDefStatusReadOnly)
{
    const PfiefferParamDef *def = TC110DriveUnit::getParamDef(309); // ActualSpd
    ASSERT_NE(def, nullptr);
    EXPECT_EQ(def->access, AccessType::READ_ONLY);
}

TEST_F(TC110Test, CreateReadCommandActualSpeed)
{
    PfiefferCommand cmd;
    bool result = pump.createReadCommand(static_cast<uint16_t>(TC110Cmd::ActualSpd_Hz), cmd);

    EXPECT_TRUE(result);
    EXPECT_EQ(cmd.paramNum, "309");
}

TEST_F(TC110Test, CreateWriteCommandErrorAckn)
{
    PfiefferCommand cmd;
    // ErrorAckn (9) is WRITE_ONLY, range 1-1
    bool result = pump.createWriteCommand(static_cast<uint16_t>(TC110Cmd::ErrorAckn), 1.0, cmd);

    EXPECT_TRUE(result);
    EXPECT_EQ(cmd.paramNum, "009");
}

TEST_F(TC110Test, CreateWriteCommandBoundsCheck)
{
    PfiefferCommand cmd;
    // RUTimeSVal (700): range 1-120
    EXPECT_TRUE(pump.createWriteCommand(static_cast<uint16_t>(TC110Cmd::RUTimeSVal), 60.0, cmd));
    EXPECT_FALSE(pump.createWriteCommand(static_cast<uint16_t>(TC110Cmd::RUTimeSVal), 0.0, cmd));  // below min
    EXPECT_FALSE(pump.createWriteCommand(static_cast<uint16_t>(TC110Cmd::RUTimeSVal), 200.0, cmd)); // above max
}

TEST_F(TC110Test, CreateWriteCommandRefusesReadOnly)
{
    PfiefferCommand cmd;
    // ActualSpd_Hz (309) is READ_ONLY
    bool result = pump.createWriteCommand(static_cast<uint16_t>(TC110Cmd::ActualSpd_Hz), 100.0, cmd);

    EXPECT_FALSE(result);
}

TEST_F(TC110Test, GetParamDefUnknown)
{
    EXPECT_EQ(TC110DriveUnit::getParamDef(5555), nullptr);
}
