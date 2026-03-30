/**
 * Core1Main.cpp
 * @brief Implementation of the main loop for Core 1.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/21/26
 */

#include "Core1Main.h"
#include "Intercore.h"
#include "SputteringManager.h"
#include "pico/util/queue.h"

// Entry point for Core 1
void core1_entry()
{
    // Initialize the Sputtering Manager locally on Core 1
    SputteringManager manager;

    bool run{true};

    while (run)
    {
        // Process the item on core 1...
        // e.g. manager.handlePacket(packet);
    }
}
