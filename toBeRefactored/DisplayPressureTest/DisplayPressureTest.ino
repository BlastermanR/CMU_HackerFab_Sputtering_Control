// Logarithmic curve test for pressure plot script
unsigned long startTime;
float desiredPressure = 50.0;

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for serial monitor to open
  startTime = millis();
}

void loop() {
  unsigned long currentTime = millis();
  float t = (currentTime - startTime) / 1000.0; // Time in seconds

  // Generate a logarithmic-style curve: measured = log(t + 1) * scale
  // +1 to avoid log(0), scale to match pressure range
  float measuredPressure = log(t + 1) * 15.0;

  // Format: DATA,<time_ms>,<desired>,<measured>
  Serial.print("DATA,");
  Serial.print(currentTime);
  Serial.print(",");
  Serial.print(desiredPressure, 2);
  Serial.print(",");
  Serial.println(measuredPressure, 2);

  delay(100); // Send data at 10 Hz
}
