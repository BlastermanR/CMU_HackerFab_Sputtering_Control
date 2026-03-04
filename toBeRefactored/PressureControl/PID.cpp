#include "PID.h"
#include <Arduino.h>
#include "Pressure.h"

// -------------------- PID Constants & Globals --------------------

// Size of the integral buffer (number of past errors remembered)
const int INTEGRAL_WINDOW = 10;

// Circular buffer to store recent error values for integral term
float integralBuffer[INTEGRAL_WINDOW] = {0};

// Integral gain
float Ki;

// Running sum of error values for integral term
float integralSum = 0;

// Final integral result
float integral = 0;

// Current position in the circular buffer
int bufferIndex = 0;

// Last pressure error (not used in current version)
float lastError = 0;

// Proportional gain (set based on desired pressure)
float baseKp;

// Flag to track if integral term is currently active
bool integral_active = false;

// -------------------- PID Helper Functions --------------------

// Initializes PID coefficients based on the order of magnitude of the target pressure
void init_coefficients(pressure_measurement desired_pressure)
{
  if (desired_pressure.exp == -2)
  {
    baseKp = 1000;
    Ki = 500;
  }
  if (desired_pressure.exp == -3)
  {
    baseKp = 1000;
    Ki = 500;
  }
  else if (desired_pressure.exp == -4)
  {
    baseKp = 800;
    Ki = 500;
  }
  else if (desired_pressure.exp == -5)
  {
    baseKp = 400;
    Ki = 500;
  }
  else
  {
    baseKp = 600;
    Ki = 500;
  }
  return;
}

// -------------------- Debug Function --------------------

// Prints desired and measured pressure components to the Serial Monitor
void debug_pressure(pressure_measurement desired_pressure, pressure_measurement measured_pressure) {
  Serial.print("Desired Pressure frac: ");
  Serial.println(desired_pressure.frac);
  Serial.print("Desired Pressure exp: ");
  Serial.println(desired_pressure.exp);

  Serial.print("Measured Pressure frac: ");
  Serial.println(measured_pressure.frac);
  Serial.print("Measured Pressure exp: ");
  Serial.println(measured_pressure.exp);
}

// -------------------- Main PID Adjustment --------------------

// Calculates the new flow setpoint based on pressure error using PI logic
float calculate_adjustment(pressure_measurement desired_pressure, pressure_measurement measured_pressure, float currSetPoint, const unsigned long sample_interval) {
  // If sensor read failed (0), keep current setpoint
  if (measured_pressure.frac == 0) return currSetPoint;

  // Print pressures for debugging
  debug_pressure(desired_pressure, measured_pressure);

  // Compute raw pressure error
  float raw_error = desired_pressure.pressure - measured_pressure.pressure;

  // Normalize the error to account for pressure magnitude
  float scale = 1.0 / desired_pressure.pressure;
  float normalized_error = raw_error * scale;

  Serial.print("Normalized Error: ");
  Serial.println(normalized_error);

  // If error is within 1%, do not adjust
  if (abs(normalized_error) < 0.01) return currSetPoint;

  // Use fixed proportional gain
  float Kp = baseKp;
  float proportional = Kp * raw_error;

  // Track how many samples are filled in the buffer
  static int integralFillCount = 0;

  // Activate integral if proportional term is small
  if (abs(proportional) < 0.01) {
    if (!integral_active) {
      Serial.println("Enabling integral mode");
      integral_active = true;
      integralFillCount = 0;
      integralSum = 0;
      for (int i = 0; i < INTEGRAL_WINDOW; i++) integralBuffer[i] = 0;
    }
    Serial.println("Integral Active");

    // Update integral buffer and sum
    integralSum -= integralBuffer[bufferIndex];
    integralBuffer[bufferIndex] = raw_error;
    integralSum += raw_error;

    bufferIndex = (bufferIndex + 1) % INTEGRAL_WINDOW;
    if (integralFillCount < INTEGRAL_WINDOW) integralFillCount++;

    // Compute integral only when buffer is full
    integral = (integralFillCount >= INTEGRAL_WINDOW) ? Ki * integralSum : 0;
  } else {
    // Disable integral mode and reset everything
    if (integral_active) {
      Serial.println("Disabling integral mode and clearing buffer");
      integral_active = false;
      integralFillCount = 0;
      integralSum = 0;
      for (int i = 0; i < INTEGRAL_WINDOW; i++) integralBuffer[i] = 0;
    }
    integral = 0;
  }

  // Compute final adjustment as sum of P and I
  float adjustment = proportional + integral;
  if (abs(adjustment) < 0.01) adjustment = 0;

  // Update setpoint and constrain to safe range
  currSetPoint += adjustment;
  currSetPoint = constrain(currSetPoint, 0.0f, 50.0f);

  Serial.print("Current Setpoint: ");
  Serial.println(currSetPoint);
  Serial.print("Adjustment: ");
  Serial.println(adjustment);

  return currSetPoint;
}
