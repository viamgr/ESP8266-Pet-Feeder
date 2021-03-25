#define LED_PIN D6

#include "LedControl.h"

LedControl led(&taskManager, LED_PIN);

void disableLedTask(){	
  led.disable();
}

void setupLed(){	
  led.setup(preferences.getLedTurnOffDelay());	
}

void onLedTimerEvent() {
  Serial.println((String)"onLedTimerEvent");
  led.toggle();
}
void turnOnLed(){
  led.on();
}

void turnOffLed(){  
  led.off();
}
