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
    pump_serial_wrapper.sendRS485Command(cmd);
    pfeiffer_pump.free_message(cmd);
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
    pump_serial_wrapper.sendRS485Command(cmd);
    pfeiffer_pump.free_message(cmd);
}


/**
 * @brief helper to convert a pressure reading into a float
 * 
 * This helper converts the value field of an RS485 reply telegram
 * to a pressure value. The argument should be the value field
 * (starting at offset 10) of the data response message. Use with
 * the `rs485_response.value` field of the struct returned by
 * SerialComms.h/cpp `parseRS485Response`.
 * 
 * @param rs485ReplyValue c-string message with the value field
 * @return float value of pressure reading
 */
float parsePressureValue(char* rs485ReplyValue)
{
  if (strlen(rs485ReplyValue) < 6) return {};

  // Parse mantissa and exponent substrings
  char mantissa[5] = {0};
  char exp[3] = {0};
  strncpy(mantissa, rs485ReplyValue, 4);
  strncpy(exp, rs485ReplyValue + 4, 2);

  // Convert to float and exponent
  int frac = atoi(mantissa);
  int real_exp = atoi(exp) - 20;
  float frac_float = frac / 1000.0;

  // Construct final pressure_measurement
  return frac_float * pow(10, real_exp);
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
float readPressure(ArduinoPfeiffer& pfeiffer_gauge, RS485Device& gauge_serial_wrapper)
{
    gauge_serial_wrapper.port().listen();
    #ifdef VERBOSE
        Serial.println("Querying pressure gauge");
    #endif

    ASCII_char cmd = pfeiffer_gauge.data_request(PUMP_PRESSURE_PARAM);
    gauge_serial_wrapper.sendRS485Command(cmd);
    delay(100);

    rs485_response response = gauge_serial_wrapper.readRS485Reply();
    pfeiffer_gauge.free_message(cmd);

    if (strncmp(response.param, PUMP_PRESSURE_PARAM, 3) != 0)
    {
        Serial.println("ERROR - Failed pressure read");
        return -1.0f;
    }

    return parsePressureValue(response.value);
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
    pump_serial_wrapper.sendRS485Command(cmd2);
    pfeiffer_pump.free_message(cmd2);


    // now set the speed parameter 707
    char data[7];
    sprintf(data, "%06d", (int)(percent * 100));

    #ifdef VERBOSE
        Serial.print("Setting pump speed to ");
        Serial.println(percent);
    #endif

    ASCII_char cmd = pfeiffer_pump.control_request(PUMP_SPEED_SET_PARAM, data);
    pump_serial_wrapper.sendRS485Command(cmd);
    pfeiffer_pump.free_message(cmd);
}

/**
 * @brief Requests the current pump speed from a Pfeiffer pump.
 *
 * Sends a data request command to retrieve the actual pump speed in Hz,
 * transmits it via RS485, and reads the response to process the pump speed data.
 *
 * @param pfeiffer_pump Reference to the ArduinoPfeiffer pump object used to
 *                      generate the speed request command.
 * @param pump_serial_wrapper Reference to the RS485Device wrapper managing
 *                            serial communication with the pump.
 * @return turbo pump fan speed in Hz
 */
int getPumpSpeed(ArduinoPfeiffer& pfeiffer_pump, RS485Device& pump_serial_wrapper)
{
    ASCII_char cmd = pfeiffer_pump.data_request(PUMP_SPEED_HZ_ACT_PARAM);

    pump_serial_wrapper.sendRS485Command(cmd);
    delay(100);

    rs485_response response = pump_serial_wrapper.readRS485Reply();
    pfeiffer_pump.free_message(cmd);
    
    if (strncmp(response.param, PUMP_SPEED_HZ_ACT_PARAM, 3) != 0)
    {
        Serial.println(response.param);
        Serial.println("ERROR reading fan speed");
        return -1;
    }

    return atoi(response.value);
}


/**
 * @brief Requests and prints the current pump speed from a Pfeiffer pump.
 *
 * Gets the pump speed with getPumpSpeed() and prints it to the Serial port.
 * 
 * @param pfeiffer_pump Reference to the ArduinoPfeiffer pump object used to
 *                      generate the speed request command.
 * @param pump_serial_wrapper Reference to the RS485Device wrapper managing
 *                            serial communication with the pump.
 */
void printPumpSpeed(ArduinoPfeiffer& pfeiffer_pump, RS485Device& pump_serial_wrapper)
{
    Serial.print("Pump fan speed: ");
    Serial.println(getPumpSpeed(pfeiffer_pump, pump_serial_wrapper));
}
