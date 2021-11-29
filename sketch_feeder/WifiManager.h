#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H
#define WIFI_MANAGER_TASK "WIFI_MANAGER_TASK"
#define WIFI_MANAGER_TASK_LONG "WIFI_MANAGER_TASK_LONG"

#define WIFI_STA_STATE_BEGIN 0
#define WIFI_STA_STATE_CONNECTING 1
#define WIFI_STA_STATE_ESTABLISHED 2
#define WIFI_STA_STATE_FAILED 4

#define WIFI_MODE_OFF 0
#define WIFI_MODE_AP 1
#define WIFI_MODE_STA 2
#define WIFI_MODE_AP_STA 3

#include "TimeoutHandler.h"

typedef std::function<void(int status)> WifiStatusListener;

class WifiManager {

  private:

    IPAddress *staticIp;
    IPAddress *gateway;
    IPAddress *subnet;
    String accessPointSsid;
    String ssid;
    String password;
    uint8_t status;
    uint8_t currentMode = -1;
    uint8_t mode = true;
    bool useDhcp = true;
    bool autoApSwitch = false;
    bool autoRetry = true;
    WifiStatusListener wifiStatusListener = NULL;
    WiFiEventHandler gotIpEventHandler, mDisConnectHandler;
    TimeoutHandler *timeoutHandler = NULL;

    void onDisconnect() {
      Serial.println ( "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD" );
      if (this->mode == WIFI_MODE_STA) {
        autoApSwitch = true;
        setMode(WIFI_MODE_AP_STA);
      }
      setStatus(WIFI_STA_STATE_FAILED);

      timeoutHandler->setInterval(10000);
      timeoutHandler->restart();


    }
    void reconnect() {
      if (autoRetry)
        WiFi.reconnect();
    }

    void setAutoRetry(bool autoRetry) {
      this->autoRetry = autoRetry;
    }

    void onConnect() {
      if (this->mode == WIFI_MODE_AP_STA && autoApSwitch) {
        setMode(WIFI_MODE_STA);
      }
      setStatus(WIFI_STA_STATE_ESTABLISHED);

//      //Serial.print("Station connected, IP: ");
//      //Serial.println(WiFi.localIP());
    }

    void turnOff() {
      wifi_station_disconnect();
      wifi_set_opmode(NULL_MODE);
      wifi_set_sleep_type(MODEM_SLEEP_T);
      wifi_fpm_open();
      //wifi_fpm_do_sleep(0xFFFFFFF);
    }


    void beginStation() {
      Serial.println("beginStation, IP: " + ssid);

      if (ssid != NULL && ssid != "") {
        if (!useDhcp) {
          WiFi.config(*staticIp, *gateway, *subnet);
        }
        WiFi.begin(ssid, password);
      }

      Serial.println((String)"beginStation finished useDhcp:" + (useDhcp == true));

    }



  public:
    WifiManager(Scheduler* scheduler) {
      timeoutHandler = new TimeoutHandler(scheduler);
      timeoutHandler->setOnTimeoutListener([this]() {
        Serial.println((String)"wifi_softap_get_station_num:" + wifi_softap_get_station_num());
        if (wifi_softap_get_station_num() == 0)
          this->reconnect();
        else {
          timeoutHandler->restart();
        }
      });
      WiFi.setAutoReconnect(false);
      WiFi.mode(WIFI_OFF);


      //      gotIpEventHandler = WiFi.onStationModeGotIP([this](const WiFiEventStationModeGotIP & event)
      //      {
      //        this->onConnect();
      //      });
      //
      //      mDisConnectHandler = WiFi.onStationModeDisconnected([this](const WiFiEventStationModeDisconnected & event)
      //      {
      //        this->onDisconnect();
      //      });

    }
    void onWiFiEvent(WiFiEvent event)
    {
      switch (event)
      {
        case WIFI_EVENT_STAMODE_DISCONNECTED:
          Serial.println("WiFi WIFI_EVENT_STAMODE_DISCONNECTED ");
          onDisconnect();
          break;
        case WIFI_EVENT_STAMODE_CONNECTED:
          Serial.println("WiFi WIFI_EVENT_STAMODE_CONNECTED ");
          onConnect();
          break;
      }
    }

    //
    //    void connectToStation() {
    //      if (mode == WIFI_MODE_OFF || mode == WIFI_MODE_STA ) {
    //        setMode(WIFI_MODE_STA);
    //        disableAp();
    //      } else if (mode == WIFI_MODE_AP) {
    //        setMode(WIFI_MODE_AP_STA);
    //      }
    //      beginStation();
    //    }


    void setup(String ssid, String password, String accessPointSsid, uint8_t mode,
               String staticIp, String gateway, String subnet, bool useDhcp)
    {
      Serial.println((String)"setup:" + ssid + " useDhcp:" + useDhcp + " staticIp:" + staticIp);


      this->staticIp = new IPAddress(192, 168, 8, 150);
      this->gateway = new IPAddress(192, 168, 8, 1);
      this->subnet = new IPAddress(255, 255, 255, 0);

      this->ssid = ssid;
      this->password = password;
      this->accessPointSsid = accessPointSsid;
      this->mode = mode;
      this->staticIp->fromString(staticIp);
      this->gateway->fromString(gateway);
      this->subnet->fromString(subnet);
      this->useDhcp = useDhcp;
    }

    uint8_t getStatus() {
      return this->status;
    }
    uint8_t getMode() {
      return this->mode;
    }

    void setOnWifiStatusListener(const WifiStatusListener& listener) {
      this->wifiStatusListener = listener;
    }

    void setStatus(uint8_t value) {
      Serial.println((String)"WifiManager setStatus value:" + value + " old:" + status );
      bool callWifiListener = wifiStatusListener != NULL && value != status;
      status = value;
      if (callWifiListener)
        wifiStatusListener(value);
    }

    void enableAp() {
      Serial.println((String) "accessPointSsid:" + accessPointSsid );
      WiFi.softAP(accessPointSsid);
    }
    void disableAp() {
      WiFi.softAPdisconnect(true);
    }
    void setMode(uint8_t mode) {
    if(currentMode == mode){
       Serial.println((String) " No need to restart currentMode:" + currentMode +" mode:"+mode);
       return;
    }
      this->mode = mode;
      this->currentMode = mode;
      Serial.println((String) "this->mode:" + this->mode );

      if (mode == WIFI_MODE_AP_STA) {
        Serial.println("Turn on station and access point");

        WiFi.mode(WIFI_AP_STA);

        beginStation();
        enableAp();
      }
      else if (mode == WIFI_MODE_AP) {
        Serial.println("Turn On Access Point and turn of station");

        WiFi.mode(WIFI_AP);
        enableAp();
      }
      else if (mode == WIFI_MODE_STA) {
        Serial.println("Turn off access Point and turn on station");
        WiFi.mode(WIFI_STA);
        disableAp();
        beginStation();
      }
      else {
        Serial.println("Turn Off wifi");
        WiFi.mode(WIFI_OFF);

        disableAp();
        turnOff();

      }

    }

    boolean  isAccessPointMode() {
      return mode == WIFI_MODE_AP_STA || mode == WIFI_MODE_AP;
    }

    boolean  isStationMode() {
      return mode == WIFI_MODE_AP_STA || mode == WIFI_MODE_STA;
    }

    void start() {
      setMode(mode);
    }

    String getWifiList() {
      String json = "[";
      int n = WiFi.scanComplete();
      if (n == -2) {
        WiFi.scanNetworks(true);
      } else if (n) {
        for (int i = 0; i < n; ++i) {
          if (i != 0) json += ",";
          json += "{";
          //            json += "\"rssi\":" + String(WiFi.RSSI(i));
          json += "\"ssid\":\"" + WiFi.SSID(i) + "\"";
          json += ",\"bssid\":\"" + WiFi.BSSIDstr(i) + "\"";
          //json += ",\"channel\":" + String(WiFi.channel(i));
          json += ",\"secure\":" + String(WiFi.encryptionType(i));
          //json += ",\"hidden\":" + String(WiFi.isHidden(i) ? "true" : "false");
          json += "}";
        }
        WiFi.scanDelete();
        if (WiFi.scanComplete() == -2) {
          WiFi.scanNetworks(true);
        }
      }
      json += "]";
      return json;
    }
};
#endif
