#include "Pressure.h"
#pragma once

// Declaration of function that prints pressure data to Serial in a formatted way
// Inputs:
//   - desired_pressure: target pressure the system aims to reach
//   - measured_pressure: current pressure read from the sensor
void displayPressure(pressure_measurement desired_pressure, pressure_measurement measured_pressure);
