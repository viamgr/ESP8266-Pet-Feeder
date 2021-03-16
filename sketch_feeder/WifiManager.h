#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H
#define WIFI_MANAGER_TASK "WIFI_MANAGER_TASK"
#define WIFI_MANAGER_TASK_LONG "WIFI_MANAGER_TASK_LONG"

#define WIFI_STA_STATE_BEGIN 0
#define WIFI_STA_STATE_CONNECTING 1
#define WIFI_STA_STATE_ESTABLISHED 2
#define WIFI_STA_STATE_FAILED 4
#include <DNSServer.h>

#define DO(x...) Serial.println(F( #x )); x; break

typedef std::function<void(int status)> WifiStatusListener;

class WifiManager {

  private:

    DNSServer *dnsServer = new DNSServer();
    IPAddress *staticip;
    IPAddress *gateway ;
    IPAddress *subnet;
    const byte DNS_PORT = 53;
    String accessPointSsid;
    String ssid;
    String password;
    uint8_t status;
    boolean isStationOn = true;
    boolean isAccessPointOn = true;
    WifiStatusListener wifiStatusListener = NULL;
    WiFiEventHandler gotIpEventHandler, mDisConnectHandler;

    void onDisconnect() {
      Serial.println ( "WiFi On Disconnect." + WiFi.status()  );
      setStatus(WIFI_STA_STATE_FAILED);

    }
    void onConnect() {
      setStatus(WIFI_STA_STATE_ESTABLISHED);
    }

    void wiFiOff() {
      wifi_station_disconnect();
      wifi_set_opmode(NULL_MODE);
      wifi_set_sleep_type(MODEM_SLEEP_T);
      wifi_fpm_open();
      //wifi_fpm_do_sleep(0xFFFFFFF);
    }

    void wiFiOn() {
      wifi_fpm_do_wakeup();
      wifi_fpm_close();
      wifi_set_opmode(STATION_MODE);
      wifi_station_connect();
    }
  public:
    WifiManager(IPAddress* staticip, IPAddress* gateway, IPAddress* subnet) {
      WiFi.mode(WIFI_OFF);

      this->staticip = staticip;
      this->gateway = gateway;
      this->subnet = subnet;

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

    void connectToStation() {
      Serial.print("connectToStation, IP: "+ssid);

      if (ssid != NULL && ssid != '\0' && ssid != "") {
        doCommand('B');
        isStationOn = true;
      }
    }

    void turnOffStation() {
      if (isAccessPointOn) {
        doCommand('1');
      }
      else {
        doCommand('F');
      }
      isStationOn = false;
    }

    void turnOff() {
      doCommand('F');
      isAccessPointOn = false;
      isStationOn = false;
    }

    void turnOffAccessPoint() {
      if (isStationOn) {
        doCommand('3');
      }
      else {
        doCommand('F');
      }
      isAccessPointOn = false;
    }

    void turnOnAccessPoint() {
      WiFi.softAP(accessPointSsid);
      Serial.println((String) "accessPointSsid:" + accessPointSsid + ", isStationOn:" + isStationOn);

      if (isStationOn) {
        doCommand('2');
      }
      else {
        doCommand('1');
      }
      isAccessPointOn = true;
    }

    void doCommand(char cmd) {
      Serial.println((String) "doCommand:" + cmd);

      switch (cmd) {
        case 'F': DO(wiFiOff());
        case 'N': DO(wiFiOn());
        case '1': DO(WiFi.mode(WIFI_AP));
        case '2': DO(WiFi.mode(WIFI_AP_STA));
        case '3': DO(WiFi.mode(WIFI_STA));
        case 'R': DO(if (((GPI >> 16) & 0xf) == 1) ESP.reset() /* else must hard reset */);
        case 'd': DO(WiFi.disconnect());
        case 'b': DO(WiFi.begin());
        case 'B': DO(WiFi.begin(ssid, password));
        case 'r': DO(WiFi.reconnect());
        case 'c': DO(wifi_station_connect());
        case 'a': DO(WiFi.setAutoReconnect(false));
        case 'A': DO(WiFi.setAutoReconnect(true));
        case 'n': DO(WiFi.setSleepMode(WIFI_NONE_SLEEP));
        case 'l': DO(WiFi.setSleepMode(WIFI_LIGHT_SLEEP));
        case 'm': DO(WiFi.setSleepMode(WIFI_MODEM_SLEEP));
        case 'S': DO(WiFi.config(*staticip, *gateway, *subnet)); // use static address
        case 's': DO(WiFi.config(0u, 0u, 0u));                // back to dhcp client
      }

    }

    void setup(String ssid, String password, String accessPointSsid, boolean isStationOn, boolean isAccessPointOn) {
      this->ssid = ssid;
      this->password = password;
      this->accessPointSsid = accessPointSsid;
      this->isStationOn = isStationOn;
      this->isAccessPointOn = isAccessPointOn;
    }

    void setOnWifiStatusListener(const WifiStatusListener& listener) {
      this->wifiStatusListener = listener;
    }

    void setStatus(uint8_t value) {
      Serial.println((String)"WifiManager setStatus value:" + value + " old:" + status );
      if (wifiStatusListener != NULL && value != status) wifiStatusListener(value);
      status = value;
    }

    void startDns() {
      
      Serial.println("startDns");
      WiFi.softAPConfig(*staticip, *gateway, *subnet);

      // modify TTL associated  with the domain name (in seconds)
      // default is 60 seconds
      dnsServer->setTTL(300);
      // set which return code will be used for all other domains (e.g. sending
      // ServerFailure instead of NonExistentDomain will reduce number of queries
      // sent by clients)
      // default is DNSReplyCode::NonExistentDomain
      dnsServer->setErrorReplyCode(DNSReplyCode::ServerFailure);

      // start DNS server for a specific domain name
      dnsServer->start(DNS_PORT, "*", *staticip);
    }
    void update() {
      dnsServer->processNextRequest();
    }
};
#endif
