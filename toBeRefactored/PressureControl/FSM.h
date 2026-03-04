#include "Pressure.h"
#pragma once

// Runs the finite state machine that controls the system.
// This function should be called repeatedly from loop()
// to manage transitions based on current system state, time, and pressure.
//
// Input:
//   - desired_pressure: the target pressure the PI loop aims to achieve
void updateFSM(pressure_measurement desired_pressure);
