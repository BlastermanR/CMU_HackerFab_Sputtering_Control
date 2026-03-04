#include <SoftwareSerial.h>

/*
Types of Commands
General Info: [unit_id]\r
Query setpoint: [unit_id]LS set_point_val units_val
*/

const char* gen_info = "A\r";
//Standard mL/s is 3
const char* change_setpoint = "ALS %d 3\r";

// Define TX and RX pins for SoftwareSerial
const uint8_t TX_pin = 2;
const uint8_t RX_pin = 3;

uint16_t setpoint;
char buffer[256];
int pressure;
int desired_pressure;
SoftwareSerial mySerial(RX_pin, TX_pin); // RX first, TX second

void setup() {
    Serial.begin(9600);   // Initialize hardware serial
    mySerial.begin(9600); // Initialize software serial
    setpoint = 0;
}


void loop() {
    mySerial.write("a\r"); //Should be to alicat pressure sensor
    uint8_t index = 0;
    while ((x = mySerial.read()) != '\r') {
        buffer[index] = mySerial.read();
        index++;
    }
    buffer[index] = "\r";
    values = parseString(buffer);
    if (values[1] < desired_pressure)
    {
        setpoint++;
        sprintf(change_setpoint, "ALS %d 3\r", setpoint);
    }
    delay(500);
}

String* parseString(char input[]) {
    static String values[10]; // Adjust size as needed
    char *token = strtok(input, " ");
    int index = 0;
    
    while (token != NULL && index < 10) {
        values[index++] = String(token);
        token = strtok(NULL, " ");
    }
    
    return values;
}
