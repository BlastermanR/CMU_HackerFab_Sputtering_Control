#include <SoftwareSerial.h>

// RS485 PUMP Serial (pins: RX 8, TX 9)
SoftwareSerial RS485Serial_PUMP(8, 9);
// RS485 GAUGE Serial (pins: RX 4, TX 5)
SoftwareSerial RS485Serial_GAUGE(4, 5);
// Alicat MFC Serial (pins: RX 13, TX 12)
SoftwareSerial ALICATSerial_MFC(13, 12);

#define SerialDEpin_PUMP 6
#define SerialREpin_PUMP 7
#define SerialDEpin_GAUGE 2
#define SerialREpin_GAUGE 3

#define READ  0
#define WRITE 1

const char endChar = '\r';
#define sentenceSize 128
char sentence[sentenceSize];
int sentenceIndex = 0;

// RS485 control functions
void RS485Mode_PUMP(bool mode) {
  digitalWrite(SerialDEpin_PUMP, mode);
  digitalWrite(SerialREpin_PUMP, mode);
}

void RS485Mode_GAUGE(bool mode) {
  digitalWrite(SerialDEpin_GAUGE, mode);
  digitalWrite(SerialREpin_GAUGE, mode);
}

enum QueryState { PUMP, GAUGE, ALICAT };
QueryState queryState = PUMP;
unsigned long lastActionTime = 0;
const unsigned long interval = 2000;

void setup() {
  // Initialize serial ports
  RS485Serial_PUMP.begin(9600);
  delay(30);
  RS485Serial_GAUGE.begin(9600);
  delay(30);
  ALICATSerial_MFC.begin(9600);
  
  Serial.begin(9600);

  // Setup RS485 control pins
  pinMode(SerialDEpin_PUMP, OUTPUT);
  pinMode(SerialREpin_PUMP, OUTPUT);
  pinMode(SerialDEpin_GAUGE, OUTPUT);
  pinMode(SerialREpin_GAUGE, OUTPUT);
  
  RS485Mode_PUMP(READ);
  RS485Mode_GAUGE(READ);
}

void loop() {
  unsigned long now = millis();
  if (now - lastActionTime >= interval) {
    switch(queryState) {
      case PUMP:
        // Pump: Activate its serial, send command, then process response.
        RS485Serial_PUMP.listen();
        Serial.println("Sending pump command");
        RS485Mode_PUMP(WRITE);
        // (For example, sending just the endChar; adjust if you have a real command)
        RS485Serial_PUMP.write(endChar);
        RS485Mode_PUMP(READ);
        delay(100); // Allow time for response
        readAndProcess(RS485Serial_PUMP);
        queryState = GAUGE;
        break;
      case GAUGE:
        // Gauge: Activate its serial, send command, then process response.
        RS485Serial_GAUGE.listen();
        Serial.println("Sending gauge command");
        RS485Mode_GAUGE(WRITE);
        RS485Serial_GAUGE.write("0020074002=?107");
        RS485Serial_GAUGE.write(endChar);
        RS485Mode_GAUGE(READ);
        delay(100); // Allow time for response
        readAndProcess(RS485Serial_GAUGE);
        queryState = ALICAT;
        break;
      case ALICAT:
        // Alicat: Activate its serial, send command, then process response.
        ALICATSerial_MFC.listen();
        Serial.println("Sending Alicat command");
        ALICATSerial_MFC.write("a\r");
        delay(100); // Allow time for response
        readAndProcess(ALICATSerial_MFC);
        queryState = PUMP;
        break;
    }
    lastActionTime = now;
  }
}

// Reads from the provided SoftwareSerial until no data is available.
// It accumulates characters until the end character is reached.
void readAndProcess(SoftwareSerial &ss) {
  while (ss.available()) {
    char c = ss.read();
    if (c != endChar && sentenceIndex < sentenceSize - 1) {
      sentence[sentenceIndex++] = c;
    } else {
      // Terminate the string and process it.
      sentence[sentenceIndex] = '\0';
      processSentence(sentence);
      sentenceIndex = 0; // Reset for the next message
    }
  }
}

// Processes the received sentence by checking its parameter code.
// For example, parameter 309 prints pump speed, 740 prints pressure.
void processSentence(char* msg) {
  // Debug print the full raw message.
  Serial.print("Received message: ");
  Serial.println(msg);

  // Ensure the message is long enough for processing.
  if (strlen(msg) < 11) {
    Serial.println("Message too short.");
    return;
  }
  
  const int param_len = 3;
  char param[param_len + 1] = {0};
  strncpy(param, msg + 5, param_len);
  int paramValue = atoi(param);
  
  if (paramValue == 309) {
    const int data_len = 6;
    char data[data_len + 1] = {0};
    strncpy(data, msg + 10, data_len);
    Serial.print("Pump speed: ");
    Serial.println(data);
  } 
  else if (paramValue == 740) {
    const int data_len = 6;
    char data[data_len + 1] = {0};
    strncpy(data, msg + 10, data_len);
    Serial.print("Pressure: ");
    Serial.println(data);
  }
}
