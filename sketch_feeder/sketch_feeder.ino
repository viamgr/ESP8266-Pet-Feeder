#include <SceneManager.h>
#include "Preferences.h"
#include "LedControl.h"
#include "MotorControl.h"
#include "AudioControl.h"
#include "ServerControl.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <TaskManager.h>
#include <ESPAsyncWebServer.h>

#include <Time.h>
#include <TimeAlarms.h>

#define AUDIO_FEEDING "/pno-cs.mp3"
#define FEEDING_INTERVAL_TASK "FEEDING_INTERVAL_TASK"
#define LED_PIN D6
#define MOTOR_PIN D5

LedControl led(LED_PIN, ledStateFeedingOn);
MotorControl motor(MOTOR_PIN);
AudioControl audioControl;
ServerControl serverControl(AUDIO_FEEDING);
const char* ssid = "Feeder-Access-Point";
int alarms[MAX_ALARM_SIZE];

Preferences preferences;
Config& config = preferences.getConfig();

void setup()
{

  Serial.begin(115200);

  setupTime();
  WiFi.softAP(ssid);

  serverControl.setEventListener([ = ](JsonObject & keyValue) {

    String key = keyValue["key"];
    auto value = keyValue["value"];
    Serial.println((String)"key " + key);
    if (key == "EVENT_FEEDING")
      onFeedingEvent(NULL);
    else if (key == "EVENT_LED_TIMER")
      onLedTimerEvent();
    else if (key == "EVENT_PLAY_FEEDING_AUDIO")
      onPlayFeedingAudioEvent(NULL);
    else if (key == "EVENT_COMPOSITE_FEEDING")
      onCompositeFeeding();
    else if (key == "SETTING_FEEDING_INTERVAL")
      onSetFeedingInterval(value);
    else if (key == "SETTING_FEEDING_ALARM")
    {
      JsonArray arr = ((JsonArray) value);
      int alarms[arr.size()];
      for (int x = 0; x < MAX_ALARM_SIZE; x++)  {
        alarms[x] = arr.size() > x ? arr[x] : -1;
      }
      onSetFeedingAlarm(alarms);
    }
    else if (key == "SETTING_SOUND_VOLUME")
      onSetSoundVolume(value);
    else if (key == "SETTING_FEED_DURATION")
      onSetFeedDuration(value);
    else if (key == "SETTING_LED_TURN_OFF_DELAY")
      onSetLedTurnOffDelay(value);

  });

  serverControl.setStopTasksListener([ = ]() {
    stopAllTasks();
  });

  serverControl.setUploadListener([ = ]() {
    onPlayFeedingAudioEvent(NULL);
  });


}

void onSetFeedingAlarm(int *alarms) {
  saveFeedingAlarm(alarms);
  onFeedingAlarm();
}
void saveFeedingAlarm(int *alarms) {
  config.schedulingMode = SCHEDULING_MODE_ALARM;
  for (int x = 0; x < MAX_ALARM_SIZE; x++)  {
    config.alarms[x] = alarms[x];
  }
  preferences.save();
}

void onSetFeedingInterval(int feedingInterval) {
  saveFeedingInterval(feedingInterval);
  onFeedingInterval();
}
void saveFeedingInterval(int feedingInterval) {
  config.schedulingMode = SCHEDULING_MODE_INTERVAL;
  config.feedingInterval = feedingInterval;
  preferences.save();
}
void setupTime() {
  setTime(0, 0, 0, 1, 1, 2022); // set time to Saturday 12:00:00am Jan 1 2022
  memset(alarms, 0, MAX_ALARM_SIZE);

  if (config.schedulingMode == SCHEDULING_MODE_INTERVAL)
    onFeedingInterval();
  else if (config.schedulingMode == SCHEDULING_MODE_ALARM)
    onFeedingAlarm();


}
void onSetLedTurnOffDelay(int value) {
  config.ledTurnOffDelay = value;
  preferences.save();
}

void onSetFeedDuration(int value) {
  config.feedingDuration = value;
  preferences.save();
}
void onSetSoundVolume(float value) {
  config.soundVolume = value;
  preferences.save();
  onPlayFeedingAudioEvent(NULL);
}

void onFeedingInterval() {
  stopAllTasks();

  Tasks.interval(FEEDING_INTERVAL_TASK, config.feedingInterval, [] {
    Serial.print("interval forever task: now = ");
    Serial.println(millis());
    onCompositeFeeding();
  });

}

void onFeedingAlarm() {

  for (int x = 0; x < MAX_ALARM_SIZE; x++)  {
    if (alarms[x] != -1) {
      Alarm.free(alarms[x]);
    }
  }

  for (int x = 0; x < MAX_ALARM_SIZE; x++)  {
    int time = config.alarms[x];
    if (time != -1) {
      float hour = (int) (time / 3600);
      int minute = ((int)((((hour ) * 3600) + time) / 60)) % 60;
      Serial.println((String)"hour hour " + hour + " minute:" + minute + " x:" + x + " time:" + time);

      Alarm.alarmRepeat((int) hour, minute, 0, onCompositeFeeding);
    }
  }

  Serial.println((String)"getTime " + now());
}

void stopAllTasks() {
  audioControl.stop();
  led.dismiss();
  motor.stop();
}
void onCompositeFeeding() {
  Serial.print((String)"onCompositeFeeding");

  stopAllTasks();
  audioControl.play(AUDIO_FEEDING, config.soundVolume, [ = ]() {
    led.on();
    motor.rotate(config.feedingDuration, [ = ]() {
      led.onDuration(config.ledTurnOffDelay);
    });
  });

}
void onFeedingEvent(void (*listener)()) {
  stopAllTasks();
  motor.rotate(config.feedingDuration, listener);
}
void onPlayFeedingAudioEvent(void (*listener)()) {
  stopAllTasks();
  audioControl.play(AUDIO_FEEDING, config.soundVolume, listener);
}

void onLedTimerEvent() {
  stopAllTasks();
  Serial.println((String)"onLedTimerEvent");
  led.onDuration(config.ledTurnOffDelay);
}

void loop()
{
  Tasks.update(); // automatically execute tasks
  Alarm.delay(0); // wait one second between clock display
//  Serial.print("1");

}
