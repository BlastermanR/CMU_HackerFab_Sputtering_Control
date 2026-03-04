#include "Pressure.h"
#include <Arduino.h>
#include <math.h>
#include <string.h>

// Size of the rolling window used for equilibrium detection
#define N 10

// Threshold for determining equilibrium (standard deviation limit)
#define Q 0.1f

// Circular buffer for storing recent pressure values
float pressureWindow[N];

// Index for the next insertion in the buffer
int head = 0;

// Count of how many values have been added (up to N)
int count = 0

// Waits for the user to input a desired pressure via serial in the format: PSET,<frac>,<exp>
// Fills the desired_pressure struct with fractional, exponent, and computed pressure
void waitForPressureInput(pressure_measurement &desired_pressure) {
  Serial.println("WAITING_FOR_PRESSURE");

  String fracStr = "";
  String expStr = "";

  // Wait for a valid "PSET" command line
  while (true) {
    if (Serial.available()) {
      String line = Serial.readStringUntil('\n');
      line.trim();
      if (line.startsWith("PSET,")) {
        int firstComma = line.indexOf(',');
        int secondComma = line.indexOf(',', firstComma + 1);
        if (secondComma != -1) {
          fracStr = line.substring(firstComma + 1, secondComma);
          expStr = line.substring(secondComma + 1);
          break;
        }
      }
    }
  }

  // Parse and store the desired pressure components
  desired_pressure.frac = fracStr.toFloat();
  desired_pressure.exp = expStr.toInt();
  desired_pressure.pressure = desired_pressure.frac * pow(10, desired_pressure.exp);

  Serial.println("PRESSURE_RECEIVED");
}

// Converts a 6-character string from the Pfeiffer gauge into a pressure_measurement struct
// Format expected: "XXXXYY" where XXXX is mantissa and YY is exponent offset from 20
pressure_measurement pressure_conversion(char *unconverted_pressure) {
  if (strlen(unconverted_pressure) < 6) return {};

  // Parse mantissa and exponent substrings
  char mantissa[5] = {0};
  char exp[3] = {0};
  strncpy(mantissa, unconverted_pressure, 4);
  strncpy(exp, unconverted_pressure + 4, 2);

  // Convert to float and exponent
  int frac = atoi(mantissa);
  int real_exp = atoi(exp) - 20;
  float frac_float = frac / 1000.0;

  // Construct final pressure_measurement
  pressure_measurement measured_pressure;
  measured_pressure.frac = frac_float;
  measured_pressure.exp = real_exp;
  measured_pressure.pressure = frac_float * pow(10, real_exp);
  return measured_pressure;
}

// Determines if pressure has reached equilibrium by evaluating standard deviation of recent readings
// If `clear` is true, the pressure window is reset
// `desired_magnitude` is used to normalize the pressure values
bool reached_equilibrium(float newPressure, bool clear, float desired_magnitude) {
  if (clear)
  {
    // Reset the buffer with placeholder values
    for (int i = 0; i < N; i++)
    {
      pressureWindow[i] = i;
    }
    count = 0;
    return false;
  }

  // Normalize new pressure based on magnitude
  int scale = desired_magnitude * -1;
  float scaled_pressure = newPressure * pow(10, scale);
  pressureWindow[head] = scaled_pressure;

  // Update circular buffer index
  head = (head + 1) % N;
  if (count < N) {
    count++;
    return false;
  }

  // Compute mean of window
  float sum = 0.0;
  for (int i = 0; i < N; i++) sum += pressureWindow[i];
  float mean = sum / N;

  // Compute standard deviation
  float variance = 0.0;
  for (int i = 0; i < N; i++) {
    float diff = pressureWindow[i] - mean;
    variance += diff * diff;
  }
  float stdDev = sqrt(variance / N);

  Serial.print("Current Standard Deivation: ");
  Serial.println(stdDev);

  // Return true if standard deviation is below threshold Q
  return stdDev < Q;
}
