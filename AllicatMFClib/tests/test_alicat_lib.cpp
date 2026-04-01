/**
 * @file test_alicat_lib.cpp
 * @brief Unit tests for AlicatLib protocol formatting and parsing.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/31/26
 */
#include <gtest/gtest.h>
#include "AlicatLib.h"

// ── formatCommand tests ─────────────────────────────────────────────────────

TEST(AlicatLibFormat, PollCommand)
{
    AlicatCommand cmd;
    cmd.id     = 'A';
    cmd.action = ALICAT_POLL;
    cmd.data   = "";

    bool valid = false;
    std::string result = AlicatLib::formatCommand(&cmd, &valid);

    EXPECT_TRUE(valid);
    EXPECT_EQ(result, "A\r");
}

TEST(AlicatLibFormat, SetpointCommand)
{
    AlicatCommand cmd;
    cmd.id     = 'A';
    cmd.action = ALICAT_CHANGE_SETPOINT;
    cmd.data   = "5.0";

    bool valid = false;
    std::string result = AlicatLib::formatCommand(&cmd, &valid);

    EXPECT_TRUE(valid);
    EXPECT_EQ(result, "AS5.0\r");
}

TEST(AlicatLibFormat, SetGasCommand)
{
    AlicatCommand cmd;
    cmd.id     = 'B';
    cmd.action = ALICAT_SET_GAS;
    cmd.data   = "3";

    bool valid = false;
    std::string result = AlicatLib::formatCommand(&cmd, &valid);

    EXPECT_TRUE(valid);
    EXPECT_EQ(result, "BG3\r");
}

TEST(AlicatLibFormat, TareFlowCommand)
{
    AlicatCommand cmd;
    cmd.id     = 'A';
    cmd.action = ALICAT_TARE_FLOW;
    cmd.data   = "";

    bool valid = false;
    std::string result = AlicatLib::formatCommand(&cmd, &valid);

    EXPECT_TRUE(valid);
    EXPECT_EQ(result, "AV\r");
}

TEST(AlicatLibFormat, NullCommandPointer)
{
    bool valid = true;
    std::string result = AlicatLib::formatCommand(nullptr, &valid);

    EXPECT_FALSE(valid);
    EXPECT_EQ(result, "");
}

TEST(AlicatLibFormat, ValidWithoutOptionalBool)
{
    AlicatCommand cmd;
    cmd.id     = 'A';
    cmd.action = ALICAT_POLL;
    cmd.data   = "";

    // Should not crash when valid pointer is nullptr
    std::string result = AlicatLib::formatCommand(&cmd, nullptr);
    EXPECT_EQ(result, "A\r");
}

// ── parseResponse tests ─────────────────────────────────────────────────────

TEST(AlicatLibParse, StandardDataFrame)
{
    // Standard Alicat response: ID Pressure Temp VolFlow MassFlow Setpoint Gas
    std::string response = "A 14.70 25.00 0.50 0.48 1.00 Air";

    AlicatDataFrame frame;
    bool valid = false;
    AlicatLib::parseResponse(response, &frame, &valid);

    EXPECT_TRUE(valid);
    EXPECT_EQ(frame.id, 'A');
    EXPECT_DOUBLE_EQ(frame.pressure, 14.70);
    EXPECT_DOUBLE_EQ(frame.temperature, 25.00);
    EXPECT_DOUBLE_EQ(frame.volumetricFlow, 0.50);
    EXPECT_DOUBLE_EQ(frame.massFlow, 0.48);
    EXPECT_DOUBLE_EQ(frame.setpoint, 1.00);
    EXPECT_EQ(frame.gasType, "Air");
}

TEST(AlicatLibParse, ZeroValues)
{
    std::string response = "B 0.00 0.00 0.00 0.00 0.00 Ar";

    AlicatDataFrame frame;
    bool valid = false;
    AlicatLib::parseResponse(response, &frame, &valid);

    EXPECT_TRUE(valid);
    EXPECT_EQ(frame.id, 'B');
    EXPECT_DOUBLE_EQ(frame.pressure, 0.0);
    EXPECT_DOUBLE_EQ(frame.massFlow, 0.0);
    EXPECT_EQ(frame.gasType, "Ar");
}

TEST(AlicatLibParse, NegativeValues)
{
    std::string response = "A -0.10 22.50 -0.01 -0.005 0.00 N2";

    AlicatDataFrame frame;
    bool valid = false;
    AlicatLib::parseResponse(response, &frame, &valid);

    EXPECT_TRUE(valid);
    EXPECT_DOUBLE_EQ(frame.pressure, -0.10);
    EXPECT_DOUBLE_EQ(frame.massFlow, -0.005);
    EXPECT_EQ(frame.gasType, "N2");
}

TEST(AlicatLibParse, TooFewTokens)
{
    std::string response = "A 14.70 25.00";

    AlicatDataFrame frame;
    bool valid = true; // start true to verify it gets cleared
    AlicatLib::parseResponse(response, &frame, &valid);

    EXPECT_FALSE(valid);
}

TEST(AlicatLibParse, EmptyResponse)
{
    AlicatDataFrame frame;
    bool valid = true;
    AlicatLib::parseResponse("", &frame, &valid);

    EXPECT_FALSE(valid);
}

TEST(AlicatLibParse, NullFramePointer)
{
    bool valid = true;
    AlicatLib::parseResponse("A 14.70 25.00 0.50 0.48 1.00 Air", nullptr, &valid);

    EXPECT_FALSE(valid);
}

TEST(AlicatLibParse, NullValidPointer)
{
    AlicatDataFrame frame;
    // Should not crash when valid pointer is nullptr
    AlicatLib::parseResponse("A 14.70 25.00 0.50 0.48 1.00 Air", &frame, nullptr);
    EXPECT_EQ(frame.id, 'A');
    EXPECT_DOUBLE_EQ(frame.pressure, 14.70);
}
