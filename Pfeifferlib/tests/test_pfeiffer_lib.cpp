/**
 * @file test_pfeiffer_lib.cpp
 * @brief Unit tests for PfeifferLib protocol formatting, parsing, and checksum.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/31/26
 */
#include "PfeifferLib.h"
#include <gtest/gtest.h>

// ── Helper: Build a valid Pfeiffer response string ──────────────────────────
// Frame: [address 3][action 2][paramNum 3][dataLen 2][data N][checksum 3]\r
// Checksum = (sum of ASCII of all fields except checksum) % 256

static std::string buildPfeifferFrame(const std::string &addr, const std::string &action, const std::string &paramNum,
                                      const std::string &data)
{
    // dataLen is the string length of data, zero-padded to 2 chars
    char dataLenStr[3];
    snprintf(dataLenStr, sizeof(dataLenStr), "%02d", static_cast<int>(data.size()));
    std::string dataLen(dataLenStr);

    // Compute checksum
    unsigned int sum = 0;
    for (char c : addr)
        sum += static_cast<unsigned char>(c);
    for (char c : action)
        sum += static_cast<unsigned char>(c);
    for (char c : paramNum)
        sum += static_cast<unsigned char>(c);
    for (char c : dataLen)
        sum += static_cast<unsigned char>(c);
    for (char c : data)
        sum += static_cast<unsigned char>(c);
    unsigned int checksum = sum % 256;

    char csStr[4];
    snprintf(csStr, sizeof(csStr), "%03d", checksum);

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

    bool        valid  = false;
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
    bool        valid  = true;
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

    // After formatting, dataLen should be set to "06"
    EXPECT_EQ(cmd.dataLen, "06");
}

// ── decryptResponse tests ───────────────────────────────────────────────────

TEST(PfeifferLibDecrypt, ValidResponse)
{
    // Build a valid frame: address 001, action 10, paramNum 309, data "001500"
    std::string frame = buildPfeifferFrame("001", "10", "309", "001500");

    PfeifferCommand cmd;
    bool            valid = false;
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
    // Corrupt the checksum (last 4 chars are checksum + \r)
    frame[frame.size() - 2] = '0';
    frame[frame.size() - 3] = '0';
    frame[frame.size() - 4] = '0';

    PfeifferCommand cmd;
    bool            valid = true;
    PfeifferLib::decryptResponse(frame, &cmd, &valid);

    EXPECT_FALSE(valid);
}

TEST(PfeifferLibDecrypt, TooShort)
{
    PfeifferCommand cmd;
    bool            valid = true;
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
    bool            valid = true;
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

// ── Data type utility tests ─────────────────────────────────────────────────

TEST(PfeifferLibDataType, GetDataLengthAllTypes)
{
    EXPECT_EQ(PfeifferLib::getDataLength(PfeifferDataType::BOOLEAN), 6);
    EXPECT_EQ(PfeifferLib::getDataLength(PfeifferDataType::U_INTEGER), 6);
    EXPECT_EQ(PfeifferLib::getDataLength(PfeifferDataType::U_REAL), 6);
    EXPECT_EQ(PfeifferLib::getDataLength(PfeifferDataType::STRING_SHORT), 6);
    EXPECT_EQ(PfeifferLib::getDataLength(PfeifferDataType::BOOLEAN_NEW), 7);
    EXPECT_EQ(PfeifferLib::getDataLength(PfeifferDataType::U_SHORT_INT), 3);
    EXPECT_EQ(PfeifferLib::getDataLength(PfeifferDataType::U_EXPO_NEW), 6);
    EXPECT_EQ(PfeifferLib::getDataLength(PfeifferDataType::STRING_LONG), 16);
}

TEST(PfeifferLibDataType, EncodeBooleanTrue)
{
    EXPECT_EQ(PfeifferLib::encodeValue(PfeifferDataType::BOOLEAN, 1.0), "111111");
}

TEST(PfeifferLibDataType, EncodeBooleanFalse)
{
    EXPECT_EQ(PfeifferLib::encodeValue(PfeifferDataType::BOOLEAN, 0.0), "000000");
}

TEST(PfeifferLibDataType, EncodeUInteger)
{
    EXPECT_EQ(PfeifferLib::encodeValue(PfeifferDataType::U_INTEGER, 633.0), "000633");
}

TEST(PfeifferLibDataType, EncodeUReal)
{
    EXPECT_EQ(PfeifferLib::encodeValue(PfeifferDataType::U_REAL, 15.71), "001571");
}

TEST(PfeifferLibDataType, EncodeBooleanNewTrue)
{
    EXPECT_EQ(PfeifferLib::encodeValue(PfeifferDataType::BOOLEAN_NEW, 1.0), "ON     ");
}

TEST(PfeifferLibDataType, EncodeBooleanNewFalse)
{
    EXPECT_EQ(PfeifferLib::encodeValue(PfeifferDataType::BOOLEAN_NEW, 0.0), "OFF    ");
}

TEST(PfeifferLibDataType, EncodeUShortInt)
{
    EXPECT_EQ(PfeifferLib::encodeValue(PfeifferDataType::U_SHORT_INT, 42.0), "042");
}

TEST(PfeifferLibDataType, EncodeUnsupportedTypeReturnsEmpty)
{
    EXPECT_EQ(PfeifferLib::encodeValue(PfeifferDataType::STRING_SHORT, 0.0), "");
    EXPECT_EQ(PfeifferLib::encodeValue(PfeifferDataType::U_EXPO_NEW, 0.0), "");
}

TEST(PfeifferLibDataType, DecodeBooleanTrue)
{
    EXPECT_DOUBLE_EQ(PfeifferLib::decodeValue(PfeifferDataType::BOOLEAN, "111111"), 1.0);
}

TEST(PfeifferLibDataType, DecodeBooleanFalse)
{
    EXPECT_DOUBLE_EQ(PfeifferLib::decodeValue(PfeifferDataType::BOOLEAN, "000000"), 0.0);
}

TEST(PfeifferLibDataType, DecodeUInteger)
{
    EXPECT_DOUBLE_EQ(PfeifferLib::decodeValue(PfeifferDataType::U_INTEGER, "000633"), 633.0);
}

TEST(PfeifferLibDataType, DecodeUReal)
{
    EXPECT_DOUBLE_EQ(PfeifferLib::decodeValue(PfeifferDataType::U_REAL, "001571"), 15.71);
}

TEST(PfeifferLibDataType, DecodeBooleanNewTrue)
{
    EXPECT_DOUBLE_EQ(PfeifferLib::decodeValue(PfeifferDataType::BOOLEAN_NEW, "ON     "), 1.0);
}

TEST(PfeifferLibDataType, DecodeBooleanNewFalse)
{
    EXPECT_DOUBLE_EQ(PfeifferLib::decodeValue(PfeifferDataType::BOOLEAN_NEW, "OFF    "), 0.0);
}

TEST(PfeifferLibDataType, DecodeUShortInt)
{
    EXPECT_DOUBLE_EQ(PfeifferLib::decodeValue(PfeifferDataType::U_SHORT_INT, "042"), 42.0);
}

// ── Response parsing edge cases ─────────────────────────────────────────────

TEST(PfeifferLibDecrypt, ErrorResponseWithUnderscore)
{
    // Error response data contains underscores (e.g., NO_DEF, _RANGE, _LOGIC)
    std::string frame = buildPfeifferFrame("001", "10", "999", "NO_DEF");

    PfeifferCommand cmd;
    bool            valid = false;
    PfeifferLib::decryptResponse(frame, &cmd, &valid);

    EXPECT_TRUE(valid);
    EXPECT_EQ(cmd.data, "NO_DEF");
}

TEST(PfeifferLibDecrypt, BooleanNewResponseWithSpaces)
{
    // Boolean_new response contains spaces
    std::string frame = buildPfeifferFrame("001", "10", "041", "ON     ");

    PfeifferCommand cmd;
    bool            valid = false;
    PfeifferLib::decryptResponse(frame, &cmd, &valid);

    EXPECT_TRUE(valid);
    EXPECT_EQ(cmd.data, "ON     ");
}
