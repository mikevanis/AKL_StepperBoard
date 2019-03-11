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

boolean isReversing = false;
boolean hasMoved = false;

//#define DOUBLEMOTOR
#define RMS_CURRENT 1000

#include <TMC2130Stepper.h>
//TMC2130Stepper driver = TMC2130Stepper(EN_PIN, DIR_PIN, STEP_PIN, CS_PIN);
TMC2130Stepper driver(EN_PIN, DIR_PIN, STEP_PIN, CS_PIN, 11, 12, 13);
#ifdef DOUBLEMOTOR
//TMC2130Stepper driver2 = TMC2130Stepper(EN2_PIN, DIR_PIN, STEP_PIN, CS2_PIN);
TMC2130Stepper driver2 (EN2_PIN, DIR_PIN, STEP_PIN, CS2_PIN, 11, 12, 13);
#endif

#include <AccelStepper.h>
AccelStepper stepper = AccelStepper(stepper.DRIVER, STEP_PIN, DIR_PIN);

// Microstepping - 0, 2, 4, 8, 16, 32, 64, 128, 255. The lower the value, the faster the motor.
byte microstepsVal = 16;

unsigned long prevMillis;

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
  digitalWrite(TX_EN, HIGH);

  Serial.begin(9600);
  while (!Serial);
  Serial.println("Start...");
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  pinMode(CS2_PIN, OUTPUT);
  digitalWrite(CS2_PIN, HIGH);

  digitalWrite(CS2_PIN, LOW);
  digitalWrite(CS_PIN, HIGH);
  driver.begin();
  driver.rms_current(RMS_CURRENT); // RMS current - increase to 800 or 1000 to get more torque. Although you'll need to put a heatsink on the chip.
  driver.stealthChop(1); // Enable extremely quiet stepping.
  driver.stealth_autoscale(1);
  driver.microsteps(microstepsVal);

#ifdef DOUBLEMOTOR
  digitalWrite(CS2_PIN, HIGH);
  digitalWrite(CS_PIN, LOW);
  driver2.begin();
  driver2.rms_current(RMS_CURRENT); // RMS current - increase to 800 or 1000 to get more torque. Although you'll need to put a heatsink on the chip.
  driver2.stealthChop(1); // Enable extremely quiet stepping.
  driver2.stealth_autoscale(1);
  driver2.microsteps(microstepsVal);
#endif

  stepper.setMaxSpeed(2000); // Max speed in steps / second. Do not set higher than 2000.
  stepper.setAcceleration(500); // Purely aesthetic, the lower the acceleration value, the slower the ramp at start / end.
  stepper.setEnablePin(EN_PIN);
  stepper.setPinsInverted(false, false, true);
  enableOutputs();

  pinMode(ENDSTOP1, INPUT_PULLUP);

  home(-16000);
  moveScaled(4000, 50, 200, microstepsVal);
}

void loop() {
  if (stepper.distanceToGo() == 0) {
    Serial.println("Finished steps.");
    
    digitalWrite(LED, HIGH);
    delay(10);
    digitalWrite(LED, LOW);
    //stepper.disableOutputs();
    delay(100);

    // If we've reached home again, Check that the endstop is pressed.
    if (isReversing) {
      if (digitalRead(ENDSTOP1) == HIGH) {
        home(-5000);
      }
      moveScaled(4000, 50, 200, microstepsVal);
      isReversing = false;
    }
    else {
      moveScaled(-4000, 50, 200, microstepsVal);
      isReversing = true;
    }
    stepper.enableOutputs();
    hasMoved = false;
  }

  checkOverFlow();
  if (digitalRead(ENDSTOP1) == HIGH && hasMoved == false) {
    hasMoved = true;
  }

  stepper.run();
}

void moveScaled(long long steps, int accel, int speed, int microstepValue) {
  if (microstepValue == 0) microstepValue = 1;

  stepper.setAcceleration(accel * microstepValue);
  stepper.setMaxSpeed(speed * microstepValue);
  stepper.move(steps * microstepValue);
}

// Home 
void home(long long steps) {
  digitalWrite(LED, HIGH);
  moveScaled(steps, 200, 300, microstepsVal);
  while (digitalRead(ENDSTOP1) == HIGH) {
    stepper.run();
  }
  stepper.setCurrentPosition(0);
  stepper.stop();
  digitalWrite(LED, LOW);
  hasMoved = false;
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

void checkOverFlow() {
  if (digitalRead(ENDSTOP1) == LOW && hasMoved == true && stepper.currentPosition() > 2000) {
    stepper.setCurrentPosition(0);
    stepper.stop();
    isReversing = true;
  }
}
