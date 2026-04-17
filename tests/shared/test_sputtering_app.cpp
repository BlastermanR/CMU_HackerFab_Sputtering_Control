/**
 * @file test_sputtering_app.cpp
 * @brief Unit tests for SputteringApp (IUserApplication implementation).
 *
 * Tests cover:
 * - Construction and initial state verification via getState()
 * - State machine transitions via transitionTo() with state assertions
 * - Command handling (SET_STATE, ABORT, SET_ARGON_FLOW, SET_OXYGEN_FLOW,
 *   SET_PUMP_SPEED, SET_VERBOSITY)
 * - forceSafeAbort() behaviour and state verification
 *
 * @author Ryan Massie (rmassie)
 * @date 4/16/26
 */

#include "SputteringApp.h"
#include "GlobalDevices.h"
#include "test_globals.h"
#include <gtest/gtest.h>

// ===========================================================================
// Fixture
// ===========================================================================

class SputteringAppTest : public ::testing::Test
{
  protected:
    SputteringApp app;

    void SetUp() override
    {
        // init() configures device gas types and sets state to IDLE.
        app.init();
    }
};

// ===========================================================================
// Construction & init
// ===========================================================================

TEST_F(SputteringAppTest, InitSetsIdleState)
{
    EXPECT_EQ(app.getState(), SputteringCfg::State::IDLE);
    // First tick should not crash (no active phase).
    app.tick(1'000'000ULL);
    EXPECT_EQ(app.getState(), SputteringCfg::State::IDLE);
}

// ===========================================================================
// transitionTo
// ===========================================================================

TEST_F(SputteringAppTest, TransitionToPumpDown)
{
    app.transitionTo(SputteringCfg::State::PUMP_DOWN);
    EXPECT_EQ(app.getState(), SputteringCfg::State::PUMP_DOWN);
    app.tick(2'000'000ULL);
}

TEST_F(SputteringAppTest, TransitionToIdleClearsPhase)
{
    app.transitionTo(SputteringCfg::State::PUMP_DOWN);
    EXPECT_EQ(app.getState(), SputteringCfg::State::PUMP_DOWN);

    app.transitionTo(SputteringCfg::State::IDLE);
    EXPECT_EQ(app.getState(), SputteringCfg::State::IDLE);
    // Tick with no active phase — should be a no-op.
    app.tick(3'000'000ULL);
}

TEST_F(SputteringAppTest, TransitionToFaultClearsPhase)
{
    app.transitionTo(SputteringCfg::State::PURGE);
    EXPECT_EQ(app.getState(), SputteringCfg::State::PURGE);

    app.transitionTo(SputteringCfg::State::FAULT);
    EXPECT_EQ(app.getState(), SputteringCfg::State::FAULT);
    app.tick(4'000'000ULL);
}

// ===========================================================================
// handleCommand
// ===========================================================================

TEST_F(SputteringAppTest, HandleSetStateCommand)
{
    SputteringCfg::Command cmd{};
    cmd.id    = SputteringCfg::CmdID::SET_STATE;
    cmd.value = static_cast<float>(static_cast<uint8_t>(SputteringCfg::State::PUMP_DOWN));

    app.handleCommand(cmd);
    EXPECT_EQ(app.getState(), SputteringCfg::State::PUMP_DOWN);
    app.tick(5'000'000ULL);
}

TEST_F(SputteringAppTest, HandleAbortCommand)
{
    app.transitionTo(SputteringCfg::State::DEPOSITING);
    EXPECT_EQ(app.getState(), SputteringCfg::State::DEPOSITING);

    SputteringCfg::Command cmd{};
    cmd.id = SputteringCfg::CmdID::ABORT;

    app.handleCommand(cmd);
    EXPECT_EQ(app.getState(), SputteringCfg::State::FAULT);
    app.tick(6'000'000ULL);
}

TEST_F(SputteringAppTest, HandleSetArgonFlowCommand)
{
    SputteringCfg::Command cmd{};
    cmd.id    = SputteringCfg::CmdID::SET_ARGON_FLOW;
    cmd.value = 25.0f;

    // Should not crash; sends setpoint to mfc2 mock.
    app.handleCommand(cmd);
    EXPECT_EQ(app.getState(), SputteringCfg::State::IDLE);
}

TEST_F(SputteringAppTest, HandleSetOxygenFlowCommand)
{
    SputteringCfg::Command cmd{};
    cmd.id    = SputteringCfg::CmdID::SET_OXYGEN_FLOW;
    cmd.value = 10.0f;

    app.handleCommand(cmd);
    EXPECT_EQ(app.getState(), SputteringCfg::State::IDLE);
}

TEST_F(SputteringAppTest, HandleSetPumpSpeedCommand)
{
    SputteringCfg::Command cmd{};
    cmd.id    = SputteringCfg::CmdID::SET_PUMP_SPEED;
    cmd.value = 50.0f;

    // Not yet implemented — should not crash or change state.
    app.handleCommand(cmd);
    EXPECT_EQ(app.getState(), SputteringCfg::State::IDLE);
}

TEST_F(SputteringAppTest, HandleSetVerbosityCommand)
{
    SputteringCfg::Command cmd{};
    cmd.id    = SputteringCfg::CmdID::SET_VERBOSITY;
    cmd.value = 2.0f;

    // Handled by TelemetryLogger — should not crash or change state.
    app.handleCommand(cmd);
    EXPECT_EQ(app.getState(), SputteringCfg::State::IDLE);
}

TEST_F(SputteringAppTest, HandleUnknownCommandDoesNotCrash)
{
    SputteringCfg::Command cmd{};
    cmd.id    = static_cast<SputteringCfg::CmdID>(99);
    cmd.value = 0.0f;

    app.handleCommand(cmd);
    EXPECT_EQ(app.getState(), SputteringCfg::State::IDLE);
}

// ===========================================================================
// forceSafeAbort
// ===========================================================================

TEST_F(SputteringAppTest, ForceSafeAbortFromDepositing)
{
    app.transitionTo(SputteringCfg::State::DEPOSITING);
    EXPECT_EQ(app.getState(), SputteringCfg::State::DEPOSITING);

    app.forceSafeAbort();
    EXPECT_EQ(app.getState(), SputteringCfg::State::FAULT);
    app.tick(7'000'000ULL);
}

TEST_F(SputteringAppTest, ForceSafeAbortFromIdle)
{
    app.forceSafeAbort();
    EXPECT_EQ(app.getState(), SputteringCfg::State::FAULT);
    app.tick(8'000'000ULL);
}

// ===========================================================================
// Full sequence smoke test
// ===========================================================================

TEST_F(SputteringAppTest, FullSequenceDoesNotCrash)
{
    const SputteringCfg::State sequence[] = {
        SputteringCfg::State::PUMP_DOWN,
        SputteringCfg::State::PURGE,
        SputteringCfg::State::IGNITION,
        SputteringCfg::State::DEPOSITING,
        SputteringCfg::State::VENTING,
        SputteringCfg::State::IDLE,
    };

    uint64_t t = 1'000'000ULL;
    for (auto state : sequence)
    {
        app.transitionTo(state);
        EXPECT_EQ(app.getState(), state);
        app.tick(t);
        t += 1'000'000ULL;
    }
}
