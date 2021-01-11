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

#include <time.h>                       // time() ctime()
#ifdef ESP8266
#include <sys/time.h>                   // struct timeval
#endif
#include "CronAlarms.h"

#define AUDIO_FEEDING "/upload/connected.mp3"
#define LED_PIN D6
#define MOTOR_PIN D5
#define BUTTON_PIN D2

LedControl led(LED_PIN, ledStateFeedingOn);
MotorControl motor(MOTOR_PIN);
AudioControl audioControl;
WifiManager wifiManager;
NtpManager ntpManager;
ServerControl serverControl(AUDIO_FEEDING, CONFIG_FILE_PATH);
const char* ssid = "Feeder-Access-Point";

CronId alarms[dtNBR_ALARMS];

Preferences preferences;

ClickButton button(BUTTON_PIN);
bool first = true;

void handleButton() {
  Tasks.framerate(1000, [ = ] {
    if (button.changed == 1) {
      if (first == false) {
        stopAllTasks();
        if (button.clicks == -1) {
          onCompositeFeeding();
        } else if (button.clicks == -2) {
          onLedToggle();
        }
      }
      first = false;
    }
  });
}

void onLedToggle() {
  led.setMode(preferences.getLedState());
  led.toggle();
  led.offAfter(preferences.getLedTurnOffDelay());
}

void connectToWifi() {
  Serial.println((String) "connectToWifi" + preferences.getWifiSsid());
  if (preferences.getWifiSsid() != NULL && preferences.getWifiSsid()[0] != '\0')
    wifiManager.connectToWifi(preferences.getWifiSsid(), preferences.getWifiPassword());
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
  Serial.println((String)"getTime " + now());
  JsonArray data = preferences.getAlarms();
  for (int x = 0; x < data.size(); x++)  {
    const char *time = data[x];
    Serial.println((String)"time:" + time);
    alarms[x] = Cron.create((char*)time, onCompositeFeeding, false); // 8:30am every day
  }
}

void stopAllTasks() {
  audioControl.stop();
  led.dismiss();
  motor.stop();
  // todo add wifi and ntp ??!!
}
void onCompositeFeeding() {
  Serial.println((String)"onCompositeFeeding");
  audioControl.play(AUDIO_FEEDING, preferences.getSoundVolume(), [ = ]() {
    led.setMode(preferences.getLedState());
    led.on();
    motor.rotate(preferences.getFeedingDuration(), [ = ]() {
      led.offAfter(preferences.getLedTurnOffDelay());
    });
  });

}
void onFeedingEvent(void (*listener)()) {
  Serial.println((String) "onFeedingEvent");
  motor.rotate(preferences.getFeedingDuration(), listener);
}
void onPlayFeedingAudioEvent(void (*listener)()) {
  Serial.println((String) "onPlayFeedingAudioEventedingEvent");

  stopAllTasks();
  audioControl.play(AUDIO_FEEDING, preferences.getSoundVolume(), listener);
}

void onLedTimerEvent() {
  Serial.println((String)"onLedTimerEvent");
  led.setMode(preferences.getLedState());
  led.on();
  led.offAfter(preferences.getLedTurnOffDelay());
}

void resetFeedingTasks() {
  stopFeedingAlarm();
  delay(100);
  onFeedingAlarm();

}
void setDeviceTime(unsigned long epochTime) {
  stopFeedingAlarm();
  Serial.println((String)"setDeviceTime epochTime... " + epochTime);
  setTime(epochTime);
  // set current day/time
  struct timeval tv;
  tv.tv_sec =   epochTime;
  settimeofday(&tv, NULL);
  resetFeedingTasks();
}

void onSetupConfig() {
  stopAllTasks();
  resetFeedingTasks();

}
void eventCallback(JsonObject & keyValue) {
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
  else if (key == "SETTING_SET_TIME")
    setDeviceTime((long) value);
}
void setup()
{

  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);
  WiFi.softAP(ssid);

  setDeviceTime(0);
  ntpManager.setOnTimeUpdateListener(setDeviceTime);

  wifiManager.setOnWifiStatusListener([ = ](int wifiState) {
    Serial.println();
    Serial.print("wifiState:");
    Serial.println(wifiState);

    if (wifiState == WIFI_STA_STATE_ESTABLISHED) {
      ntpManager.start();
    }
  });
  serverControl.setEventListener(eventCallback);

  connectToWifi();

  serverControl.setStopTasksListener([ = ]() {
    stopAllTasks();
  });

  serverControl.setUploadListener([ = ](String filename) {
    if (filename == AUDIO_FEEDING) {
      onPlayFeedingAudioEvent(NULL);
    }
    else if (filename == CONFIG_FILE_PATH) {
      preferences.reload();
      onSetupConfig();
    }

  });

  onSetupConfig();

  handleButton();
}

void loop()
{
  button.Update();
  Tasks.update(); // automatically execute tasks
  Cron.delay(0); // wait one second between clock display
}
