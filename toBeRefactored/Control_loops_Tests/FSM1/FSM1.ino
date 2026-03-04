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
const unsigned long interval = 10000;

//PID control variables
//If you want faster change increase the values of the coeffs
float Kp = 1000; float Ki = 0.0; float Kd = 1000;
float measuredPressure;
float measuredPressure_frac;
int measuredPressure_exp;
float desiredPressure;
float desiredPressure_frac;
int desiredPressure_exp;
float integralTerm = 0;
float lastError = 0;
float currSetPoint = 0;
float error;


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

  Serial.print("Desired pressure frac (hPa): ");
    
  while (!Serial.available()); // Wait for user input
  Serial.println();
  
  String frac = Serial.readStringUntil('\n'); // Read input
  frac.trim();
  desiredPressure_frac = frac.toFloat();

  Serial.print("Desired pressure magnitude (hPa): ");
    
  while (!Serial.available()); // Wait for user input
  Serial.println();
  
  String exp = Serial.readStringUntil('\n'); // Read input
  exp.trim();
  desiredPressure_exp = exp.toInt();

  desiredPressure = desiredPressure_frac * pow(10, desiredPressure_exp);

}

void loop() {
  unsigned long now = millis();
  if (now - lastActionTime >= interval) {
    switch(queryState) {
      case PUMP:
        Serial.println("Pumping Chamber");
        // Pump: Activate its serial, send command, then process response.
        RS485Serial_PUMP.listen();
        //Serial.println("Sending pump command");
        RS485Mode_PUMP(WRITE);
        //Turn on pump
        RS485Serial_PUMP.write("0011001006111111015");
        //Turn off Pump
        //RS485Serial_PUMP.write("0011001006000000009");
        RS485Serial_PUMP.write(endChar);
        RS485Mode_PUMP(READ);
        delay(100); // Allow time for response
        readAndProcess(RS485Serial_PUMP);
        queryState = GAUGE;
        break;
      case GAUGE:
        // Gauge: Activate its serial, send command, then process response.
        RS485Serial_GAUGE.listen();
        //Serial.println("Sending gauge command");
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
        //Serial.println("Sending Alicat command");
        String command = "AS " + String(currSetPoint) + "\r";
        ALICATSerial_MFC.print(command);
        delay(100); // Allow time for response
        readAndProcess(ALICATSerial_MFC);
        queryState = PUMP;
        break;
    }
    //Not yet read a pressure
    if (measuredPressure_frac == 0) return;
    lastActionTime = now;
    error = desiredPressure - measuredPressure;
    Serial.print("Desired Pressure frac: ");
    Serial.println(desiredPressure_frac);
    Serial.print("Desired Pressure exp: ");
    Serial.println(desiredPressure_exp);
    

    Serial.print("Measured Pressure frac: ");
    Serial.println(measuredPressure_frac);
    Serial.print("Measured Pressure exp: ");
    Serial.println(measuredPressure_exp);

    if (error == 0.0) Serial.println("Reached Pressure");
    if (abs(error) < 0.00001) return;

    float proportional = Kp * error;
    Serial.print("Proportional Contribution: ");
    Serial.println(proportional);

    integralTerm = integralTerm + (error * interval);
    float integral = Ki * integralTerm;
    Serial.print("Integral Contribution: ");
    Serial.println(integral);

    float derivative = (error - lastError) / interval;
    lastError = error;
    float derivativeOutput = Kd * derivative;
    Serial.print("Derivative Contribution: ");
    Serial.println(derivativeOutput);

    float adjustment = proportional + integral + derivativeOutput;
    if (abs(adjustment) < 0.01) adjustment = 0;
    currSetPoint += adjustment;
    if (currSetPoint > 50) currSetPoint = 50;
    else if (currSetPoint < 0) currSetPoint = 0;

    Serial.print("Current Setpoint: ");
    Serial.println(currSetPoint);
    Serial.print("Adjustment: ");
    Serial.println(adjustment);

    //Uncomment when trying to turn off
    //currSetPoint = 0;

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
  //Serial.print("Received message: ");
  //Serial.println(msg);

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
    measuredPressure = pressure_conversion(data);
  }
}

float pressure_conversion(char *unconverted_pressure)
{
    if (strlen(unconverted_pressure) < 6) return -1; // Check if string is at least 6 chars

    const int mantissa_len = 4;
    const int exp_len = 2;
    char mantissa[mantissa_len + 1] = {0};
    char exp[exp_len + 1] = {0};

    strncpy(mantissa, unconverted_pressure, mantissa_len);
    strncpy(exp, unconverted_pressure + 4, exp_len);

    int frac = atoi(mantissa);
    int real_exp = atoi(exp) - 20;

    float frac_float = frac / 1000.0;
    /*
    Serial.print("Frac: ");
    Serial.println(frac_float);
    Serial.print("Exp: ");
    Serial.println(real_exp);
    */

    measuredPressure_frac = frac_float;
    measuredPressure_exp = real_exp;

    float pressure = frac_float * pow(10, real_exp);

    return pressure;
}