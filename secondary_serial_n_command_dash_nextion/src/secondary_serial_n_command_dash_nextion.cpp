#include <Arduino.h>
#include <Servo.h>

// Configurations for Speeduino Secondary Serial
#define SPEEDUINO_SERIAL Serial1 
#define BAUDRATE 115200          // Standard Speeduino secondary baud rate
#define nexSerial Serial2        // 

Servo servo0; //creates first servo object to control servo
Servo servo1; //creates second servo object to control servo
int servo0_val;
int servo1_val;
// Array and sizing configuration
const int TOTAL_BYTES_EXPECTED = 126; // Total bytes including 3 header bytes
uint8_t dataArray[TOTAL_BYTES_EXPECTED];
int byteCount = 0;

// Request timing control
unsigned long lastRequestTime = 0;
const unsigned long requestInterval = 50; // Request every 50ms (20Hz)
bool waitingForResponse = false;
unsigned long responseTimeout = 200;      // 200ms safety timeout

//Array values
int mapkpa;
int iat_raw;
int iat;
int clt_raw;
int clt;
int volt_raw;
float volt;
int afr_raw;
float afr;
int rpm;
int vss;
int fuelp_raw;
float fuelpress;
int oilp_raw;
float oilpress;
int peakboost;

void processSpeeduinoData() {
  // Validate the 3-byte header echo
  if (dataArray[0] == 0x6E && dataArray[1] == 0x32) {
    Serial.print("Data Verified. Payload Length: ");
    Serial.println(dataArray[2]); // Third byte contains payload size
    
    // Example: Read RPM (Engine status structure shifts by 3 header bytes)
    // In Speeduino structure, RPM is usually at payload offset 14 and 15 (bytes 17 and 18 in array)
    mapkpa = ((dataArray[8] << 8) | (dataArray[7])); // Manifold Absolute Pressure
    iat_raw = dataArray[9]; // Intake Air Temperature
    iat = iat_raw - 40; // Convert to Celsius
    clt_raw = dataArray[10]; // Coolant Temperature
    clt = clt_raw - 40; // Convert to Celsius
    volt_raw = (dataArray[12]); // Battery Voltage
    volt = volt_raw * 0.1; // Convert to volts
    afr_raw = dataArray[13]; // Air-Fuel Ratio
    afr = afr_raw * 0.1; // Convert to AFR
    rpm = ((dataArray[18] << 8) | (dataArray[17])); // Engine RPM 
    vss = ((dataArray[104] << 8) | (dataArray[103])); // Vehicle Speed Sensor
    fuelp_raw = dataArray[106]; // Fuel Pressure
    fuelpress = fuelp_raw * 0.069; // Convert to Fuel Pressure
    oilp_raw = dataArray[107]; // Oil Pressure
    oilpress = oilp_raw * 0.069; // Convert to Oil Pressure 
  } else {
    Serial.println("Error: Invalid header received.");
  }
  if (mapkpa > peakboost){ // If current map value is higher than peak, store in peak memory
   peakboost = mapkpa; }// Store current boost in peak memory
}

void displayData() {
    Serial.print("MAP: "); Serial.print(mapkpa); Serial.print(" ");
    Serial.print("IAT: "); Serial.print(iat); Serial.print(" ");
    Serial.print("CLT: "); Serial.print(clt); Serial.print(" ");
    Serial.print("Volt: "); Serial.print(volt); Serial.print(" ");
    Serial.print("AFR: "); Serial.print(afr); Serial.print(" ");
    Serial.print("RPM: "); Serial.print(rpm); Serial.print(" ");
    Serial.print("VSS: "); Serial.print(vss); Serial.print(" ");
    Serial.print("FuelP: "); Serial.print(fuelpress); Serial.print(" ");
    Serial.print("OilP: "); Serial.println(oilpress);
}

void sendCmd() { // wrapper to send commands to Nextion screen
  nexSerial.print("x0.val="); nexSerial.print(afr_raw);
  nexSerial.write(0xff);nexSerial.write(0xff); nexSerial.write(0xff);
  nexSerial.print("n0.val="); nexSerial.print(mapkpa);
  nexSerial.write(0xff);nexSerial.write(0xff); nexSerial.write(0xff);
  nexSerial.print("n1.val=");nexSerial.print(iat,0);
  nexSerial.write(0xff);nexSerial.write(0xff); nexSerial.write(0xff);
  nexSerial.print("x1.val="); nexSerial.print(volt);
  nexSerial.write(0xff);nexSerial.write(0xff); nexSerial.write(0xff);
  nexSerial.print("n3.val="); nexSerial.print(peakboost);
  nexSerial.write(0xff);nexSerial.write(0xff); nexSerial.write(0xff);
}

void servoData() {
  servo0_val = map(rpm, 0, 7500, 2500, 550);  //converts RPM to output value in micro seconds
  servo0.writeMicroseconds(servo0_val);  //sets the servo position
  delay(25);
}

void setup() {
  Serial.begin(BAUDRATE);          // Debugging to PC
  nexSerial.begin(38400);        // Connection to Nextion display
  SPEEDUINO_SERIAL.begin(BAUDRATE); // Connection to Speeduino
  Serial.println("Mega 2560 Speeduino Reader Initialized.");
  // Servo for dash gauge
  servo0.attach(8);  //output to servo on pin 9
  servo1.attach(10);  //output to servo on pin 11
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
      displayData();
      servoData();
      sendCmd();
      waitingForResponse = false; // Ready for next request
    }
  }

  // 4. Safety Timeout: Reset if Speeduino drops packets or disconnects
  if (waitingForResponse && (currentMillis - lastRequestTime > responseTimeout)) {
    Serial.println("Error: Speeduino response timed out.");
    waitingForResponse = false; 
  }

}




