#pragma once

// Global flag indicating if the pump is currently running
extern bool running;

// Interrupt-safe wrapper function to toggle pump and Alicat state
// Debounces input and switches between ON and OFF modes
void turn_off_wrapper();
