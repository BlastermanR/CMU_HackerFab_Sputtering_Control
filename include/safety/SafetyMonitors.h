/**
 * @file SafetyMonitors.h
 * @brief ISafetyMonitor adapters for the CMU HackerFab sputtering system.
 *
 * Wraps domain safety conditions — over-pressure and pump health — in
 * `SputterOS::ISafetyMonitor` implementations. The kernel `ControlTask`
 * evaluates these before every application tick and calls
 * `forceSafeAbort()` on the first failure.
 *
 * @author Ryan Massie (rmassie)
 * @date 4/16/26
 */
#ifndef SAFETY_MONITORS_H
#define SAFETY_MONITORS_H

#include "PfeifferGauge.h"
#include "PfeifferPump.h"
#include "sputteros/kernel/interfaces/ISafetyMonitor.h"

/**
 * @brief Safety monitor that triggers a fault when chamber pressure exceeds
 * a configurable upper limit.
 *
 * This is a Tier-1 kernel safety check. If the pressure reading is above
 * `thresholdHPa`, the kernel calls `IUserApplication::forceSafeAbort()`.
 */
class OverPressureMonitor : public SputterOS::ISafetyMonitor
{
  public:
    /**
     * @brief Constructor.
     * @param gauge Pointer to the PfeifferGauge instance to monitor.
     * @param thresholdHPa Maximum safe chamber pressure in hPa.
     *        Readings above this value trigger a fault.
     */
    explicit OverPressureMonitor(PfeifferGauge *gauge, double thresholdHPa = 10.0)
        : m_gauge(gauge), m_thresholdHPa(thresholdHPa)
    {
    }

    /**
     * @brief Returns true when chamber pressure is within the safe range.
     * @return false if pressure exceeds the configured threshold.
     */
    bool isSafe() const override { return m_gauge->getPressure() <= m_thresholdHPa; }

    /**
     * @brief Human-readable identifier for diagnostics.
     */
    const char *name() const override { return "OverPressureMonitor"; }

  private:
    PfeifferGauge *m_gauge;
    double         m_thresholdHPa;
};

/**
 * @brief Safety monitor that triggers a fault when the vacuum pump reports
 * an error condition.
 *
 * Wraps `PfeifferPump::hasError()`. If the pump is in an error state while
 * an active process is running, the kernel forces a safe abort.
 */
class PumpHealthMonitor : public SputterOS::ISafetyMonitor
{
  public:
    /**
     * @brief Constructor.
     * @param pump Pointer to the PfeifferPump instance to monitor.
     */
    explicit PumpHealthMonitor(PfeifferPump *pump) : m_pump(pump) {}

    /**
     * @brief Returns true when the pump is not reporting an error.
     * @return false if the pump has entered a fault state.
     */
    bool isSafe() const override { return !m_pump->hasError(); }

    /**
     * @brief Human-readable identifier for diagnostics.
     */
    const char *name() const override { return "PumpHealthMonitor"; }

  private:
    PfeifferPump *m_pump;
};

#endif // SAFETY_MONITORS_H
