#ifndef SERVER_CONTROL_H
#define SERVER_CONTROL_H
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>   // Include the SPIFFS library
#include <ESPAsyncTCP.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include "ESP8266WiFi.h"
#include <TimeLib.h>

typedef std::function<void(JsonObject &keyValue)> GetEventFunction;

typedef std::function<void(void)> UploadListener;

typedef std::function<void(void)> StopTasksListener;

class ServerControl {
  private:
    GetEventFunction getEventListener = NULL;
    UploadListener uploadListener = NULL;
    StopTasksListener stopTasksListener = NULL;

    AsyncWebServer *server = new AsyncWebServer(80);

    File tempUploadFile;
    String feedingFileName;
    const String feedingFileNameTemp = "feeding.mp3.tmp";

    static String processor(const String & var) {
      Serial.println(var);
      Serial.println((String)"hour() " + hour() + " minute():" + minute() + " day():" + day() + " month():" + month() + " year():" + year());

      if (var == "EPOCH")
        return String(now());
      return String();
    }

    void setup() {
      setupStaticPage();
      setupEventApi();
      setupUploadApi();
      setupWifiApi();
      server->begin();
    }

    void setupStaticPage() {
      server->serveStatic("/", SPIFFS, "/").setDefaultFile("index.html").setTemplateProcessor(processor);

    }

    void setupEventApi() {

      AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/events/", [ = ](AsyncWebServerRequest * request, JsonVariant & json) {
        JsonObject jsonObj = json.as<JsonObject>();
        request->send(200);
        getEventListener(jsonObj);
      });
      server->addHandler(handler);

    }

    void setupWifiApi() {

      server->on("/wifi/list", HTTP_GET, [ = ](AsyncWebServerRequest * request) {
        String json = "[";
        int n = WiFi.scanComplete();
        if (n == -2) {
          WiFi.scanNetworks(true);
        } else if (n) {
          for (int i = 0; i < n; ++i) {
            if (i) json += ",";
            json += "{";
            //            json += "\"rssi\":" + String(WiFi.RSSI(i));
            json += ",\"ssid\":\"" + WiFi.SSID(i) + "\"";
            json += ",\"bssid\":\"" + WiFi.BSSIDstr(i) + "\"";
            //json += ",\"channel\":" + String(WiFi.channel(i));
            json += ",\"secure\":" + String(WiFi.encryptionType(i));
            //json += ",\"hidden\":" + String(WiFi.isHidden(i) ? "true" : "false");
            json += "}";
          }
          WiFi.scanDelete();
          if (WiFi.scanComplete() == -2) {
            WiFi.scanNetworks(true);
          }
        }
        json += "]";
        request->send(200, "application/json", json);
        json = String();
      });

    }
    
    void setupUploadApi() {

      server->on("/upload/", HTTP_POST, [](AsyncWebServerRequest * request) {}, [ = ](AsyncWebServerRequest * request, const String & filename, size_t index, uint8_t *data, size_t len, bool final) {
        // Initialize SPIFFS
        if (!SPIFFS.begin()) {
          Serial.println("An Error has occurred while mounting SPIFFS");
          return;
        }
        handleUpload(request, filename, index, data, len, final);
      });
    }

    void handleUpload(AsyncWebServerRequest * request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
      if (!index) {
        stopTasksListener();
        // open the file on first call and store the file handle in the request object
        //    request->_tempFile = SPIFFS.open(filename, "w");
        SPIFFS.remove(feedingFileNameTemp);
        tempUploadFile = SPIFFS.open(feedingFileNameTemp, "w");
      }

      if (len) {
        tempUploadFile.write(data, len);
      }

      if (final) {
        // close the file handle as the upload is now done
        tempUploadFile.close();
        SPIFFS.remove(feedingFileName);
        SPIFFS.rename(feedingFileNameTemp, feedingFileName);
        request->send(200, "text/plain", "File Uploaded !");
        if (uploadListener != NULL)
          uploadListener();
      }
    }

  public:

    ServerControl(const char *feedingFileName) {
      Serial.begin(115200);
      this->feedingFileName = feedingFileName;
      setup();
    }

    void setEventListener(const GetEventFunction& listener) {
      this->getEventListener = listener;
    }

    void setUploadListener(const UploadListener& listener) {
      this->uploadListener = listener;
    }
    void setStopTasksListener(const StopTasksListener& listener) {
      this->stopTasksListener = listener;
    }
};
#endif
