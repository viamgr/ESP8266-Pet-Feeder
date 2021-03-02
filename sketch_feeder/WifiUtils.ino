
void connectToWifi() {
  Serial.println((String) "connectToWifi:" + preferences.getWifiSsid() + ", pass:" + preferences.getWifiPassword());
  if (preferences.isStationOn()) {
    wifiManager.connectToStation();
  }

}