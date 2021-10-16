#include <WebSocketsClient.h>
#include <WebSocketsServer.h>

#define _CHUNK_SIZE 15360 //15*1024
#define _SEND_CHUNK_SIZE 512
#define SOCKET_BASE_URL "192.168.1.50"
WebSocketsClient clientWebSocket;
WebSocketsServer serverWebSocket = WebSocketsServer(4200);

String saveFileName = "";
unsigned int totalFileSize = 0;

void updateSocketHandler() {
  uint8_t mode = getWifiManager()->getMode();
  if (getWifiManager()->isStationMode()) {
    clientWebSocket.loop();
  }
  if (getWifiManager()->isAccessPointMode()) {
    serverWebSocket.loop();
  }
}

void setupSocketHandler() {

}

void callSocketAboutWifiConfigChanged() {
  Serial.println((String)"isStationMode():" + getWifiManager()->isStationMode() + " getStatus:" + getWifiManager()->getStatus());
  if (getWifiManager()->isStationMode() && getWifiManager()->getStatus() == WIFI_STA_STATE_ESTABLISHED) {
    startClientSocket();
  }
  else {
    stopClientSocket();
  }

  if (getWifiManager()->isAccessPointMode() || getWifiManager()->getStatus() == WIFI_STA_STATE_FAILED) {
    startServerSocket();
  }
  else {
    stopServerSocket();
  }


}

void startServerSocket() {
  Serial.println("startServerSocket");

  serverWebSocket.begin();
  serverWebSocket.onEvent(serverWebSocketEvent);
}

void stopServerSocket() {
  Serial.println("stopServerSocket");

  serverWebSocket.disconnect();

}

void serverWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      onUnpairedSignal();
      USE_SERIAL.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = serverWebSocket.remoteIP(num);
        USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        sendText("{\"key\": \"device:time\"}");
      }
      break;
    case WStype_TEXT:
      USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);
      parseText(payload, length);
      // send message to client
      // serverWebSocket.sendTXT(num, "message here");

      // send data to all connected clients
      // serverWebSocket.broadcastTXT("message here");
      break;
    case WStype_BIN:
      USE_SERIAL.printf("[%u] get binary length: %u\n", num, length);
      //hexdump(payload, length);
      writeFile(payload, length);

      // send message to client
      // serverWebSocket.sendBIN(num, payload, length);
      break;
  }

}
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      USE_SERIAL.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED: {
        USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);
        sendText("{\"key\": \"device:time\"}");
        // send message to server when Connected
        sendText("{\"key\": \"device:subscribe\", \"value\": \"" + deviceId + "\"}");
      }
      break;
    case WStype_TEXT:
      //      USE_SERIAL.printf("[WSc] get text: %s\n", payload);
      parseText(payload, length);
      // send message to server
      // sendText("message here");
      break;
    case WStype_BIN:
      USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
      //hexdump(payload, length);
      writeFile(payload, length);
      // send data to server
      // clientWebSocket.sendBIN(payload, length);
      break;
    case WStype_PING:
      // pong will be send automatically
      //      USE_SERIAL.printf("[WSc] get ping\n");
      break;
    case WStype_PONG:
      // answer to a ping we send
      //      USE_SERIAL.printf("[WSc] get pong\n");
      break;
  }
}
void stopClientSocket() {
  Serial.println("stopClientSocket");
  clientWebSocket.disconnect();
  //  delete webSocket;
}

void startClientSocket() {
  Serial.println("startClientSocket");

  //  webSocket = new WebSocketsClient();
  // server address, port and URL
  clientWebSocket.begin(SOCKET_BASE_URL, 4200, "/");

  // event handler
  clientWebSocket.onEvent(webSocketEvent);

  // use HTTP Basic Authorization this is optional remove if not needed
  //clientWebSocket.setAuthorization("user", "Password");

  // try ever 5000 again if connection has failed
  clientWebSocket.setReconnectInterval(5000);

  // start heartbeat (optional)
  // ping server every 15000 ms
  // expect pong from server within 3000 ms
  // consider connection disconnected if pong is not received 2 times
  clientWebSocket.enableHeartbeat(15000, 3000, 2);

}
int totalWrittenBytes = 0;
void sendSliceMessage() {

  if (totalWrittenBytes < totalFileSize) {
    int endSize = std::min(_CHUNK_SIZE, (int) totalFileSize - totalWrittenBytes) + totalWrittenBytes;
    String message = "{\"key\":\"file:send:slice\",\"start\":" + String(totalWrittenBytes) + ",\"end\":" + String(endSize) + "}";
    sendText(message);
  }
  else {
    onFinishSaveFile();
  }
}
void sendFailedSaveMessage() {
  String message = "{\"key\":\"file:send:error\",\"name\":\"" + String(saveFileName) + "\",\"size\":" + String(totalWrittenBytes) + "}";
  sendText(message);
  SPIFFS.remove(saveFileName);
}
void writeFile(uint8_t * &payload, size_t length) {

  File writingFile = SPIFFS.open(saveFileName, "a");

  int bytesWritten = writingFile.write(payload, length);
  if (bytesWritten == 0) {
    sendFailedSaveMessage();
    USE_SERIAL.println((String)"[WSc] ------------------- failed to write : " + bytesWritten);
  }
  else {
    totalWrittenBytes += bytesWritten;
    sendSliceMessage();
  }
  writingFile.close();
}

String sendFileName;

void onRequestFileDetail(String filename) {
  sendFileName = filename;
  File file = SPIFFS.open(sendFileName, "r");    //open destination file to read

  if (!file) {
    Serial.println("Can't open SPIFFS file !\r\n");
    sendText("{\"key\":\"file:request:error\",\"code\":401}");
  }
  else {
    sendText("{\"key\":\"file:detail:callback\",\"buffer\":" + String(_SEND_CHUNK_SIZE) + ",\"size\":" + String(file.size()) + "}");
  }
  file.close();

}
void sendBinary(unsigned int startIndex) {
  USE_SERIAL.println((String)"[WSc] sendBinary: " + sendFileName);
  File file = SPIFFS.open(sendFileName, "r");    //open destination file to read
  if (!file) {
    Serial.println("Can't open SPIFFS file !\r\n");
  }
  else {
    char buf[_SEND_CHUNK_SIZE];
    int siz = file.size();
    USE_SERIAL.printf("File size: %u\n", siz);
    if (siz > 0 && startIndex < siz) {
      unsigned int readLen = std::min((int)(_SEND_CHUNK_SIZE), (int) (siz - startIndex));
      file.seek(startIndex);
      file.read((uint8_t *)buf, readLen);
      clientWebSocket.sendBIN((uint8_t *)buf, readLen);
      serverWebSocket.broadcastBIN((uint8_t *)buf, readLen);
    }
  }

  file.close();
}
void onStartSaveFile() {
  USE_SERIAL.println((String)"[WSc] open :" + saveFileName);
  String file = saveFileName;
  USE_SERIAL.println((String)"[WSc] openfile :" + file);
  totalWrittenBytes = 0;
  SPIFFS.remove(saveFileName);
  sendSliceMessage();
}
void onFinishSaveFile() {
  USE_SERIAL.printf("[WSc] close file");
  String pathTo = saveFileName;
  File writingFile = SPIFFS.open(saveFileName, "r");
  unsigned int fileSize = writingFile.size();
  writingFile.close();
  String fullPath = saveFileName;
  pathTo.replace("/upload", "");
  USE_SERIAL.println((String)"[WSc] onFinishSaveFile :" + fullPath + " fullName:" + pathTo );
  sendText("{\"key\":\"file:send:finished\",\"name\":\"" + pathTo + "\",\"size\":" + String(fileSize) + "}");
  SPIFFS.remove(pathTo);
  SPIFFS.rename(fullPath, pathTo);
}
void onGetTime() {
  String deviceTime = getDeviceTime();
  sendText("{\"key\":\"time:is\",\"value\":" + deviceTime + "}");
}
void sendWifiList() {
  String json = getWifiList();
  sendText("{\"key\":\"wifi:list:is\",\"value\":" + json + "}");
  json = String();
}
void handleServerText(StaticJsonDocument<256> &doc) {
  if (!doc.containsKey("key") ) {
    return;
  }
  const char* messageKey = doc["key"];
  String stringMessageKey = messageKey;
  USE_SERIAL.printf("[WSc] messageKey : %s\n", messageKey);
  if (stringMessageKey == "file:send:start") {
    String filename = doc["name"];
    saveFileName = "/upload/" + filename;
    totalFileSize = doc["size"];
    onStartSaveFile();
  } else if (stringMessageKey == "file:detail:request") {
    const char* filename = doc["name"];
    onRequestFileDetail(filename);
  } else if (stringMessageKey == "file:request:slice") {
    unsigned int startIndex = doc["start"];
    sendBinary( startIndex);
  } else if (stringMessageKey == "file:request:finished") {
    USE_SERIAL.println((String)"[WSc] --------------------- file:send:finished :" + sendFileName );
    // TODO
  } else if (stringMessageKey == "config:reset") {
    onSetupConfig();
  } else if (stringMessageKey == "composit:start") {
    onCompositeFeeding();
  } else if (stringMessageKey == "motor:start") {
    onFeedingEvent();
  } else if (stringMessageKey == "motor:finish") {
    stopFeeding();
  } else if (stringMessageKey == "lamp:start") {
    turnOnLed();
  } else if (stringMessageKey == "lamp:finish") {
    turnOffLed();
  } else if (stringMessageKey == "audio:start") {
    const char* filename = doc["name"];
    playAudio(filename, NULL);
  }  else if (stringMessageKey == "audio:finish") {
    stopAudio();
  } else if (stringMessageKey == "time:get") {
    onGetTime();
  } else if (stringMessageKey == "time:set") {
    long timestamp = doc["value"];
    setDeviceTime(timestamp);
  } else if (stringMessageKey == "wifi:connect") {
    restartWifi();
  } else if (stringMessageKey == "wifi:list:get") {
    sendWifiList();
  } else if (stringMessageKey == "update:start") {
    updateSketchFromFile();
  } else if (stringMessageKey == "pair" ) {
    sendPairedSignal();
  } else if (stringMessageKey == "paired") {
    onPairedSignal();
  } else if (stringMessageKey == "unpaired") {
    onUnpairedSignal();
  } else if (stringMessageKey == "time") {
    unsigned long timestamp = doc["value"];
    setDeviceTime(timestamp);
  }
  doc.clear();
}

void sendText(const char* payload) {
  clientWebSocket.sendTXT(payload);
  serverWebSocket.broadcastTXT(payload);
}
void sendText(String & payload) {
  clientWebSocket.sendTXT(payload);
  serverWebSocket.broadcastTXT(payload);
}
void sendText(const uint8_t * payload) {
  clientWebSocket.sendTXT(payload);
  serverWebSocket.broadcastTXT(payload);
}
void sendText(char * payload) {
  clientWebSocket.sendTXT(payload);
  serverWebSocket.broadcastTXT(payload);
}

void sendPairedSignal() {
  sendText("{\"key\": \"device:paired\"}");
}

void onPairedSignal() {
  USE_SERIAL.println("onPairedSignal");
}

void onUpdateFileNotFound() {
  sendText("{\"key\":\"update:error\",\"code\":404}");
}
void onUpdateError() {
  sendText("{\"key\":\"update:error\",\"code\":500}");
}
void onUpdateFinished() {
  sendText("{\"key\":\"update:finished\"}");
}
void onUpdateStarted() {
  sendText("{\"key\":\"update:started\"}");
}
void onUnpairedSignal() {
  USE_SERIAL.println("onUnpairedSignal");
}

void parseText(uint8_t * payload, size_t length) {
  StaticJsonDocument<256> doc; //Memory pool
  auto error = deserializeJson(doc, payload);
  if (error) {   //Check for errors in parsing
    Serial.println("Parsing failed");
  }
  else {
    const char* key = doc["key"];
    String stringKey = key;
    USE_SERIAL.printf("[WSc] parseText : %s\n", key);
    handleServerText(doc);
  }
  // Clearing Buffer
  doc.clear();

  doc = NULL;
}
