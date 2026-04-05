/**
 * @file PfeifferPump.h
 * @brief Defines the Pfeiffer vacuum pump communication interface.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/7/26
 */
#ifndef PFEIFFER_PUMP_H
#define PFEIFFER_PUMP_H

#include "Devices/TC110DriveUnit.h"
#include "IDevice.h"
#include "ISerialDevice.h"
#include "PfeifferLib.h"

// Address of Pfeiffer Gauge
#define PFEIFFER_PUMP_ADDRESS 1

class PfeifferPump : public IDevice
{
  private:
    // Define the serial port
    ISerialDevice *serialPort;

    // Abstract device handling parameter definitions for TC110 pump
    Pfeiffer::TC110DriveUnit pumpDef;

    // Time tracking
    uint32_t lastPollTime = 0;

    // Flag to indicate a new response has been received for state logic
    bool newResponse = false;

    // Polling Interval to send command
    uint64_t pollingInterval_ms = 50;

    // Speed read from pump
    double actualPumpSpeed_hz{0};

    // Set pump speed read from pump
    double setPumpSpeed{0};

    // Bool set when pump signal activation
    bool pumpActivated{false};

    // Flag to indicate new data has arrived
    bool newDataFlag = false;

    /**
     * @brief Utilizes UART port to send message to device
     * @param message Null terminating message to send to device.
     * \0 is not sent
     */
    void sendMessage(const char *message) override;

  public:
    /**
     * @brief Constructor
     * @param dev ISerialDevice instance to use.
     */
    PfeifferPump(ISerialDevice *dev);

    /**
     * @brief Destructor
     */
    ~PfeifferPump();

    /**
     * @brief Initialize the device (e.g., configuring hardware peripherals,
     * setting up ports)
     */
    void init() override;

    /**
     * @brief logic update function
     *
     * Polls
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
     * @brief Returns whether a new valid pump speed reading was received
     * @return True if new speed data is available
     */
    bool hasNewSpeedData();

    /**
     * @brief Get actual pump speed in Hz
     * @return Speed in Hz
     */
    double getActualPumpSpeed_hz() { return actualPumpSpeed_hz; }

    /***************** Pump *****************/

    /**
     * @brief Non-blocking signal to activate pump
     */
    void signalPumpOn();

    /**
     * @brief Blocking signal to activate pump
     *
     * Calls signalPumpOn under the hood and waits for a response
     *
     * @return True if pump activates, False if failure
     */
    bool activatePump();

    /**
     * @brief Non-blocking signal to deactivatePump
     */
    bool signalPumpOff();

    /**
     * @brief Blocking signal to deactivate pump
     *
     * Calls signalPumpOff under the hood and waits for a response
     *
     * @return True if pump activates, False if failure
     */
    bool deactivatePump();

    /**
     * @brief Blocking signal to vent chamber
     * @return True if pump activates, False if error returned
     */
    bool ventPump();

    /**
     * @brief Gets the pump speed from the latest reading
     * @return Speed in hertz
     */
    double getPumpSpeed() { return actualPumpSpeed_hz; }
};

#endif // PFEIFFER_PUMP_H
