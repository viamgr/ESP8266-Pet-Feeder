#ifndef NTP_MANAGER_H
#define NTP_MANAGER_H
#include <Arduino.h>
#include <TaskManager.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#define NTP_MANAGER_TASK "NTP_MANAGER_TASK"
#define NTP_MANAGER_TASK_LONG "NTP_MANAGER_TASK_LONG"
typedef std::function<void(unsigned long epochTime)> OnTimeUpdateListener;

#define NTP_MANAGER_TASK_LONG_INTERVAL 60 * 60 * 1000
#define NTP_MANAGER_TASK_SHORT_INTERVAL 3 * 1000
#define NTP_MANAGER_TASK_MAX_INTERVAL 5 * 60 * 1000


class NtpManager: public Task  {
  private:
    OnTimeUpdateListener onTimeUpdateListener = NULL;
    WiFiUDP udp;
    NTPClient *timeClient = new NTPClient(udp, "pool.ntp.org");
    void update() {
      Serial.println("getting time");
      bool updated = timeClient->update();
      if (updated) {
        if (onTimeUpdateListener != NULL) onTimeUpdateListener(timeClient->getEpochTime());
        Serial.println(timeClient->getFormattedTime());
        setInterval(NTP_MANAGER_TASK_LONG_INTERVAL);
        restartDelayed();
      }
      else {
        unsigned long aInterval = NTP_MANAGER_TASK_SHORT_INTERVAL * getRunCounter();
        if (aInterval > NTP_MANAGER_TASK_MAX_INTERVAL) {
          aInterval = NTP_MANAGER_TASK_MAX_INTERVAL;
        }
        Serial.println((String)"Failed getting time, Retry after:" + aInterval);

        setInterval(aInterval);
      }
    }

  public:
    NtpManager(Scheduler* scheduler) : Task(NTP_MANAGER_TASK_SHORT_INTERVAL, TASK_FOREVER, scheduler) {

    }

    bool Callback() {
      Serial.println((String)"NtpManager Callback");
      update();
      return true;
    }
    bool OnEnable() {
      Serial.println((String)"NtpManager OnEnable");
      restart();
      timeClient->begin();
      return true;
    }

    void OnDisable() {
      Serial.println((String)"NtpManager OnDisable");
      timeClient->end();
    }

    void setOnTimeUpdateListener(const OnTimeUpdateListener& listener) {
      this->onTimeUpdateListener = listener;
    }

};
#endif
