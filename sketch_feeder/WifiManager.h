#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H
#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <TaskManager.h>
#define WIFI_MANAGER_TASK "WIFI_MANAGER_TASK"
#define WIFI_MANAGER_TASK_LONG "WIFI_MANAGER_TASK_LONG"

#define WIFI_STA_STATE_BEGIN 0
#define WIFI_STA_STATE_CONNECTING 1
#define WIFI_STA_STATE_ESTABLISHED 2
#define WIFI_STA_STATE_FAILED 4

typedef std::function<void(int status)> WifiStatusListener;

class WifiManager {

  private:
    const int retryCount = 4;
    String ssid;
    String password;
    WifiStatusListener wifiStatusListener = NULL;
    WiFiEventHandler mDisConnectHandler;

    void onDisconnect() {
      Serial.println ( "WiFi On Disconnect." );
      WiFi.disconnect();

    }
  public:
    WifiManager(String accessPointSsid) {
      WiFi.softAP(accessPointSsid);
      WiFi.setAutoConnect(false);
      WiFi.setAutoReconnect(false);
      mDisConnectHandler = WiFi.onStationModeDisconnected([this](const WiFiEventStationModeDisconnected & event)
      {
        this->onDisconnect();
      });

    }
    void setup(String ssid, String password) {
      this->ssid = ssid;
      this->password = password;
      runLongTimeConnectingTask();
    }
    void setOnWifiStatusListener(const WifiStatusListener& listener) {
      this->wifiStatusListener = listener;
    }
    void connectToWifi() {
      Tasks.stop(WIFI_MANAGER_TASK);
      this->ssid = ssid;
      this->password = password;
      WiFi.mode(WIFI_AP_STA);
      WiFi.begin(ssid, password);
      if (wifiStatusListener != NULL) wifiStatusListener(WIFI_STA_STATE_BEGIN);
      Tasks.interval(WIFI_MANAGER_TASK, 5000, retryCount, [this] {
        Serial.println((String)"count:" + Tasks.count(WIFI_MANAGER_TASK) + " ssid:" + this->ssid + ", password:" + this->password);

        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("Connected... ");
          if (wifiStatusListener != NULL) wifiStatusListener(WIFI_STA_STATE_ESTABLISHED);
          Tasks.stop(WIFI_MANAGER_TASK);
        }
        else{
          if (Tasks.count(WIFI_MANAGER_TASK) >= retryCount) {
            Tasks.stop(WIFI_MANAGER_TASK);

            if (wifiStatusListener != NULL) {
              wifiStatusListener(WIFI_STA_STATE_FAILED);
            }
          }
          else{
            if (wifiStatusListener != NULL) {
              wifiStatusListener(WIFI_STA_STATE_CONNECTING);
            }
            Serial.println("Connecting to Wifi");
          }

        }
      });
    }

    void runLongTimeConnectingTask() {
      if (Tasks.getTaskByName(WIFI_MANAGER_TASK_LONG) != nullptr)
        Tasks.erase(WIFI_MANAGER_TASK_LONG);
      Tasks.interval(WIFI_MANAGER_TASK_LONG, 120000, [this] {
        Serial.println((String) + "wifiState ssid:" + this->ssid + ", password:" + this->password + " state:" + (WiFi.status()) );
        Serial.println((String) + "flag:" + (this->ssid != NULL && this->ssid != "") + " WL_CONNECTED:" + WL_CONNECTED );
        if (WiFi.status() != WL_CONNECTED && this->ssid != NULL && this->ssid != "") {
          Serial.println("Trying To Connect");

          WiFi.begin(this->ssid, this->password);
          Tasks.once(5000, [this] {
            if (WiFi.status() == WL_CONNECTED && this->wifiStatusListener != NULL) this->wifiStatusListener(WIFI_STA_STATE_ESTABLISHED);
          });
        }
      });

    }
};
#endif
