#include <stdio.h>
#include "pico/stdlib.h"
#include "picoDefinitions.h"
#include "SputteringManager.h"
#include "USBSerial.h"

#define DEBUG // Enables debug functionality

// Define USB Serial
USBSerial pcTerminal;

void onPcCommand(const std::string& command) 
{
    pcTerminal.println(command.c_str());
}

int main()
{
    stdio_init_all();

    // Setup the serial communication
    pcTerminal.begin();
    pcTerminal.setCallback(onPcCommand);
\
    // Initialize the Sputtering Manager
    SputteringManager manager;
    manager.init();

    // Alicat Test message
    const char* s = "A\r"; 
    
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
