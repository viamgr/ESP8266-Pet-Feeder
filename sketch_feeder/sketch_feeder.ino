
#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass 
#define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
#define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
#define _TASK_PRIORITY          // Support for layered scheduling priority
#define _TASK_TIMEOUT           // Support for overall task timeout 
#define _TASK_OO_CALLBACKS

#define AUDIO_FEEDING "/feeding.mp3"
#define AUDIO_START_UP "/miracle.mp3"
#define LED_PIN D6
#define MOTOR_PIN D5
#define BUTTON_PIN D2

#include <SceneManager.h>
#include "Preferences.h"
#include "LedControl.h"
#include "MotorControl.h"
#include "ClickButton.h"
#include "AudioControl.h"
#include "ServerControl.h"
#include "WifiManager.h"
#include "NtpManager.h"
#include <ESP8266WiFi.h>
#include <TaskManager.h>
#include <Time.h>
#include <TaskScheduler.h>



#include <time.h>                       // time() ctime()
#ifdef ESP8266M
#include <sys/time.h>                   // struct timeval
#endif
#include "CronAlarms.h"

const char* ssid = "Feeder-Access-Point";
Scheduler taskManager;

LedControl led(&taskManager, LED_PIN);
MotorControl motor(&taskManager, MOTOR_PIN);
AudioControl audioControl(&taskManager);
WifiManager wifiManager(&taskManager, ssid);
NtpManager ntpManager(&taskManager);
ServerControl serverControl;
bool firstUpdateTime = true;
CronId alarms[dtNBR_ALARMS];

Preferences preferences;

ClickButton button(BUTTON_PIN);
bool first = true;

void setupButton() {
  pinMode(BUTTON_PIN, INPUT);
}

void connectToWifi() {
  Serial.println((String) "connectToWifi" + preferences.getWifiSsid());
  if (preferences.getWifiSsid() != NULL && preferences.getWifiSsid()[0] != '\0')
    wifiManager.enable();
}

void stopFeedingAlarm() {
  for (int x = 0; x < dtNBR_ALARMS; x++)  {
    if (alarms[x] != dtINVALID_ALARM_ID) {
      Cron.free(alarms[x]);
    }
  }
  memset(alarms, dtINVALID_ALARM_ID, dtNBR_ALARMS);
}

void onFeedingAlarm() {
  Serial.println((String)"getTime " + now());
  JsonArray data = preferences.getAlarms();
  for (int x = 0; x < data.size(); x++)  {
    const char *time = data[x];
    Serial.println((String)"time:" + time);
    alarms[x] = Cron.create((char*)time, onCompositeFeeding, false);
  }
}

void stopAllTasks() {
  audioControl.stop();
  led.disable();
  motor.stop();
  //  ntpManager.restartDelayed();
  //  wifiManager.restartDelayed();
}
void onCompositeFeeding() {
  Serial.println((String)"onCompositeFeeding");
  audioControl.play(AUDIO_FEEDING, preferences.getSoundVolume(), [ = ]() {
    led.on();
    motor.rotate(preferences.getFeedingDuration(), [ = ]() {
      led.on();
    });
  });

}
void onFeedingEvent(void (*listener)()) {
  Serial.println((String) "onFeedingEvent");
  motor.rotate(preferences.getFeedingDuration(), listener);
}
void onPlayFeedingAudioEvent(void (*listener)()) {
  Serial.println((String) "onPlayFeedingAudioEventedingEvent");

  stopAllTasks();
  audioControl.play(AUDIO_FEEDING, preferences.getSoundVolume(), listener);
}

void onLedTimerEvent() {
  Serial.println((String)"onLedTimerEvent");
  led.toggle();
}

void resetFeedingTasks() {
  stopFeedingAlarm();
  onFeedingAlarm();

}
void setDeviceTime(unsigned long epochTime) {

  stopFeedingAlarm();
  Serial.println((String)"setDeviceTime epochTime... " + epochTime);
  setTime(epochTime);

  struct timeval tv;
  tv.tv_sec = epochTime;

  settimeofday(&tv, NULL);
  resetFeedingTasks();
}

void onSetupConfig() {
  Serial.println("OnSetupConfig");

  resetFeedingTasks();
  wifiManager.setup(preferences.getWifiSsid(), preferences.getWifiPassword());
  led.setup(preferences.getLedTurnOffDelay());

}
void setStatusCallback(String key, String value) {
  if (key == "STATUS_TIME")
    setDeviceTime(string_to_long(value));
}

unsigned long string_to_long (String number)
{
  number = number + ' ';
  char buf[number.length()];
  //Serial.println(number);
  number.toCharArray(buf, number.length());
  unsigned long result = atol(buf);
  //Serial.println(result);
  return result;
}

String getStatusCallback(String key) {
  if (key == "STATUS_TIME")
    return String(now());
}
void eventCallback(String name) {
  stopAllTasks();
  Serial.println((String)"name " + name);
  if (name == "EVENT_FEEDING")
    onFeedingEvent(NULL);
  else if (name == "EVENT_LED_TIMER")
    onLedTimerEvent();
  else if (name == "EVENT_PLAY_FEEDING_AUDIO")
    onPlayFeedingAudioEvent(NULL);
  else if (name == "EVENT_WIFI_CONNECT")
    connectToWifi();
  else if (name == "EVENT_COMPOSITE_FEEDING")
    onCompositeFeeding();
  //  else if (key == "SETTING_SET_TIME")
  //    setDeviceTime((long) value);
}

void showDeviceInfo() {

  SPIFFS.begin();
  FSInfo fs_info;
  SPIFFS.info(fs_info);

  float fileTotalKB = (float)fs_info.totalBytes / 1024.0;
  float fileUsedKB = (float)fs_info.usedBytes / 1024.0;

  float flashChipSize = (float)ESP.getFlashChipSize() / 1024.0 / 1024.0;
  float realFlashChipSize = (float)ESP.getFlashChipRealSize() / 1024.0 / 1024.0;
  float flashFreq = (float)ESP.getFlashChipSpeed() / 1000.0 / 1000.0;
  FlashMode_t ideMode = ESP.getFlashChipMode();

  Serial.printf("\n#####################\n");

  Serial.printf("__________________________\n\n");
  Serial.println("Firmware: ");
  Serial.printf("    Chip Id: %08X\n", ESP.getChipId());
  Serial.print("    Core version: "); Serial.println(ESP.getCoreVersion());
  Serial.print("    SDK version: "); Serial.println(ESP.getSdkVersion());
  Serial.print("    Boot version: "); Serial.println(ESP.getBootVersion());
  Serial.print("    Boot mode: "); Serial.println(ESP.getBootMode());


  Serial.printf("__________________________\n\n");

  Serial.println("Flash chip information: ");
  Serial.printf("    Flash chip Id: %08X (for example: Id=001640E0  Manuf=E0, Device=4016 (swap bytes))\n", ESP.getFlashChipId());
  Serial.printf("    Sketch thinks Flash RAM is size: ");  Serial.print(flashChipSize); Serial.println(" MB");
  Serial.print("    Actual size based on chip Id: "); Serial.print(realFlashChipSize); Serial.println(" MB ... given by (2^( \"Device\" - 1) / 8 / 1024");
  Serial.print("    Flash frequency: "); Serial.print(flashFreq); Serial.println(" MHz");
  Serial.printf("    Flash write mode: %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));

  Serial.printf("__________________________\n\n");

  Serial.println("File system (SPIFFS): ");
  Serial.print("    Total KB: "); Serial.print(fileTotalKB); Serial.println(" KB");
  Serial.print("    Used KB: "); Serial.print(fileUsedKB); Serial.println(" KB");
  Serial.printf("    Block size: %lu\n", fs_info.blockSize);
  Serial.printf("    Page size: %lu\n", fs_info.pageSize);
  Serial.printf("    Maximum open files: %lu\n", fs_info.maxOpenFiles);
  Serial.printf("    Maximum path length: %lu\n\n", fs_info.maxPathLength);

  Dir dir = SPIFFS.openDir("/");
  Serial.println("SPIFFS directory {/} :");
  while (dir.next()) {
    Serial.print("  "); Serial.println(dir.fileName());
  }

  Serial.printf("__________________________\n\n");

  Serial.printf("CPU frequency: %u MHz\n\n", ESP.getCpuFreqMHz());
  Serial.print("#####################");
}
void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);

  //showDeviceInfo();
  onSetupConfig();
  initialSetup();
}

void onTimeUpdate(unsigned long epochTime) {
  if (firstUpdateTime == true) {
    firstUpdateTime = false;
    audioControl.play(AUDIO_START_UP, preferences.getSoundVolume(), [ = ]() {
      setDeviceTime(epochTime);
    });
  }
  else {
    setDeviceTime(epochTime);
  }
}

void initialSetup() {
  setenv("TZ", "UTC-03:30", 0);
  ntpManager.setOnTimeUpdateListener(onTimeUpdate);

  wifiManager.setOnWifiStatusListener([ = ](int wifiState) {
    Serial.println();
    Serial.print("wifiState:");
    Serial.println(wifiState);

    if (wifiState == WIFI_STA_STATE_ESTABLISHED) {
      ntpManager.enable();
    }
    else {
      ntpManager.disable();
    }
  });


  serverControl.setEventListener(eventCallback);
  serverControl.setOnSetStatusListener(setStatusCallback);
  serverControl.setOnGetStatusListener(getStatusCallback);
  serverControl.setStopTasksListener([ = ]() {
    stopAllTasks();
  });
  serverControl.setUploadListener([ = ](String filename) {
    Serial.print((String)"filename uploaded:" + filename);
    stopAllTasks();

    if (filename == AUDIO_FEEDING) {
      //      onPlayFeedingAudioEvent(NULL);
    }
    else if (filename == CONFIG_FILE_PATH) {
      preferences.reload();
      onSetupConfig();
    }
  });

  connectToWifi();
  setupButton();
}
void loop()
{
  button.Update();
  Tasks.update(); // automatically execute tasks
  Cron.delay(0); // wait one second between clock display
  serverControl.loop();
  taskManager.execute();

  if (button.changed == 1) {
    if (first == false) {
      stopAllTasks();
      if (button.clicks == -1) {
        onCompositeFeeding();
      } else if (button.clicks == -2) {
        onLedTimerEvent();
      }
    }
    first = false;
  }

}
