/**
 * PfeifferGauge.h
 * @brief Defines the Pfeiffer pressure gauge communication interface.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/7/26
 */
#ifndef PFEIFFER_GAUGE_H
#define PFEIFFER_GAUGE_H

#include "Devices/MPT200.h"
#include "IDevice.h"
#include "ISerialDevice.h"
#include "PfeifferLib.h"
#include "pico/types.h"
#include "picoDefinitions.h"

// Address of Pfeiffer Gauge
#define PFEIFFER_GAUGE_ADDRESS 2

class PfeifferGauge : public IDevice
{
  private:
    // Define the serial port
    ISerialDevice *serialPort;

    // Abstract device handling parameter definitions for MPT200 gauge
    Pfeiffer::MPT200 gaugeDef;

    // Time tracking
    uint32_t lastPollTime = 0;

    // Flag to indicate a new response has been received for state logic
    bool newResponse = false;

    // Polling Interval to send command
    uint64_t pollingInterval_ms = 1000;

    // Variable to store the latest chamber pressure reading
    double chamberPressure_hPa = 0.0;

    // Flag to indicate new data has arrived
    bool newDataFlag = false;

    /**
     * @brief Utilizes UART port to send message to device
     * @param message Null terminating message to send to device.
     * \0 is not sent
     *
     */
    void sendMessage(const char *message) override;

  public:
    /**
     * @brief Constructor
     * @param dev ISerialDevice instance to use.
     */
    PfeifferGauge(ISerialDevice *dev);

    /**
     * @brief Destructor
     */
    ~PfeifferGauge();

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

    /***************** Polling *****************/

    /**
     * @brief Sets the interval for the poll command.
     * @param ms Interval in ms
     */
    void setPollingInterval_ms(uint64_t ms) { pollingInterval_ms = ms; }

    /**
     *  @brief Returns the polling interval in ms
     *  @return Polling Interval
     */
    uint64_t getPollingInterval_ms() { return pollingInterval_ms; }

    /**
     * @brief Polls device for up to date pressure
     */
    void pollDevice();

    /**
     * @brief Returns whether a new valid pressure reading was received
     * @return True if new data is available
     */
    bool hasNewData();

    /***************** Gauge *****************/

    /**
     * @brief Retrieve the latest chamber pressure reading
     * @return The latest chamber pressure value
     */
    double getPressure() { return chamberPressure_hPa; }
};

#endif // PFEIFFER_GAUGE_H
