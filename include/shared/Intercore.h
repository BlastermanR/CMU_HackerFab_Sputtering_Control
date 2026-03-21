/**
 * Intercore.h
 * @brief Definitions for inter-core communication data structures and queues.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/21/26
 */
#ifndef INTERCORE_H
#define INTERCORE_H

#include "pico/util/queue.h"

// Define a simple Data Packet structure for inter-core communication
struct DataPacket
{
    int command_id;
    float value;
    // Add additional fields as needed
};

// Queue to pass DataPackets between cores
extern queue_t core_queue;

#endif
