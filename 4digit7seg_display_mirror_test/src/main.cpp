#include <Arduino.h>
#include <TM1637TinyDisplay.h> // Use <TM1637TinyDisplay6.h> if you have a 6-digit display

// Define the digital pins connected to the display
#define CLK_PIN 3
#define DIO_PIN 2

// Initialize the display object
TM1637TinyDisplay display(CLK_PIN, DIO_PIN);

void setup() {
    // Start the display and set to maximum brightness
    display.begin();
    display.setBrightness(BRIGHT_HIGH);
    display.clear();
    
    // Show a welcome message (strings automatically scroll if too long)
    display.showString("HELLO");
    delay(2000);
}

void loop() {
    // Count from 0 to 100
    for (int i = 0; i <= 100; i++) {
        display.showNumber(i);
        delay(100);
    }
    
    // Flash "DONE"
    display.clear();
    display.showString("DONE");
    delay(2000);
}
