#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#define MOTOR_CONTROL_TASK "MOTOR_CONTROL_TASK"
#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass 
#define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
#define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
#define _TASK_PRIORITY          // Support for layered scheduling priority
#define _TASK_TIMEOUT           // Support for overall task timeout 
#define _TASK_OO_CALLBACKS

#include <Arduino.h>
#include <Servo.h>
#include <TaskScheduler.h>

typedef std::function<void(void)> OnMotorStopListener;

class MotorControl : public Task {

  private:
    byte pin;
    Servo myservo;
    OnMotorStopListener listener = NULL;

  public:
    MotorControl(Scheduler* scheduler, byte pin) : Task(TASK_IMMEDIATE, TASK_ONCE, scheduler) {
      Serial.begin(115200);
      this->pin = pin;
    }

    void rotate(const uint32_t duration, OnMotorStopListener listener) {
      Serial.print("start MotorControl");
      setInterval(duration);
      restartDelayed();
      myservo.attach(pin);
      myservo.write(15);
      this->listener = listener;

    }

    bool Callback() {
      Serial.println((String)"MotorControl Callback");
      if (listener != NULL)
        listener();
      myservo.detach();
      return true;
    }

    void stop() {
      Serial.println((String)"MotorControl stop");
      myservo.detach();
      disable();
    }
};
#endif
