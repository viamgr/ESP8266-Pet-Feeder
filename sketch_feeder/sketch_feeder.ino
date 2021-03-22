#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass 
#define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
#define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
#define _TASK_PRIORITY          // Support for layered scheduling priority
#define _TASK_TIMEOUT           // Support for overall task timeout 
#define _TASK_OO_CALLBACKS

#include <WebSocketsClient.h>

#include <TaskScheduler.h>
#include <ESP8266WiFi.h>
#include "Preferences.h"
IPAddress staticip(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);
String deviceId = "Feeder1";
WebSocketsClient webSocket;
Scheduler taskManager;

Scheduler* getScheduler() {
  return &taskManager;
}
Preferences preferences;


#define USE_SERIAL Serial
File uploadFile;

void writeFile(uint8_t * payload, size_t length) {
  size_t bytesWritten = uploadFile.write(payload, length);
  USE_SERIAL.printf("Write file: %u\n", bytesWritten);
}
void sendBinary(String filename) {
  USE_SERIAL.println((String)"[WSc] sendBinary: " + filename);

  File file = SPIFFS.open(filename, "r");    //open destination file to read

  if (!file) {
    Serial.println("Can't open SPIFFS file !\r\n");
  }
  else {
    char buf[256];
    int siz = file.size();
    USE_SERIAL.printf("File size: %u\n", siz);

    while (siz > 0) {
      yield();
      size_t len = std::min((int)(sizeof(buf)), siz);
      file.read((uint8_t *)buf, len);
      webSocket.sendBIN((uint8_t *)buf, len);
      siz -= len;
    }
    //free(buf);
    file.close();
  }


}
void parseText(uint8_t * payload, size_t length) {
  StaticJsonDocument<500> doc; //Memory pool
  auto error = deserializeJson(doc, payload);
  if (error) {   //Check for errors in parsing
    Serial.println("Parsing failed");
  }
  else {
    const char* key = doc["key"];
    String stringKey = key;
    USE_SERIAL.printf("[WSc] parseText : %s\n", key);

    if (stringKey == "server:text") {
      const char* messageKey = doc["message"]["key"];
      String stringMessageKey = messageKey;
      USE_SERIAL.printf("[WSc] messageKey : %s\n", messageKey);
      if (stringMessageKey == "file:start") {
        const char* filename = doc["message"]["name"];
        String file = "/upload/" + String(filename);
        uploadFile = SPIFFS.open(file, "w");
        USE_SERIAL.printf("[WSc] open : %s\n", filename);
      } else if (stringMessageKey == "file:finish") {
        uploadFile.close();
        USE_SERIAL.printf("[WSc] close file");
      } else if (stringMessageKey == "file:request") {
        const char* filename = doc["message"]["name"];
        sendBinary(filename);
      }
    }
  }
}
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      USE_SERIAL.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED: {
        USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);

        // send message to server when Connected
        webSocket.sendTXT("{\"key\": \"device:subscribe\", \"message\": \"" + deviceId + "\"}");
      }
      break;
    case WStype_TEXT:
      USE_SERIAL.printf("[WSc] get text: %s\n", payload);
      parseText(payload, length);
      // send message to server
      // webSocket.sendTXT("message here");
      break;
    case WStype_BIN:
      //USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
      //hexdump(payload, length);
      writeFile(payload, length);
      // send data to server
      // webSocket.sendBIN(payload, length);
      break;
    case WStype_PING:
      // pong will be send automatically
      USE_SERIAL.printf("[WSc] get ping\n");
      break;
    case WStype_PONG:
      // answer to a ping we send
      USE_SERIAL.printf("[WSc] get pong\n");
      break;
  }
}
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
  setenv("TZ", "UTC-03:30", 0);
  Serial.begin(115200);
  onSetupConfig();
  initialSetup();

  Dir dir = SPIFFS.openDir("/upload");
  while (dir.next()) {
    Serial.println((String)"remove:" + dir.fileName());
    SPIFFS.remove((String)"/upload/" + dir.fileName());
  }

  // server address, port and URL
  webSocket.begin("192.168.1.100", 4200, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);

  // use HTTP Basic Authorization this is optional remove if not needed
  //webSocket.setAuthorization("user", "Password");

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);

  // start heartbeat (optional)
  // ping server every 15000 ms
  // expect pong from server within 3000 ms
  // consider connection disconnected if pong is not received 2 times
  webSocket.enableHeartbeat(15000, 3000, 2);
}

void initialSetup() {
  initNtp();
  initWifiManager();
  initServer();
  connectToWifi();
  turnOnAccessPoint();
  initClickButton();
  Serial.println("initialSetup");
}
void loop()
{
  updateWifiManager();
  updateClickButton();
  updateFeedingLoop();
  updateServerControl();
  taskManager.execute();
  webSocket.loop();


}
