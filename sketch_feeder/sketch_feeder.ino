#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass 
#define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
#define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
#define _TASK_PRIORITY          // Support for layered scheduling priority
#define _TASK_TIMEOUT           // Support for overall task timeout 
#define _TASK_OO_CALLBACKS

#include <WebSocketsClient.h>
#include <TaskScheduler.h>
#include <ESP8266WiFi.h>
#include "NtpManager.h"
#include "WifiManager.h"
#include "Preferences.h"

String deviceId = "Feeder1";


NtpManager* getNtpManager();
Scheduler taskManager;

Scheduler* getScheduler() {
  return &taskManager;
}

Preferences preferences;

Preferences* getPreferences() {
  return &preferences;
}

#define USE_SERIAL Serial

void stopAllTasks() {
  stopAudio();
  disableLedTask();
  stopFeeding();
}

void onSetupConfig() {
  Serial.println("OnSetupConfig");
  reloadPreferences();
  resetFeedingTasks();
  setupWifi();
  setupLed();
  setupAudioConfig();
  configDnsManager();
}
void reloadPreferences() {
  preferences.reload();
}
void setup()
{
  setenv("TZ", "UTC-03:30", 0);
  Serial.begin(115200);
  showDeviceInfo();
  onSetupConfig();
  initialSetup();


  Dir dir = SPIFFS.openDir("/upload");
  while (dir.next()) {
    Serial.println((String)"remove:" + dir.fileName());
    SPIFFS.remove( dir.fileName());
  }


  

//  WiFi.mode(WIFI_STA);
//
//  WiFi.softAPdisconnect (true);
//  
//  IPAddress *staticIp = new IPAddress(1, 1, 1, 1);
//  IPAddress *gateway = new IPAddress(1, 2, 3, 4);
//  IPAddress *subnet = new IPAddress(255, 255, 255, 0);
//
//  staticIp->fromString("192.168.8.150");
//  gateway->fromString("192.168.8.1");
//  subnet->fromString("255.255.255.0");
//
//
//  WiFi.config(*staticIp, *gateway, *subnet);
//
//  WiFi.begin("BMV2", "6037991302");

}

void initialSetup() {
  //  initNtp();
  initWifiManager();
  initServer();
  initClickButton();
  setupDnsManager();
  setupSocketHandler();
  Serial.println("initialSetup");
}
void loop()
{
  updateWifiManager();
  updateClickButton();
  updateFeedingLoop();
  updateServerControl();
  updateSocketHandler();
  updateDnsManager();
  taskManager.execute();
}
