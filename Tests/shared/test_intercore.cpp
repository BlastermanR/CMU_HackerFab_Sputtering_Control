/**
 * @file test_intercore.cpp
 * @brief Unit tests for Intercore status register bitmask operations.
 *
 * These functions are defined inline in Intercore.h and operate on a
 * global std::atomic<uint16_t> statusReg.  We reset it before each test.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/31/26
 */
#include <gtest/gtest.h>

// Pull in the stubs so that pico/util/queue.h resolves
#include "Intercore.h"
#include "pico_stubs.h"

class IntercoreTest : public ::testing::Test
{
  protected:
    void SetUp() override { statusReg.store(0); }
};

// ── setStatus / getStatus ───────────────────────────────────────────────────

TEST_F(IntercoreTest, SetAndGetSingleBit)
{
    EXPECT_FALSE(getStatus(Status_Core0Err));

    setStatus(Status_Core0Err);
    EXPECT_TRUE(getStatus(Status_Core0Err));
}

TEST_F(IntercoreTest, SetMultipleBits)
{
    setStatus(Status_Core0Err);
    setStatus(Status_PumpErr);

    EXPECT_TRUE(getStatus(Status_Core0Err));
    EXPECT_TRUE(getStatus(Status_PumpErr));
    EXPECT_FALSE(getStatus(Status_GaugeErr));
}

// ── clearStatus ─────────────────────────────────────────────────────────────

TEST_F(IntercoreTest, ClearBit)
{
    setStatus(Status_Core0Err);
    setStatus(Status_PumpErr);

    clearStatus(Status_Core0Err);
    EXPECT_FALSE(getStatus(Status_Core0Err));
    EXPECT_TRUE(getStatus(Status_PumpErr));
}

TEST_F(IntercoreTest, ClearAlreadyClearedBit)
{
    clearStatus(Status_Core0Err); // no-op
    EXPECT_FALSE(getStatus(Status_Core0Err));
}

// ── isError ─────────────────────────────────────────────────────────────────

TEST_F(IntercoreTest, IsErrorWhenNoErrors) { EXPECT_FALSE(isError()); }

TEST_F(IntercoreTest, IsErrorWithCoreError)
{
    setStatus(Status_Core1Err);
    EXPECT_TRUE(isError());
}

TEST_F(IntercoreTest, IsErrorIgnoresNonErrorBits)
{
    setStatus(ExecuteSputteringProcess);
    setStatus(Core0_Begin);
    EXPECT_FALSE(isError());
}

// ── getError ────────────────────────────────────────────────────────────────

TEST_F(IntercoreTest, GetErrorReturnsNoneWhenClean) { EXPECT_EQ(getError(), Status_None); }

TEST_F(IntercoreTest, GetErrorReturnsPrioritized)
{
    // Core0Err (bit 0) has higher priority than PumpErr (bit 4)
    setStatus(Status_PumpErr);
    setStatus(Status_Core0Err);

    EXPECT_EQ(getError(), Status_Core0Err);
}

TEST_F(IntercoreTest, GetErrorReturnsLowPriorityAlone)
{
    setStatus(Status_GaugeErr);
    EXPECT_EQ(getError(), Status_GaugeErr);
}
// ── isCommand ───────────────────────────────────────────────────────────────

TEST_F(IntercoreTest, IsCommandReturnsNoneWhenNone) { EXPECT_EQ(isCommand(), Status_None); }

TEST_F(IntercoreTest, IsCommandReturnsLowestBit)
{
    // Multiple commands: ExecuteSputteringProcess (1<<7), VentChamber (1<<9)
    setStatus(ExecuteSputteringProcess);
    setStatus(VentChamber);

    EXPECT_EQ(isCommand(), ExecuteSputteringProcess);

    // Clear the first one, should return the next
    clearStatus(ExecuteSputteringProcess);
    EXPECT_EQ(isCommand(), VentChamber);
}

TEST_F(IntercoreTest, IsCommandIgnoresNonCommandBits)
{
    // Set an error bit and a startup bit
    setStatus(Status_Core0Err);
    setStatus(Core0_Begin);

    EXPECT_EQ(isCommand(), Status_None);
}
// ── Startup / command bits ──────────────────────────────────────────────────

TEST_F(IntercoreTest, StartupBitsRoundTrip)
{
    setStatus(Core0_Begin);
    setStatus(Core1_Begin);

    EXPECT_TRUE(getStatus(Core0_Begin));
    EXPECT_TRUE(getStatus(Core1_Begin));

    clearStatus(Core0_Begin);
    EXPECT_FALSE(getStatus(Core0_Begin));
    EXPECT_TRUE(getStatus(Core1_Begin));
}

TEST_F(IntercoreTest, ExecutionBits)
{
    setStatus(ExecuteSputteringProcess);
    EXPECT_TRUE(getStatus(ExecuteSputteringProcess));
    EXPECT_FALSE(isError()); // execution bits are not errors
}
