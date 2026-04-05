/**
 * SputteringConfig.h
 * @brief Defines values for controlling CMUs Sputtering Process
 * 
 * Note: Modify to match individual chamber.
 *
 * @author Ryan Massie (rmassie)
 * @date 4/3/26
 */
#ifndef SPUTTERING_CONFIG_H
#define SPUTTERING_CONFIG_H

#include <stdint.h> 
#include <vector>
#include <atomic>
#include "pico/critical_section.h"

class SputteringProcess
{
    public:

    /**
     * @brief Define the operating modes of the sputtering process
     */
    enum class OperatingMode
    {
        Manual, // Fully Manual Control
        Dummy, // Timer based Operation
        Automatic // Fully Automatic Reactive Control
    };

    /**
     * @brief Defines the discrete stages of the sputtering/cleaning process.
     */
    enum class ProcessStage
    {
        Standby,
        PumpDown,
        Purge,
        Ignition,
        SteadyState,
        Vent,
        Complete
    };

    /**
     * @brief Configuration values for a single stage of the process.
     */
    struct StageConfig
    {
        ProcessStage stage;
        uint32_t     duration_ms;           // Duration of the stage in milliseconds
        float        targetPressure_hPa;    // Target chamber pressure in hPa
        float        argonFlowSetpoint_sccm;  // Argon MFC setpoint in sccm
        float        oxygenFlowSetpoint_sccm; // Oxygen MFC setpoint in sccm
        float        pumpSpeedSetpoint_rpm;  // Pump speed setpoint in rpm
    };

    private:

    /**
     * @brief The currently active recipe sequence.
     */
    static std::vector<StageConfig> currentRecipe;

    /**
     * @brief is the process Executing
     */
    static std::atomic<bool> isExecuting;

    /**
     * @brief Controls whether sputterer is operating in a reactive mode or blind operation based on time
     */
    static OperatingMode mode;

    /**
     * @brief Mutex for protecting access to the shared recipe and mode.
     */
    static critical_section_t configLock;

    /**
     * @brief Validates a recipe sequence based on safety requirements.
     * @return true if valid (PumpDown -> ... -> Vent), false otherwise.
     */
    static bool validateRecipe(const std::vector<StageConfig>& recipe);

public:
    /**
     * @brief Initialize the sputtering process object.
     * Note: This must be called from Core 0 during setup.
     */
    static void init();

    /**
     * Sputtering Object Destructor
     */
    ~SputteringProcess()
    {
        // Intentionally Empty
    }

    // Operating Mode Functions

    /**
     * @brief Set the current sputtering process operating mode.
     * @param newMode The mode to switch to (Manual, Dummy, Automatic).
     */
    static void setOperatingMode(OperatingMode newMode)
    {
        critical_section_enter_blocking(&configLock);
        mode = newMode;
        critical_section_exit(&configLock);
    }

    /**
     * @brief Get the current sputtering process operating mode.
     * @return The active operating mode.
     */
    static OperatingMode getOperatingMode()
    {
        critical_section_enter_blocking(&configLock);
        OperatingMode currentMode = mode;
        critical_section_exit(&configLock);
        return currentMode;
    }

    /**
     * @brief Loads a new recipe after validation.
     * @return true if successfully loaded, false if rejected due to validation.
     */
    static bool loadRecipe(const std::vector<StageConfig>& newRecipe);

    /**
     * @brief Returns a copy of the current recipe for safe processing.
     */
    static std::vector<StageConfig> getRecipe()
    {
        critical_section_enter_blocking(&configLock);
        std::vector<StageConfig> copy = currentRecipe;
        critical_section_exit(&configLock);
        return copy;
    }

    /**
     * @brief Sets the execution status of the process.
     * @param executing True if the process is running, false otherwise.
     */
    static void setExecuting(bool executing)
    {
        isExecuting.store(executing, std::memory_order_release);
    }

    /**
     * @brief Check if the sputtering/cleaning process is currently executing.
     * @note This is atomic and does not require the configLock, allowing Core 1 
     *       to check status without blocking Core 0.
     * @return True if executing, false otherwise.
     */
    static bool getIsExecuting()
    {
        return isExecuting.load(std::memory_order_acquire);
    }

};

#endif // SPUTTERING_CONFIG_H



