#include <stdio.h>
#include "pico/stdlib.h"
#include "picoDefinitions.h"
#include "SputteringManager.h"
#include "USBSerial.h"

#define DEBUG // Enables debug functionality

// Define Global Systems
USBSerial pcTerminal;
SputteringManager manager;

void onPcCommand(const std::string& command) 
{
    // Echo the command back to the PC
    pcTerminal.println(command.c_str());

    // Future: Pass the command to the Sputtering Manager to parse and control the system
    // manager.parseCommand(command);
}

int main()
{
    stdio_init_all();

    // Setup the serial communication
    pcTerminal.begin();
    pcTerminal.setCallback(onPcCommand);

    // Initialize the Sputtering Manager
    manager.init();
    
    // Store the time we last sent a message
    uint32_t lastSendTime = to_ms_since_boot(get_absolute_time());

    while (true) 
    {
        pcTerminal.update();
        manager.update();
        
        // Get the current time
        uint32_t currentTime = to_ms_since_boot(get_absolute_time());

        // Check if 5 seconds have passed since the last send
        if (currentTime - lastSendTime >= 10000) 
        {
            // manager.sendMessage(s); // Example if manager had a wrapper
            
            // Reset the timer
            lastSendTime = currentTime;
        }
    }
}
