#ifndef SERVER_CONTROL_H
#define SERVER_CONTROL_H
#include <Arduino.h>
#include <FS.h>   // Include the SPIFFS library
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <ESP8266WebServer.h>

typedef std::function<void(String name)> GetEventFunction;
typedef std::function<String(String key)> GetStatusFunction;
typedef std::function<void(String key, String value)> SetStatusFunction;
typedef std::function<void(String filename)> UploadListener;
typedef std::function<void(void)> StopTasksListener;

static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char APPLICATION_JSON[] PROGMEM = "application/json";
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";

#define DBG_OUTPUT_PORT Serial

class ServerControl {
  private:
    bool fsOK;
    GetEventFunction getEventListener = NULL;
    SetStatusFunction onSetStatusListener = NULL;
    GetStatusFunction onGetStatusListener = NULL;
    UploadListener uploadListener = NULL;
    StopTasksListener stopTasksListener = NULL;
    ESP8266WebServer *server;
    File uploadFile;

    static String processor(const String & var) {
      Serial.println(var);
      Serial.println((String)"hour() " + hour() + " minute():" + minute() + " day():" + day() + " month():" + month() + " year():" + year());

      if (var == "EPOCH")
        return String(now());
      return String();
    }

    void setup() {
      fsOK = SPIFFS.begin();
      setupEventApi();
      setupUploadApi();
      setupWifiApi();
      setupStaticPage();
      server->begin();
      server->onNotFound([this]() {
        server->sendHeader("Location", "http://192.168.4.1/", true); //Redirect to our html web page
        server->send(302, "text/plane", "");
      }); 

    }

    void setupStaticPage() {
      server->serveStatic("/", SPIFFS, "/");

      //      server->serveStatic("/upload/", SPIFFS, "/upload/");

    }

    void setupEventApi() {

      server->on("/events",  HTTP_GET, [&]() {
        String name = server->arg("name");
        replyOK();
        getEventListener(name);
      });
      server->on("/setStatus",  HTTP_GET, [&]() {
        String key = server->arg("key");
        String value = server->arg("value");
        replyOK();
        onSetStatusListener(key, value);
      });
      server->on("/getStatus",  HTTP_GET, [&]() {
        String key = server->arg("key");
        replyOKWithMsg(onGetStatusListener(key));
      });
    }

    void setupWifiApi() {

      server->on("/wifi/list", HTTP_GET, [ = ]() {
        String json = "[";
        int n = WiFi.scanComplete();
        if (n == -2) {
          WiFi.scanNetworks(true);
        } else if (n) {
          for (int i = 0; i < n; ++i) {
            if (i != 0) json += ",";
            json += "{";
            //            json += "\"rssi\":" + String(WiFi.RSSI(i));
            json += "\"ssid\":\"" + WiFi.SSID(i) + "\"";
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
        server->send(200,  FPSTR(APPLICATION_JSON), json);
        json = String();
      });

    }
    void replyOK() {
      server->send(200, FPSTR(TEXT_PLAIN), "");
    }
    void replyOKWithMsg(String msg) {
      server->send(200, FPSTR(TEXT_PLAIN), msg);
    }


    void replyServerError(String msg) {
      DBG_OUTPUT_PORT.println(msg);
      server->send(500);
    }

    /*
       Handle a file upload request
    */
    void handleFileUpload() {
      if (!fsOK) {
        return replyServerError(FPSTR(FS_INIT_ERROR));
      }
      if (server->uri() != "/upload") {
        return;
      }
      yield();
      HTTPUpload& upload = server->upload();
      if (upload.status == UPLOAD_FILE_START) {
        stopTasksListener();
        String filename = upload.filename;
        // Make sure paths always start with "/"
        if (!filename.startsWith("/")) {
          filename = "/" + filename;
        }
        DBG_OUTPUT_PORT.println(String("handleFileUpload Name: ") + filename);
        uploadFile = SPIFFS.open(filename, "w");
        if (!uploadFile) {
          return replyServerError(F("CREATE FAILED"));
        }
        DBG_OUTPUT_PORT.println(String("Upload: START, filename: ") + filename);
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (uploadFile) {
          size_t bytesWritten = uploadFile.write(upload.buf, upload.currentSize);
          if (bytesWritten != upload.currentSize) {
            return replyServerError(F("WRITE FAILED"));
          }
        }
        DBG_OUTPUT_PORT.println(String("Upload: WRITE, Bytes: ") + upload.currentSize);
      } else if (upload.status == UPLOAD_FILE_END) {
        DBG_OUTPUT_PORT.println(String("Upload: END, Size: ") + upload.totalSize);
        if (uploadFile) {
          uploadFile.close();

          String filename = upload.filename;
          // Make sure paths always start with "/"
          if (!filename.startsWith("/")) {
            filename = "/" + filename;
          }
          uploadListener(filename);
        }
      }
    }

    void setupUploadApi() {
      server->on("/upload",  HTTP_POST, [&]() {
        replyOK();
      }, [&]() {
        handleFileUpload();
      });
    }

  public:

    ServerControl() {
      Serial.begin(115200);
      this->server = new ESP8266WebServer(80);
      setup();
    }

    void setEventListener(const GetEventFunction& listener) {
      this->getEventListener = listener;
    }
    void setOnGetStatusListener(const GetStatusFunction& listener) {
      this->onGetStatusListener = listener;
    }
    void setOnSetStatusListener(const SetStatusFunction& listener) {
      this->onSetStatusListener = listener;
    }

    void setUploadListener(const UploadListener& listener) {
      this->uploadListener = listener;
    }
    void setStopTasksListener(const StopTasksListener& listener) {
      this->stopTasksListener = listener;
    }

    void loop() {

      server->handleClient();

    }
};
#endif
