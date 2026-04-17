/**
 * @file test_process_phases.cpp
 * @brief Unit tests for the sputtering process phase state machines.
 *
 * Tests cover:
 * - Each phase constructs with defaults and custom parameters
 * - onEnter / onExit do not crash
 * - Timed phases transition after their configured duration
 * - PumpDown transitions on pressure or timeout
 * - Venting transitions on pressure or timeout
 * - State assertions verify transitions actually took effect
 *
 * All tests use mock-backed global devices (test_globals.cpp) so no
 * hardware is required. The mock gauge returns 0.0 hPa by default.
 *
 * @author Ryan Massie (rmassie)
 * @date 4/16/26
 */

#include "ProcessPhases.h"
#include "SputteringApp.h"
#include "GlobalDevices.h"
#include "test_globals.h"
#include <gtest/gtest.h>

// ===========================================================================
// Fixture — provides a SputteringApp and helpers for phase testing
// ===========================================================================

class ProcessPhaseTest : public ::testing::Test
{
  protected:
    SputteringApp app;

    void SetUp() override { app.init(); }
};

// ===========================================================================
// PumpDownPhase
// ===========================================================================

TEST_F(ProcessPhaseTest, PumpDownConstructsWithDefaults)
{
    PumpDownPhase phase(&app);
    // onEnter should not crash.
    phase.onEnter();
    phase.onExit();
}

TEST_F(ProcessPhaseTest, PumpDownTransitionsToPurgeOnLowPressure)
{
    // Default gauge returns 0.0 hPa which is <= 0.1 hPa target.
    // So the first execute() should trigger a transition to PURGE.
    app.transitionTo(SputteringCfg::State::PUMP_DOWN);
    EXPECT_EQ(app.getState(), SputteringCfg::State::PUMP_DOWN);

    app.tick(1'000'000ULL); // first tick latches time
    app.tick(2'000'000ULL); // pressure check — 0.0 <= 0.1, transition
    EXPECT_EQ(app.getState(), SputteringCfg::State::PURGE);
}

TEST_F(ProcessPhaseTest, PumpDownTransitionsToFaultOnTimeout)
{
    // Use a threshold of -1.0 which 0.0 hPa will never satisfy.
    PumpDownPhase phase(&app, -1.0, 100'000ULL); // 100 ms timeout

    phase.onEnter();
    phase.execute(1'000'000ULL);               // latch entry time
    phase.execute(1'200'000ULL);               // 200,000 µs > 100,000 µs timeout
    EXPECT_EQ(app.getState(), SputteringCfg::State::FAULT);
    phase.onExit();
}

// ===========================================================================
// PurgePhase
// ===========================================================================

TEST_F(ProcessPhaseTest, PurgeConstructsWithDefaults)
{
    PurgePhase phase(&app);
    phase.onEnter();
    phase.onExit();
}

TEST_F(ProcessPhaseTest, PurgeTransitionsAfterDuration)
{
    PurgePhase phase(&app, 50.0, 100'000ULL);  // 100,000 µs duration
    phase.onEnter();
    phase.execute(1'000'000ULL);               // latch entry time
    phase.execute(1'200'000ULL);               // 200,000 µs > 100,000 µs — transition
    EXPECT_EQ(app.getState(), SputteringCfg::State::IGNITION);
}

TEST_F(ProcessPhaseTest, PurgeDoesNotTransitionEarly)
{
    PurgePhase phase(&app, 50.0, 1'000'000ULL); // 1,000,000 µs (1 s)
    phase.onEnter();
    phase.execute(1'000'000ULL);               // latch entry time
    phase.execute(1'500'000ULL);               // 500,000 µs < 1,000,000 µs
    // State should NOT have changed from whatever it was.
    EXPECT_NE(app.getState(), SputteringCfg::State::IGNITION);
}

// ===========================================================================
// IgnitionPhase
// ===========================================================================

TEST_F(ProcessPhaseTest, IgnitionConstructsWithDefaults)
{
    IgnitionPhase phase(&app);
    phase.onEnter();
    phase.onExit();
}

TEST_F(ProcessPhaseTest, IgnitionTransitionsAfterDuration)
{
    IgnitionPhase phase(&app, 20.0, 100'000ULL);
    phase.onEnter();
    phase.execute(1'000'000ULL);               // latch entry time
    phase.execute(1'200'000ULL);               // 200,000 µs > 100,000 µs
    EXPECT_EQ(app.getState(), SputteringCfg::State::DEPOSITING);
}

// ===========================================================================
// DepositingPhase
// ===========================================================================

TEST_F(ProcessPhaseTest, DepositingConstructsWithDefaults)
{
    DepositingPhase phase(&app);
    phase.onEnter();
    phase.onExit();
}

TEST_F(ProcessPhaseTest, DepositingTransitionsAfterDuration)
{
    DepositingPhase phase(&app, 15.0, 100'000ULL);
    phase.onEnter();
    phase.execute(1'000'000ULL);               // latch entry time
    phase.execute(1'200'000ULL);               // 200,000 µs > 100,000 µs
    EXPECT_EQ(app.getState(), SputteringCfg::State::VENTING);
}

// ===========================================================================
// VentingPhase
// ===========================================================================

TEST_F(ProcessPhaseTest, VentingConstructsWithDefaults)
{
    VentingPhase phase(&app);
    phase.onEnter();
    phase.onExit();
}

TEST_F(ProcessPhaseTest, VentingTransitionsOnTimeout)
{
    // Default gauge reads 0.0 hPa which is < 900 hPa, so only timeout works.
    VentingPhase phase(&app, 900.0, 100'000ULL);
    phase.onEnter();
    phase.execute(1'000'000ULL);               // latch entry time
    phase.execute(1'200'000ULL);               // 200,000 µs > 100,000 µs timeout
    EXPECT_EQ(app.getState(), SputteringCfg::State::IDLE);
}

// ===========================================================================
// End-to-end sequence through app.transitionTo
// ===========================================================================

TEST_F(ProcessPhaseTest, SequenceThroughAllPhases)
{
    const SputteringCfg::State phases[] = {
        SputteringCfg::State::PUMP_DOWN,
        SputteringCfg::State::PURGE,
        SputteringCfg::State::IGNITION,
        SputteringCfg::State::DEPOSITING,
        SputteringCfg::State::VENTING,
        SputteringCfg::State::IDLE,
    };

    uint64_t t = 1'000'000ULL;
    for (auto s : phases)
    {
        app.transitionTo(s);
        EXPECT_EQ(app.getState(), s);
        app.tick(t);
        t += 10'000'000ULL;
    }
}
