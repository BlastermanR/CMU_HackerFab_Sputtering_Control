#include "SerialComms.h"
#include <Arduino.h>
#include "ArduinoPfieffer.h"

// External instance of ArduinoPfieffer for controlling the pump
extern ArduinoPfieffer pfieffer;

// Global flag to track pump state (on/off)
bool running;

// Timestamp of last interrupt, used for debouncing
volatile unsigned long last_interrupt_time = 0;

// Sends a setpoint of 0 to the Alicat MFC to stop gas flow
void turn_off_Alicat() {
  float setPoint = 0.0;
  Serial.println("Setpoint to 0");
  ALICATSerial_MFC.listen(); // Enable listening on the Alicat SoftwareSerial
  String command = "AS " + String(setPoint) + "\r";
  ALICATSerial_MFC.print(command); // Send flow command
  delayMicroseconds(100);         // Short delay to allow processing
  readAndProcess(ALICATSerial_MFC); // Read back and process response
}

// Sends command to turn the pump ON
void turn_on_pump() {
  Serial.println("Turning on pump");

  // Create control command to turn on pump
  ASCII_char cmd = pfieffer.control_request("100", "111111");

  RS485Serial_PUMP.listen();       // Enable pump serial
  RS485Mode_PUMP(WRITE);           // Set RS-485 to write mode
  RS485Serial_PUMP.print(cmd);     // Send command to pump
  RS485Mode_PUMP(READ);            // Switch to read mode
  delay(100);                      // Wait for response
  readAndProcess(RS485Serial_PUMP); // Process pump response

  pfieffer.free_message(cmd);      // Free command memory
}

// Sends command to turn the pump OFF
void turn_off_pump() {
  Serial.println("Turning off pump");

  // Create control command to turn off pump
  ASCII_char cmd = pfieffer.control_request("100", "000000");

  RS485Serial_PUMP.listen();       // Enable pump serial
  RS485Mode_PUMP(WRITE);           // Set RS-485 to write mode
  RS485Serial_PUMP.print(cmd);     // Send command to pump
  RS485Mode_PUMP(READ);            // Switch to read mode
  delayMicroseconds(100);          // Short delay before reading
  readAndProcess(RS485Serial_PUMP); // Process pump response

  pfieffer.free_message(cmd);      // Free command memory
}

// Toggles Alicat flow and pump ON/OFF based on running state
void turn_off() {
  Serial.println("Turning Off");
  turn_off_Alicat();               // Always shut off Alicat flow

  // Toggle pump based on current state
  if (running) {
    turn_off_pump();
  } else {
    turn_on_pump();
  }

  running = !running; // Flip state flag
}

// Wrapper function for use in interrupt context
void turn_off_wrapper() {
  unsigned long interrupt_time = millis();

  // Debounce: only allow state change if 200ms has passed since last change
  if (interrupt_time - last_interrupt_time > 200) {
    turn_off(); // Toggle pump and Alicat
  }

  last_interrupt_time = interrupt_time;
}
