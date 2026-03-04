#include <SoftwareSerial.h>

//Turn on pump
//RS485Serial.write("0011001006111111015");
//Turn off Pump
//RS485Serial.write("0011001006000000009");


SoftwareSerial RS485Serial(10, 11); // RX, TX

//RS485 Communication control pins
#define SerialDEpin 2
#define SerialREpin 3

// Values used for RS485 communication control
#define READ 0
#define WRITE 1

const char endChar = '\r';

#define sentenceSize 128
char sentence[sentenceSize];

// Helper function for changing both RS485 comm pins
// to receive or transmit
void RS485Mode(bool mode) {
  digitalWrite(SerialDEpin, mode);
  digitalWrite(SerialREpin, mode);
}

void setup() {
  // Serial to RS485:
  RS485Serial.begin(9600);
  // Serial to USB for debugging
  Serial.begin(9600);

  //RS485 Communication control pins
  pinMode(SerialDEpin, OUTPUT);
  pinMode(SerialREpin, OUTPUT);
  RS485Mode(READ);
}


// Used to cycle through messages for Serial.write()
// It seems that Pfeiffer TC 110 only accepts one message per Serial.write()
// or the RS485 r/w timing is slightly off,
// thus queryTurn cycles through messages
int queryTurn = 0;

bool newData = false;


unsigned long lastCheckMillis = 0;
int interval = 10000;

void loop() {
  // Current runtime in ms
  // Sending of serial messages, LCD value refresh, and "blank" LCD (no new data)
  // depend on independent update intervals
  unsigned long currentMillis = millis();
  // Only one message can be sent at a time to the pump drive it seems
  // Also, less than or greater than ~20s delay seems to either not send the message
  // or not give enough time to receive
  if (currentMillis - lastCheckMillis > interval) {
    RS485Mode(WRITE);
    if (queryTurn == 0) {

      //Current measured pressure
      RS485Serial.write("0010074002=?106");
      
      RS485Serial.write(endChar);
      queryTurn++;
    } else if (queryTurn == 1) {
      //[P:346] TempMotor, 0010034602=?108\r
      //RS485Serial.write("0010034602=?108");
      //Turn on pump
      //RS485Serial.write("0011001006111111015");
      //Turn off Pump
      RS485Serial.write("0011001006000000009");
      RS485Serial.write(endChar);
      
      queryTurn = 0;
      //queryTurn++;
    } else if (queryTurn == 2) {
      //[P:316] DrvPower, 0010031602=?105\r
      RS485Serial.write("0010031602=?105");
      RS485Serial.write(endChar);
      //queryTurn = 0;
    }
    // Delay to allow outgoing serial buffer to clear before
    // switching back to read from RS485
    // More or less corresponds to length of char array sent
    // delay(17) get data
    //delay(10);
    // No delay gets good values
    RS485Mode(READ);
    lastCheckMillis = currentMillis;
  }

  readSerial(currentMillis);
}

void readSerial(unsigned long currentMillis) {
  RS485Mode(READ);
  if (RS485Serial.available()) {
    static int i = 0;
    char ch = RS485Serial.read();
    Serial.println((uint8_t)ch, HEX);
    if (ch != endChar && ch != -1 && i < sentenceSize - 1) {
      sentence[i] = ch;
      i++;
    } else if (ch == endChar || i == sentenceSize - 1) {
      sentence[i] = '\0';
      i++;
      while (i < sentenceSize) {
        sentence[i] = '\0';
        i++;
      }
      i = 0;
      newData = true;
    }
  }
}

// Sums ASCII values for chars in a char string
int asciisum(int st, int en) {
  int asciitotal = 0;
  for (int i = st; i <= en; i++) {
    asciitotal = asciitotal + sentence[i];
  }
  return asciitotal;
}

// Makes sure all parameters included in message through the data
// section equal the checksum value passed at the end of the received message
bool validCheckSum(char sentence[]) {
  char checksum[4];
  strncpy(checksum, sentence + 16, 3);
  //checksum[3] = '\0';
  int intchecksum = atoi(checksum);
  int modulo = asciisum(0, 15) % 256;
  return (intchecksum == modulo);
}