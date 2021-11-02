#pragma once
#ifndef ARDUINO_PREFERENCES_H // change depending on your class
#define ARDUINO_PREFERENCES_H // change depending on your class
#include <FS.h>   // Include the SPIFFS library
#include <ArduinoJson.h>
#define defaultFeedingDuration 5000
#define defaultSoundVolume 3.99
#define ledStateAlwaysOff 0
#define ledStateAlwaysOn 1
#define ledStateFeedingOn 2
#define DEFAULT_USE_DHCP 1
#define defaultLedTurnOffDelay 5000
#define defaultLedState ledStateFeedingOn
#define DEFAULT_WIFI_MODE WIFI_MODE_AP
#define MAX_ALARM_SIZE 10
#define CONFIG_FILE_PATH "/config.json"
#define DEFAULT_STATIC_IP "192.168.8.150"
#define DEFAULT_GATEWAY "192.168.8.1"
#define DEFAULT_SUBNET "255.255.255.0"
#define DHCP_ENABLED 1

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

    char* getStaticIp() {
      const char* staticIp = doc["staticIp"];
      return (char*)staticIp;
    }

    char* getGateway() {
      const char* gateway = doc["gateway"];
      return (char*)gateway;
    }

    char* getSubnet() {
      const char* subnet = doc["subnet"];
      return (char*)subnet;
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

    uint8_t getWifiMode() {
      return doc["wifiMode"] | DEFAULT_WIFI_MODE;
    }
    uint8_t useDhcp() {
      return doc["useDhcp"] | DEFAULT_USE_DHCP;
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

    void setWifiMode(uint8_t value) {
      doc["wifiMode"] = value;
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
