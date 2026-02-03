#include "SerialComms.h"
#include "Pressure.h"

// ASCII carriage return used as message delimiter
const char endChar = '\r';

// Buffer for incoming messages
#define sentenceSize 128
char sentence[sentenceSize];
int sentenceIndex = 0;


// Reads a complete message from the serial port and processes it
pressure_measurement readAndProcess(SoftwareSerial &ss) {
  pressure_measurement result = {};  // Default empty pressure

  while (ss.available()) {
    char c = ss.read();

    // Accumulate characters until carriage return is found
    if (c != endChar && sentenceIndex < sentenceSize - 1) {
      sentence[sentenceIndex++] = c;
    } else {
      sentence[sentenceIndex] = '\0';         // Null-terminate the string
      result = processSentence(sentence);     // Parse and process message
      sentenceIndex = 0;                      // Reset for next message
    }
  }
  return result;
}

// Parses a received message and returns pressure if applicable
pressure_measurement processSentence(char* msg) {
  pressure_measurement result = {};  // Default empty result

  // Ignore short or malformed messages
  if (strlen(msg) < 11) return result;

  char param[4] = {0};
  strncpy(param, msg + 5, 3);  // Parameter code is located at offset 5
  int paramValue = atoi(param);

  if (paramValue == 309) {
    char data[7] = {0};
    strncpy(data, msg + 10, 6);  // Read pump speed data
    Serial.print("Pump speed: ");
    Serial.println(data);
  } else if (paramValue == 740) {
    char data[7] = {0};
    strncpy(data, msg + 10, 6);  // Read pressure data
    result = pressure_conversion(data);
  } else if (paramValue == 717) {
    char data[7] = {0};
    strncpy(data, msg + 10, 6);  // Read standby setpoint
    Serial.print("Standby Val: ");
    Serial.println(data);
  } else if (paramValue == 707) {
    char data[7] = {0};
    strncpy(data, msg + 10, 6);  // Read set speed value
    Serial.print("Set Speed Val: ");
    Serial.println(data);
  } else if (paramValue == 2) {
    char data[7] = {0};
    strncpy(data, msg + 10, 6);  // Read standby status
    Serial.print("Standby Status: ");
    Serial.println(data);
  } else if (paramValue == 26) {
    char data[7] = {0};
    strncpy(data, msg + 10, 6);  // Read set speed status
    Serial.print("Set Speed Status: ");
    Serial.println(data);
  }

  return result;
}
