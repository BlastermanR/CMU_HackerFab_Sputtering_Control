#include "DisplayPressure.h"
#include "Pressure.h"
#include <Arduino.h>

// Function to print desired and measured pressure values to the Serial Monitor
// in a comma-separated format suitable for parsing in external scripts
void displayPressure(pressure_measurement desired_pressure, pressure_measurement measured_pressure)
{
    Serial.print(F("DATA,")); // Prefix for identifying pressure data
    Serial.print(millis());   // Timestamp in milliseconds since Arduino started
    Serial.print(F(","));
    Serial.print(desired_pressure.pressure, 6); // Print desired pressure with 6 decimal places
    Serial.print(F(","));
    Serial.println(measured_pressure.pressure, 6); // Print measured pressure with 6 decimal places and newline
}
