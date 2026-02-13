/**
 * This module provides a testing framework for communicating with and controlling
 * Alicat Mass Flow Controllers (MFCs) and Pfeiffer Vacuum pump/gauge device.
 * 
 */
#include "config.h"
#include "SerialComms.h"
#include "ArduinoPfeiffer.h"
#include "PfeifferPumpCommands.h"
#include "AlicatMfcCommands.h"

#define interruptPin 2

// ---------- User-tunable constants ----------
// TODO: set actual values for these
float EVAC_PRESSURE      = 1.0e-3;   // target evacuation pressure
float IGNITE_PUMP_HZ     = 250.0;    // pump speed in Hz for ignition
float ARGON_IGNITE_FLOW_SCCM   = 10.0;     // MFC flow during ignition
float SPUTTER_SETPOINT   = 5.0e-3;   // target sputter pressure
unsigned long long SPUTTER_MS = 60UL * 1000UL; // sputter duration in ms
// --------------------------------------------

// state for different sputtering phases
enum State {
  STATE_EVACUATE,
  STATE_IGNITE,
  STATE_SPUTTER,
  STATE_DEPRESSURIZE,
  STATE_IDLE,
};

State state = STATE_IDLE;
unsigned long stateStartMs;

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

unsigned long last_interrupt_time = 0; // global to debounce interrupt

/**
 * One-time setup function initializes the serial communication and runs basic
 * tests for the vacuum pump and alicat MFC
 */
void setup()
{
    initializeSerials();         // Set up hardware serial ports
    setupInterrupt();

    delay(100);  // Short delay to allow Serial to initialize

    state = STATE_IDLE;
    stateStartMs = millis();
}


/**
 * Switch current state and handle commands
 */
void loop() {
  handleSerialCommands();  // read user input and store command

  switch (state) {
    case STATE_IDLE:
    {
      /* Idle state - keep everything off, wait for start request */
      // Ensure everything is safe
      shutoffAll(false);

      if (userRequestedStart())
      {
        Serial.println(F("Starting EVACUATE sequence..."));
        Serial.println(F("REMINDER: Ensure the gas cylinders have been opened!"));
        turnOnPump(pfeiffer_pump, pump_serial_wrapper);
        setPumpSpeed(pfeiffer_pump, pump_serial_wrapper, 100); // 100% speed
        state = STATE_EVACUATE;
        stateStartMs = millis();
      }

      // print message
      Serial.println(F("IDLE - Type 'evac' to proceed to EVACUATION"));
      Serial.print(F("IDLE - Pump speed: "));
      Serial.print(getPumpSpeed(pfeiffer_pump, pump_serial_wrapper));
      Serial.println(F("Hz"));
      break;
    }

    case STATE_EVACUATE:
    {
      /* Evacuate state - max out vacuum pump, wait for evac pressure */
      float pressure = readPressure(pfeiffer_gauge, gauge_serial_wrapper);
      Serial.print(F("EVAC - pressure: "));
      Serial.println(pressure);

      Serial.print(F("EVAC - Pump speed: "));
      Serial.print(getPumpSpeed(pfeiffer_pump, pump_serial_wrapper));
      Serial.println(F("Hz"));

      if (pressure <= EVAC_PRESSURE && pressure != -1.0) {
        Serial.println(F("EVAC - Reached EVAC_PRESSURE. Type 'ignite' to proceed to IGNITE."));
      }

      if (userGoToIgnite())
      {
        Serial.println(F("Transitioning to IGNITE..."));
        Serial.println(F("REMINDER: Ensure shutter is closed."));
        Serial.println(F("REMINDER: Power on the RF source and begin impedance matching."));
        setPumpSpeed(pfeiffer_pump, pump_serial_wrapper, IGNITE_PUMP_HZ/10);
          // divide pump HZ by 10, since setPumpSpeed excpects fan % of 1000Hz
        state = STATE_IGNITE;
        stateStartMs = millis();
      }

      // TODO: only allow gotoignite when pump speed reached?
      break;
    }

    case STATE_IGNITE:
    {
      /**
       * Ignite state
       * - Wait for pump to reach 250Hz
       * - Flow argon at 10sccm
       */

      // Wait until pump actually reaches target speed
      int fanSpeedHz = getPumpSpeed(pfeiffer_pump, pump_serial_wrapper);
      if ((fanSpeedHz > IGNITE_PUMP_HZ) || (fanSpeedHz < 0.8*IGNITE_PUMP_HZ))
      {
        Serial.println(F("IGNITE - Waiting for pump to reach ignition speed..."));
        Serial.print(F("IGNITE - Pump speed: "));
        Serial.print(getPumpSpeed(pfeiffer_pump, pump_serial_wrapper));
        Serial.println(F("Hz"));
        break;  // stay in IGNITE
      }

      // Once at speed, open MFC to ignition flow (only once)
      static bool mfcOpened = false;
      if (!mfcOpened)
      {
        Serial.println(F("IGNITE - Pump at ignition speed. Opening MFC for ignition."));
        setAlicatPressure(argon_mfc_serial, ARGON_IGNITE_FLOW_SCCM);
        // TODO: add oxygen mfc
        mfcOpened = true;
      }

      Serial.println(F("IGNITE - To proceed to sputtering, type 'sputter' when ignition is achieved."));

      if (userConfirmIgnition())
      {
        Serial.println(F("Ignition confirmed. Entering SPUTTER."));
        Serial.println(F("REMINDER: Let the target clean for 15 mins before despositing."));
        mfcOpened = false; // reset for next cycle
        state = STATE_SPUTTER;
        stateStartMs = millis();
      }

      break;
    }

    case STATE_SPUTTER:
    {
      /** Sputter state - maintain pressure, wait for specified time */
      unsigned long elapsed = millis() - stateStartMs;

      unsigned long totalSeconds = elapsed / 1000;
      unsigned int minutes = totalSeconds / 60;
      unsigned int seconds = totalSeconds % 60;

      // Example status line
      Serial.print(F("SPUTTERING - Elapsed time: "));
      Serial.print(minutes);
      Serial.print(F("m "));
      if (seconds < 10) Serial.print('0');  // leading zero for 0–9
      Serial.print(seconds);
      Serial.println(F("s"));


      // TODO: Run PID loop to hold SPUTTER_SETPOINT by adjusting MFCs

      if (elapsed >= SPUTTER_MS)
      {
        Serial.println(F("Sputter time complete. Depressurizing..."));
        shutoffAll(true);
        state = STATE_IDLE;
        stateStartMs = millis();
        break;
      }

      Serial.print(F("SPUTTERING - Pressure at "));
      Serial.print(readPressure(pfeiffer_gauge, gauge_serial_wrapper));
      Serial.print(F("hPa - pump speed "));
      Serial.print(getPumpSpeed(pfeiffer_pump, pump_serial_wrapper));
      Serial.println(F("Hz"));

      if (userRequestedStop())
      {
        Serial.println(F("Sputtering stopped by user. Depressurizing..."));
        shutoffAll(true);
        state = STATE_IDLE;
        stateStartMs = millis();
        break;
      }

      Serial.println(F("SPUTTERING - If plasma is lost, type 'reignite' to return to ignition"));
      Serial.println(F("SPUTTERING - To end sputtering early, type 'stop'"));

      if (userReignite())
      {
        Serial.println(F("Returning to ignition phase."));
        state = STATE_IGNITE;
        stateStartMs = millis();
      }

      break;
    }

    case STATE_DEPRESSURIZE:
    {
      /** Depressurize state - wait for fan to reach 0 */

      int fanSpeedHz = getPumpSpeed(pfeiffer_pump, pump_serial_wrapper);
      if (fanSpeedHz == 0)
      {
        Serial.println(F("Pump fan stopped - chamber safe to vent. Ensure MFCs are closed. Entering IDLE state"));
        state = STATE_IDLE;
        stateStartMs = millis();
        break;
      }

      Serial.println(F("DEPRESSURIZE - wait for fan to stop before venting chamber"));

      Serial.print(F("IGNITE - Pump speed: "));
      Serial.print(fanSpeedHz);
      Serial.println(F("Hz"));

      break;
    }
  }

  delay(1000);
}


/* ======================================================
 *  Serial command handling (high level)
 * ====================================================*/

#define CMD_BUFFER_LEN 16

char cmdBuffer[CMD_BUFFER_LEN];
char lastCommand[CMD_BUFFER_LEN];
uint8_t cmdIndex = 0;
bool newCommandAvailable = false;


/**
 * @brief read user commands and store to buffer
 */
void handleSerialCommands() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (cmdIndex > 0) {
        cmdBuffer[cmdIndex] = '\0';
        // copy to lastCommand
        strncpy(lastCommand, cmdBuffer, CMD_BUFFER_LEN);
        lastCommand[CMD_BUFFER_LEN - 1] = '\0';
        cmdIndex = 0;
        newCommandAvailable = true;

        Serial.print(F("Received command: "));
        Serial.println(lastCommand);
      }
    } else {
      if (cmdIndex < CMD_BUFFER_LEN - 1) {
        // normalize to lowercase
        if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
        cmdBuffer[cmdIndex++] = c;
      }
    }
  }
}


/** @brief check if user command was 'start'/'evac' */
bool userRequestedStart()
{
  if (!newCommandAvailable) return false;

  if (strncmp(lastCommand, "start", 5) == 0 || strncmp(lastCommand, "evac", 4) == 0) {
    newCommandAvailable = false;
    return true;
  }
  return false;
}


/** @brief check if user command was 'stop' */
bool userRequestedStop()
{
  if (!newCommandAvailable) return false;

  if (strncmp(lastCommand, "stop", 5) == 0) {
    newCommandAvailable = false;
    return true;
  }
  return false;
}


/** @brief check if user command was 'ignite' */
bool userGoToIgnite()
{
  if (!newCommandAvailable) return false;

  if (strncmp(lastCommand, "ignite", 6) == 0) {
    newCommandAvailable = false;
    return true;
  }
  return false;
}


/** @brief check if user command was 'sputter' */
bool userConfirmIgnition()
{
  if (!newCommandAvailable) return false;

  if (strncmp(lastCommand, "sputter", 7) == 0) {
    newCommandAvailable = false;
    return true;
  }
  return false;
}


/** @brief check if user command was 'reignite' */
bool userReignite()
{
  if (!newCommandAvailable) return false;

  if (strncmp(lastCommand, "reignite", 8) == 0) {
    newCommandAvailable = false;
    return true;
  }
  return false;
}


/* ======================================================
 *  Test procedures
 * ====================================================*/


/**
 * Run a basic procedure to test the vacuum pump, including turning it on, reading
 * the pressure, varying the pump speed, and turning it off.
 */
void testVacuum()
{
    Serial.println(F("Beginning vacuum pump test"));

    turnOnPump(pfeiffer_pump, pump_serial_wrapper);
    delay(10000); // 10s delay

    float pressure = readPressure(pfeiffer_gauge, gauge_serial_wrapper);
    Serial.print(F("Current Pressure: "));
    Serial.print(pressure);

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
    Serial.println(F("Vacuum pump test complete!"));
}


/**
 * Run a basic procedure to test the Alicat MFC by setting the control to
 * different values before reverting to 0.
 */
void testAlicat()
{
    Serial.println(F("Beginning alicat test"));

    float test_setting = 10;

    Serial.print(F("Setting sccm to "));
    Serial.println(test_setting);

    setAlicatPressure(argon_mfc_serial, test_setting);
    delay(5000);

    test_setting = 5;

    Serial.print(F("Setting sccm to "));
    Serial.println(test_setting);

    setAlicatPressure(argon_mfc_serial, test_setting);
    delay(5000);

    test_setting = 0;

    Serial.print(F("Setting sccm to "));
    Serial.println(test_setting);

    setAlicatPressure(argon_mfc_serial, test_setting);

    Serial.println(F("Done testing alicat"));
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


/*
 * =============================================
 *              EMERGENCY SHUTOFF
 * =============================================
 */


/**
 * @brief shut off all equipment
 * 
 * this function shuts off vacuum pump and sets MFCs to zero
 */
void shutoffAll(bool printMsg)
{
    turnOffPump(pfeiffer_pump, pump_serial_wrapper);

    setAlicatPressure(argon_mfc_serial, 0);
    // TODO: oxygen mfc
    if (printMsg)
        Serial.println(F("Shutoff complete"));
}


/**
 * @brief interrupt to trigger emergency shutoff
 * 
 * Interrupt triggers the shutoffAll function. This can only be triggered every 200ms
 * to help debounce.
 */
void emergencyInterrupt()
{
    unsigned long interrupt_time = millis();

    // Debounce: only trigger if 200ms has passed since last trigger
    if (interrupt_time - last_interrupt_time > 200) {
      Serial.println(F("EMERGENCY STOP TRIGGERED"));
      shutoffAll(true);
      state = STATE_IDLE;
      stateStartMs = millis();
    }

    last_interrupt_time = interrupt_time;
}


/**
 * @brief set up pin and attach interrupt for emergency stop
 * 
 * sets interruptPin to INPUT_PULLUP so that an interrupt is triggered by
 * tying interruptPin to GND
 */
void setupInterrupt()
{
    // Attach interrupt for manual shutdown button
    pinMode(interruptPin, INPUT_PULLUP);
    attachInterrupt(
        digitalPinToInterrupt(interruptPin),
        emergencyInterrupt,
        FALLING
    );
}
