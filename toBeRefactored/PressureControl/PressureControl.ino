#include "SerialComms.h"
#include "Pressure.h"
#include "FSM.h"
#include "Interrupt.h"
#include "PID.h"
#include "ArduinoPfieffer.h"

#define interruptPin 2  // Pin for external interrupt button

// Global pressure setpoint entered by user
pressure_measurement desired_pressure;

// Instantiate two Pfieffer device handlers: one for the pump, one for the gauge
ArduinoPfieffer pfieffer((ASCII_char)"001");
ArduinoPfieffer pfieffer_gauge((ASCII_char)"002");

void setup() {
  running = false;  // Start in idle mode until 's' command is received from Python

  initializeSerials();         // Set up hardware serial ports
  initializeRS485Pins();       // Configure RS485 direction control pins
  attachInterrupt(
    digitalPinToInterrupt(interruptPin),
    turn_off_wrapper,
    RISING
  );  // Attach interrupt for manual shutdown button

  delay(100);  // Short delay to allow Serial to initialize
  waitForPressureInput(desired_pressure);  // Receive desired pressure from host
  init_coefficients(desired_pressure);     // Configure PID gains based on input
}

void loop() {
  // Check for incoming serial commands
  if (Serial.available()) {
    char input = Serial.read();

    // Start control loop when 's' or 'S' is received
    if (input == 's' || input == 'S') {
      running = true;
      Serial.println(F("Starting pressure control..."));
    }
    // Manual shutdown when 'i' or 'I' is received
    else if (input == 'i' || input == 'I') {
      turn_off_wrapper();
    }
  }

  // If system is running, update the finite state machine
  if (running) {
    updateFSM(desired_pressure);
  }
}
