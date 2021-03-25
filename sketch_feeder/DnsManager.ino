
#include <DNSServer.h>

const byte DNS_PORT = 53;
DNSServer* dnsServer = NULL;

void callDnsManagerAboutWifiConfigChanged() {
  uint8_t mode = getWifiManager()->getMode();
//  if (mode == WIFI_MODE_AP_STA || mode == WIFI_MODE_AP) {
//    startDns();
//  }
//  else {
//    stopDns();
//  }

}

void updateDnsManager() {
  uint8_t mode = getWifiManager()->getMode();
  if (mode == WIFI_MODE_AP_STA || mode == WIFI_MODE_AP) {
    //dnsServer->processNextRequest();
  }

}
void startDns() {
  Serial.println("startDns");
//  WiFi.softAPConfig(staticip, gateway, subnet);
//
//  dnsServer = new DNSServer();
//  // modify TTL associated  with the domain name (in seconds)
//  // default is 60 seconds
//  dnsServer->setTTL(300);
//  // set which return code will be used for all other domains (e.g. sending
//  // ServerFailure instead of NonExistentDomain will reduce number of queries
//  // sent by clients)
//  // default is DNSReplyCode::NonExistentDomain
//  dnsServer->setErrorReplyCode(DNSReplyCode::ServerFailure);
//
//  // start DNS server for a specific domain name
//  dnsServer->start(DNS_PORT, "*", *staticip);
}

void stopDns() {
//  dnsServer->stop();
//  delete dnsServer;
}


void configDnsManager() {


}
void setupDnsManager() {


}
