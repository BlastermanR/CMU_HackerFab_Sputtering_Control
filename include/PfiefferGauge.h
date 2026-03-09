#include "UARTInterface.h"
#include "IDevice.h"
#include "picoDefinitions.h"

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

    // Override the debug print for specifically this device
    void printRecieved() override;

    public:
    /**
     * @brief Constructor
     * @param uart Uart instance to use.
     */
    PfiefferGauge(IUart* uart);

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
     */
    void update() override;

    /**
     * @brief Utilizes UART port to send message to device
     * @param message Null terminating message to send to device.
     * \0 is not sent
     */
    void sendMessage(const char* message) override;
};