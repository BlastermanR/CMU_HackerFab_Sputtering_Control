/**
 * @file test_messages.cpp
 * @brief Unit tests verifying Messages.h struct sizes and enum values.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/31/26
 */
#include "Messages.h"
#include <cstring>
#include <gtest/gtest.h>

// ── Verbosity enum ──────────────────────────────────────────────────────────

TEST(MessagesVerbosity, LevelOrder)
{
    EXPECT_LT(V_CRITICAL, V_STATUS);
    EXPECT_LT(V_STATUS, V_INFO);
    EXPECT_LT(V_INFO, V_DEBUG);
}

// ── CommandId enum ──────────────────────────────────────────────────────────

TEST(MessagesCommandId, NoneIsZero) { EXPECT_EQ(Cmd_None, 0); }

TEST(MessagesCommandId, AllUnique)
{
    // Ensure no accidental duplicates in the enum
    uint8_t ids[]      = {Cmd_None,       Cmd_StartProcess, Cmd_StopProcess,  Cmd_PressurizeChamber, Cmd_VentChamber,
                          Cmd_ShutOffGas, Cmd_SetArgonFlow, Cmd_SetOxygenFlow, Cmd_SetPumpSpeed,
                          Cmd_EnablePump, Cmd_DisablePump,  Cmd_Exit,         Cmd_SetVerbosity};
    constexpr size_t N = sizeof(ids) / sizeof(ids[0]);

    for (size_t i = 0; i < N; ++i)
        for (size_t j = i + 1; j < N; ++j)
            EXPECT_NE(ids[i], ids[j]) << "Duplicate at index " << i << " and " << j;
}

// ── CommandMessage struct ───────────────────────────────────────────────────

TEST(MessagesCommand, DefaultInit)
{
    CommandMessage msg{};
    msg.id     = Cmd_SetArgonFlow;
    msg.param1 = 10.5f;
    msg.param2 = 0.0f;

    EXPECT_EQ(msg.id, Cmd_SetArgonFlow);
    EXPECT_FLOAT_EQ(msg.param1, 10.5f);
}

// ── OutputMessage struct ────────────────────────────────────────────────────

TEST(MessagesOutput, LogPayload)
{
    OutputMessage msg{};
    msg.source = Source_Core0;
    msg.level  = V_INFO;
    msg.type   = Msg_Log;
    strncpy(msg.text, "Hello", OUTPUT_MSG_TEXT_LEN);

    EXPECT_EQ(msg.source, Source_Core0);
    EXPECT_EQ(msg.type, Msg_Log);
    EXPECT_STREQ(msg.text, "Hello");
}

TEST(MessagesOutput, DataPayload)
{
    OutputMessage msg{};
    msg.source     = Source_Core1;
    msg.level      = V_STATUS;
    msg.type       = Msg_Data;
    msg.data.id    = Data_ChamberPressure;
    msg.data.value = 1.5e-3f;

    EXPECT_EQ(msg.data.id, Data_ChamberPressure);
    EXPECT_FLOAT_EQ(msg.data.value, 1.5e-3f);
}

// ── DataId enum ─────────────────────────────────────────────────────────────

TEST(MessagesDataId, ValuesMatch)
{
    EXPECT_EQ(Data_PumpSpeed, 0);
    EXPECT_EQ(Data_ChamberPressure, 1);
    EXPECT_EQ(Data_ArgonFlow, 2);
    EXPECT_EQ(Data_OxygenFlow, 3);
}
