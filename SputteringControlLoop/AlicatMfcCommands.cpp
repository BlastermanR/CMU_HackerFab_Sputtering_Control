#include "AlicatMfcCommands.h"


/**
 * @brief Sets the Alicat mass flow controller to a specified pressure/flow rate.
 * 
 * Sends a command to the Alicat MFC device to set the desired setpoint value (in SCCM).
 * The function constructs an "AS" (Analog Setpoint) command, transmits it via serial,
 * waits briefly for the device to process, and then reads the response.
 * 
 * @note Requires ALICATSerial_MFC to be initialized.
 * 
 * @param sccm The desired setpoint value in Standard Cubic Centimeters per Minute (SCCM).
 */
void setAlicatPressure(SoftwareSerial& serialPort, float sccm)
{
    #ifdef VERBOSE
        Serial.print("Setting Alicat to ");
        Serial.println(sccm);
    #endif
        
    String command = "AS " + String(sccm) + "\r";
    serialPort.listen();
    serialPort.print(command);
    delay(100);
}

// TODO: do we need to be able to read SCCM?
