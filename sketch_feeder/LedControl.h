#ifndef LED_CONTROL_H
#define LED_CONTROL_H
#include <Arduino.h>
#include <TaskManager.h>
#define offDelayedTask "offDelayedTask"

class LedControl {
  private:
    byte pin;
    uint32_t offDelay;
    bool state = false;
    void setup() {

    }

  public:
    LedControl(byte pin) {
      this->pin = pin;
      pinMode(pin, OUTPUT);
      setup();

    }

    void setup(const uint32_t offDelay) {
      this->offDelay = offDelay;
    }
    void on() {
      Serial.println("Turn On LED");
      dismiss();
      turnOffLater();
      state = true;
      digitalWrite(pin, HIGH);

    }

    void off() {
      Serial.println("Turn Off LED");

      dismiss();
      digitalWrite(pin, LOW);
      state = false;
    }

    void turnOffLater() {
      Serial.println((String)"Led Turn Off After: " + offDelay);
      Tasks.once(offDelayedTask, offDelay, [&] {
        off();
      });
    }

    void dismiss() {
      Serial.println((String)"Don't Need To Turn Off Led");
      Tasks.stop(offDelayedTask);
    }

    void toggle() {

      Serial.println((String)"Toggle Led Currend State Is:" + state);
      if (!state)
      {
        on();
      }
      else {
        off();
      }
    }
};
#endif
