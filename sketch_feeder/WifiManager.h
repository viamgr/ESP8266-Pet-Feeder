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


#define WIFI_MANAGER_TASK_LONG_INTERVAL 3 * 60 * 1000
#define WIFI_MANAGER_TASK_SHORT_INTERVAL 1 * 1000
#define WIFI_MANAGER_TASK_MAX_INTERVAL 5 * 60 * 1000

typedef std::function<void(int status)> WifiStatusListener;

class WifiManager: public Task  {

  private:
    const int retryCount = 4;
    String ssid;
    String password;
    uint8_t status;
    WifiStatusListener wifiStatusListener = NULL;
    WiFiEventHandler mDisConnectHandler;

    void onDisconnect() {
      Serial.println ( "WiFi On Disconnect." );
      setStatus(WIFI_STA_STATE_FAILED);
      WiFi.disconnect();
      // chera bayad restart beshe?
      // restart();
    }
  public:
    WifiManager(Scheduler* scheduler, String accessPointSsid) : Task(WIFI_MANAGER_TASK_SHORT_INTERVAL, TASK_FOREVER, scheduler) {
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
      restartDelayed();
    }
    void setOnWifiStatusListener(const WifiStatusListener& listener) {
      this->wifiStatusListener = listener;
    }

    bool OnEnable() {
      Serial.println((String)"WifiManager OnEnable");
      WiFi.mode(WIFI_AP_STA);
      WiFi.begin(ssid, password);
      return true;
    }

    void setStatus(uint8_t value) {
      Serial.println((String)"WifiManager setStatus value:" + value + " old:" + status );
      if (wifiStatusListener != NULL && value != status) wifiStatusListener(value);
      status = value;
    }

    void update() {
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected... ");
        setInterval(WIFI_MANAGER_TASK_LONG_INTERVAL);
        restartDelayed(WIFI_MANAGER_TASK_LONG_INTERVAL);
        setStatus(WIFI_STA_STATE_ESTABLISHED);

      }
      else {
        unsigned long aInterval = WIFI_MANAGER_TASK_SHORT_INTERVAL * getRunCounter();
        if (aInterval > WIFI_MANAGER_TASK_MAX_INTERVAL) {
          aInterval = WIFI_MANAGER_TASK_MAX_INTERVAL;
        }
        Serial.println((String)"Failed connecting to wifi, Retry after:" + aInterval);

        setInterval(aInterval);
        setStatus(WIFI_STA_STATE_CONNECTING);

      }
    }

    void OnDisable() {
      int i = getId();
      Serial.print(millis()); Serial.print(":\t");
      Serial.print("WifiManager: TaskID=");
      Serial.println(i);
      WiFi.disconnect();
      //      mDisConnectHandler = NULL;
    }

    bool Callback() {
      Serial.println((String)"WifiManager Callback");
      update();
      return true;
    }

};
#endif
