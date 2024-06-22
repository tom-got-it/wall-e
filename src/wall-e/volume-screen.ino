void setupVolume() {
  setupVolumeImpl();
  drawWaitingScreen();

  if(myMP3.isPlaying()) {
    delay(1000);
    myMP3.stop();
  }

  delay(1000);
  myMP3.volume(notificationVolume);
  delay(1000);
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
      drawTwoDigitsAndGetXPos(notificationVolume, 125, 110, TFT_MAIN_FONT);
      reprint = false;
    }
    
    uint16_t x,y;
    boolean touched = tft.getTouch(&x, &y);

    if (touched) {
      if(touchedRect(x, y, &rOk)) {
        return;
      }

      if(touchedRect(x, y, &rTop)) {
        notificationVolume = notificationVolume + 1;
        if (notificationVolume > 30) { notificationVolume = 30; }
        reprint = true;
      }

      if(touchedRect(x, y, &rBottom)) {
        notificationVolume = notificationVolume - 1;
        if (notificationVolume < 1) { notificationVolume = 1; }
        reprint = true;
      }

      if(touchedRect(x, y, &rTest)) {
        testVolume();
      }

      if(touchedRect(x, y, &rStop)) {
        stopPlaying();
      }

      delay(80);
    }
  }
}

void testVolume() {
  if(mp3TrackCount < 1) {
    return;
  }

  myMP3.volume(notificationVolume);
  delay(1000);

  if(! myMP3.isPlaying()) {
    delay(1000);
    myMP3.playFolder(1, random(1, mp3TrackCount + 1));
    delay(1000);
  }
}

void stopPlaying() {
  if(myMP3.isPlaying()) {
    delay(1000);
    myMP3.stop();
    delay(1000);
  }
}