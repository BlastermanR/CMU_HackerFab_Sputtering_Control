/**
 * IDevice.h
 * @brief Interface definition for generic devices on the Sputtering automation system.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/7/26
 */
#ifndef I_DEVICE_H
#define I_DEVICE_H

#include "picoDefinitions.h"
#include <stdio.h>

class IDevice
{
  public:
    /**
     * @brief Default destructor
     */
    virtual ~IDevice() = default;

    /**
     * @brief Initialize the device (e.g., configuring hardware peripherals,
     * setting up ports)
     */
    virtual void init() = 0;

    /**
     * @brief Send a bare message or command to the device
     * @param message Null-terminated string or bytes to send
     */
    virtual void sendMessage(const char *message) = 0;

    /**
     * @brief Routine update function for non-blocking state machines and
     * background tasks. Used to parse incoming buffers, run logic, etc.
     */
    virtual void update() = 0;
};

#endif // I_DEVICE_H
