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
  this->allowRotate = true;
  if (!rotating) {
    rotating = true;
    myservo.attach(pin);
  }
}
void MotorControl::stop() {
  this->allowRotate = false;
  if (rotating) {
    Serial.println((String)"stopped MOTOR");
    rotating = false;
    myservo.detach();
  }

}


bool MotorControl::isRotating() {
  return this->rotating;
}

void MotorControl::backward() {
  myservo.write(99);
}
void MotorControl::forward() {
  myservo.write(15);
}

bool MotorControl::canRotate() {
  if (millis() - this->rotationStartTime < this->duration && this->allowRotate == true) {
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


bool MotorControl::loop() {
  bool can = canRotate();
  if (can==true) {
    rotate();
  }
  return can;
}
