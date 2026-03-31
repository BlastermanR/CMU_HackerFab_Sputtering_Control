/**
 * Intercore.cpp
 * @brief Implementation of inter-core shared variables.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/21/26
 */
#include "Intercore.h"

// Communication Atomics
std::atomic<uint16_t> statusReg{0};