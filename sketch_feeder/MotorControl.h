#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H
#include <Arduino.h>
#include <Servo.h>
#define MOTOR_CONTROL_TASK "MOTOR_CONTROL_TASK"

class MotorControl {

  private:
    byte pin;
    Servo myservo;
  public:
    MotorControl(byte pin) {
      this->pin = pin;
    }

    void rotate(const uint32_t duration, void (*listener)()) {
      myservo.attach(pin);
      myservo.write(15);
      Serial.print("start MotorControl");

      Tasks.once(MOTOR_CONTROL_TASK, duration, [ = ] {
        Serial.print("stop MotorControl");
        myservo.detach();
        if (listener != NULL)
          listener();
      });
    }
    void stop() {
      myservo.detach();
      Tasks.erase(MOTOR_CONTROL_TASK);
    }
};
#endif
