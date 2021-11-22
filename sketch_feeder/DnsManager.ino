
#include <DNSServer.h>

const byte DNS_PORT = 53;
DNSServer* dnsServer = NULL;
bool dnsStarted = false;
void callDnsManagerAboutWifiConfigChanged() {

  uint8_t mode = getWifiManager()->getMode();

  if (!dnsStarted && (mode == WIFI_MODE_AP_STA || mode == WIFI_MODE_AP) && getWifiManager()->getStatus() != WIFI_STA_STATE_ESTABLISHED) {
    startDns();
  }
  else {
    stopDns();
  }

}

void updateDnsManager() {
  uint8_t mode = getWifiManager()->getMode();
  if (dnsServer != NULL) {
    if (mode == WIFI_MODE_AP_STA || mode == WIFI_MODE_AP) {
      dnsServer->processNextRequest();
    }
  }

}
void startDns() {
  if (dnsStarted) {
    Serial.println("startDns already");
    return;
  }
  if (preferences.getStaticIp() == NULL || preferences.getGateway() == NULL || preferences.getSubnet() == NULL) {

    Serial.println("startDns ip problem");
    return;
  }
  Serial.println("startDns");

  IPAddress staticIpAddress;
  staticIpAddress.fromString(preferences.getStaticIp());
  IPAddress gatewayAddress;
  gatewayAddress.fromString(preferences.getGateway());
  IPAddress subnetAddress;
  subnetAddress.fromString(preferences.getSubnet());
  WiFi.softAPConfig(staticIpAddress, gatewayAddress, subnetAddress);

  dnsServer = new DNSServer();
  // modify TTL associated  with the domain name (in seconds)
  // default is 60 seconds
  dnsServer->setTTL(300);
  // set which return code will be used for all other domains (e.g. sending
  // ServerFailure instead of NonExistentDomain will reduce number of queries
  // sent by clients)
  // default is DNSReplyCode::NonExistentDomain
  dnsServer->setErrorReplyCode(DNSReplyCode::ServerFailure);

  // start DNS server for a specific domain name
  dnsServer->start(DNS_PORT, "*", staticIpAddress);
  dnsStarted = true;
}

void stopDns() {
  if (dnsStarted) {
    Serial.println("stopDns");
    dnsServer->stop();
    dnsStarted = false;
  }
}


void configDnsManager() {


}
void setupDnsManager() {


}
