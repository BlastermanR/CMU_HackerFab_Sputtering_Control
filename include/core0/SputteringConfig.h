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
    Cooldown,
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


// Standard Recipe for testing
const StageConfig SPUTTERING_RECIPE[] = {
    // Stage                      Duration (ms)   Pressure  Ar Flow  O2 Flow  Pump Speed
    { ProcessStage::PumpDown,     60000,          0.0f,     0.0f,    0.0f,    100.0f },
    { ProcessStage::SteadyState,  300000,         5.0f,     20.0f,   0.0f,    80.0f  },
    { ProcessStage::Cooldown,     10000,          0.0f,     0.0f,    0.0f,    100.0f },
    { ProcessStage::Complete,     0,              0.0f,     0.0f,    0.0f,    0.0f   }
};

const StageConfig CLEANING_RECIPE[] = {
    // Stage                      Duration (ms)   Pressure  Ar Flow  O2 Flow  Pump Speed
    { ProcessStage::PumpDown,     60000,          0.0f,     0.0f,    0.0f,    100.0f },
    { ProcessStage::SteadyState,  300000,         5.0f,     20.0f,   0.0f,    80.0f  },
    { ProcessStage::Cooldown,     10000,          0.0f,     0.0f,    0.0f,    100.0f },
    { ProcessStage::Complete,     0,              0.0f,     0.0f,    0.0f,    0.0f   }
};

#endif // SPUTTERING_CONFIG_H



