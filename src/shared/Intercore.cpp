/**
 * Intercore.cpp
 * @brief Implementation of inter-core shared variables.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/21/26
 */
#include "Intercore.h"

// Communication Atomics
std::atomic<uint32_t> statusReg{0};
std::atomic<uint8_t>  verbosityLevel{V_DEBUG}; // Set default to V_DEBUG to see initialization logs

// Inter-core communication queues
queue_t commandQueue;
queue_t core0OutQueue;
queue_t core1OutQueue;

void initQueues()
{
    queue_init(&commandQueue, sizeof(CommandMessage), COMMAND_QUEUE_SIZE);
    queue_init(&core0OutQueue, sizeof(OutputMessage), OUTPUT_QUEUE_SIZE);
    queue_init(&core1OutQueue, sizeof(OutputMessage), OUTPUT_QUEUE_SIZE);
}