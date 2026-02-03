#pragma once
#include <SoftwareSerial.h>
#include "Pressure.h"

// External declarations for software serial interfaces to RS-232/RS-485 devices
extern SoftwareSerial RS485Serial_PUMP;   // Serial connection to the vacuum pump
extern SoftwareSerial RS485Serial_GAUGE;  // Serial connection to the pressure gauge
extern SoftwareSerial ALICATSerial_MFC;   // Serial connection to the mass flow controller (MFC)

// RS485 direction control macros
#define READ 0   // Set RS485 device to receive mode
#define WRITE 1  // Set RS485 device to transmit mode

// Initializes all serial ports used in the system
void initializeSerials();

// Sets pin modes for RS485 direction control lines and defaults them to READ
void initializeRS485Pins();

// Sets pump RS485 mode (READ or WRITE) by toggling DE/RE pins
void RS485Mode_PUMP(bool mode);

// Sets gauge RS485 mode (READ or WRITE) by toggling DE/RE pins
void RS485Mode_GAUGE(bool mode);

// Reads a message from a device and processes it
// Returns a pressure_measurement if the message is pressure data
pressure_measurement readAndProcess(SoftwareSerial &ss);

// Parses a complete message string and extracts pressure if applicable
pressure_measurement processSentence(char* msg);
