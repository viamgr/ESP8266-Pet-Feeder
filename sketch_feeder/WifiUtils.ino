#include "WifiManager.h"
WifiManager wifiManager(&staticip);

WifiManager* getWifiManager() {
  return &wifiManager;
}

void updateWifiManager() {

}

void initWifiManager() {

  wifiManager.setOnWifiStatusListener([ = ](int wifiState) {
    Serial.println();
    Serial.print("wifiState:");
    Serial.println(wifiState);

//    if (wifiState == WIFI_STA_STATE_ESTABLISHED) {
//      getNtpManager()->enable();
//    }
//    else {
//      getNtpManager()->disable();
//    }
  });


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
  wifiManager.setup(preferences.getWifiSsid(), preferences.getWifiPassword(),
                    preferences.getAccessPointName(), preferences.getWifiMode());
  restartWifi();
  callDnsManagerAboutWifiConfigChanged();
  callSocketAboutWifiConfigChanged();
}

String getWifiList() {
  return wifiManager.getWifiList();
}
