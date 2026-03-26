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
#include "AlicatLib.h"

class AlicatMFC : public IDevice
{
  private:
    // Define the serial port
    ISerialDevice *serialPort; 

    // Device state
    char deviceId{'A'};
    AlicatDataFrame lastData;

    /**
     * @brief Helper function to format and send a command to the device
     * @param cmd Command structure to format and send
     */
    void sendCommand(const AlicatCommand& cmd);

  public:
    /**
     * @brief Constructor
     * @param dev ISerialDevice instance to use.
     */
    AlicatMFC(ISerialDevice *dev, char id = 'A');

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
     * @brief Send a raw message directly to the device
     * @param message Text string to send
     */
    void sendMessage(const char *message) override;

    // --- MFC Specific Commands ---

    /**
     * @brief Polls current data from the MFC. Populates lastData on response.
     */
    void pollData();

    /**
     * @brief Sets the target flow rate (setpoint).
     * @param setpoint Value in the device's selected units.
     */
    void setSetpoint(double setpoint);

    /**
     * @brief Changes the active gas.
     * @param gasNumber The gas index to switch to.
     */
    void setGas(int gasNumber);

    /**
     * @brief Tares the flow sensor (creates a no-flow reference).
     */
    void tareFlow();


    // --- Data Getters ---

    /**
     * @brief Get the most recently parsed data frame.
     * @return Const reference to the last AlicatDataFrame.
     */
    const AlicatDataFrame& getLastData() const;

    /**
     * @brief Get the mass flow reading from the last data frame.
     * @return Mass flow value.
     */
    double getMassFlow() const;

    /**
     * @brief Get the volumetric flow reading from the last data frame.
     * @return Volumetric flow value.
     */
    double getVolumetricFlow() const;

    /**
     * @brief Get the absolute pressure reading from the last data frame.
     * @return Absolute pressure value.
     */
    double getPressure() const;

    /**
     * @brief Get the temperature reading from the last data frame.
     * @return Temperature value.
     */
    double getTemperature() const;

    /**
     * @brief Get the setpoint reading from the last data frame.
     * @return Setpoint value.
     */
    double getSetpoint() const;

    /**
     * @brief Get the gas type reading from the last data frame.
     * @return Gas type string.
     */
    std::string getGasType() const;

};

#endif // ALICAT_MFC