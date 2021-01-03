#include <SceneManager.h>
#include "Preferences.h"
#include "LedControl.h"
#include "MotorControl.h"
#include "ClickButton.h"
#include "AudioControl.h"
#include "ServerControl.h"
#include "WifiManager.h"
#include "NtpManager.h"
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
#define BUTTON_PIN D2

LedControl led(LED_PIN, ledStateFeedingOn);
MotorControl motor(MOTOR_PIN);
AudioControl audioControl;
WifiManager wifiManager;
NtpManager ntpManager;
ServerControl serverControl(AUDIO_FEEDING);
const char* ssid = "Feeder-Access-Point";
int alarms[MAX_ALARM_SIZE];

Preferences preferences;
Config& config = preferences.getConfig();

ClickButton button(BUTTON_PIN);

void handleButton() {

  Tasks.framerate(1000, [ = ] {
    if (button.changed == 1) {
      stopAllTasks();
      if (button.clicks == -1) {
        onFeedingEvent(NULL);
      } else if (button.clicks == -2) {
        onLedToggle();

      }
    }

  });

}

void onLedToggle() {
  led.toggle();
  led.offAfter(config.ledTurnOffDelay);
}
void onSetWifiSetting(String ssid, String password) {
  saveWifiSetting(ssid, password);
  connectToWifi();

}
void connectToWifi() {
  if (config.wifiSsid != NULL && config.wifiSsid != "")
    wifiManager.connectToWifi(config.wifiSsid, config.wifiPassword);
}
void saveWifiSetting(String ssid, String password) {
  strlcpy(config.wifiSsid, ssid.c_str(), sizeof(config.wifiSsid));
  strlcpy(config.wifiPassword, password.c_str(), sizeof(config.wifiPassword));

  preferences.save();
}
char* string2char(String command) {
  if (command.length() != 0) {
    char *p = const_cast<char*>(command.c_str());
    return p;
  }
}

void onSetFeedingAlarm(int *alarms) {
  stopFeedingInterval();
  saveFeedingAlarm(alarms);
  onFeedingAlarm();
}
void stopFeedingInterval() {
  Tasks.erase(FEEDING_INTERVAL_TASK);
}
void stopFeedingAlarm() {
  for (int x = 0; x < MAX_ALARM_SIZE; x++)  {
    if (alarms[x] != -1) {
      Alarm.free(alarms[x]);
    }
  }
}
void saveFeedingAlarm(int *alarms) {
  config.schedulingMode = SCHEDULING_MODE_ALARM;
  for (int x = 0; x < MAX_ALARM_SIZE; x++)  {
    config.alarms[x] = alarms[x];
  }
  preferences.save();
}

void onSetFeedingInterval(int feedingInterval) {
  Serial.println((String) "feedingInterval000:" + feedingInterval);

  stopFeedingAlarm();
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
  Serial.println((String)"value " + value);
  config.ledTurnOffDelay = value;
  preferences.save();
}

void onSetLedState(int value) {
  config.ledState = value;
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
  stopFeedingInterval();
  Tasks.once(100, [ = ] {
    Serial.println((String) "feedingInterval:" + config.feedingInterval);

    Tasks.interval(FEEDING_INTERVAL_TASK, config.feedingInterval, [] {
      onCompositeFeeding();
    });
  });

}

void onFeedingAlarm() {
  stopFeedingAlarm();

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
  audioControl.play(AUDIO_FEEDING, config.soundVolume, [ = ]() {
    led.on();
    motor.rotate(config.feedingDuration, [ = ]() {
      led.offAfter(config.ledTurnOffDelay);
    });
  });

}
void onFeedingEvent(void (*listener)()) {
  motor.rotate(config.feedingDuration, listener);
}
void onPlayFeedingAudioEvent(void (*listener)()) {
  stopAllTasks();
  audioControl.play(AUDIO_FEEDING, config.soundVolume, listener);
}

void onLedTimerEvent() {
  Serial.println((String)"onLedTimerEvent");
  led.on();
  led.offAfter(config.ledTurnOffDelay);
}


void setup()
{

  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);

  setupTime();
  WiFi.softAP(ssid);
  ntpManager.setOnTimeUpdateListener([ = ](unsigned long epochTime) {
    Serial.println((String)"epochTime... " + epochTime);

    setTime(epochTime);
  });

  wifiManager.setOnWifiStatusListener([ = ](int wifiState) {
    Serial.println();
    Serial.print("wifiState:");
    Serial.println(wifiState);

    if (wifiState == WIFI_STA_STATE_ESTABLISHED) {
      ntpManager.start();
    }
  });
  serverControl.setEventListener([ = ](JsonObject & keyValue) {

    stopAllTasks();
    delay(50);
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
      onSetFeedingInterval((int) value);
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
    else if (key == "SETTING_LED_STATE")
      onSetLedState(value);
    else if (key == "SETTING_WIFI_SETTINGS") {
      JsonObject obj = ((JsonObject) value);
      onSetWifiSetting(obj["ssid"], obj["password"]);
    }

  });

  connectToWifi();

  led.setMode(config.ledState);
  serverControl.setStopTasksListener([ = ]() {
    stopAllTasks();
  });

  serverControl.setUploadListener([ = ]() {
    onPlayFeedingAudioEvent(NULL);
  });

  handleButton();
}

void loop()
{
  button.Update();
  Tasks.update(); // automatically execute tasks
  Alarm.delay(0); // wait one second between clock display
}
