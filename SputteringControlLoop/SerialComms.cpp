#include "SerialComms.h"
#include "Pressure.h"
#include <Arduino.h>

// RS485 driver enable and receiver enable pins for pump and gauge
#define SerialDEpin_PUMP 5
#define SerialREpin_PUMP 4
#define SerialDEpin_GAUGE 9
#define SerialREpin_GAUGE 8

// ASCII carriage return used as message delimiter
const char endChar = '\r';

// Define software serial ports for each device
SoftwareSerial RS485Serial_PUMP(6, 7);     // Pump: RO, DI
SoftwareSerial RS485Serial_GAUGE(10, 11);  // Gauge: RO, DI
SoftwareSerial ALICATSerial_MFC(13, 12);   // MFC: RX, TX

// RS485 mode flags
#define READ 0
#define WRITE 1

// Buffer for incoming messages
#define sentenceSize 128
char sentence[sentenceSize];
int sentenceIndex = 0;

// Initializes all serial ports used in the system
void initializeSerials() {
  RS485Serial_PUMP.begin(9600);
  delay(30);
  RS485Serial_GAUGE.begin(9600);
  delay(30);
  ALICATSerial_MFC.begin(9600);
  Serial.begin(9600);
}

// Configures the RS485 direction control pins
void initializeRS485Pins() {
  pinMode(SerialDEpin_PUMP, OUTPUT);
  pinMode(SerialREpin_PUMP, OUTPUT);
  pinMode(SerialDEpin_GAUGE, OUTPUT);
  pinMode(SerialREpin_GAUGE, OUTPUT);

  // Default mode is read
  RS485Mode_PUMP(READ);
  RS485Mode_GAUGE(READ);
}

// Set RS485 mode for the pump
void RS485Mode_PUMP(bool mode) {
  digitalWrite(SerialDEpin_PUMP, mode);
  digitalWrite(SerialREpin_PUMP, mode);
}

// Set RS485 mode for the gauge
void RS485Mode_GAUGE(bool mode) {
  digitalWrite(SerialDEpin_GAUGE, mode);
  digitalWrite(SerialREpin_GAUGE, mode);
}

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
