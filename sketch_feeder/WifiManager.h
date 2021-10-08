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

typedef std::function<void(int status)> WifiStatusListener;

class WifiManager {

  private:

    IPAddress *staticip;
    String accessPointSsid;
    String ssid;
    String password;
    uint8_t status;
    uint8_t mode = true;
    WifiStatusListener wifiStatusListener = NULL;
    WiFiEventHandler gotIpEventHandler, mDisConnectHandler;

    void onDisconnect() {
      Serial.println ( "WiFi On Disconnect." + WiFi.status()  );
      setStatus(WIFI_STA_STATE_FAILED);

    }
    void onConnect() {
      setStatus(WIFI_STA_STATE_ESTABLISHED);
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

      if (ssid != NULL && ssid != '\0' && ssid != "") {
        WiFi.begin(ssid, password);
      }

      Serial.println("beginStation finished ");

    }

  public:
    WifiManager(IPAddress* staticip) {
      WiFi.mode(WIFI_OFF);

      this->staticip = staticip;

      gotIpEventHandler = WiFi.onStationModeGotIP([this](const WiFiEventStationModeGotIP & event)
      {
        Serial.print("Station connected, IP: ");
        Serial.println(WiFi.localIP());
        this->onConnect();
      });

      mDisConnectHandler = WiFi.onStationModeDisconnected([this](const WiFiEventStationModeDisconnected & event)
      {
        this->onDisconnect();
      });
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

    void setup(String ssid, String password, String accessPointSsid, uint8_t mode) {
      this->ssid = ssid;
      this->password = password;
      this->accessPointSsid = accessPointSsid;
      this->mode = mode;
    }

    uint8_t getMode() {
      return this->mode;
    }

    void setOnWifiStatusListener(const WifiStatusListener& listener) {
      this->wifiStatusListener = listener;
    }

    void setStatus(uint8_t value) {
      Serial.println((String)"WifiManager setStatus value:" + value + " old:" + status );
      if (wifiStatusListener != NULL && value != status) wifiStatusListener(value);
      status = value;
    }

    void enableAp() {
      Serial.println((String) "accessPointSsid:" + accessPointSsid );
      WiFi.softAP(accessPointSsid);
    }
    void disableAp() {
      WiFi.softAPdisconnect(true);
    }
    void setMode(uint8_t mode) {
      this->mode = mode;
      
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
