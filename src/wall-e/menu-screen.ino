const rect rMenuScreenExit = {30, 5, -20, 25};

const rect rMenuScreen1 = {60, 5, 200, 35};
const rect rMenuScreen2 = {60, 54, 200, 35};
const rect rMenuScreen3 = {60, 103, 200, 35};
const rect rMenuScreen4 = {60, 152, 200, 35};
const rect rMenuScreen5 = {60, 201, 200, 35};

DateTime menuOpened;


void showMenuScreen() {
  menuOpened = rtc.now();

  printMenuScreenMain();
  delay(300);
  loopMenuScreen();
}

void loopMenuScreen() {
  uint menuScreen = 0; //-1 = Exit, 0 = Main, 1 = Setup
  while(true) {
    if(isAlarmClockTriggered()) {
      Serial.println("Leaving menu screen because an alarm has fired");
      return;
    }

    if(isBatteryLowVoltage() && rtc.now().unixtime() - menuOpened.unixtime() >= 10) {
      Serial.println("Leaving menu screen after 10 seconds due to low battery voltage");
      drawVoltageWarningScreen();
      delay(3000);
      return;
    }

    uint nextScreen;
    if(menuScreen == 0) {
      nextScreen = handleMenuScreenMain();
    } else {
      nextScreen = handleMenuScreenSetup();
    }

    if(nextScreen != menuScreen) {
      menuScreen = nextScreen;
      switch(nextScreen) {
        case -1: return;
        case 0: 
          printMenuScreenMain();
          break;
        case 1:
          printMenuScreenSetup();
          break;
      }
    }

    delay(150);
  }
}

void printMenuScreenMain() {
  clearScreen();
  printTriangleLeftRight(&rMenuScreenExit, TFT_MAIN_COLOR);

  tft.drawString("Setup...", rMenuScreen1.x + 55, rMenuScreen1.y + 7, TFT_BIG_FONT);
  printRect(&rMenuScreen1, TFT_MAIN_COLOR, false);

  tft.drawString("Timer", rMenuScreen2.x + 65, rMenuScreen2.y + 7, TFT_BIG_FONT);
  printRect(&rMenuScreen2, TFT_MAIN_COLOR, false);

  tft.drawString("Notifications", rMenuScreen3.x + 23, rMenuScreen3.y + 7, TFT_BIG_FONT);
  printRect(&rMenuScreen3, TFT_MAIN_COLOR, false);

  tft.drawString("MP3 Player", rMenuScreen4.x + 35, rMenuScreen4.y + 7, TFT_BIG_FONT);
  printRect(&rMenuScreen4, TFT_MAIN_COLOR, false);

  tft.drawString("Firmware", rMenuScreen5.x + 45, rMenuScreen5.y + 7, TFT_BIG_FONT);
  printRect(&rMenuScreen5, TFT_MAIN_COLOR, false);
}

void printMenuScreenSetup() {
  clearScreen();
  printTriangleLeftRight(&rMenuScreenExit, TFT_MAIN_COLOR);

  tft.drawString("Alarm", rMenuScreen1.x + 65, rMenuScreen1.y + 7, TFT_BIG_FONT);
  printRect(&rMenuScreen1, TFT_MAIN_COLOR, false);

  tft.drawString("Volume", rMenuScreen2.x + 55, rMenuScreen2.y + 7, TFT_BIG_FONT);
  printRect(&rMenuScreen2, TFT_MAIN_COLOR, false);

  tft.drawString("Clock", rMenuScreen3.x + 65, rMenuScreen3.y + 7, TFT_BIG_FONT);
  printRect(&rMenuScreen3, TFT_MAIN_COLOR, false);
}

uint handleMenuScreenMain() {
  uint16_t x,y;
  boolean touched = tft.getTouch(&x, &y);
  if(touched) {
    if(touchedRect(x, y, &rMenuScreenExit)) {
      Serial.println("Touched menu exit");
      return -1;
    }

    if(touchedRect(x, y, &rMenuScreen1)) {
      Serial.println("Touched menu setup");
      return 1;
    }

    if(touchedRect(x, y, &rMenuScreen2)) {
      Serial.println("Touched menu timer");
      showTimerScreen();
      return -1;
    }

    if(touchedRect(x, y, &rMenuScreen3)) {
      Serial.println("Touched menu notifications");
      showNotificationScreen();
      return -1;
    }

    if(touchedRect(x, y, &rMenuScreen4)) {
      Serial.println("Touched menu mp3 player");
      showMp3Player();
      return -1;
    }
    
    if(touchedRect(x, y, &rMenuScreen5)) {
      Serial.println("Touched menu firmware updates");
      showFirmwareUpdateScreen();
      return -1;
    }
  }

  return 0;
}

uint handleMenuScreenSetup() {
  uint16_t x,y;
  boolean touched = tft.getTouch(&x, &y);
  if(touched) {
    if(touchedRect(x, y, &rMenuScreenExit)) {
      Serial.println("Touched menu exit");
      return 0;
    }

    if(touchedRect(x, y, &rMenuScreen1)) {
      Serial.println("Touched menu alarm setup");
      showAlarmSetupScreen();
      return -1;
    }

    if(touchedRect(x, y, &rMenuScreen2)) {
      Serial.println("Touched menu volume setup");
      setupVolume();
      return -1;
    }

    if(touchedRect(x, y, &rMenuScreen3)) {
      Serial.println("Touched menu clock setup");
      showClockSetupScreen();
      return -1;
    }
  }

  return 1;
}