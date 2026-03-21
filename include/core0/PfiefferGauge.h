/**
 * PfiefferGauge.h
 * @brief Defines the Pfeiffer pressure gauge communication interface.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/7/26
 */
#ifndef PFIEFFER_GAUGE_H
#define PFIEFFER_GAUGE_H

#include "IDevice.h"
#include "ISerialDevice.h"
#include "PfiefferLib.h"
#include "pico/types.h"
#include "picoDefinitions.h"

class PfiefferGauge : public IDevice
{
  private:
    // Define the serial port
    ISerialDevice *serialPort;

    // Flag to indicate a new response has been received for state logic        
    bool newResponse = false;

    // Variable to store the latest chamber pressure reading
    double chamberPressure_hPa = 0.0;

  public:
    /**
     * @brief Utilizes UART port to send message to device
     * @param message Null terminating message to send to device.
     * \0 is not sent
     *
     * TODO: MARK PRIVATE AFTER TESTING
     */
    void sendMessage(const char *message) override;

    /**
     * @brief Constructor
     * @param dev ISerialDevice instance to use.
     */
    PfiefferGauge(ISerialDevice *dev);

    /**
     * @brief Destructor
     */
    ~PfiefferGauge();

    /**
     * @brief Initialize the device (e.g., configuring hardware peripherals,    
     * setting up ports)
     */
    void init() override;

    /**
     * @brief logic update function
     * Used to parse incoming buffers, run logic, etc.
     * Updates Values:
     * - Chamber Pressure
     */
    void update() override;

    /**
     * @brief Retrieve the latest chamber pressure reading
     * @return The latest chamber pressure value
     */
    double readPressure() { return chamberPressure_hPa; }
};

#endif // PFIEFFER_GAUGE_H
