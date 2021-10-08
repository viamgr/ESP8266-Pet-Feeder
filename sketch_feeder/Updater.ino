
void updateSketchFromFile() {

  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);
  File file = SPIFFS.open("/update.bin", "r");
  if (file.available() == false) {
    Serial.println("------------------------------------- Udpater file not found");
    onUpdateFileNotFound();
    return;
  }

  uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;

  Serial.println("------------------------------------- Start updating");
  onUpdateStarted();

  if (!Update.begin(maxSketchSpace, U_FLASH)) { //start with max available size
    Update.printError(Serial);
    onUpdateError();
    Serial.println("------------------------------------- ERROR");
  }

  while (file.available()) {
    uint8_t ibuffer[128];
    file.read((uint8_t *)ibuffer, 128);
    Serial.print(".");
    Update.write(ibuffer, sizeof(ibuffer));
  }


  Serial.print(Update.end(true));
  digitalWrite(BUILTIN_LED, HIGH);
  file.close();
  onUpdateFinished();
  Serial.println("------------------------------------- Update Finished!");
}
