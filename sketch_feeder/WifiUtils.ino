#include "WifiManager.h"
WifiManager wifiManager( &staticip, &gateway, &subnet);

void updateWifiManager() {
  wifiManager.update();

}

void initWifiManager() {

  wifiManager.setOnWifiStatusListener([ = ](int wifiState) {
    Serial.println();
    Serial.print("wifiState:");
    Serial.println(wifiState);

    if (wifiState == WIFI_STA_STATE_ESTABLISHED) {
      ntpManager.enable();
      wifiManager.startDns();
    }
    else {
      ntpManager.disable();
    }
  });

}
void connectToWifi() {
  Serial.println((String) "connectToWifi:" + preferences.getWifiSsid() + ", pass:" + preferences.getWifiPassword() + " isStationOn:" + preferences.isStationOn());
  wifiManager.connectToStation();

}


void turnOnAccessPoint() {
  Serial.println((String) "turnOnAccessPoint:" + preferences.getAccessPointName() + " isAccessPointOn:" + preferences.isAccessPointOn());
  wifiManager.turnOnAccessPoint();
}


void shiftWifiState() {
  if (preferences.isAccessPointOn() && preferences.isStationOn()) {
    preferences.setStationOn(false);
    wifiManager.turnOffStation();
    preferences.setAccessPointOn(true);
    wifiManager.turnOnAccessPoint();
    Serial.println("Turn On Access Point and turn of station");
  }
  else if (preferences.isAccessPointOn() ) {
    preferences.setAccessPointOn(false);
    preferences.setStationOn(true);
    wifiManager.connectToStation();
    wifiManager.turnOffAccessPoint();
    Serial.println("Turn off access Point and turn on station");
  }
  else if (preferences.isStationOn() ) {
    preferences.setStationOn(false);
    preferences.setAccessPointOn(false);
    Serial.println("Turn Off wifi");
    wifiManager.turnOff();
  }
  else {
    preferences.setAccessPointOn(true);
    preferences.setStationOn(true);
    wifiManager.connectToStation();
    wifiManager.turnOnAccessPoint();
    Serial.println("Turn on station and access point");
  }
  preferences.save();
}
void setupWifi() {

  wifiManager.setup(preferences.getWifiSsid(), preferences.getWifiPassword(),
                    preferences.getAccessPointName(), preferences.isAccessPointOn(), preferences.isStationOn());
}
