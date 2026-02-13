#include "SerialComms.h"


/**
 * @brief send a command to the RS485 device
 * 
 * This class method sends a formatted command to the RS485
 * device over the serial port. The command should be formatted using
 * the ArduinoPfeiffer.h/cpp helper functions
 */
void RS485Device::sendRS485Command (ASCII_char cmd)
{
  this->port().listen();
  this->setWriteMode();
  this->port().print(cmd);
  this->setReadMode();
}


/**
 * @brief extract the parameter and value from an RS485 message
 * 
 * This class method is used to read the parameter number and value
 * return from an RS485 device over serial.
 * 
 * @param msg c-string with the RS485 reply
 * @return rs485_repsonse struct with the param and value strings populated.
 *          return.param will be "000\0" on error
 */
rs485_response RS485Device::parseRS485Response (char* msg)
{
  rs485_response response;
  if (strlen(msg) < 11) 
  {
    Serial.println("ERROR response from RS485 device too short");
    strncpy(response.param, "000\0", 4);
    return response;
  }

  strncpy(response.param, msg + 5, 3);  // Parameter code is located at offset 5
  strncpy(response.value, msg + 10, 6);  // Read pump speed data

  return response;
}


/**
 * @brief read a reply from an RS485 device
 * 
 * This class method reads the serial port associated with the device and parses
 * the reply with RS485Device::parseRS485Response(). The function reads the serial
 * port until the endChar (\r) is received, and then the sentence is parsed.
 * The function returns an rs485_response struct with the parameter and value
 * populated as c-strings. See parseRS485Response() for error returns.
 * 
 * @return rs485_repsonse struct with the param and value strings populated.
 */
rs485_response RS485Device::readRS485Reply ()
{
  rs485_response result;

  while (this->port().available()) {
    char c = this->port().read();

    // Accumulate characters until carriage return is found
    if (c != endChar && this->sentenceIndex < SENTENCE_SIZE - 1) {
      this->sentence[this->sentenceIndex++] = c;
    } else {
      this->sentence[this->sentenceIndex] = '\0';   // Null-terminate the string
      result = parseRS485Response(this->sentence);  // Parse and process message
      this->sentenceIndex = 0;                      // Reset for next message
    }
  }
  return result;
}
