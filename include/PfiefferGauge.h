#include "UARTInterface.h"
#include "IDevice.h"
#include "picoDefinitions.h"
#include "PfiefferLib.h"
#include "pico/types.h"

/**
 * TODO
 * 
 * @author Ryan Massie (rmassie)
 * @date 3/7/26
 */
class PfiefferGauge : public IDevice
{
    private:
    // Define the serial port
    IUart* serialPort;

    // Transmit/Receive GPIO pin for RS485 chip
    unsigned int rtsPin;

    // Flag to indicate a new response has been received for state logic
    bool newResponse = false;

    // Variable to store the latest chamber pressure reading
    double chamberPressure_hPa = 0.0;

    /**
     * @brief Override the default data received handler to parse pressure readings from the Pfieffer Gauge.
     */
    void onDataReceived(char c) override;

    /**
     * @brief Override the debug print function to prefix messages with "Pressure Gauge"
     */
    void printRecieved() override;

    public:

    /**
     * @brief Utilizes UART port to send message to device
     * @param message Null terminating message to send to device.
     * \0 is not sent
     * 
     * TODO: MARK PRIVATE AFTER TESTING
     */
    void sendMessage(const char* message) override;

    /**
     * @brief Constructor
     * @param uart Uart instance to use.
     * @param rtsPin GPIO pin number for transmit/receive control.
     */
    PfiefferGauge(IUart* uart, unsigned int rtsPin);

    /**
     * @brief Destructor
     */
    ~PfiefferGauge();

    /**
     * @brief Initialize the device (e.g., configuring hardware peripherals, setting up ports)
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
    double readPressure()
    {
        return chamberPressure_hPa;
    }
};