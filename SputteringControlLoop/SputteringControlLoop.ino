/**
 * This module provides a testing framework for communicating with and controlling
 * Alicat Mass Flow Controllers (MFCs) and Pfeiffer Vacuum pump/gauge device.
 * 
 * TODO: 
 */
#include "config.h"
#include "SerialComms.h"
#include "ArduinoPfeiffer.h"
#include "PfeifferPumpCommands.h"
#include "AlicatMfcCommands.h"

// Create serial devices for pump/gauge
RS485Device pump_serial_wrapper(/*rx=*/6, /*tx=*/7,     // Pump: RO, DI
                                /*de=*/5, /*re=*/4);    // Driver/Receiver enable
RS485Device gauge_serial_wrapper(/*rx=*/10, /*tx=*/11,  // Gauge: RO, DI
                                 /*de=*/9,  /*re=*/8);  // Driver/Receiver enable
SoftwareSerial argon_mfc_serial(13, 12);                // MFC: RX, TX
// TODO: add oxygen mfc

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
    // TODO: attach interrupt

    delay(100);  // Short delay to allow Serial to initialize

    testVacuum();
    testAlicat();
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

    turnOnPump(pfeiffer_pump, pump_serial_wrapper);
    delay(10000); // 10s delay

    pressure_measurement reading = readPressure(pfeiffer_gauge, gauge_serial_wrapper);
    Serial.print("Current Pressure: ");
    printPressureReading(reading);

    delay(1000);

    setPumpSpeed(pfeiffer_pump, pump_serial_wrapper, 21);

    for(int i = 0; i < 10; i++)
    {
        printPumpSpeed(pfeiffer_pump, pump_serial_wrapper);
        delay(1000);
    }

    setPumpSpeed(pfeiffer_pump, pump_serial_wrapper, 99);

    for(int i = 0; i < 10; i++)
    {
        printPumpSpeed(pfeiffer_pump, pump_serial_wrapper);
        delay(1000);
    }

    turnOffPump(pfeiffer_pump, pump_serial_wrapper);
    delay(5000);
    Serial.println("Vacuum pump test complete!");
}


/**
 * Run a basic procedure to test the Alicat MFC by setting the control to
 * different values before reverting to 0.
 */
void testAlicat()
{
    Serial.println("Beginning alicat test");

    float test_setting = 10;

    Serial.print("Setting sccm to ");
    Serial.println(test_setting);

    setAlicatPressure(argon_mfc_serial, test_setting);
    delay(5000);

    test_setting = 5;

    Serial.print("Setting sccm to ");
    Serial.println(test_setting);

    setAlicatPressure(argon_mfc_serial, test_setting);

    test_setting = 0;

    Serial.print("Setting sccm to ");
    Serial.println(test_setting);

    setAlicatPressure(argon_mfc_serial, test_setting);

    Serial.println("Done testing alicat");
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


/**
 * @brief Initializes all serial communications for the sputtering control system.
 * 
 * Sets up serial connections for the pump controller, pressure gauge, argon mass flow controller,
 * and the main Serial interface. Each serial connection is initialized at 9600 baud.
 * 
 * The Pfeiffer vacuum pump and gauge use a RS485Device wrapper, which combines the SoftwareSerial
 * port with the RS485 mode pins. The MFCs use SoftwareSerial directly.
 */
void initializeSerials() {
  pump_serial_wrapper.begin(9600);
  delay(30);
  gauge_serial_wrapper.begin(9600);
  delay(30);
  argon_mfc_serial.begin(9600);
  // TODO: add oxygen mfc
  Serial.begin(9600);
}
