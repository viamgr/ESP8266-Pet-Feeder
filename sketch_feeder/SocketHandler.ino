#include <WebSocketsClient.h>

#define _CHUNK_SIZE 15360 //15*1024
WebSocketsClient* webSocket = NULL;
File* tempFile = NULL;

void updateSocketHandler() {
  uint8_t mode = getWifiManager()->getMode();
  if (mode == WIFI_MODE_AP_STA || mode == WIFI_MODE_STA) {
    webSocket->loop();
  }
}


void callSocketAboutWifiConfigChanged() {
  uint8_t mode = getWifiManager()->getMode();
  if (mode == WIFI_MODE_AP_STA || mode == WIFI_MODE_STA) {
    startSocket();
  }
  else {
    stopSocket();
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
        webSocket->sendTXT("{\"key\": \"device:subscribe\", \"message\": \"" + deviceId + "\"}");
      }
      break;
    case WStype_TEXT:
      parseText(payload, length);
      USE_SERIAL.printf("[WSc] get text: %s\n", payload);
      // send message to server
      // webSocket->sendTXT("message here");
      break;
    case WStype_BIN:
      USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
      //hexdump(payload, length);
      writeFile(payload, length);
      // send data to server
      // webSocket->sendBIN(payload, length);
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
void stopSocket() {
  webSocket->disconnect();
  delete webSocket;
}

void startSocket() {
  webSocket = new WebSocketsClient();
  // server address, port and URL
  webSocket->begin("192.168.1.100", 4200, "/");

  // event handler
  webSocket->onEvent(webSocketEvent);

  // use HTTP Basic Authorization this is optional remove if not needed
  //webSocket->setAuthorization("user", "Password");

  // try ever 5000 again if connection has failed
  webSocket->setReconnectInterval(5000);

  // start heartbeat (optional)
  // ping server every 15000 ms
  // expect pong from server within 3000 ms
  // consider connection disconnected if pong is not received 2 times
  webSocket->enableHeartbeat(15000, 3000, 2);

}
void setupSocketHandler() {

}
size_t sentBytes = 0;
void sendSliceMessage() {
  String message = "{\"key\": \"device:text\", \"message\": {\"key\":\"file:request:slice\",\"start\":" + String(sentBytes) + ",\"end\":" + String(_CHUNK_SIZE) + "}}";
  webSocket->sendTXT(message);
}
void writeFile(uint8_t * &payload, size_t length) {
  size_t bytesWritten = tempFile->write(payload, length);
  USE_SERIAL.println((String)"[WSc] writeFile :" + tempFile->position());
  sentBytes += tempFile->position();
  sendSliceMessage(sendBytes);
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
    webSocket->sendTXT("{\"key\": \"device:text\", \"message\": {\"key\":\"file:start\",\"name\":" + filename + ",\"length\":" + siz + "}}");
    USE_SERIAL.printf("File size: %u\n", siz);

    while (siz > 0 && webSocket->isConnected()) {
      yield();
      size_t len = std::min((int)(sizeof(buf)), siz);
      file.read((uint8_t *)buf, len);
      webSocket->sendBIN((uint8_t *)buf, len);
      siz -= len;
    }

    webSocket->sendTXT("{\"key\": \"device:text\", \"message\": {\"key\":\"file:finish\",\"name\":" + filename + ",\"remain\":" + siz + "}}");

    //free(buf);
    file.close();
  }
}
void onStartSaveFile(StaticJsonDocument<500> &doc) {
  const char* filename = doc["message"]["name"];
  String file = "/upload/" + String(filename);
  tempFile = new File(SPIFFS.open(file, "w"));
  USE_SERIAL.println((String)"[WSc] open :" + tempFile->name());
  sentBytes = 0;
  sendSliceMessage(sentBytes);
}
void onFinishSaveFile() {
  USE_SERIAL.printf("[WSc] close file");
  String pathTo = tempFile->name();
  pathTo.replace("/upload", "");
  USE_SERIAL.println((String)"[WSc] onFinishSaveFile :" + tempFile->name() + " fullName:" + pathTo );
  SPIFFS.remove(tempFile->name());
  SPIFFS.rename(tempFile->name(), pathTo);
  tempFile->close();
  tempFile = NULL;
  delete tempFile;
  webSocket->sendTXT("{\"key\": \"device:text\", \"message\": {\"key\":\"file:request:finish\",\"name\":\"" + pathTo + "\",\"size\":" + String(sentBytes) + "}}");
}
void onGetTime() {
  String deviceTime = getDeviceTime();
  webSocket->sendTXT("{\"key\": \"device:text\", \"message\": {\"key\":\"time:is\",\"value\":" + deviceTime + "}}");
}
void sendWifiList() {
  String json = getWifiList();
  webSocket->sendTXT("{\"key\": \"device:text\", \"message\": {\"key\":\"wifi:list:is\",\"value\":" + json + "}}");
  json = String();
}
void handleServerText(StaticJsonDocument<500> &doc) {
  if (!doc.containsKey("message") || !doc["message"].containsKey("key") ) {
    return;
  }
  const char* messageKey = doc["message"]["key"];
  String stringMessageKey = messageKey;
  USE_SERIAL.printf("[WSc] messageKey : %s\n", messageKey);
  if (stringMessageKey == "file:send:start") {
    onStartSaveFile(doc);
  } if (stringMessageKey == "file:send:finish") {
    onFinishSaveFile();
  } else if (stringMessageKey == "file:request") {
    const char* filename = doc["message"]["name"];
    sendBinary(filename);
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
    const char* filename = doc["message"]["name"];
    playAudio(filename, NULL);
  }  else if (stringMessageKey == "audio:finish") {
    stopAudio();
  } else if (stringMessageKey == "time:get") {
    onGetTime();
  } else if (stringMessageKey == "time:set") {
    long timestamp = doc["message"]["value"];
    setDeviceTime(timestamp);
  } else if (stringMessageKey == "wifi:connect") {
    restartWifi();
  } else if (stringMessageKey == "wifi:list:witch") {
    sendWifiList();
  }
  doc.clear();
}
void sendPairedSignal() {
  webSocket->sendTXT("{\"key\": \"device:paired\"}");
}

void onPairedSignal() {
  USE_SERIAL.println("onPairedSignal");
}

void onUnpairedSignal() {
  USE_SERIAL.println("onUnpairedSignal");
}

void parseText(uint8_t * &payload, size_t length) {
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
      handleServerText(doc);
    } else if (stringKey == "server:pair") {
      sendPairedSignal();
    } else if (stringKey == "server:paired") {
      onPairedSignal();
    } else if (stringKey == "server:unpaired") {
      onUnpairedSignal();
    }
  }
  // Clearing Buffer
  doc.clear();

  doc = NULL;
}
