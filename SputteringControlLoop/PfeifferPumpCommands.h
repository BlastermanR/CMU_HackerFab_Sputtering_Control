/** See PfeifferPumpCommands for decriptions. */

#include "config.h"
#include "SerialComms.h"
#include "ArduinoPfeiffer.h"
#include <Arduino.h>

void turnOnPump(ArduinoPfeiffer pfeiffer_pump);

void turnOffPump(ArduinoPfeiffer pfeiffer_pump);

pressure_measurement readPressure(ArduinoPfeiffer pfeiffer_gauge);

void setPumpSpeed(ArduinoPfeiffer pfeiffer_pump, float percent);

void send_command(ASCII_char command, SoftwareSerial& serial, ArduinoPfeiffer& device);

void send_and_process(ASCII_char command, SoftwareSerial& serial, ArduinoPfeiffer& device);
