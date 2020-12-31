#pragma once
#ifndef ARDUINO_PREFERENCES_H // change depending on your class
#define ARDUINO_PREFERENCES_H // change depending on your class
#include <TaskManager.h>
#include <FS.h>   // Include the SPIFFS library
#include <ArduinoJson.h>
#define defaultFeedingInterval 18000
#define defaultFeedingDuration 3000
#define defaultSoundVolume 3.99
#define ledStateAlwaysOff 0
#define ledStateAlwaysOn 1
#define ledStateFeedingOn 2
#define defaultLedState ledStateFeedingOn

struct Config {
  float soundVolume;
  int feedingInterval;
  int feedingDuration;
  int ledState;
  char wifiSsid[32];
  char wifiPassword[64];
  int alarms[10];
};

class Preferences
{
  public:
    Config *config = new Config();

    Preferences() {
      loadConfiguration(filename, *config);
    }

    Config& getConfig()
    {
      return *config;
    }
    void save()
    {
      saveConfiguration(filename, *config);
    }
  private:
    const char *filename = "/config.json";

    void loadConfiguration(const char *filename, Config &config) {

      bool success = SPIFFS.begin();

      if (success) {
        Serial.println("File system mounted with success");
      } else {
        Serial.println("Error mounting the file system");
        return;
      }

      File file = SPIFFS.open(filename, "r");

      if (!file) {
        Serial.println("Error opening file for reading");
        return;
      }


      // Allocate a temporary JsonDocument
      // Don't forget to change the capacity to match your requirements.
      // Use arduinojson.org/v6/assistant to compute the capacity.
      StaticJsonDocument<512> doc;

      // Deserialize the JSON document
      DeserializationError error = deserializeJson(doc, file);
      if (error)
        Serial.println(F("Failed to read file, using default configuration"));

      // Copy values from the JsonDocument to the Config
      config.feedingInterval = doc["feedingInterval"] | defaultFeedingInterval;
      config.feedingDuration = doc["feedingDuration"] | defaultFeedingDuration;
      config.ledState = doc["ledState"] | defaultLedState;
      config.soundVolume = doc["soundVolume"] | defaultSoundVolume;
      strlcpy(config.wifiSsid, doc["wifiSsid"], sizeof(config.wifiSsid));
      strlcpy(config.wifiPassword, doc["wifiPassword"], sizeof(config.wifiPassword));

      config.alarms[0] = doc["alarms"][0] | 28800; // 08:00
      config.alarms[1] = doc["alarms"][1] | 50400; // 14:00
      config.alarms[2] = doc["alarms"][2] | 79200; // 22:00

      // Close the file (Curiously, File's destructor doesn't close the file)
      file.close();
    }
    void saveConfiguration(const char *filename, Config &config) {
      // Delete existing file, otherwise the configuration is appended to the file
      SPIFFS.remove(filename);

      // Open file for writing
      File file = SPIFFS.open(filename, "w");
      if (!file) {
        Serial.println(F("Failed to create file"));
        return;
      }

      // Allocate a temporary JsonDocument
      // Don't forget to change the capacity to match your requirements.
      // Use arduinojson.org/assistant to compute the capacity.
      StaticJsonDocument<512> doc;

      // Set the values in the document
      doc["feedingInterval"] = config.feedingInterval;
      doc["feedingDuration"] = config.feedingDuration;
      doc["ledState"] = config.ledState;
      doc["soundVolume"] = config.soundVolume;

      doc["wifiSsid"] = config.wifiSsid;
      doc["wifiPassword"] = config.wifiPassword;

      doc["alarms"][0] = config.alarms[0];
      doc["alarms"][1] = config.alarms[1];
      doc["alarms"][2] = config.alarms[2];

      // Serialize JSON to file
      if (serializeJson(doc, file) == 0) {
        Serial.println(F("Failed to write to file"));
      }

      // Close the file
      file.close();
    }

};

#endif // ARDUINO_PREFERENCES_H
