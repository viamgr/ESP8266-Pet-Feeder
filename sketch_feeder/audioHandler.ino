#define AUDIO_FEEDING "/feeding.mp3"
#define AUDIO_START_UP "/miracle.mp3"

#include "AudioControl.h"

AudioControl audioControl(&taskManager);

void stopAudio(){	
  audioControl.stop();
}
void playAudio(char *filename,void (*listener)()){
  Serial.println((String) "onPlayFeedingAudioEventedingEvent");

  stopAllTasks();
  audioControl.play(filename,listener);
	
}
void setupAudioConfig(){
  audioControl.setSoundVolume(preferences.getSoundVolume());
}