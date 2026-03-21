/**
 * AlicatMFC.h
 * @brief Defines the Alicat Mass Flow Controller (MFC).
 *
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */
#ifndef ALICAT_MFC
#define ALICAT_MFC

#include "IDevice.h"
#include "ISerialDevice.h"
#include "picoDefinitions.h"

class AlicatMFC : public IDevice
{
  private:
    // Define the serial port
    ISerialDevice *serialPort;

  public:
    /**
     * @brief Constructor
     * @param dev ISerialDevice instance to use.
     */
    AlicatMFC(ISerialDevice *dev);

    /**
     * @brief Destructor
     */
    ~AlicatMFC();

    /**
     * @brief Initialize the device (e.g., configuring hardware peripherals,
     * setting up ports)
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
    void sendMessage(const char *message) override;
};

#endif // ALICAT_MFC