/**
 * @file test_pfeiffer_lib.cpp
 * @brief Unit tests for PfeifferLib protocol formatting, parsing, and checksum.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/31/26
 */
#include <gtest/gtest.h>
#include "PfeifferLib.h"

// ── Helper: Build a valid Pfeiffer response string ──────────────────────────
// Frame: [address 3][action 2][paramNum 3][dataLen 3][data N][checksum 2]\r
// Checksum = (sum of ASCII of all fields except checksum) % 256

static std::string buildPfeifferFrame(const std::string &addr, const std::string &action,
                                      const std::string &paramNum, const std::string &data)
{
    // dataLen is the string length of data, zero-padded to 3 chars
    char dataLenStr[4];
    snprintf(dataLenStr, sizeof(dataLenStr), "%03d", static_cast<int>(data.size()));
    std::string dataLen(dataLenStr);

    // Compute checksum
    unsigned int sum = 0;
    for (char c : addr)     sum += static_cast<unsigned char>(c);
    for (char c : action)   sum += static_cast<unsigned char>(c);
    for (char c : paramNum) sum += static_cast<unsigned char>(c);
    for (char c : dataLen)  sum += static_cast<unsigned char>(c);
    for (char c : data)     sum += static_cast<unsigned char>(c);
    unsigned int checksum = sum % 256;

    char csStr[4];
    snprintf(csStr, sizeof(csStr), "%02d", checksum);

    return addr + action + paramNum + dataLen + data + std::string(csStr) + "\r";
}

// ── formatCommand tests ─────────────────────────────────────────────────────

TEST(PfeifferLibFormat, ReadCommand)
{
    PfeifferCommand cmd;
    cmd.address  = "001";
    cmd.action   = READ_PARAMETER; // "00"
    cmd.paramNum = "740";
    cmd.data     = QUERY_DATA_STR; // "=?"

    bool valid = false;
    std::string result = PfeifferLib::formatCommand(&cmd, &valid);

    // Should contain the address, action, paramNum, dataLen, data, checksum, \r
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result.back(), '\r');

    // The data length for "=?" is 2, so dataLen should be "2"
    EXPECT_NE(result.find("001"), std::string::npos);
    EXPECT_NE(result.find("00"), std::string::npos);
    EXPECT_NE(result.find("740"), std::string::npos);
}

TEST(PfeifferLibFormat, NullPointer)
{
    bool valid = true;
    std::string result = PfeifferLib::formatCommand(nullptr, &valid);

    EXPECT_FALSE(valid);
    EXPECT_EQ(result, "");
}

TEST(PfeifferLibFormat, DataLengthCalculated)
{
    PfeifferCommand cmd;
    cmd.address  = "001";
    cmd.action   = DATA_RESPONSE; // "10"
    cmd.paramNum = "010";
    cmd.data     = "000001"; // 6 chars

    PfeifferLib::formatCommand(&cmd, nullptr);

    // After formatting, dataLen should be set to "6"
    EXPECT_EQ(cmd.dataLen, "6");
}

// ── decryptResponse tests ───────────────────────────────────────────────────

TEST(PfeifferLibDecrypt, ValidResponse)
{
    // Build a valid frame: address 001, action 10, paramNum 309, data "001500"
    std::string frame = buildPfeifferFrame("001", "10", "309", "001500");

    PfeifferCommand cmd;
    bool valid = false;
    PfeifferLib::decryptResponse(frame, &cmd, &valid);

    EXPECT_TRUE(valid);
    EXPECT_EQ(cmd.address, "001");
    EXPECT_EQ(cmd.action, "10");
    EXPECT_EQ(cmd.paramNum, "309");
    EXPECT_EQ(cmd.data, "001500");
}

TEST(PfeifferLibDecrypt, ChecksumMismatch)
{
    std::string frame = buildPfeifferFrame("001", "10", "309", "001500");
    // Corrupt the checksum (last 3 chars before \r are checksum + \r)
    frame[frame.size() - 2] = '0';
    frame[frame.size() - 3] = '0';

    PfeifferCommand cmd;
    bool valid = true;
    PfeifferLib::decryptResponse(frame, &cmd, &valid);

    EXPECT_FALSE(valid);
}

TEST(PfeifferLibDecrypt, TooShort)
{
    PfeifferCommand cmd;
    bool valid = true;
    PfeifferLib::decryptResponse("001", &cmd, &valid);

    EXPECT_FALSE(valid);
}

TEST(PfeifferLibDecrypt, NullPointer)
{
    bool valid = true;
    PfeifferLib::decryptResponse("something", nullptr, &valid);

    EXPECT_FALSE(valid);
}

TEST(PfeifferLibDecrypt, MissingCarriageReturn)
{
    std::string frame = buildPfeifferFrame("001", "10", "309", "001500");
    // Remove trailing \r
    frame.pop_back();

    PfeifferCommand cmd;
    bool valid = true;
    PfeifferLib::decryptResponse(frame, &cmd, &valid);

    EXPECT_FALSE(valid);
}

// ── isRead tests ────────────────────────────────────────────────────────────

TEST(PfeifferLibIsRead, ReadAction)
{
    PfeifferCommand cmd;
    cmd.action = READ_PARAMETER; // "00"

    EXPECT_TRUE(PfeifferLib::isRead(&cmd));
}

TEST(PfeifferLibIsRead, NonReadAction)
{
    PfeifferCommand cmd;
    cmd.action = DATA_RESPONSE; // "10"

    EXPECT_FALSE(PfeifferLib::isRead(&cmd));
}

TEST(PfeifferLibIsRead, NullPointer)
{
    bool valid = true;
    EXPECT_FALSE(PfeifferLib::isRead(nullptr, &valid));
    EXPECT_FALSE(valid);
}
