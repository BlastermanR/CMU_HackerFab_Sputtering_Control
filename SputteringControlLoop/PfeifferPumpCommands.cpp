/**
 * This file implements command functions for controlling a Pfeiffer vacuum pump
 * via RS485 serial communication. It provides high-level functions to turn the
 * pump on/off and send commands to the pump while managing communication modes
 * and device responses.
 * 
 * @note Requires RS485Serial_PUMP to be initialized for communication
 * 
 * @author HackerFab Supttering Control Automation Team
 * @date Spring 2026
 */

#include "PfeifferPumpCommands.h"


/**
 * @brief Turns on the Pfeiffer vacuum pump
 * 
 * Sends a control request to the Pfeiffer pump to turn it off by sending
 * command code PUMP_POWER_PARAM with PUMP_ON_VALUE. If VERBOSE mode is enabled,
 * prints a debug message to Serial. See header file for param/data definitions.
 * 
 * @param pfeiffer_pump The ArduinoPfeiffer pump object to control
 */
void turnOnPump(ArduinoPfeiffer pfeiffer_pump, RS485Device& pump_serial_wrapper)
{
    #ifdef VERBOSE
        Serial.println("Turning on vacuum pump");
    #endif
    // TODO: set turbo pump to on (should default)
    ASCII_char cmd = pfeiffer_pump.control_request(PUMP_POWER_PARAM, PUMP_ON_VALUE);
    send_command(cmd, pump_serial_wrapper, pfeiffer_pump);
}


/**
 * @brief Turns off the Pfeiffer vacuum pump
 * 
 * Sends a control request to the Pfeiffer pump to turn it off by sending
 * command code PUMP_POWER_PARAM with PUMP_OFF_VALUE. If VERBOSE mode is enabled,
 * prints a debug message to Serial. See header file for param/data definitions.
 * 
 * @param pfeiffer_pump The ArduinoPfeiffer pump object to control
 */
void turnOffPump(ArduinoPfeiffer pfeiffer_pump, RS485Device& pump_serial_wrapper)
{
    #ifdef VERBOSE
        Serial.println("Turning off vacuum pump");
    #endif

    ASCII_char cmd = pfeiffer_pump.control_request(PUMP_POWER_PARAM, PUMP_OFF_VALUE);
    send_command(cmd, pump_serial_wrapper, pfeiffer_pump);
}


/**
 * @brief Reads the current pressure measurement from the Pfeiffer vacuum gauge via RS485 serial communication.
 * 
 * This function queries the pressure gauge using command PUMP_PRESSURE_PARAM, transmits the request,
 * and processes the response to obtain a pressure measurement. If the read fails
 * (indicated by zero values in both exponent and fraction), an error message is printed.
 * 
 * @param pfeiffer_gauge An ArduinoPfeiffer object configured for gauge communication.
 * @return the pressure measurement as a pressure_measurement struct. Error is indicated by pressure being 0
 */
pressure_measurement readPressure(ArduinoPfeiffer pfeiffer_gauge, RS485Device& gauge_serial_wrapper)
{
    gauge_serial_wrapper.port().listen();
    #ifdef VERBOSE
        Serial.println("Querying pressure gauge");
    #endif

    ASCII_char cmd = pfeiffer_gauge.data_request(PUMP_PRESSURE_PARAM);

    gauge_serial_wrapper.setWriteMode();
    gauge_serial_wrapper.port().print(cmd);
    gauge_serial_wrapper.setReadMode();
    delay(100);
    pfeiffer_gauge.free_message(cmd);

    pressure_measurement measured_pressure = readAndProcess(gauge_serial_wrapper.port());

    // Handle failed pressure reading
    if (measured_pressure.exp == 0.0 && measured_pressure.frac == 0.0) {
        Serial.println("ERROR - Failed Pressure Read");
    }

    return measured_pressure;
}


/**
 * @brief Sets the pump speed to a specified percentage.
 * 
 * This function configures the Pfeiffer pump to operate in speed control mode
 * and then sets the pump speed to the desired percentage. Ensure the pump is properly
 * initialized and RS485 communication is active before calling this function.
 * 
 * @param pfeiffer_pump The ArduinoPfeiffer pump object to control.
 * @param percent The desired pump speed as a percentage (20-100).
 *                The value is converted to a 6-digit format with two decimal places
 *                (e.g., 50.5% becomes 005050).
 */
void setPumpSpeed(ArduinoPfeiffer pfeiffer_pump, RS485Device& pump_serial_wrapper, float percent)
{
    // TODO: confirm pump may only be set betwee 20-100
    // check value is in valid range
    if ((percent < 20) || (percent > 100))
    {
        Serial.println("ERROR - pump speed may only be set between 20% and 100%");
        return;
    }
    // first we set the pump to speed control mode with param 026
    #ifdef VERBOSE
        Serial.println("Setting pump to speed mode");
    #endif
    ASCII_char cmd2 = pfeiffer_pump.control_request(PUMP_OPMODE_PARAM, SPEED_MODE_ENABLED_VALUE);
    send_and_process(cmd2, pump_serial_wrapper, pfeiffer_pump);


    // now set the speed parameter 707
    char data[7];
    sprintf(data, "%06d", (int)(percent * 100));

    #ifdef VERBOSE
        Serial.print("Setting pump speed to ");
        Serial.println(percent);
    #endif

    ASCII_char cmd = pfeiffer_pump.control_request(PUMP_SPEED_SET_PARAM, data);
    send_command(cmd, pump_serial_wrapper, pfeiffer_pump);
}


/**
 * @brief Requests and prints the current pump speed from a Pfeiffer pump.
 *
 * Sends a data request command to retrieve the actual pump speed in Hz,
 * transmits it via RS485, and reads the response to process the pump speed data.
 *
 * @param pfeiffer_pump Reference to the ArduinoPfeiffer pump object used to
 *                      generate the speed request command.
 * @param pump_serial_wrapper Reference to the RS485Device wrapper managing
 *                            serial communication with the pump.
 * @see readAndProcess()
 */
void printPumpSpeed(ArduinoPfeiffer& pfeiffer_pump, RS485Device& pump_serial_wrapper)
{
    ASCII_char cmd = pfeiffer_pump.data_request(PUMP_SPEED_HZ_ACT_PARAM);

    pump_serial_wrapper.setWriteMode();
    pump_serial_wrapper.port().print(cmd);
    pump_serial_wrapper.setReadMode();
    delay(100);
    pfeiffer_pump.free_message(cmd);

    readAndProcess(pump_serial_wrapper.port());
}


/**
 * @brief Sends a command to the Pfeiffer pump via RS485 serial communication.
 * 
 * This function configures the RS485 transceiver for write mode, transmits an ASCII
 * command character to the pump, switches to read mode to listen for responses, and
 * cleans up the associated message buffer.
 * 
 * @param command The ASCII character command to send to the pump.
 * @param serial Reference to the RS485Device object for communication.
 * @param device Reference to the ArduinoPfeiffer device object for memory management.
 */
void send_command(ASCII_char command, RS485Device& serial_wrapper, ArduinoPfeiffer& device)
{
    serial_wrapper.port().listen();
    serial_wrapper.setWriteMode();
    serial_wrapper.port().print(command);
    serial_wrapper.setReadMode();
    delay(100); // Allow time for device to respond
    device.free_message(command); // Free memory for command
}

/**
 * @brief Sends a command to the Pfeiffer pump and processes the response
 * 
 * This function configures the RS485 transceiver for write mode, transmits an ASCII
 * command character to the pump, switches to read mode to listen for responses, 
 * processes the response with SerialComms::readAndProcess, and cleans up the associated
 * message buffer.
 * 
 * @param command The ASCII character command to send to the pump.
 * @param serial Reference to the SoftwareSerial object for communication.
 * @param device Reference to the ArduinoPfeiffer device object for memory management.
 */
void send_and_process(ASCII_char command, RS485Device& serial_wrapper, ArduinoPfeiffer& device)
{
  serial_wrapper.port().listen();
  serial_wrapper.setWriteMode();
  serial_wrapper.port().print(command);
  serial_wrapper.setReadMode();
  delay(100); // Allow time for device to respond
  readAndProcess(serial_wrapper.port());
  device.free_message(command); // Free memory for command
}
