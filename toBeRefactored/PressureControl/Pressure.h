#pragma once

// Structure representing a pressure value in both fractional/exponential form and absolute value
struct pressure_measurement {
  float pressure; // Full pressure value (e.g., 5.4e-4)
  float frac;     // Fractional part (e.g., 5.4)
  float exp;      // Exponent part (e.g., -4)
};

// Waits for user input over Serial to specify a desired pressure
// Fills the provided pressure_measurement struct
void waitForPressureInput(pressure_measurement &desired_pressure);

// Converts a raw pressure string from the Pfeiffer gauge to a pressure_measurement struct
pressure_measurement pressure_conversion(char* unconverted_pressure);

// Tracks pressure stability over time to determine if equilibrium is reached
// If 'clear' is true, resets the rolling pressure window
// 'desired_magnitude' is used to normalize error thresholds
bool reached_equilibrium(float newPressure, bool clear, float desired_magnitude);

// Optional global variables to expose pressure values externally (currently unused in some files)
extern float measuredPressure;
extern float measuredPressure_frac;
extern int measuredPressure_exp;
