const int totalPins = 20;  // Pins from 0-13, A0-A5

// List of pin numbers
int pins[totalPins] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, A0, A1, A2, A3, A4, A5};

void setup() {
  // Set all pins as output mode
  for (int i = 0; i < totalPins; i++) {
    pinMode(pins[i], OUTPUT);
  }
}

void loop() {
  for(int i = 0; i < 3; i++) {
    blinkAllLEDs();       // Blink all LEDs once and then turn them off
    sequentialBlink();    // Light up the LEDs in sequence
  }
}

// Function to blink all LEDs once and then turn them off
void blinkAllLEDs() {
  setAllLEDs(HIGH);  // Turn on all LEDs
  delay(1000);
  setAllLEDs(LOW);   // Turn off all LEDs
  delay(500);
}

// Function to set the state of all LEDs (HIGH or LOW)
void setAllLEDs(int state) {
  for (int i = 0; i < totalPins; i++) {
    digitalWrite(pins[i], state);
  }
}

// Function to light up the LEDs in sequence
void sequentialBlink() {
  for (int i = 0; i < totalPins; i++) {
    digitalWrite(pins[i], HIGH);  // Turn on the LED
    delay(100);
    digitalWrite(pins[i], LOW);   // Turn off the LED
  }
}
