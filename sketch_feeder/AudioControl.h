#ifndef AUDIO_CONTROL_H
#define AUDIO_CONTROL_H
#include <Arduino.h>
#define AUDIO_CONTROL_TASK "AUDIO_CONTROL_TASK"
#include "AudioFileSourceSPIFFS.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"
#include <avr/pgmspace.h>

class AudioControl {

  private:
    AudioGeneratorMP3 *mp3 = NULL;
    AudioFileSourceSPIFFS *file = NULL;
    AudioFileSourceID3 *id3 = NULL;
    AudioOutputI2SNoDAC *out = NULL;
    bool stopped = false;
  public:
    AudioControl() {
    }

    void play(char *filename, float soundVolume, void (*listener)()) {
      stop();
      delay(100);
      //audioLogger = &Serial;
      file = new AudioFileSourceSPIFFS(filename);
      id3 = new AudioFileSourceID3(file);
      // id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
      out = new AudioOutputI2SNoDAC();
      out->SetGain(soundVolume);

      mp3 = new AudioGeneratorMP3();
      mp3->begin(id3, out);
      stopped = false;
      Tasks.framerate(AUDIO_CONTROL_TASK, 1000, [ = ] {
        if (mp3->isRunning()) {
          if (!mp3->loop()) mp3->stop();
        } else if (!stopped) {          
          stop();
          if ( (listener != NULL))  {
            listener();
          }
          Serial.print("MP3 done\n");
        }
      });
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


    void stop() {
      Serial.println((String)"Tasks AUDIO_CONTROL_TASK:" + Tasks.isRunning(AUDIO_CONTROL_TASK));

      stopped = true;
      if (Tasks.isRunning(AUDIO_CONTROL_TASK) == true)
        Tasks.erase(AUDIO_CONTROL_TASK);

      if (NULL != file) {
        delete file;
        file = NULL;
      }

      if (NULL != id3) {
        delete id3;
        id3 = NULL;
      }

      if (NULL != out) {
        delete out;
        out = NULL;
      }

      if (NULL != mp3) {
        delete mp3;
        mp3 = NULL;
      }

    }
};
#endif
