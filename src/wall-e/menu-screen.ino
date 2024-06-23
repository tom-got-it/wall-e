const rect rMenuScreenExit = {30, 5, -20, 25};

const rect rMenuScreenSetup = {60, 5, 200, 35};
const rect rMenuScreenVolume = {60, 54, 200, 35};
const rect rMenuScreenNotifications = {60, 103, 200, 35};
const rect rMenuScreenMp3Player = {60, 152, 200, 35};
const rect rMenuScreenFirmwareUpdate = {60, 201, 200, 35};

void showMenuScreen() {
  clearScreen();

  printTriangleLeftRight(&rMenuScreenExit, TFT_MAIN_COLOR);

  tft.drawString("Clock/Alarm", rMenuScreenSetup.x + 27, rMenuScreenSetup.y + 7, TFT_BIG_FONT);
  printRect(&rMenuScreenSetup, TFT_MAIN_COLOR, false);

  tft.drawString("Alarm Volume", rMenuScreenVolume.x + 22, rMenuScreenVolume.y + 7, TFT_BIG_FONT);
  printRect(&rMenuScreenVolume, TFT_MAIN_COLOR, false);

  tft.drawString("Notifcations", rMenuScreenNotifications.x + 25, rMenuScreenNotifications.y + 7, TFT_BIG_FONT);
  printRect(&rMenuScreenNotifications, TFT_MAIN_COLOR, false);

  tft.drawString("MP3 Player", rMenuScreenMp3Player.x + 35, rMenuScreenMp3Player.y + 7, TFT_BIG_FONT);
  printRect(&rMenuScreenMp3Player, TFT_MAIN_COLOR, false);

  tft.drawString("Firmware", rMenuScreenFirmwareUpdate.x + 45, rMenuScreenFirmwareUpdate.y + 7, TFT_BIG_FONT);
  printRect(&rMenuScreenFirmwareUpdate, TFT_MAIN_COLOR, false);

  delay(300);

  while(true) {
    if(hasAlarmFired()) {
      Serial.println("Leaving menu screen because an alarm has fired");
      return;
    }

    if(isBatteryLowVoltage()) {
      Serial.println("Leaving menu screen due to low battery voltage");
      //Make sure we are not trapped in the main-menu forever f the user dose not react
      return;
    }

    uint16_t x,y;
    boolean touched = tft.getTouch(&x, &y);
    if(touched) {
      if(touchedRect(x, y, &rMenuScreenExit)) {
        Serial.println("Touched menu exit");
        return;
      }

      if(touchedRect(x, y, &rMenuScreenSetup)) {
        Serial.println("Touched menu clock/alarm setup");
        showSetupScreen();
        return;
      }

      if(touchedRect(x, y, &rMenuScreenVolume)) {
        Serial.println("Touched menu volume setup");
        setupVolume();
        return;
      }

      if(touchedRect(x, y, &rMenuScreenNotifications)) {
        Serial.println("Touched menu notifications");
        showNotificationScreen();
        return;
      }

      if(touchedRect(x, y, &rMenuScreenMp3Player)) {
        Serial.println("Touched menu mp3 player");
        showMp3Player();
        return;
      }
      
      if(touchedRect(x, y, &rMenuScreenFirmwareUpdate)) {
        Serial.println("Touched menu firmware updates");
        showFirmwareUpdateScreen();
        return;
      }
    }

    delay(150);
  }
}