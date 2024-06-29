void setupVolume() {
  if(isBatteryLowVoltage()) {
    Serial.println("Exit volume setuop screen due to low battery voltage");
    drawVoltageWarningScreen();
    delay(3000);
    return;
  }

  setupVolumeImpl();

  int additionalDelay = 0;
  if(isBatteryLowVoltage()) {
    drawVoltageWarningScreen();
    additionalDelay = 2000;
  } else {
    drawWaitingScreen();
  }

  stopMp3Playback();
  setMp3Volume(pNotificationVolume);

  if(additionalDelay > 0) {
    delay(additionalDelay);
  }
}

void setupVolumeImpl() {
  const rect rTop = {140, 80, 40, -40};
  const rect rBottom = {140, 190, 40, 40};
  const rect rOk = {270, 190, 50, 50};
  const rect rTest = {0, 0, 63, 50};
  const rect rStop = {257, 0, 63, 50};
  const rect rReset = {100, 100, 120, 70};

  clearScreen();
  tft.drawString("Volume", 115, 10, 4);

  printRect(&rTest, TFT_MAIN_COLOR, false);
  tft.drawString("Test", 7, 15, 4);
  printRect(&rStop, TFT_MAIN_COLOR, false);
  tft.drawString("Stop", 262, 15, 4);
  printRect(&rOk, TFT_MAIN_COLOR, false);
  tft.drawString("OK", 276, 205, 4);

  printTriangleUpDown(&rTop, TFT_MAIN_COLOR);
  printTriangleUpDown(&rBottom, TFT_MAIN_COLOR);

  boolean reprint = true;
  while (true) {
    if(reprint) {
      printRect(&rReset, TFT_BLACK, true);
      drawTwoDigitsAndGetXPos(pNotificationVolume, 125, 110, TFT_MAIN_FONT);
      reprint = false;
    }

    if(isBatteryLowVoltage()) {
      return;
    }
    
    uint16_t x,y;
    boolean touched = tft.getTouch(&x, &y);

    if (touched) {
      if(touchedRect(x, y, &rOk)) {
        return;
      }

      if(touchedRect(x, y, &rTop)) {
        pNotificationVolume = pNotificationVolume + 1;
        if (pNotificationVolume > getMp3MaxPossibleVolume()) { pNotificationVolume = getMp3MaxPossibleVolume(); }
        savePrefs();
        reprint = true;
      }

      if(touchedRect(x, y, &rBottom)) {
        pNotificationVolume = pNotificationVolume - 1;
        if (pNotificationVolume < 1) { pNotificationVolume = 1; }
        savePrefs();
        reprint = true;
      }

      if(touchedRect(x, y, &rTest)) {
        testVolume();
      }

      if(touchedRect(x, y, &rStop)) {
        stopMp3Playback();
      }

      delay(80);
    }
  }
}

void testVolume() {
  if(mp3TrackCount < 1) {
    return;
  }

  setMp3Volume(pNotificationVolume);
  if(! isMp3Playing()) {
    playMp3RandomFile();
  }
}