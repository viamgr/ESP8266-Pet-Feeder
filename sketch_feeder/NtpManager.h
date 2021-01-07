#ifndef NTP_MANAGER_H
#define NTP_MANAGER_H
#include <Arduino.h>
#include <TaskManager.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#define NTP_MANAGER_TASK "NTP_MANAGER_TASK"
typedef std::function<void(unsigned long epochTime)> OnTimeUpdateListener;

class NtpManager {
  private:
    OnTimeUpdateListener onTimeUpdateListener = NULL;
    WiFiUDP udp;
    NTPClient *timeClient = new NTPClient(udp, "pool.ntp.org");
    const int interval = 60 * 60 * 1000;
    void update() {
      for (int i = 0; i < 30; i++) {
        Serial.println("getting time");
        bool updated = timeClient->update();
        if (updated) {
          if (onTimeUpdateListener != NULL)
            onTimeUpdateListener(timeClient->getEpochTime());
          Serial.println(timeClient->getFormattedTime());
          break;
        }
        delay(20000);
      }
    }

  public:
    NtpManager() {
      //timeClient->setTimeOffset(offset);
    }
    void start() {
      stop();
      Tasks.once(NTP_MANAGER_TASK, 100, [ = ] {
        timeClient->begin();
        update();
        Tasks.interval(NTP_MANAGER_TASK, interval, [ = ]{
          update();
        });
      });
    }

    void stop() {
      Tasks.erase(NTP_MANAGER_TASK);
    }

    void setOnTimeUpdateListener(const OnTimeUpdateListener& listener) {
      this->onTimeUpdateListener = listener;
    }

};
#endif
