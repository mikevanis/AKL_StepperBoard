//#define EN_PIN 3
//#define DIR_PIN 4
//#define STEP_PIN 5
//#define CS_PIN 6
//#define CS2_PIN 7
//#define LED 8
//
#define EN_PIN 2
#define EN2_PIN 9
#define DIR_PIN 3
#define STEP_PIN 4
#define CS_PIN 5
#define CS2_PIN 7
#define LED 8

boolean isClockwise = false;

#include <TMC2130Stepper.h>
TMC2130Stepper driver = TMC2130Stepper(EN_PIN, DIR_PIN, STEP_PIN, CS_PIN);
TMC2130Stepper driver2 = TMC2130Stepper(EN2_PIN, DIR_PIN, STEP_PIN, CS2_PIN);

#include <AccelStepper.h>
AccelStepper stepper = AccelStepper(stepper.DRIVER, STEP_PIN, DIR_PIN);
AccelStepper stepper2 = AccelStepper(stepper.DRIVER, STEP_PIN, DIR_PIN);

#include <SPI.h>

// Microstepping - 0, 2, 4, 8, 16, 32, 64, 128, 255. The lower the value, the faster the motor.
byte microstepsVal = 16;

unsigned long prevMillis;

void setup() {
  pinMode(LED, OUTPUT);
  for(int i=0; i<4; i++) {
    digitalWrite(LED, HIGH);
    delay(10);
    digitalWrite(LED, LOW);
    delay(100);
  }

  SPI.begin();
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
  driver.rms_current(600); // RMS current - increase to 800 or 1000 to get more torque. Although you'll need to put a heatsink on the chip.
  driver.stealthChop(1); // Enable extremely quiet stepping.
  driver.stealth_autoscale(1);
  driver.microsteps(microstepsVal);

  digitalWrite(CS2_PIN, HIGH);
  digitalWrite(CS_PIN, LOW);
  driver2.begin();
  driver2.rms_current(600); // RMS current - increase to 800 or 1000 to get more torque. Although you'll need to put a heatsink on the chip.
  driver2.stealthChop(1); // Enable extremely quiet stepping.
  driver2.stealth_autoscale(1);
  driver2.microsteps(microstepsVal);

  stepper.setMaxSpeed(2000); // Max speed in steps / second. Do not set higher than 2000.
  stepper.setAcceleration(8000); // Purely aesthetic, the lower the acceleration value, the slower the ramp at start / end.
  stepper.setEnablePin(EN_PIN);
  stepper.setPinsInverted(false, false, true);
  stepper.disableOutputs();
  stepper2.setMaxSpeed(2000); // Max speed in steps / second. Do not set higher than 2000.
  stepper2.setAcceleration(8000); // Purely aesthetic, the lower the acceleration value, the slower the ramp at start / end.
  stepper2.setEnablePin(EN2_PIN);
  stepper2.setPinsInverted(false, false, true);
  stepper2.enableOutputs();
}

void loop() {
  if (stepper.distanceToGo() == 0) {
    digitalWrite(LED, HIGH);
    delay(10);
    digitalWrite(LED, LOW);
    //stepper.disableOutputs();
    delay(100);
    if (isClockwise) {
      stepper.move(400 * microstepsVal); // Move 400 steps.
      stepper2.move(400 * microstepsVal); // Move 400 steps.
      isClockwise = false;
    }
    else {
      stepper.move(-400 * microstepsVal); // Move 400 steps in the opposite direction.
      stepper2.move(-400 * microstepsVal); // Move 400 steps in the opposite direction.
      isClockwise = true;
    }
//    stepper.enableOutputs();
//    stepper2.enableOutputs();
  }

//  if (millis() - prevMillis >= 10) {
//    Serial.println(driver.PWM_SCALE());
//    prevMillis = millis();
//  }
  stepper.run();
  stepper2.run();
}
