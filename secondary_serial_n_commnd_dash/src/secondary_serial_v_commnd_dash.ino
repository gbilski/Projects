#include <Arduino.h>

// Configurations for Speeduino Secondary Serial
#define SPEEDUINO_SERIAL Serial1 
#define BAUDRATE 115200          // Standard Speeduino secondary baud rate

// Array and sizing configuration
const int TOTAL_BYTES_EXPECTED = 126; // Total bytes including 3 header bytes
uint8_t dataArray[TOTAL_BYTES_EXPECTED];
int byteCount = 0;

// Request timing control
unsigned long lastRequestTime = 0;
const unsigned long requestInterval = 50; // Request every 50ms (20Hz)
bool waitingForResponse = false;
unsigned long responseTimeout = 200;      // 200ms safety timeout

void setup() {
  Serial.begin(115200);          // Debugging to PC
  SPEEDUINO_SERIAL.begin(BAUDRATE); // Connection to Speeduino
  Serial.println("Mega 2560 Speeduino Reader Initialized.");
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Send the 'n' command at regular intervals if not already waiting
  if (!waitingForResponse && (currentMillis - lastRequestTime >= requestInterval)) {
    SPEEDUINO_SERIAL.write(0x6E); // 0x6E is 'n' in hex
    byteCount = 0;                // Reset counter for new packet
    waitingForResponse = true;
    lastRequestTime = currentMillis;
  }

  // 2. Read incoming bytes asynchronously
  while (SPEEDUINO_SERIAL.available() > 0) {
    uint8_t incomingByte = SPEEDUINO_SERIAL.read();

    // Prevent buffer overflow array bounds violation
    if (byteCount < TOTAL_BYTES_EXPECTED) {
      dataArray[byteCount] = incomingByte;
      byteCount++;
    }

    // 3. Process array once all bytes are collected
    if (byteCount >= TOTAL_BYTES_EXPECTED) {
      processSpeeduinoData();
      waitingForResponse = false; // Ready for next request
    }
  }

  // 4. Safety Timeout: Reset if Speeduino drops packets or disconnects
  if (waitingForResponse && (currentMillis - lastRequestTime > responseTimeout)) {
    Serial.println("Error: Speeduino response timed out.");
    waitingForResponse = false; 
  }
}

void processSpeeduinoData() {
  // Validate the 3-byte header echo
  if (dataArray[0] == 0x6E && dataArray[1] == 0x32) {
    Serial.print("Data Verified. Payload Length: ");
    Serial.println(dataArray[2]); // Third byte contains payload size
    
    // Example: Read RPM (Engine status structure shifts by 3 header bytes)
    // In Speeduino structure, RPM is usually at payload offset 14 and 15 (bytes 17 and 18 in array)
    uint16_t map = (dataArray[8] << 8) | dataArray[7]; // Manifold Absolute Pressure
    uint8_t iat = dataArray[9]; // Intake Air Temperature
    uint8_t clt = dataArray[10]; // Coolant Temperature
    uint16_t rpm = (dataArray[18] << 8) | dataArray[17]; // Engine RPM 
    uint16_t vss = (dataArray[104] << 8) | dataArray[103]; // Vehicle Speed Sensor
    uint8_t fuelpress = dataArray[106]; // Fuel Pressure
    uint8_t oilpress = dataArray[107]; // Oil Pressure
    Serial.print("MAP: "); Serial.print(map); Serial.print(" ");
    Serial.print("IAT: "); Serial.print(iat); Serial.print(" ");
    Serial.print("CLT: "); Serial.print(clt); Serial.print(" ");
    Serial.print("RPM: "); Serial.println(rpm); Serial.print(" ");
    Serial.print("VSS: "); Serial.println(vss); Serial.print(" ");
    Serial.print("FuelP: "); Serial.print(fuelpress); Serial.print(" ");
    Serial.print("OilP: "); Serial.println(oilpress); Serial.print(" ");
  } else {
    Serial.println("Error: Invalid header received.");
  }
}
