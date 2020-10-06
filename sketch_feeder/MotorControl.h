#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H
#include <Arduino.h>
#include <Servo.h>

class MotorControl {

  private:
    byte pin;
    unsigned long rotationStartTime = 0;
    unsigned long duration = 0;
    Servo myservo;  // create servo object to control a servo
    bool rotating = false;
    bool allowRotate = false;

    void init();
    void rotate();
    void backward();
    void forward();
    bool canRotate();
  public:
    MotorControl(byte pin);
    void start(unsigned long duration);
    bool loop();
    void stop();
    bool isRotating();

};
#endif
