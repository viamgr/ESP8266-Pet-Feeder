#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H
#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <TaskManager.h>
#define WIFI_MANAGER_TASK "WIFI_MANAGER_TASK"

#define WIFI_STA_STATE_BEGIN 0
#define WIFI_STA_STATE_CONNECTING 1
#define WIFI_STA_STATE_ESTABLISHED 2
#define WIFI_STA_STATE_FAILED 4

typedef std::function<void(int status)> WifiStatusListener;

class WifiManager {

  private:
    const int retryCount = 40;
    WifiStatusListener wifiStatusListener = NULL;

  public:
    WifiManager() {}

    void setOnWifiStatusListener(const WifiStatusListener& listener) {
      this->wifiStatusListener = listener;
    }
    void connectToWifi(String ssid, String password) {
      WiFi.mode(WIFI_AP_STA);
      WiFi.begin(ssid, password);
      if (wifiStatusListener != NULL) wifiStatusListener(WIFI_STA_STATE_BEGIN);

      Tasks.interval(WIFI_MANAGER_TASK, 500, retryCount, [ = ] {
        Serial.println((String)"count:" + Tasks.count(WIFI_MANAGER_TASK));

        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("Connected... ");
          if (wifiStatusListener != NULL) wifiStatusListener(WIFI_STA_STATE_ESTABLISHED);
          Tasks.erase(WIFI_MANAGER_TASK);
        }
        else{
          if (Tasks.count(WIFI_MANAGER_TASK) >= retryCount) {
            if (wifiStatusListener != NULL) wifiStatusListener(WIFI_STA_STATE_FAILED);
            Serial.println("Failed connect to Wifi");
          }
          else{
            if (wifiStatusListener != NULL) wifiStatusListener(WIFI_STA_STATE_CONNECTING);
            Serial.println("Connecting to Wifi");
          }

        }
      });
    }

};
#endif
