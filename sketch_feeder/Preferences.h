#pragma once
#ifndef ARDUINO_PREFERENCES_H // change depending on your class
#define ARDUINO_PREFERENCES_H // change depending on your class
#include <TaskManager.h>
#include <FS.h>   // Include the SPIFFS library
#include <ArduinoJson.h>
#define defaultFeedingDuration 5000
#define defaultSoundVolume 3.99
#define ledStateAlwaysOff 0
#define ledStateAlwaysOn 1
#define ledStateFeedingOn 2
#define defaultLedTurnOffDelay 5000
#define defaultLedState ledStateFeedingOn
#define MAX_ALARM_SIZE 10
#define CONFIG_FILE_PATH "/config.json"

class Preferences
{
  public:
    StaticJsonDocument<512> doc;

    Preferences() {
      Serial.begin(115200);
      reload();
    }

    JsonArray getAlarms() {
      return doc["alarms"];
    }

    char* getWifiSsid() {
      const char* ssid = doc["wifiSsid"];
      return (char*)ssid;
    }
    char* getWifiPassword() {
      const char* password = doc["wifiPassword"];
      return (char*)password;
    }

    int getLedTurnOffDelay() {
      return doc["ledTurnOffDelay"] | defaultLedTurnOffDelay;
    }

    int getFeedingDuration() {
      return doc["feedingDuration"] | defaultFeedingDuration;
    }
    int getSoundVolume() {
      return doc["soundVolume"] | defaultSoundVolume;
    }
    int getLedState() {
      return doc["ledState"] | defaultLedState;
    }

    char* getAccessPointName() {
      const char* ssid = doc["accessPointName"] | "Feeder-Access-Point";
      return (char*)ssid;
    }

    boolean isAccessPointOn() {
      return doc["isAccessPointOn"] | true;
    }

    boolean isStationOn() {
      return doc["isStationOn"] | false;
    }


    void setLedTurnOffDelay(int value) {
      doc["ledTurnOffDelay"] = value;
    }
    void setWifiSsid(char* value) {
      doc["wifiSsid"] = value;
    }
    void setWifiPassword(char* value) {
      doc["wifiPassword"] = value;
    }

    void setFeedingDuration(int value) {
      doc["feedingDuration"] = value;
    }
    void setSoundVolume(int value) {
      doc["soundVolume"] = value;
    }
    void setLedState(int value) {
      doc["ledState"] = value;
    }

    void setAlarms(JsonArray data) {
      doc["alarms"] = data;
      //  JsonArray data = doc.createNestedArray("alarms");
      //for (int x = 0; alarms[x] != '\0'; x++)
      // {
      //   data.add(alarms[x]);
      // }
    }


    void setAccessPointName(char* value) {
      doc["setAccessPointName"] = value;
    }
    
    void setAccessPointOn(boolean value) {
      doc["isAccessPointOn"] = value;
    }
    
    void setStationOn(boolean value) {
      doc["isStationOn"] = value;
    }    
    
    void save() {
      // Delete existing file, otherwise the configuration is appended to the file
      SPIFFS.remove(CONFIG_FILE_PATH);

      // Open file for writing
      File file = SPIFFS.open(CONFIG_FILE_PATH, "w");
      if (!file) {
        Serial.println(F("Failed to create file"));
        return;
      }
      // Serialize JSON to file
      if (serializeJson(doc, file) == 0) {
        Serial.println(F("Failed to write to file"));
      }

      // Close the file
      file.close();
    }

    void reload() {

      bool success = SPIFFS.begin();

      if (success) {
        Serial.println(F("File system mounted with success"));
      } else {
        Serial.println(F("Error mounting the file system"));
        return;
      }

      File file = SPIFFS.open(CONFIG_FILE_PATH, "r");

      if (!file) {
        Serial.println(F("Error opening file for reading"));
        Serial.println(CONFIG_FILE_PATH);
        return;
      }

      // Deserialize the JSON document
      DeserializationError error = deserializeJson(doc, file);

      if (error)
        Serial.println(F("Failed to read file, using default configuration"));

      file.close();
    }

  private:


};

#endif // ARDUINO_PREFERENCES_H
