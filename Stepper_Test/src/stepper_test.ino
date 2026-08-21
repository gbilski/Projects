/*
  X12.017 Quad Driver Test
  Drive the motor forward and backwards through 270 degrees
  at constant speed.
 */

const int LED = 13;
const int DIR_A = 6; // pin for CW/CCW
const int STEP_A = 5; // pin for f(scx)
const int RESET = 7; // pin for RESET
const int DELAY = 500; // μs between steps
const int ANGLE = 270; // of 315 available
const int RANGE = ANGLE * 3 * 4;
int steps = 0;
bool forward = true;

// pull RESET low to reset, high for normal operation.

void setup() {
  pinMode(DIR_A, OUTPUT);
  pinMode(STEP_A, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(RESET, OUTPUT);

  digitalWrite(RESET, LOW);
  digitalWrite(LED, HIGH);
  digitalWrite(STEP_A, LOW);
  digitalWrite(DIR_A, HIGH);
  delay(1);  // keep reset low min 1ms
  digitalWrite(RESET, HIGH);
}

// The motor steps on rising edge of STEP
// The step line must be held low for at least 450ns
// which is so fast we probably don't need a delay,
// put in a short delay for good measure.

void loop() {
  digitalWrite(STEP_A, LOW);
  delayMicroseconds(1);  // not required

  steps++;
  if (steps > RANGE) {
    forward = !forward;
    steps = 0;
    digitalWrite(DIR_A, forward ? LOW : HIGH);
    digitalWrite(LED, forward ? HIGH : LOW);
  }

  digitalWrite(STEP_A, HIGH);
  delayMicroseconds(DELAY);
}
