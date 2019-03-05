#define EN_PIN 3
#define DIR_PIN 4
#define STEP_PIN 5
#define CS_PIN 6
#define LED 8

#include <TimerOne.h>
#include <TMC2130Stepper.h>
TMC2130Stepper driver = TMC2130Stepper(EN_PIN, DIR_PIN, STEP_PIN, CS_PIN);

int microstepsVal = 2;
int hz = 10000;

// Movement variables
int currentSpeed;
int acceleration;
int maxSpeed;
volatile int64_t distanceToGo;
int64_t targetDistance;
unsigned long lastUpdate;

void setup() {
  // put your setup code here, to run once:
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
  driver.begin();
  driver.rms_current(600); // RMS current - increase to 800 or 1000 to get more torque. Although you'll need to put a heatsink on the chip.
  driver.stealthChop(1); // Enable extremely quiet stepping.
  driver.stealth_autoscale(1);
  driver.microsteps(microstepsVal);
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(DIR_PIN, HIGH);
  pinMode(STEP_PIN, OUTPUT);
  digitalWrite(STEP_PIN, LOW);

  Timer1.initialize(1000000/100);
  Timer1.attachInterrupt(step);
  Timer1.stop();
}

void loop() {
  update();
}

// Move - distance in steps, speed in steps / sec, accel in steps / sec ^ 2
void move(int64_t distance, int speed, int accel) {
  Timer1.stop();
  noInterrupts();
  targetDistance = distance;
  distanceToGo = distance;
  maxSpeed = speed;
  acceleration = accel;
  interrupts();
  Timer1.start();
}

void update() {
  distanceToGo
}

void step() {
  PORTD |= 1 << 5;
  PORTD &= ~(1 << 5);
  distanceToGo--;
}
