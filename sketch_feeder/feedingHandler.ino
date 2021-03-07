#include <Time.h>
#include <time.h>                       // time() ctime()

#include "MotorControl.h"
#include "CronAlarms.h"

#define MOTOR_PIN D5

MotorControl motor(&taskManager, MOTOR_PIN);

CronId alarms[dtNBR_ALARMS];
void updateFeedingLoop(){	
	Cron.delay(0); // wait one second between clock display

}

void stopFeeding(){
   motor.stop();
}

void stopFeedingAlarm() {
  for (int x = 0; x < dtNBR_ALARMS; x++)  {
    if (alarms[x] != dtINVALID_ALARM_ID) {
      Cron.free(alarms[x]);
    }
  }
  memset(alarms, dtINVALID_ALARM_ID, dtNBR_ALARMS);
}

void onFeedingAlarm() {
  JsonArray data = preferences.getAlarms();
  for (int x = 0; x < data.size(); x++)  {
    const char *time = data[x];
    Serial.println((String)"time:" + time);
    alarms[x] = Cron.create((char*)time, onCompositeFeeding, false);
  }
}
void onCompositeFeeding() {
  Serial.println((String)"onCompositeFeeding");
  playAudio(AUDIO_FEEDING, [ = ]() {
	turnOnLed();  
    motor.rotate(preferences.getFeedingDuration(), [ = ]() {
	  turnOnLed();
    });
  });

}
void onFeedingEvent(void (*listener)()) {
  Serial.println((String) "onFeedingEvent");
  motor.rotate(preferences.getFeedingDuration(), listener);
}
void onPlayFeedingAudioEvent() { 
  playAudio(AUDIO_FEEDING,NULL);
}

void resetFeedingTasks() {
  stopFeedingAlarm();
  onFeedingAlarm();

}
