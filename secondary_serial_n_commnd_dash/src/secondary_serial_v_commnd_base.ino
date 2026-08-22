#include <Arduino.h>

const unsigned long POLLING_INTERVAL = 5000; // Milliseconds between requests
unsigned long lastPollTime = 0;

void setup() {
  Serial.begin(115200);   // Monitor/Debug output
  Serial1.begin(115200);  // Connected to Speeduino Secondary Serial port
}

void loop() {
  unsigned long currentTime = millis();
  
  // Poll Speeduino periodically to avoid buffer flooding
  if (currentTime - lastPollTime >= POLLING_INTERVAL) {
    lastPollTime = currentTime;
    Serial.println();
    Serial.println("Start Read");
    // Send the enhanced real-time data request command 'n'
    Serial1.print('n');
  }
  
  // Read incoming enhanced real-time data stream
  if (Serial1.available() > 0) {
    // Read the confirmation/response or parse the data stream block
    byte incomingByte = Serial1.read();
    
    // Print hex/dec values to your main serial monitor for debugging
    Serial.println(incomingByte, HEX);
    //Serial.print(" ");
  }
    //Serial.println("END");
//  delay(2000);
}
