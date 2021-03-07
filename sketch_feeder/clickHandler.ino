#define BUTTON_PIN D2
#include <AwesomeClickButton.h>

AwesomeClickButton awesomeClickButton(BUTTON_PIN);

void updateClickButton(){	
  awesomeClickButton.update();
}

void initClickButton(){
	 
  awesomeClickButton.setOnClickListener([]() {
    stopAllTasks();
    onCompositeFeeding();
  });
  awesomeClickButton.setOnMultiClickListener([] (int count) {
	stopAllTasks();
	if (count == 2) {
	  onLedTimerEvent();
	} else if (count == 3) {
	  shiftWifiState();
	}
  });
 }