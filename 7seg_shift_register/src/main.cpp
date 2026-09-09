#include <Arduino.h>

#define DATA_PIN  0  // PB0 -> 74HC595 DS
#define CLOCK_PIN 1  // PB1 -> 74HC595 SHCP
#define VSS_PIN   2  // PB2 -> Interrupt 0 Input
#define LATCH_PIN 3  // PB3 -> 74HC595 STCP

// Common Cathode 7-segment display mapping (0-9). 
// Set bit to 1 to light up segment. If Common Anode, invert these bytes (~).
const byte SEGMENT_MAP[] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111  // 9
};

volatile unsigned long totalPulses = 0;
volatile unsigned long pulseCount = 0;
unsigned long lastMillis = 0;

void vssInterrupt() {
  pulseCount++;
  totalPulses++;
}

void setup() {
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  
  // Use internal pull-up for the signal input conditioned by the NPN transistor
  pinMode(VSS_PIN, INPUT_PULLUP); 
  attachInterrupt(0, vssInterrupt, FALLING); // Triggers on every VSS pulse
  
  lastMillis = millis();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Update speedometer every 500ms for smooth display
  if (currentMillis - lastMillis >= 500) {
    noInterrupts(); // Temporarily pause to safely copy volatile variables
    unsigned long localPulseCount = pulseCount;
    pulseCount = 0; // Reset sample window pulse counter
    unsigned long localTotalPulses = totalPulses;
    interrupts();
    
    // Calculate frequency (pulses per second)
    float elapsedSeconds = (currentMillis - lastMillis) / 1000.0;
    float frequency = localPulseCount / elapsedSeconds;
    
    // Convert to Speed (km/h)
    int kmh = round(frequency * 1.4484);
    if (kmh > 999) kmh = 999; // Cap at 3 digits
    if (kmh < 0) kmh = 0;
    
    // Calculate Odometer (Total Kilometers Traveled)
    // 2485.52 pulses = 1 kilometer
    unsigned long totalKm = localTotalPulses / 2486; 

    // Choose what to display: standard is speed. 
    // You could alternate or hold a button to show the totalKm odometer.
    displayNumber(kmh); 
    
    lastMillis = currentMillis;
  }
}

void displayNumber(int num) {
  // Extract separate digits
  int hundreds = (num / 100) % 10;
  int tens     = (num / 10) % 10;
  int ones     = num % 10;

  // Suppress leading zero for hundreds place if it's 0
  byte hundredsSeg = (num < 100) ? 0x00 : SEGMENT_MAP[hundreds];
  // Suppress leading zero for tens place if speed is single digit
  byte tensSeg     = (num < 10) ? 0x00 : SEGMENT_MAP[tens];
  byte onesSeg     = SEGMENT_MAP[ones];

  // Send data out to the shift registers
  digitalWrite(LATCH_PIN, LOW);
  
  // Because they are daisy-chained, shift out the LAST register's data first
  // Reg 3 (Ones) -> Reg 2 (Tens) -> Reg 1 (Hundreds)
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, onesSeg);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, tensSeg);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, hundredsSeg);
  
  digitalWrite(LATCH_PIN, HIGH);
}
