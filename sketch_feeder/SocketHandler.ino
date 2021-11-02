#include <WebSocketsClient.h>
#include <WebSocketsServer.h>

#define _CHUNK_SIZE 15360 //15*1024
#define _SEND_CHUNK_SIZE 512
#define SOCKET_BASE_URL "193.108.115.160"
WebSocketsClient clientWebSocket;
WebSocketsServer serverWebSocket = WebSocketsServer(4200);

String saveFileName = "";
unsigned int totalFileSize = 0;

void updateSocketHandler() {
  uint8_t mode = getWifiManager()->getMode();
//  if (getWifiManager()->isStationMode()) {
    clientWebSocket.loop();
//  }
//  if (getWifiManager()->isAccessPointMode()) {
    serverWebSocket.loop();
//  }
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

  if (getWifiManager()->isAccessPointMode() || getWifiManager()->isStationMode() || getWifiManager()->getStatus() == WIFI_STA_STATE_FAILED) {
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
        sendText((String)"{\"key\": \""+TIME_GET+"\"}");
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
        sendText((String)"{\"key\": \""+TIME_GET+"\"}");
        // send message to server when Connected
        sendText((String)"{\"key\": \""+SUBSCRIBE+"\", \"value\": \"" + deviceId + "\"}");
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
    Serial.println("***************************************#&@(*#&*(@&#(*@&(*#&( sendSliceMessage ");
    String message = (String)"{\"key\":\""+FILE_SEND_SLICE+"\",\"start\":" + String(totalWrittenBytes) + ",\"end\":" + String(endSize) + "}";
    sendText(message);
  }
  else {
    onFinishSaveFile();
  }
}
void sendFailedSaveMessage() {
  String message = (String)"{\"key\":\""+FILE_SEND_ERROR+"\",\"name\":\"" + String(saveFileName) + "\",\"size\":" + String(totalWrittenBytes) + "}";
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
    sendText((String)"{\"key\":\""+FILE_REQUEST_ERROR+"\",\"code\":401}");
  }
  else {
    sendText((String)"{\"key\":\""+FILE_DETAIL_CALLBACK+"\",\"buffer\":" + String(_SEND_CHUNK_SIZE) + ",\"size\":" + String(file.size()) + "}");
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
  sendText((String)"{\"key\":\""+FILE_SEND_FINISHED+"\",\"name\":\"" + pathTo + "\",\"size\":" + String(fileSize) + "}");
  SPIFFS.remove(pathTo);
  SPIFFS.rename(fullPath, pathTo);
}
void onGetTime() {
  String deviceTime = getDeviceTime();
  sendText((String)"{\"key\":\""+TIME_IS+"\",\"value\":" + deviceTime + "}");
}
void sendWifiList() {
  String json = getWifiList();
  sendText((String)"{\"key\":\""+WIFI_LIST_IS+"\",\"value\":" + json + "}");
  json = String();
}
void handleServerText(StaticJsonDocument<256> &doc) {
  if (!doc.containsKey("key") ) {
    return;
  }
  const char* messageKey = doc["key"];
  String stringMessageKey = messageKey;
  USE_SERIAL.printf("[WSc] messageKey : %s\n", messageKey);
  if (stringMessageKey == FILE_SEND_START) {
    String filename = doc["name"];
    saveFileName = "/upload/" + filename;
    totalFileSize = doc["size"];
    onStartSaveFile();
  } else if (stringMessageKey == FILE_DETAIL_REQUEST) {
    const char* filename = doc["name"];
    onRequestFileDetail(filename);
  } else if (stringMessageKey == FILE_REQUEST_SLICE) {
    unsigned int startIndex = doc["start"];
    sendBinary( startIndex);
  } else if (stringMessageKey == FILE_REQUEST_FINISHED) {
    USE_SERIAL.println((String)"[WSc] --------------------- file:send:finished :" + sendFileName );
    // TODO
  } else if (stringMessageKey == CONFIG_RESET) {
    onSetupConfig();
  } else if (stringMessageKey == COMPOSITE_START) {
    onCompositeFeeding();
  } else if (stringMessageKey == MOTOR_START) {
    onFeedingEvent();
  } else if (stringMessageKey == MOTOR_FINISH) {
    stopFeeding();
  } else if (stringMessageKey == LAMP_START) {
    turnOnLed();
  } else if (stringMessageKey == LAMP_FINISH) {
    turnOffLed();
  } else if (stringMessageKey == AUDIO_START) {
    const char* filename = doc["name"];
    playAudio(filename, NULL);
  }  else if (stringMessageKey == AUDIO_FINISH) {
    stopAudio();
  } else if (stringMessageKey == TIME_GET) {
    onGetTime();
  } else if (stringMessageKey == TIME_SET) {
    long timestamp = doc["value"];
    setDeviceTime(timestamp);
  } else if (stringMessageKey == WIFI_CONNECT) {
    restartWifi();
  } else if (stringMessageKey == WIFI_LIST_GET) {
    sendWifiList();
  } else if (stringMessageKey == UPDATE_START) {
    updateSketchFromFile();
  } else if (stringMessageKey == PAIR ) {
    sendPairedSignal();
  } else if (stringMessageKey == SUBSCRIBE ) {
    onSubscribeEvent();
  }  else if (stringMessageKey == SUBSCRIBE_DONE ) {
    onSubscribed();
  } else if (stringMessageKey == PAIR_DONE) {
    onPairedSignal();
  } else if (stringMessageKey == UNPAIR) {
    onUnpairedSignal();
  } else if (stringMessageKey == TIME_SET) {
    unsigned long timestamp = doc["value"];
    setDeviceTime(timestamp);
  }
  doc.clear();
}
void onSubscribeEvent(){
    sendText((String)"{\"key\": \""+SUBSCRIBE_DONE+"\"}");
}
void onSubscribed(){
  USE_SERIAL.println("onSubscribed");

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
  sendText((String)"{\"key\": \""+PAIR_DONE+"\"}");
}

void onPairedSignal() {
  USE_SERIAL.println("onPairedSignal");
}

void onUpdateFileNotFound() {
  sendText((String)(String)"{\"key\":\""+UPDATE_ERROR+"\",\"code\":404}");
}
void onUpdateError() {
  sendText((String)"{\"key\":\""+UPDATE_ERROR+"\",\"code\":500}");
}
void onUpdateFinished() {
  sendText((String)"{\"key\":\""+UPDATE_FINISHED+"\"}");
}
void onUpdateStarted() {
  sendText((String)(String)"{\"key\":\""+UPDATE_STARTED+"\"}");
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
