#ifndef AUDIO_CONTROL_H
#define AUDIO_CONTROL_H
#include <Arduino.h>
#define AUDIO_CONTROL_TASK "AUDIO_CONTROL_TASK"

#include "AudioFileSourceSPIFFS.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"

class AudioControl {

  private:
    AudioGeneratorMP3 *mp3;

  public:
    AudioControl() {
    }

    void play(char *filename, float soundVolume, void (*listener)()) {
      stop();
      audioLogger = &Serial;
      AudioFileSourceSPIFFS *file = new AudioFileSourceSPIFFS(filename);
      AudioFileSourceID3 *id3 = new AudioFileSourceID3(file);
      id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
      AudioOutputI2SNoDAC *out = new AudioOutputI2SNoDAC();
      out->SetGain(soundVolume);

      mp3 = new AudioGeneratorMP3();
      mp3->begin(id3, out);

      Tasks.framerate(AUDIO_CONTROL_TASK, 1000, [ = ] {
        if (mp3->isRunning()) {
          if (!mp3->loop()) mp3->stop();
        } else {
          stop();
          if ( (listener != NULL))  {
            listener();
          }
          Serial.printf("MP3 done\n");
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
      if (mp3 != NULL && mp3->isRunning()) {
        mp3->stop();
      }
      Tasks.erase(AUDIO_CONTROL_TASK);
    }
};
#endif
