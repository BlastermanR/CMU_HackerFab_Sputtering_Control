/**
 * This module provides a testing framework for communicating with and controlling
 * Alicat Mass Flow Controllers (MFCs) and Pfeiffer Vacuum pump/gauge device.
 * 
 * TODO: Make SerialComms.h/cpp modular
 * TODO: 
 */
#include "config.h"
#include "SerialComms.h"
#include "ArduinoPfeiffer.h"
#include "PfeifferPumpCommands.h"

// Instantiate two Pfeiffer device handlers: one for the pump, one for the gauge
ArduinoPfeiffer pfeiffer_pump((ASCII_char)"001");
ArduinoPfeiffer pfeiffer_gauge((ASCII_char)"002");

/**
 * One-time setup function initializes the serial communication and runs basic
 * tests for the vacuum pump and alicat MFC
 */
void setup()
{
    initializeSerials();         // Set up hardware serial ports
    initializeRS485Pins();       // Configure RS485 direction control pins
    // TODO: attach interrupt

    delay(100);  // Short delay to allow Serial to initialize

    testVacuum();
}


/**
 * Do nothing
 */
void loop() {
    Serial.println("Testing complete");
    delay(5000);
}


/**
 * Run a basic procedure to test the vacuum pump, including turning it on, reading
 * the pressure, varying the pump speed, and turning it off.
 */
void testVacuum()
{
    Serial.println("Beginning vacuum pump test");

    turnOnPump(pfeiffer_pump);
    delay(10000); // 10s delay

    pressure_measurement reading = readPressure(pfeiffer_gauge);
    Serial.print("Current Pressure: ");
    printPressureReading(reading);

    setPumpSpeed(pfeiffer_pump, 21);
    delay(10000);
    setPumpSpeed(pfeiffer_pump, 99);
    delay(10000);

    turnOffPump(pfeiffer_pump);
    delay(5000);
    Serial.println("Vacuum pump test complete!");
}


/**
 * @brief helper function to print a pressure reading
 * 
 * Prints given specified pressure_measurement struct from both the .pressure and
 * .frac/.exp members -- these should be equivalent
 * 
 * @param reading the pressure_measurement struct to print
 */
void printPressureReading(pressure_measurement reading)
{
    Serial.print(reading.pressure);
    Serial.print(" - ");
    Serial.print(reading.frac);
    Serial.print("e");
    Serial.println(reading.exp);
}
