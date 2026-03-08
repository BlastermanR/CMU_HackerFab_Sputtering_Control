#ifndef I_DEVICE_H
#define I_DEVICE_H

#include <stdio.h>

/**
 * Interface definition for generic devices on the Sputtering automation system.
 * This ensures that Mass Flow Controllers (MFCs), Vacuum Pumps, and Pressure Gauges
 * follow a unified initialization and communication interface.
 * 
* @author Ryan Massie (rmassie)
 * @date 3/7/26
 */

class IDevice {
protected:
    // Common receive buffer properties
    static constexpr int BUFFER_SIZE = 100;
    char recieveBuffer[BUFFER_SIZE + 1];
    int bufferIndex = 0;

public:

    /**
     * @brief Default destructor
     */
    virtual ~IDevice() = default;

    /**
     * @brief Initialize the device (e.g., configuring hardware peripherals, setting up ports)
     */
    virtual void init() = 0;

    /**
     * @brief Send a bare message or command to the device
     * @param message Null-terminated string or bytes to send
     */
    virtual void sendMessage(const char* message) = 0;

    /**
     * @brief Routine update function for non-blocking state machines and background tasks.
     * Used to parse incoming buffers, run logic, etc.
     */
    virtual void update() = 0;

    /**
     * @brief Default handler for receiving data.
     * @param c Character (byte) received
     */
    virtual void onDataReceived(char c) 
    {
        if (bufferIndex < BUFFER_SIZE) {
            recieveBuffer[bufferIndex] = c;

            // Check if the character just added is a line terminator
            if (c == '\r' || c == '\n') 
            {
                recieveBuffer[bufferIndex] = '\0';
#ifdef DEBUG
                printRecieved();
#endif
                bufferIndex = 0;
            } 
            else 
            {
                bufferIndex++; // Move to next slot
            }
        } 
        else 
        {
            // Handle overflow: buffer is full without finding a newline
            recieveBuffer[BUFFER_SIZE] = '\0'; 
            printf("FULL BUFFER ERROR: %s\n", recieveBuffer);
            bufferIndex = 0;
        }
    }

private:
    /**
     * @brief Debug hook to print the received buffer.
     * Can be overridden by each device to prefix their specific name.
     */
    virtual void printRecieved() 
    {
        printf("Device Message Received: %s\n", recieveBuffer);
    }
};

#endif // I_DEVICE_H
