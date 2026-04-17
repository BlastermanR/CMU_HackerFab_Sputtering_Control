/**
 * @file test_sputtering_cfg.cpp
 * @brief Unit tests for SputteringCfg compile-time configuration struct.
 *
 * @author Ryan Massie (rmassie)
 * @date 4/16/26
 */

#include "SputteringCfg.h"
#include <gtest/gtest.h>

// ===========================================================================
// State enum
// ===========================================================================

TEST(SputteringCfg_State, IdleIsZero)
{
    EXPECT_EQ(static_cast<uint8_t>(SputteringCfg::State::IDLE), 0);
}

TEST(SputteringCfg_State, FaultIsSix)
{
    EXPECT_EQ(static_cast<uint8_t>(SputteringCfg::State::FAULT), 6);
}

TEST(SputteringCfg_State, AllUniqueValues)
{
    const uint8_t values[] = {
        static_cast<uint8_t>(SputteringCfg::State::IDLE),
        static_cast<uint8_t>(SputteringCfg::State::PUMP_DOWN),
        static_cast<uint8_t>(SputteringCfg::State::PURGE),
        static_cast<uint8_t>(SputteringCfg::State::IGNITION),
        static_cast<uint8_t>(SputteringCfg::State::DEPOSITING),
        static_cast<uint8_t>(SputteringCfg::State::VENTING),
        static_cast<uint8_t>(SputteringCfg::State::FAULT),
    };
    for (int i = 0; i < 7; ++i)
    {
        for (int j = i + 1; j < 7; ++j)
        {
            EXPECT_NE(values[i], values[j]) << "State " << i << " and " << j << " collide";
        }
    }
}

// ===========================================================================
// CmdID enum
// ===========================================================================

TEST(SputteringCfg_CmdID, SetStateIsZero)
{
    EXPECT_EQ(static_cast<uint8_t>(SputteringCfg::CmdID::SET_STATE), 0);
}

TEST(SputteringCfg_CmdID, AllUniqueValues)
{
    const uint8_t values[] = {
        static_cast<uint8_t>(SputteringCfg::CmdID::SET_STATE),
        static_cast<uint8_t>(SputteringCfg::CmdID::ABORT),
        static_cast<uint8_t>(SputteringCfg::CmdID::SET_ARGON_FLOW),
        static_cast<uint8_t>(SputteringCfg::CmdID::SET_OXYGEN_FLOW),
        static_cast<uint8_t>(SputteringCfg::CmdID::SET_PUMP_SPEED),
        static_cast<uint8_t>(SputteringCfg::CmdID::SET_VERBOSITY),
    };
    for (int i = 0; i < 6; ++i)
    {
        for (int j = i + 1; j < 6; ++j)
        {
            EXPECT_NE(values[i], values[j]) << "CmdID " << i << " and " << j << " collide";
        }
    }
}

TEST(SputteringCfg_CmdID, MaxValidCommandIDMatchesHighestEnum)
{
    EXPECT_EQ(SputteringCfg::kMaxValidCommandID,
              static_cast<uint8_t>(SputteringCfg::CmdID::SET_VERBOSITY));
}

// ===========================================================================
// Command struct
// ===========================================================================

TEST(SputteringCfg_Command, SizeIsBounded)
{
    // The command packet must fit in the lock-free queue element.
    EXPECT_LE(sizeof(SputteringCfg::Command), 8u);
}

TEST(SputteringCfg_Command, FieldsAreAccessible)
{
    SputteringCfg::Command cmd{};
    cmd.id           = SputteringCfg::CmdID::SET_ARGON_FLOW;
    cmd.targetDevice = 0;
    cmd.value        = 20.0f;

    EXPECT_EQ(cmd.id, SputteringCfg::CmdID::SET_ARGON_FLOW);
    EXPECT_EQ(cmd.targetDevice, 0);
    EXPECT_FLOAT_EQ(cmd.value, 20.0f);
}

// ===========================================================================
// Kernel constants
// ===========================================================================

TEST(SputteringCfg_Constants, CoreCountIsTwo)
{
    EXPECT_EQ(SputteringCfg::kCoreCount, 2u);
}

TEST(SputteringCfg_Constants, QueueCapacityIsPowerOf2OrReasonable)
{
    EXPECT_GE(SputteringCfg::kQueueCapacity, 4u);
    EXPECT_LE(SputteringCfg::kQueueCapacity, 256u);
}

TEST(SputteringCfg_Constants, ControlBudgetIs100Hz)
{
    EXPECT_EQ(SputteringCfg::kControlBudgetUs, 10000u);
}
