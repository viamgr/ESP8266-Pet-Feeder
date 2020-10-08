/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com/esp8266-nodemcu-access-point-ap-web-server/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFi.h>
#include <FS.h>   // Include the SPIFFS library

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "AudioFileSourceSPIFFS.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"

#include "AsyncJson.h"
#include "ArduinoJson.h"
#include "MotorControl.h"

const char* ssid = "Feeder-Access-Point";
struct ip4_addr *IPaddress;
uint32 uintaddress;

AsyncWebServer server(80);
bool motorState = false;
AudioGeneratorMP3 * mp3;
AudioFileSourceSPIFFS *file;
AudioOutputI2SNoDAC *out;
AudioFileSourceID3 *id3;

#define LED_PIN D6
#define BOTTON_PIN D2
#define MOTOR_PIN D5

MotorControl motor(MOTOR_PIN);
bool startMotorAfterEat = false;
unsigned long feedTime = 3000;
unsigned long intervalTime = 3600000;
unsigned long lastFeed = millis();
void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  // Initialize SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  WiFi.softAP(ssid);
  Serial.println(WiFi.localIP());

  pinMode(LED_PIN, OUTPUT);
  pinMode(BOTTON_PIN, INPUT);

  enableServer();
}


int lastButtonState = 0;         // variable for reading the pushbutton status
bool ledState = false;
void loop() {

  if (mp3 && mp3->isRunning()) {
    if (!mp3->loop()) {
      digitalWrite(LED_PIN, LOW);
      ledState = false;
      mp3->stop();
    }
    else {
      if (ledState == false)
      {
        digitalWrite(LED_PIN, HIGH);
        ledState = true;
      }
    }
  }
  else {
    showConnectedDevices  ();
    int buttonState = digitalRead(BOTTON_PIN);
    Serial.println(lastButtonState);

    if (buttonState != lastButtonState && buttonState == HIGH) {
      buttonPressed();  
    }
    else if (millis() - lastFeed > intervalTime) {
      lastFeed = millis();
      playEat();
    }
    else if(startMotorAfterEat) {
      
        startMotorAfterEat = false;
        Serial.println("startMotorAfterEat");
        motor.start(feedTime);
     
    }
      lastButtonState = buttonState;


    if (motor.isRotating()) {
      if (!motor.loop()) {
        digitalWrite(LED_PIN, LOW);
        ledState = false;
        motor.stop();
      }
      else {
        if (ledState == false)
        {
          digitalWrite(LED_PIN, HIGH);
          ledState = true;
        }
      }

    }

    delay(50);
  }

}
void buttonPressed() {
  playEat();
}

const unsigned MAX_DEVICES = 5;

String devices[MAX_DEVICES] = { "", "", "", "", "" };


void showConnectedDevices()
{
  auto client_count = wifi_softap_get_station_num();
  //  Serial.println("");
  //  Serial.printf("Total devices connected = %d\n", client_count);

  auto i = 0;
  struct station_info *station_list = wifi_softap_get_station_info();
  while (station_list != NULL) {
    auto station_ip = IPAddress((&station_list->ip)->addr).toString().c_str();
    char station_mac[18] = {0};
    sprintf(station_mac, "%02X:%02X:%02X:%02X:%02X:%02X", MAC2STR(station_list->bssid));
    if (!isConnectedAlready(station_mac))
    {
      deviceAdded();
    }
    devices[i] = station_mac;
    station_list = STAILQ_NEXT(station_list, next);
    i++;
    //    Serial.printf("%d. %s %s", i++, station_ip, station_mac);
    //    Serial.println("");
  }

  for (int x = i; x < MAX_DEVICES; x++)  {
    devices[x] = "";
  }


  wifi_softap_free_station_info();
  //  Serial.printf("devices: %d" , i);

}

bool isConnectedAlready(String mac) {
  for (int x = 0; x < MAX_DEVICES; x++)  {
    if (mac == devices[x]) return true;
  }
  return false;
}

void deviceAdded() {
  Serial.println("device Added.");
  playWelcome();

}


// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  (void)cbData;
  Serial.printf("ID3 callback for: %s = '", type);

  if (isUnicode) {
    string += 2;
  }

  while (*string) {
    char a = *(string++);
    if (isUnicode) {
      string++;
    }
    Serial.printf("%c", a);
  }
  Serial.printf("'\n");
  Serial.flush();
}


void playWelcome() {
  print("playWelcome");
  if (mp3) {
    if (mp3->loop()) mp3->stop();
  }
  audioLogger = &Serial;
  file = new AudioFileSourceSPIFFS("/upload/connected.mp3");
  id3 = new AudioFileSourceID3(file);
  id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
  out = new AudioOutputI2SNoDAC();
  mp3 = new AudioGeneratorMP3();
  mp3->begin(id3, out);

}

void playEat() {

  print("playEat");
  startMotorAfterEat = true;
  if (mp3) {
    if (mp3->loop()) mp3->stop();
  }
  audioLogger = &Serial;
  file = new AudioFileSourceSPIFFS("/upload/eat.mp3");
  id3 = new AudioFileSourceID3(file);
  id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
  out = new AudioOutputI2SNoDAC();
  mp3 = new AudioGeneratorMP3();
  mp3->begin(id3, out);
}

void print(String message) {
  // Print ESP8266 Local IP Address
  Serial.println(message);
}


// Replaces placeholder with LED state value
String processor(const String & var) {
  Serial.println(var);
  return String();
}

void enableServer() {
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/upload/", HTTP_POST, [](AsyncWebServerRequest * request) {}, [](AsyncWebServerRequest * request, const String & filename, size_t index, uint8_t *data, size_t len, bool final) {
    handleUpload(request, filename, index, data, len, final);
  });

  server.on("/base/status/", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["heap"] = ESP.getFreeHeap();
    root["ssid"] = ssid;
    root.printTo(*response);
    request->send(response);
  });

  server.on("/motor/status/", HTTP_GET, [](AsyncWebServerRequest * request) {
    responseMotor(request);
  });

  AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/motor/status/", [](AsyncWebServerRequest * request, JsonVariant & json) {
    JsonObject& jsonObj = json.as<JsonObject>();
    motorState = jsonObj["enabled"];
    responseMotor(request);
    if (motorState == true) {
      playEat();
    }
    else {
      motor.stop();
    }

  });

  AsyncCallbackJsonWebHandler* interval = new AsyncCallbackJsonWebHandler("/motor/interval/", [](AsyncWebServerRequest * request, JsonVariant & json) {
    JsonObject& jsonObj = json.as<JsonObject>();
    intervalTime = jsonObj["time"];
    responseMotor(request);
  });
  server.addHandler(interval);
  server.addHandler(handler);
  // Start server
  server.begin();
}

void responseMotor(AsyncWebServerRequest * request) {
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["enabled"] = motorState;
  root.printTo(*response);
  request->send(response);
}

File uploadFile;

void handleUpload(AsyncWebServerRequest * request, String filename, size_t index, uint8_t *data, size_t len, bool final) {

  if (!index) {
    Serial.print((String)"UploadStart: " + filename);
    // open the file on first call and store the file handle in the request object
    //    request->_tempFile = SPIFFS.open(filename, "w");
    String path = (String)"/upload/eat.mp3";
    uploadFile = SPIFFS.open(path, "w");
    Serial.println((String)"file: " + uploadFile.size());
  }

  if (len) {
    // stream the incoming chunk to the opened file
    uploadFile.write(data, len);
  }
  if (final) {

    Serial.print((String)"UploadEnd: " + filename + "," + index + len + " , " + uploadFile.size());

    // close the file handle as the upload is now done
    uploadFile.close();
    playWelcome();
    request->send(200, "text/plain", "File Uploaded !");
  }
}
