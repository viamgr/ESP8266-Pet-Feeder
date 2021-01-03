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
      if (mode != ledStateAlwaysOff)
        digitalWrite(pin, HIGH);
    }
    void onWithDismiss() {
      dismiss();
      on();
    }

    void off() {
      if (mode != ledStateAlwaysOn)
        digitalWrite(pin, LOW);
    }

    void offWithDismiss() {
      dismiss();
      off();
    }

    void offAfter(const int offDelay) {
      Serial.println((String)"offDelay "+ offDelay);
      // task is executed only once after offDelay[ms]
      Tasks.once(offDelayedTask, offDelay, [&] {
        Serial.println((String)"state "+ Tasks.isStopping(offDelayedTask)+" isRunning"+Tasks.isRunning(offDelayedTask));
        off();
      });
    }

    void dismiss() {
      off();
      Serial.println((String)"stopping " + offDelayedTask);
      Tasks.erase(offDelayedTask);
    }

    void toggle(){
          Serial.println((String)"toggle " + digitalRead(pin));

        if(digitalRead(pin) == LOW)
            onWithDismiss();
        else
            offWithDismiss();
    }
};
#endif
