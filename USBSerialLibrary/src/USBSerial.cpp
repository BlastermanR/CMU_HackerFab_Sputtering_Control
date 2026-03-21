#include "USBSerial.h"
#include "pico/stdlib.h"
#include <stdio.h>

USBSerial::USBSerial() : inputBuffer("") {}

void USBSerial::begin()
{
    // stdio_init_all() should usually be called once at the very top of main(),
    // but we can put our startup delay and welcome message here.
    sleep_ms(2000);
    printf("USB Serial Interface Initialized.\n");
}

void USBSerial::setCallback(UsbCommandCallback cb) { commandCallback = cb; }

void USBSerial::print(const char *str) { printf("%s", str); }

void USBSerial::println(const char *str) { printf("%s\n", str); }

// THIS MUST BE CALLED IN YOUR MAIN WHILE() LOOP
void USBSerial::update()
{
    int c = getchar_timeout_us(0); // Non-blocking read

    if (c != PICO_ERROR_TIMEOUT)
    {
        // If the user presses 'Enter'
        if (c == '\n' || c == '\r')
        {
            if (!inputBuffer.empty())
            {

                // Fire the callback and pass the completed string!
                if (commandCallback)
                {
                    commandCallback(inputBuffer);
                }

                // Clear the buffer for the next command
                inputBuffer.clear();
            }
        }
        // Ignore backspaces for now, just append standard characters
        else if (c >= 32 && c <= 126)
        {
            inputBuffer += (char)c;
        }
    }
}