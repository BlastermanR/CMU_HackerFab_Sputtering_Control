#include <SoftwareSerial.h>

// Define TX and RX pins for SoftwareSerial
const uint8_t TX_pin = 12;
const uint8_t RX_pin = 13;
SoftwareSerial mySerial(RX_pin, TX_pin); // RX first, TX second

void setup() {
    Serial.begin(9600);   // Initialize hardware serial
    mySerial.begin(9600); // Initialize software serial
}


void loop() {
    mySerial.write("a\r");
    int x;
    while ((x = mySerial.read()) != '\r' && x != -1) {
        Serial.write(x);
        
        //Serial.print("Received from Software Serial: ");
        //Serial.println(data);
    }
    Serial.write("\r\n");
    delay(500);
}
