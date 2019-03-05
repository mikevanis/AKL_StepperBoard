/*
 * TODO
 * - Home. Send R. Go half way. Wait. Send F.
 */

#define EN_PIN 3
#define EN2_PIN A5
#define DIR_PIN 4
#define STEP_PIN 5
#define CS_PIN 8
#define CS2_PIN 7
#define LED 6
#define ENDSTOP1 9
#define ENDSTOP2 10
#define TX_EN 2

boolean isClockwise = false;

//#define DOUBLEMOTOR
#define RMS_CURRENT 1000

#include <TMC2130Stepper.h>
TMC2130Stepper driver = TMC2130Stepper(EN_PIN, DIR_PIN, STEP_PIN, CS_PIN);
#ifdef DOUBLEMOTOR
TMC2130Stepper driver2 = TMC2130Stepper(EN2_PIN, DIR_PIN, STEP_PIN, CS2_PIN);
#endif

#include <AccelStepper.h>
AccelStepper stepper = AccelStepper(stepper.DRIVER, STEP_PIN, DIR_PIN);

#include <SPI.h>

// Microstepping - 0, 2, 4, 8, 16, 32, 64, 128, 255. The lower the value, the faster the motor.
byte microstepsVal = 16;

boolean endstopHit = false;
boolean hasBeltHomed = false;

unsigned long prevMillis;

long totalStepsPerRevolution = 107655;

void setup() {
  pinMode(LED, OUTPUT);
  for (int i = 0; i < 4; i++) {
    digitalWrite(LED, HIGH);
    delay(10);
    digitalWrite(LED, LOW);
    delay(100);
  }

  pinMode(ENDSTOP1, INPUT_PULLUP);
  pinMode(ENDSTOP2, INPUT_PULLUP);
  pinMode(TX_EN, OUTPUT);
  digitalWrite(TX_EN, LOW);

  SPI.begin();
  Serial.begin(115600);
  while (!Serial);
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  pinMode(CS2_PIN, OUTPUT);
  digitalWrite(CS2_PIN, HIGH);

  digitalWrite(CS2_PIN, LOW);
  digitalWrite(CS_PIN, HIGH);
  driver.begin();
  driver.rms_current(RMS_CURRENT);
  driver.stealthChop(1);
  driver.stealth_autoscale(1);
  driver.microsteps(microstepsVal);

#ifdef DOUBLEMOTOR
  digitalWrite(CS2_PIN, HIGH);
  digitalWrite(CS_PIN, LOW);
  driver2.begin();
  driver2.rms_current(RMS_CURRENT);
  driver2.stealthChop(1);
  driver2.stealth_autoscale(1);
  driver2.microsteps(microstepsVal);
#endif

  stepper.setEnablePin(EN_PIN);
  stepper.setPinsInverted(false, false, true);
  enableOutputs();
  digitalWrite(LED, HIGH);

  while (!hasBeltHomed) {
    if (Serial.available() > 0) {
      char inChar = Serial.read();
      if (inChar == 'H') hasBeltHomed = true;
    }
  }

  home(10000);

  digitalWrite(LED, LOW);
  digitalWrite(TX_EN, HIGH);
  delay(1000);
}

void loop() {
  if (stepper.distanceToGo() == 0) {
    digitalWrite(LED, HIGH);
    delay(10);
    digitalWrite(LED, LOW);
    
    // Check if endstop is pressed. If not, home. 
    if (digitalRead(ENDSTOP1) == HIGH) {
      home(10000);
    }
    
    stepper.setCurrentPosition(0);
    moveScaled(totalStepsPerRevolution/microstepsVal, 100, 200, microstepsVal);
    stepper.enableOutputs();
    Serial.print('R');
  }
  else if (stepper.distanceToGo() == totalStepsPerRevolution / 2) {
    Serial.print('F');
  }
  else {
    // If endstop is hit, zero position and start sequence from scratch.
    if (digitalRead(ENDSTOP1) == LOW && endstopHit == false) {
      stepper.setCurrentPosition(0);
      stepper.stop();
    }
    if (digitalRead(ENDSTOP1) == HIGH && endstopHit == true) {
      endstopHit = true;
    }
  }

  stepper.run();
}

// Move, with scaled values based on microsteps. All values in full steps / sec.
void moveScaled(long long steps, int accel, int speed, int microstepValue) {
  if (microstepValue == 0) microstepValue = 1;

  stepper.setAcceleration(accel * microstepValue);
  stepper.setMaxSpeed(speed * microstepValue);
  stepper.move(steps * microstepValue);
}

// Home 
void home(long long steps) {
  moveScaled(steps, 50, 200, microstepsVal);
  while (digitalRead(ENDSTOP1) == HIGH) {
    stepper.run();
  }
  stepper.setCurrentPosition(0);
  stepper.stop();
  endstopHit = true;
}

// Enable driver outputs
void enableOutputs() {
  digitalWrite(EN_PIN, LOW);
#ifdef DOUBLEMOTOR
  digitalWrite(EN2_PIN, LOW);
#endif
}

// Disable driver outputs
void disableOutputs() {
  digitalWrite(EN_PIN, HIGH);
#ifdef DOUBLEMOTOR
  digitalWrite(EN2_PIN, HIGH);
#endif
}
