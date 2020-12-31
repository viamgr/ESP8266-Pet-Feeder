#ifndef LED_CONTROL_H
#define LED_CONTROL_H
#include <Arduino.h>
#include <TaskManager.h>
#define offDelayedTask "offDelayedTask"

class LedControl {
  private:
    byte pin;
    unsigned int mode;
    void setup() {
      if (mode == ledStateAlwaysOff || mode == ledStateFeedingOn) {
        off();
      }
      else if (mode == ledStateFeedingOn) {
        on();
      }
    }

  public:
    LedControl(byte pin, unsigned int mode) {
      this->pin = pin;
      pinMode(pin, OUTPUT);
      setMode(mode);
      setup();

    }

    void setMode(unsigned int mode) {
      this->mode = mode;
    }

    void on() {
      dismiss();
      if (mode != ledStateAlwaysOff)
        digitalWrite(pin, HIGH);
    }

    void off() {
      dismiss();
      if (mode != ledStateAlwaysOn)
        digitalWrite(pin, LOW);
    }

    void onDuration(const uint32_t offDelay) {
      on();
      // task is executed only once after offDelay[ms]
      Tasks.once(offDelayedTask, offDelay, [&] {
        Serial.println((String)"state "+ Tasks.isStopping(offDelayedTask)+" isRunning"+Tasks.isRunning(offDelayedTask));
        off();
      });
    }

    void dismiss() {
      Serial.println((String)"stopping " + offDelayedTask);
      Tasks.erase(offDelayedTask);
    }
};
#endif
