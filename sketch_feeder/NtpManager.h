#ifndef NTP_MANAGER_H
#define NTP_MANAGER_H
#include <Arduino.h>
#include <TaskManager.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#define NTP_MANAGER_TASK "NTP_MANAGER_TASK"
#define NTP_MANAGER_TASK_LONG "NTP_MANAGER_TASK_LONG"
typedef std::function<void(unsigned long epochTime)> OnTimeUpdateListener;

class NtpManager {
  private:
    OnTimeUpdateListener onTimeUpdateListener = NULL;
    WiFiUDP udp;
    NTPClient *timeClient = new NTPClient(udp, "pool.ntp.org");
    const int interval = 60 * 60 * 1000;
    void update() {
      Serial.println("getting time");
      bool updated = timeClient->update();
      if (updated) {
        if (onTimeUpdateListener != NULL)
          onTimeUpdateListener(timeClient->getEpochTime());
        Serial.println(timeClient->getFormattedTime());
        stop();
      }
    }

  public:
    NtpManager() {
      //timeClient->setTimeOffset(offset);
    }
    void start() {
      stop();
      startLongTask();
      timeClient->begin();
      update();
      Tasks.interval(NTP_MANAGER_TASK, 3000, 20, [ = ] {
        update();
      });
    }

    void stop() {
      if (Tasks.getTaskByName(NTP_MANAGER_TASK) != nullptr && Tasks.isRunning(NTP_MANAGER_TASK))
        Tasks.erase(NTP_MANAGER_TASK);
    }
    void startLongTask() {
      if (Tasks.getTaskByName(NTP_MANAGER_TASK_LONG) != nullptr && Tasks.isRunning(NTP_MANAGER_TASK_LONG))
        Tasks.erase(NTP_MANAGER_TASK_LONG);
      Tasks.interval(NTP_MANAGER_TASK_LONG, interval, [ = ] {
        Serial.println("startLongTask");
        update();
      });
    }

    void setOnTimeUpdateListener(const OnTimeUpdateListener& listener) {
      this->onTimeUpdateListener = listener;
    }

};
#endif
