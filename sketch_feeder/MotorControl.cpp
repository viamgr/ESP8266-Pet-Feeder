#include <Servo.h>
#include "MotorControl.h"

MotorControl::MotorControl(byte pin) {
  this->pin = pin;

  init();
}
void MotorControl::init() {
  pinMode(pin, OUTPUT);
}

void MotorControl::start(unsigned long duration) {

  Serial.println((String)"started MOTOR");
  this->duration = duration;
  this->rotationStartTime = millis();

  if (!rotating) {
    rotating = true;
    myservo.attach(pin);
  }
}
void MotorControl::stop() {

  if (rotating) {
    Serial.println((String)"stopped MOTOR");
    rotating = false;
    myservo.detach();
  }

}


void MotorControl::backward() {
  myservo.write(99);

}
void MotorControl::forward() {
  myservo.write(15);
}

bool MotorControl::canRotate() {
  if (millis() - this->rotationStartTime < this->duration) {
    return true;
  }
  else {
    return false;
  }
}

void MotorControl::rotate() {
  if (millis() - this->rotationStartTime < (this->duration / 2)) {
    forward();
  }
  else {
    backward();
  }
}


void MotorControl::loop() {
  if (canRotate()) {
    rotate();
  }
  else {
    stop();
  }
}
