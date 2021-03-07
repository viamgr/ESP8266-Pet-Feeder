#include <TaskScheduler.h>
#include <ESP8266WiFi.h>
#include "Preferences.h"

#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass 
#define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
#define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
#define _TASK_PRIORITY          // Support for layered scheduling priority
#define _TASK_TIMEOUT           // Support for overall task timeout 
#define _TASK_OO_CALLBACKS

IPAddress staticip(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

Scheduler taskManager;

Preferences preferences;

void stopAllTasks() {
  stopAudio();
  disableLedTask();
  stopFeeding();
}

void onSetupConfig() {
  Serial.println("OnSetupConfig");
  resetFeedingTasks();
  setupWifi();
  setupLed();
  setupAudioConfig();
}

void setup()
{
  Serial.begin(115200);
  onSetupConfig();
  initialSetup();
}

void initialSetup() {
  initNtp();
  initWifiManager();
  initServer(); 
  connectToWifi();
  turnOnAccessPoint();
  initClickButton();
}
void loop()
{
  updateWifiManager();
  updateClickButton();
  updateFeedingLoop();
  updateServerControl();
  taskManager.execute();
}
