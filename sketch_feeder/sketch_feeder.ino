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

#define AUDIO_FEEDING "/pno-cs.mp3"
#define FEEDING_INTERVAL_TASK "FEEDING_INTERVAL_TASK"
#define LED_PIN D6
#define MOTOR_PIN D5

LedControl led(LED_PIN, ledStateFeedingOn);
MotorControl motor(MOTOR_PIN);
AudioControl audioControl;
ServerControl serverControl(AUDIO_FEEDING);
const char* ssid = "Feeder-Access-Point";

Preferences preferences;
Config& config = preferences.getConfig();

void setup()
{

  Serial.begin(115200);

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
      onFeedingInterval(value);
    else if (key == "SETTING_SOUND_VOLUME")
      onSetSoundVolume(value);

  });

  serverControl.setStopTasksListener([ = ]() {
    stopAllTasks();
  });

  serverControl.setUploadListener([ = ]() {
    onPlayFeedingAudioEvent(NULL);
  });

  Serial.println((String)"key " + config.feedingInterval);

  onFeedingInterval(config.feedingInterval);

}

void onSetSoundVolume(float value) {
  config.soundVolume = value;
  preferences.save();
  onPlayFeedingAudioEvent(NULL);
}

void onFeedingInterval(int feedingInterval) {
  Tasks.stop(FEEDING_INTERVAL_TASK);

  config.feedingInterval = feedingInterval;
  preferences.save();

  Tasks.interval(FEEDING_INTERVAL_TASK, feedingInterval, [] {
    Serial.print("interval forever task: now = ");
    Serial.println(millis());
    onCompositeFeeding();
  });

}

void stopAllTasks() {
  audioControl.stop();
  led.dismiss();
  motor.stop();
}
void onCompositeFeeding() {
  stopAllTasks();
  audioControl.play(AUDIO_FEEDING, config.soundVolume, [ = ]() {
    led.on();
    motor.rotate(config.feedingDuration, [ = ]() {
      led.onDuration(5000);
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
  led.onDuration(3000);
}

void loop()
{
  Tasks.update(); // automatically execute tasks

  Scenes.update();

}