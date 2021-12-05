#define BUTTON_PIN D2
#include <AwesomeClickButton.h>

AwesomeClickButton awesomeClickButton(BUTTON_PIN);

void updateClickButton(){	
  awesomeClickButton.update();
}
void resetFactoryConfig(){
     bool success = SPIFFS.begin();

     if (!success) {
       Serial.println("Error mounting the file system");
       return;
     }

     SPIFFS.remove("/config.json");

     File writeFile = SPIFFS.open("/config.json", "w");

     if (!writeFile) {
       Serial.println("Error opening file for writing");
       return;
     }

       File readFile = SPIFFS.open("/config-backup.json", "r");
       if (!readFile) {
         Serial.println("Failed to open file for reading");

          writeFile.close();
         return;
       }


      Serial.println("File Content:");

          while (readFile.available()) {

            writeFile.write(readFile.read());
          }

     writeFile.close();
     readFile.close();

}

void onLongClickListener(int duration) {
  Serial.println((String)"onLongClickListener " + duration );

  if(duration>5000){
    resetFactoryConfig();
    onSetupConfig();
  }
}

void initClickButton(){
	 
  awesomeClickButton.setOnClickListener([]() {
    Serial.println("setOnClickListener " );
    stopAllTasks();
    onCompositeFeeding();
  });
  awesomeClickButton.setOnMultiClickListener([] (int count) {
	stopAllTasks();
	if (count == 2) {
	  toggleLamp();
	} else if (count == 3) {
	  shiftWifiState();
	}
  });

  awesomeClickButton.setOnLongClickListener(onLongClickListener);

 }
