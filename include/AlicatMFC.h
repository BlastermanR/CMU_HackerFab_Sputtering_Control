#ifndef ALICAT_MFC
#define ALICAT_MFC

#include <stdio.h>
#include "UARTInterface.h"
#include "IDevice.h"

/**
 * Defines the Alicat Mass Flow Controller (MFC).
 * - Utilizes UART --> RS232 converter (MAX3232)
 * - Interrupt driven reciever handler
 * 
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */
class AlicatMFC : public IDevice
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
    AlicatMFC(IUart* uart);

    /**
     * @brief Destructor
     */
    ~AlicatMFC();

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


#endif // ALICAT_MFC