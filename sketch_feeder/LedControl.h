#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass 
#define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
#define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
#define _TASK_PRIORITY          // Support for layered scheduling priority
#define _TASK_TIMEOUT           // Support for overall task timeout 
#define _TASK_OO_CALLBACKS


#include <Arduino.h>
#include <TaskManager.h>
#define offDelayedTask "offDelayedTask"
#include <TaskScheduler.h>

#include <TaskSchedulerDeclarations.h>
class LedControl : public Task {
  private:
    byte pin;
    bool state = false;

  public:
    LedControl(Scheduler* scheduler, byte pin) : Task(TASK_IMMEDIATE, TASK_ONCE, scheduler) {
      this->pin = pin;
      pinMode(pin, OUTPUT);
    }
    ~LedControl() {};

    bool Callback() {
      Serial.println((String)"LedControl Callback");
      off();
      return true;
    }

    void setup(const uint32_t offDelay) {
      Serial.println((String)"Lamp setup: " + offDelay);
      setInterval(offDelay);
    }
    void on() {
      Serial.println("Turn On LED");
      restartDelayed();
      state = true;
      digitalWrite(pin, HIGH);
      Serial.println((String)"on Toggle Led Currend State Is:" + state);

    }

    void off() {
      Serial.println("Turn Off LED");
      dismiss();
      digitalWrite(pin, LOW);
      state = false;
      Serial.println((String)"off Toggle Led Currend State Is:" + state);

    }

    void turnOffLater() {
      Serial.println((String)"Led Turn Off After A While");
      enableDelayed();
    }
    bool OnEnable() {
      Serial.println("LedControl: OnEnable");
      return true;
    }

    void dismiss() {
      Serial.println((String)"Don't Need To Turn Off Led");
      disable();
    }

    void toggle() {
      Serial.println((String)"Toggle Led Currend State Is:" + state);
      if (state) off(); else on();
    }
};
#endif
