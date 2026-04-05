/**
 * @file test_alicat_lib.cpp
 * @brief Unit tests for AlicatLib protocol formatting and parsing.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/31/26
 */
#include "AlicatErrors.h"
#include "AlicatGases.h"
#include "AlicatLib.h"
#include <gtest/gtest.h>

// ── formatCommand tests ─────────────────────────────────────────────────────

TEST(AlicatLibFormat, PollCommand)
{
    AlicatCommand cmd;
    cmd.id     = 'A';
    cmd.action = ALICAT_POLL;
    cmd.data   = "";

    bool        valid  = false;
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

    bool        valid  = false;
    std::string result = AlicatLib::formatCommand(&cmd, &valid);

    EXPECT_TRUE(valid);
    EXPECT_EQ(result, "AS 5.0\r");
}

TEST(AlicatLibFormat, SetGasCommand)
{
    AlicatCommand cmd;
    cmd.id     = 'B';
    cmd.action = ALICAT_SET_GAS;
    cmd.data   = "3";

    bool        valid  = false;
    std::string result = AlicatLib::formatCommand(&cmd, &valid);

    EXPECT_TRUE(valid);
    EXPECT_EQ(result, "BG 3\r");
}

TEST(AlicatLibFormat, SetGasArgon)
{
    AlicatCommand cmd;
    cmd.id     = 'A';
    cmd.action = ALICAT_SET_GAS;
    cmd.data   = std::to_string(ALICAT_GAS_AR);

    bool        valid  = false;
    std::string result = AlicatLib::formatCommand(&cmd, &valid);

    EXPECT_TRUE(valid);
    EXPECT_EQ(result, "AG 1\r");
    EXPECT_STREQ(AlicatLib::getGasShortName(ALICAT_GAS_AR), "Ar");
    EXPECT_STREQ(AlicatLib::getGasLongName(ALICAT_GAS_AR), "Argon");
    EXPECT_TRUE(AlicatLib::isValidGasId(ALICAT_GAS_AR));
}

TEST(AlicatLibFormat, TareFlowCommand)
{
    AlicatCommand cmd;
    cmd.id     = 'A';
    cmd.action = ALICAT_TARE_FLOW;
    cmd.data   = "";

    bool        valid  = false;
    std::string result = AlicatLib::formatCommand(&cmd, &valid);

    EXPECT_TRUE(valid);
    EXPECT_EQ(result, "AV\r");
}

TEST(AlicatLibFormat, NullCommandPointer)
{
    bool        valid  = true;
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
    bool            valid = false;
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
    bool            valid = false;
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
    bool            valid = false;
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
    bool            valid = true; // start true to verify it gets cleared
    AlicatLib::parseResponse(response, &frame, &valid);

    EXPECT_FALSE(valid);
}

TEST(AlicatLibParse, EmptyResponse)
{
    AlicatDataFrame frame;
    bool            valid = true;
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

// ── Gas validation tests ────────────────────────────────────────────────────

// Every entry in the gas table must be found as valid and return matching names.
TEST(AlicatGasValidation, AllTableEntriesValid)
{
    for (std::size_t i = 0; i < ALICAT_GAS_TABLE_SIZE; ++i)
    {
        const uint8_t id = ALICAT_GAS_TABLE[i].id;
        SCOPED_TRACE(::testing::Message()
                     << "gas id=" << static_cast<unsigned>(id) << " (" << ALICAT_GAS_TABLE[i].shortName << ")");
        EXPECT_TRUE(AlicatLib::isValidGasId(id));
        EXPECT_STREQ(AlicatLib::getGasShortName(id), ALICAT_GAS_TABLE[i].shortName);
        EXPECT_STREQ(AlicatLib::getGasLongName(id), ALICAT_GAS_TABLE[i].longName);
    }
}

// IDs that fall in the known gaps must be rejected.
TEST(AlicatGasValidation, GapIdsAreInvalid)
{
    // Gap 37-79
    for (uint8_t id = 37; id <= 79; ++id)
    {
        SCOPED_TRACE(::testing::Message() << "id=" << static_cast<unsigned>(id));
        EXPECT_FALSE(AlicatLib::isValidGasId(id));
        EXPECT_EQ(AlicatLib::getGasShortName(id), nullptr);
        EXPECT_EQ(AlicatLib::getGasLongName(id), nullptr);
    }
    // Gap 87-99
    for (uint8_t id = 87; id <= 99; ++id)
    {
        SCOPED_TRACE(::testing::Message() << "id=" << static_cast<unsigned>(id));
        EXPECT_FALSE(AlicatLib::isValidGasId(id));
    }
    // Gap 118-139
    for (uint8_t id = 118; id <= 139; ++id)
    {
        SCOPED_TRACE(::testing::Message() << "id=" << static_cast<unsigned>(id));
        EXPECT_FALSE(AlicatLib::isValidGasId(id));
    }
    // Gap 207-209
    for (uint8_t id = 207; id <= 209; ++id)
    {
        SCOPED_TRACE(::testing::Message() << "id=" << static_cast<unsigned>(id));
        EXPECT_FALSE(AlicatLib::isValidGasId(id));
    }
    // Beyond last entry (210 is D-2, so 211+ are invalid)
    for (uint16_t id = 211; id <= 255; ++id)
    {
        SCOPED_TRACE(::testing::Message() << "id=" << id);
        EXPECT_FALSE(AlicatLib::isValidGasId(static_cast<uint8_t>(id)));
    }
}

// Spot-check well-known gases by their ALICAT_GAS_* defines.
TEST(AlicatGasValidation, KnownGasNames)
{
    EXPECT_STREQ(AlicatLib::getGasShortName(ALICAT_GAS_AIR), "Air");
    EXPECT_STREQ(AlicatLib::getGasLongName(ALICAT_GAS_AIR), "Air (Clean Dry)");

    EXPECT_STREQ(AlicatLib::getGasShortName(ALICAT_GAS_AR), "Ar");
    EXPECT_STREQ(AlicatLib::getGasLongName(ALICAT_GAS_AR), "Argon");

    EXPECT_STREQ(AlicatLib::getGasShortName(ALICAT_GAS_N2), "N2");
    EXPECT_STREQ(AlicatLib::getGasLongName(ALICAT_GAS_N2), "Nitrogen");

    EXPECT_STREQ(AlicatLib::getGasShortName(ALICAT_GAS_HE), "He");
    EXPECT_STREQ(AlicatLib::getGasLongName(ALICAT_GAS_HE), "Helium");

    EXPECT_STREQ(AlicatLib::getGasShortName(ALICAT_GAS_O2), "O2");
    EXPECT_STREQ(AlicatLib::getGasLongName(ALICAT_GAS_O2), "Oxygen");

    EXPECT_STREQ(AlicatLib::getGasShortName(ALICAT_GAS_SF6), "SF6");
    EXPECT_STREQ(AlicatLib::getGasLongName(ALICAT_GAS_SF6), "Sulfur Hexafluoride");

    EXPECT_STREQ(AlicatLib::getGasShortName(ALICAT_GAS_D_2), "D-2");
    EXPECT_STREQ(AlicatLib::getGasLongName(ALICAT_GAS_D_2), "Deuterium");
}

// ── Status/error code tests ──────────────────────────────────────────────────

TEST(AlicatStatusCodes, NoStatusInCleanFrame)
{
    std::string     response = "A 14.70 25.00 0.50 0.48 1.00 Ar";
    AlicatDataFrame frame;
    bool            valid = false;
    AlicatLib::parseResponse(response, &frame, &valid);

    EXPECT_TRUE(valid);
    EXPECT_TRUE(frame.statusCodes.empty());
}

TEST(AlicatStatusCodes, SingleStatusCode)
{
    std::string     response = "A 14.70 25.00 0.50 0.48 1.00 Ar MOV";
    AlicatDataFrame frame;
    bool            valid = false;
    AlicatLib::parseResponse(response, &frame, &valid);

    EXPECT_TRUE(valid);
    ASSERT_EQ(frame.statusCodes.size(), 1u);
    EXPECT_EQ(frame.statusCodes[0], "MOV");
}

TEST(AlicatStatusCodes, MultipleStatusCodes)
{
    std::string     response = "A 14.70 25.00 0.50 0.48 1.00 Ar POV TOV MOV";
    AlicatDataFrame frame;
    bool            valid = false;
    AlicatLib::parseResponse(response, &frame, &valid);

    EXPECT_TRUE(valid);
    ASSERT_EQ(frame.statusCodes.size(), 3u);
    EXPECT_EQ(frame.statusCodes[0], "POV");
    EXPECT_EQ(frame.statusCodes[1], "TOV");
    EXPECT_EQ(frame.statusCodes[2], "MOV");
}

TEST(AlicatStatusCodes, UnknownTrailingTokenIgnored)
{
    // "XYZ" is not a known status code and should be silently ignored
    std::string     response = "A 14.70 25.00 0.50 0.48 1.00 Ar XYZ";
    AlicatDataFrame frame;
    bool            valid = false;
    AlicatLib::parseResponse(response, &frame, &valid);

    EXPECT_TRUE(valid);
    EXPECT_TRUE(frame.statusCodes.empty());
}

TEST(AlicatStatusCodes, AllTableEntriesRecognised)
{
    for (std::size_t i = 0; i < ALICAT_STATUS_TABLE_SIZE; ++i)
    {
        SCOPED_TRACE(::testing::Message() << "code=" << ALICAT_STATUS_TABLE[i].code);
        EXPECT_TRUE(AlicatLib::isStatusCode(ALICAT_STATUS_TABLE[i].code));
        EXPECT_NE(AlicatLib::getStatusDescription(ALICAT_STATUS_TABLE[i].code), nullptr);
    }
}

TEST(AlicatStatusCodes, UnknownCodeRejected)
{
    EXPECT_FALSE(AlicatLib::isStatusCode("XYZ"));
    EXPECT_EQ(AlicatLib::getStatusDescription("XYZ"), nullptr);
    EXPECT_FALSE(AlicatLib::isStatusCode(nullptr));
}

TEST(AlicatStatusCodes, KnownDescriptions)
{
    EXPECT_STREQ(AlicatLib::getStatusDescription("MOV"), "Mass flow rate overage (outside measurable range)");
    EXPECT_STREQ(AlicatLib::getStatusDescription("POV"), "Pressure reading overage (outside measurable range)");
    EXPECT_STREQ(AlicatLib::getStatusDescription("VOV"), "Volumetric flow rate overage (outside measurable range)");
    EXPECT_STREQ(AlicatLib::getStatusDescription("HLD"), "Valve drive hold enabled (bypass active loop control)");
    EXPECT_STREQ(AlicatLib::getStatusDescription("ADC"), "ADC internal communication error");
}
