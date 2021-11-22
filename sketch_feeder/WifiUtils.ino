#include "WifiManager.h"


WifiManager wifiManager(getScheduler());

WifiManager* getWifiManager() {
  return &wifiManager;
}

void updateWifiManager() {

}

void onWiFiEvent(WiFiEvent event)
{
  Serial.println("WiFi onWiFiEvent ");
  wifiManager.onWiFiEvent(event);
}


void initWifiManager() {

  WiFi.onEvent(onWiFiEvent, WIFI_EVENT_STAMODE_DISCONNECTED);
  WiFi.onEvent(onWiFiEvent, WIFI_EVENT_STAMODE_CONNECTED);
  wifiManager.setOnWifiStatusListener([ = ](int wifiState) {
    Serial.println();
    Serial.print("wifiState:");
    Serial.println(wifiState);

//    callDnsManagerAboutWifiConfigChanged();
    callSocketAboutWifiConfigChanged();

    //    if (wifiState == WIFI_STA_STATE_ESTABLISHED) {
    //      getNtpManager()->enable();
    //    }
    //    else {
    //      getNtpManager()->disable();
    //    }
  });
  callSocketAboutWifiConfigChanged();

//  callDnsManagerAboutWifiConfigChanged();

}
void restartWifi() {
  Serial.println("restartWifi:");
  wifiManager.start();
}

void saveWifiMode(uint8_t mode) {
  Serial.println((String) "setWifiMode:" + mode);
  preferences.setWifiMode(mode);
  preferences.save();
}

void shiftWifiState() {
  saveWifiMode((preferences.getWifiMode() + 1) % 4);
  setupWifi();
  restartWifi();
}

void setupWifi() {
  Serial.println((String) "setupWifi:" + preferences.getWifiMode());
  wifiManager.setup(preferences.getWifiSsid(), preferences.getWifiPassword(), preferences.getAccessPointName(),
                    preferences.getWifiMode(), preferences.getStaticIp(), preferences.getGateway(), preferences.getSubnet(), preferences.useDhcp() == DHCP_ENABLED);
  restartWifi();
}

String getWifiList() {
  return wifiManager.getWifiList();
}
