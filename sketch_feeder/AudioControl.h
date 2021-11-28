#ifndef AUDIO_CONTROL_H
#define AUDIO_CONTROL_H


#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass 
#define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
#define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
#define _TASK_PRIORITY          // Support for layered scheduling priority
#define _TASK_TIMEOUT           // Support for overall task timeout 
#define _TASK_OO_CALLBACKS

#include <Arduino.h>
#define AUDIO_CONTROL_TASK "AUDIO_CONTROL_TASK"
#include "AudioFileSourceSPIFFS.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"
#include <avr/pgmspace.h>

typedef std::function<void(void)> OnStopListener;

class AudioControl: public Task {

  private:
    OnStopListener listener = NULL;
    AudioGeneratorMP3 *mp3 = NULL;
    AudioFileSourceSPIFFS *file = NULL;
    AudioFileSourceID3 *id3 = NULL;
    AudioOutputI2SNoDAC *out = NULL;
    float soundVolume = 3.99;
  public:
    AudioControl(Scheduler* scheduler) : Task(TASK_IMMEDIATE , TASK_FOREVER, scheduler) {
    }

    void setSoundVolume(float soundVolume) {
      this->soundVolume = soundVolume;
    }

    void play(const char *filename, OnStopListener listener) {
      Serial.println((String)"play "+filename);

      audioLogger = &Serial;
      file = new AudioFileSourceSPIFFS(filename);
      id3 = new AudioFileSourceID3(file);
      //Serial.println("play1");

      id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
      out = new AudioOutputI2SNoDAC();
      out->SetGain(soundVolume);
      this->listener = listener;
      mp3 = new AudioGeneratorMP3();
      //Serial.println("play2");

      mp3->begin(id3, out);
      restart();
    }

    void update() {
      if (mp3!=NULL && mp3->isRunning()) {
//            Serial.println("play3");
        if (!mp3->loop()) mp3->stop();
      } else {
        Serial.print("MP3 done\n");
        if ( (listener != NULL))  {
          listener();
        }
        stop();
      }
    }

    bool Callback() {
      update();
      return true;
    }

    // Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
    static void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
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
    void OnDisable() {
      int i = getId();
      Serial.print(millis()); Serial.print(":\t");
      Serial.print("AudioControl: TaskID=");
      Serial.println(i);


      if (NULL != file) {
        delete file;
        file = NULL;
      }
      //Serial.println("2");
      if (NULL != id3) {
        delete id3;
        id3 = NULL;
      }
      //Serial.println("3");

      if (NULL != out) {
        delete out;
        out = NULL;
      }

      //Serial.println("4");

      if (NULL != mp3) {
        delete mp3;
        mp3 = NULL;
      }

      //Serial.println("5");

      listener = NULL;

    }


    void stop() {
      disable();
      
    }
};
#endif
