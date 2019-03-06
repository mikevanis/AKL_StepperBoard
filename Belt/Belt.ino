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
#define RMS_CURRENT 800

#include <TMC2130Stepper.h>
TMC2130Stepper driver = TMC2130Stepper(EN_PIN, DIR_PIN, STEP_PIN, CS_PIN);
#ifdef DOUBLEMOTOR
TMC2130Stepper driver2 = TMC2130Stepper(EN2_PIN, DIR_PIN, STEP_PIN, CS2_PIN);
#endif

#include <AccelStepper.h>
AccelStepper stepper = AccelStepper(stepper.DRIVER, STEP_PIN, DIR_PIN);

#include <SPI.h>

// Microstepping - 0, 2, 4, 8, 16, 32, 64, 128, 255. The lower the value, the faster the motor.
byte microstepsVal = 8;

unsigned long prevMillis;

boolean hasSentDone = true;

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

  SPI.begin();
  Serial.begin(115200);
  //while (!Serial);
  //Serial.println("Start...");
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
  home(10000);
  moveScaled(-5750, 200, 480, microstepsVal);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
  Serial.print("H");
  delay(100);
  digitalWrite(TX_EN, LOW);
}

void loop() {
  if (Serial.available() > 0) {
    char inChar = Serial.read();
    if (inChar == 'F') forward();
    else if (inChar == 'R') reverse();
    else if (inChar == 0) {
      digitalWrite(LED, HIGH);
      delay(100);
      digitalWrite(LED, LOW);
    }
  }

  if (stepper.distanceToGo() == 0 && hasSentDone == false) {
    digitalWrite(TX_EN, HIGH);
    delay(100);
    Serial.print('D');
    delay(100);
    digitalWrite(TX_EN, LOW);
    hasSentDone = true;
  }
  
  stepper.run();
}

void forward() {
  digitalWrite(LED, HIGH);
  delay(10);
  digitalWrite(LED, LOW);
  moveScaled(-5500, 120, 380, microstepsVal);
  hasSentDone = false;
}

void reverse() {
  digitalWrite(LED, HIGH);
  delay(10);
  digitalWrite(LED, LOW);
  moveScaled(5500, 120, 380, microstepsVal);
  hasSentDone = false;
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
