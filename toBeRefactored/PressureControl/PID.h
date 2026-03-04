#pragma once
#include "Pressure.h"

// Global flag that tracks whether the integral term is currently active
extern bool integral_active;

// Computes the flow adjustment based on current and desired pressure readings
// Uses a proportional–integral (PI) control algorithm
// Inputs:
//   - desired_pressure: the target pressure to reach
//   - measured_pressure: the current pressure from the sensor
//   - currSetPoint: the current flow rate (e.g., % of max flow)
//   - sample_interval: time step between updates (in milliseconds)
// Returns:
//   - Updated flow setpoint
float calculate_adjustment(pressure_measurement desired_pressure, pressure_measurement measured_pressure, float currSetPoint, const unsigned long sample_interval);

// Debugging utility to print fractional and exponential pressure components
void debug_pressure(pressure_measurement desired_pressure, pressure_measurement measured_pressure);

// Optional helper to calculate a Kp based on normalized error (currently unused)
float proportional_coefficient(float normalized_error);

// Optional helper to fill the integral buffer (currently unused externally)
void fill_integral_buffer(pressure_measurement desired_pressure, pressure_measurement measured_pressure);

// Initializes PI controller coefficients based on the pressure magnitude (exponent)
void init_coefficients(pressure_measurement desired_pressure);
