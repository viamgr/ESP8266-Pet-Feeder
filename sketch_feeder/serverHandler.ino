#include "ServerControl.h"

ServerControl serverControl(preferences.getStaticIp());
void configServerControl() {
  serverControl.setBaseUrl(preferences.getStaticIp());
}
void initServer() {

  serverControl.setEventListener(eventCallback);
  serverControl.setOnSetStatusListener(setStatusCallback);
  serverControl.setOnGetStatusListener(getStatusCallback);
  serverControl.setStopTasksListener([ = ]() {
    stopAllTasks();
  });
  serverControl.setUploadListener([ = ](String filename) {
    Serial.print((String)"filename uploaded:" + filename);
    stopAllTasks();

    if (filename == AUDIO_FEEDING) {
      //      onPlayFeedingAudioEvent(NULL);
    }
    else if (filename == CONFIG_FILE_PATH) {
      preferences.reload();
      onSetupConfig();
    }
  });
}
void updateServerControl() {
  serverControl.loop();
}

void setStatusCallback(String key, String value) {
  if (key == "STATUS_TIME")
    setDeviceTime(string_to_long(value));
}

String getStatusCallback(String key) {
  if (key == "STATUS_TIME")
    return getDeviceTime();
}

void eventCallback(String name) {
  stopAllTasks();
  //Serial.println((String)"name " + name);
  if (name == "EVENT_FEEDING")
    onFeedingEvent();
  else if (name == "EVENT_LED_TIMER")
    toggleLamp();
  else if (name == "EVENT_PLAY_FEEDING_AUDIO")
    onPlayFeedingAudioEvent();
  else if (name == "EVENT_WIFI_CONNECT")
    restartWifi();
  else if (name == "EVENT_COMPOSITE_FEEDING")
    onCompositeFeeding();
  //  else if (key == "SETTING_SET_TIME")
  //    setDeviceTime((long) value);
}
