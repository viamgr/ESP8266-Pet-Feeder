#include <TaskScheduler.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

//NtpManager ntpManager(getScheduler());

//NtpManager* getNtpManager() {
//  return &ntpManager;
//}
//bool firstUpdateTime = true;

//void initNtp() {
//  ntpManager.setOnTimeUpdateListener(onTimeUpdate);
//}
//void onTimeUpdate(unsigned long epochTime) {
//  if (firstUpdateTime == true) {
//    firstUpdateTime = false;
//    setDeviceTime(epochTime);
//    playAudio(AUDIO_START_UP, NULL);
//  }
//  else {
//    setDeviceTime(epochTime);
//  }
//}


void setDeviceTime(unsigned long epochTime) {

  stopFeedingAlarm();
  Serial.println((String)"setDeviceTime epochTime... " + epochTime);
  setTime(epochTime);

  struct timeval tv;
  tv.tv_sec = epochTime;

  settimeofday(&tv, NULL);
  resetFeedingTasks();
}
String getDeviceTime() {
  return String(now());
}
