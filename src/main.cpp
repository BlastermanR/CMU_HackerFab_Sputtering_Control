#include "SputteringManager.h"
#include "USBSerial.h"
#include "pico/stdlib.h"
#include "picoDefinitions.h"
#include <stdio.h>

// Define Global Systems
USBSerial         pcTerminal;
SputteringManager manager;

void onPcCommand(const std::string &command)
{
    // Echo the command back to the PC
    pcTerminal.println(command.c_str());

    // Future: Pass the command to the Sputtering Manager to parse and control the
    // system manager.parseCommand(command);
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
        if (currentTime - lastSendTime >= 3000)
        {
#ifdef DEBUG
            manager.sendTestMessage();
#endif
            // Reset the timer
            lastSendTime = currentTime;
        }
    }
}
