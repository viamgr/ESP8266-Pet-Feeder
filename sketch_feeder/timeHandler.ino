#include "NtpManager.h"
NtpManager ntpManager(&taskManager);

bool firstUpdateTime = true;

void initNtp(){
	setenv("TZ", "UTC-03:30", 0);
	ntpManager.setOnTimeUpdateListener(onTimeUpdate);
}
void onTimeUpdate(unsigned long epochTime) {
  if (firstUpdateTime == true) {
    firstUpdateTime = false;
	setDeviceTime(epochTime);
    playAudio(AUDIO_START_UP, NULL);
  }
  else {
    setDeviceTime(epochTime);
  }
}


void setDeviceTime(unsigned long epochTime) {

  stopFeedingAlarm();
  Serial.println((String)"setDeviceTime epochTime... " + epochTime);
  setTime(epochTime);

  struct timeval tv;
  tv.tv_sec = epochTime;

  settimeofday(&tv, NULL);
  resetFeedingTasks();
}