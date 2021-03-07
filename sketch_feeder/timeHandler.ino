#include "NtpManager.h"
#include <TaskScheduler.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

NtpManager ntpManager(getScheduler());

bool firstUpdateTime = true;

void initNtp() {
  ntpManager.setOnTimeUpdateListener(onTimeUpdate);
}
void onTimeUpdate(unsigned long epochTime) {
  downloadFile();
  if (firstUpdateTime == true) {
    firstUpdateTime = false;
    setDeviceTime(epochTime);
    playAudio(AUDIO_START_UP, NULL);
  }
  else {
    setDeviceTime(epochTime);
  }
}


void downloadFile() {
  Serial.print("downloadFile\n");

  WiFiClient client;

  HTTPClient http;
  File f = SPIFFS.open("/upload/file.mp3", "w");

  Serial.print("[HTTP] begin...\n");
  if (http.begin(client, "http://www.hochmuth.com/mp3/Vivaldi_Sonata_eminor_.mp3")) {  // HTTP


    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK) {
        http.writeToStream(&f);
      }
    } else {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.printf("[HTTP} Unable to connect\n");
  }
}

void setDeviceTime(unsigned long epochTime) {

  stopFeedingAlarm();
  Serial.println((String)"setDeviceTime epochTime... " + epochTime);
  setTime(epochTime);

  struct timeval tv;
  tv.tv_sec = epochTime;

  settimeofday(&tv, NULL);
  resetFeedingTasks();
}
