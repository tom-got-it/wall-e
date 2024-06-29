const rect rMp3Exit = {30, 5, -20, 25};
const rect rMp3Voltage = {75, 220, 320, 20};

const rect rMp3PlayerReset = {0, 60, 320, 31};
const rect rMp3BackTri = {30, 60, -25, 30};
const rect rMp3BackRect = {35, 60, 10, 30};
const rect rMp3BackTouch = {5, 60, 40, 30};
const rect rMp3Info = {100, 65, 80, 20};
const rect rMp3PlayStop = {220, 60, 25, 30};
const rect rMp3ForwardTri = {290, 60, 25, 30};
const rect rMp3ForwardRect = {275, 60, 10, 30};
const rect rMp3ForwardTouch = {275, 60, 40, 30};

const rect rMp3VolumeReset = {0, 120, 320, 31};
const rect rMp3VolumeMinus = {5, 120, 40, 30};
const rect rMp3VolumeInfo = {100, 125, 100, 20};
const rect rMp3VolumePlus = {275, 120, 40, 30};

const rect rMp3AddonButtonsReset = {0, 170, 320, 31};
const rect rMp3Display = {5, 170, 140, 30};
const rect rMp3LightBulb = {175, 170, 140, 30};


DateTime mp3VoltageLastPrinted;
DateTime mp3PlaybackLastChecked;
DateTime mp3LastTouched;

boolean mp3IsPlaying;
int mp3TitlePlayed;
int mp3CurrentVolume;

boolean mp3DisplayIsOn = true;
boolean mp3DisplayAuto = false;
boolean mp3LightBulbOn = false;


void showMp3Player() {
  if(isBatteryLowVoltage()) {
    Serial.println("Exit MP3 player due to low battery voltage");
    drawVoltageWarningScreen();
    delay(3000);
    return;
  }

  clearScreen();

  DateTime now = rtc.now();
  mp3CurrentVolume = pNotificationVolume;
  mp3IsPlaying = false;
  mp3TitlePlayed = 1;

  mp3VoltageLastPrinted = DateTime(now.unixtime() - 5);
  mp3PlaybackLastChecked = now;
  mp3LastTouched = now;
  mp3DisplayIsOn = true;
  mp3DisplayAuto = false;
  mp3LightBulbOn = false;

  drawMp3PlayerStaticSymbols();
  drawMp3PlayerMain();
  drawMp3PlayerVolume();
  drawMp3PlayerAddonButtons();

  delay(300);
  mp3PlayerLoop();

  exitMp3Player();
}

void exitMp3Player() {
  //Force dispay to turn on backlight
  mp3PlayerTurnOnDisplay();

  //Drwa waiting / warning screens while resetting everything
  int delayMillis = 1;
  if(isBatteryLowVoltage()) {
    Serial.println("Exit MP3 player due to low battery voltage");
    drawVoltageWarningScreen();
    delayMillis = 3000;
  } else {
    drawWaitingScreen();
  }

  //Do not call MP3-Player functions here, since they will repaint on the screen
  stopMp3Playback();
  setMp3Volume(pNotificationVolume);
  disableLightBulb();

  delay(delayMillis);
}

void drawMp3PlayerStaticSymbols() {
  //Top
  tft.drawString("MP3 Player", 100, 10, TFT_BIG_FONT);
  printTriangleLeftRight(&rMp3Exit, TFT_MAIN_COLOR);

  //Bottom
  tft.drawString("Battery:", 10, 220, TFT_SMALL_FONT);
}

void drawMp3PlayerMain() {
  printRect(&rMp3PlayerReset, TFT_BLACK, true);

  if(mp3TrackCount < 1) {
    tft.setTextColor(TFT_RED);
    tft.drawString("No MP3 files found", 55, rMp3Info.y, TFT_BIG_FONT);
    tft.setTextColor(TFT_MAIN_COLOR);
    return;
  }

  printTriangleLeftRight(&rMp3BackTri, TFT_MAIN_COLOR);
  printRect(&rMp3BackRect, TFT_MAIN_COLOR, true);

  if(mp3IsPlaying) {
      String title = padByZeroThreeDigits(mp3TitlePlayed);
      title = "--" + title + "--"; 
      tft.drawString(title, rMp3Info.x, rMp3Info.y, TFT_BIG_FONT);
      printRect(&rMp3PlayStop, TFT_MAIN_COLOR, true);
  } else {
      tft.drawString("-Stop-", rMp3Info.x, rMp3Info.y, TFT_BIG_FONT);
      printTriangleLeftRight(&rMp3PlayStop, TFT_MAIN_COLOR);
  }

  printRect(&rMp3ForwardRect, TFT_MAIN_COLOR, true);
  printTriangleLeftRight(&rMp3ForwardTri, TFT_MAIN_COLOR);
}

void drawMp3PlayerVolume() {
  printRect(&rMp3VolumeReset, TFT_BLACK, true);

  if(mp3CurrentVolume > 1) {
    printRect(&rMp3VolumeMinus, TFT_MAIN_COLOR, false);
    tft.drawString("-", rMp3VolumeMinus.x + 15, rMp3VolumeMinus.y + 4, TFT_BIG_FONT);
  }

  String volInfo = "Volume: ";
  volInfo = volInfo + padByZeroTwoDigits(mp3CurrentVolume);
  tft.drawString(volInfo, rMp3VolumeInfo.x, rMp3VolumeInfo.y, TFT_BIG_FONT);

  if(mp3CurrentVolume < getMp3MaxPossibleVolume()) {
    printRect(&rMp3VolumePlus, TFT_MAIN_COLOR, false);
    tft.drawString("+", rMp3VolumePlus.x + 15, rMp3VolumePlus.y + 4, TFT_BIG_FONT);
  }
}

void drawMp3PlayerAddonButtons() {
  printRect(&rMp3AddonButtonsReset, TFT_BLACK, true);

  if(mp3DisplayAuto) {
    printRect(&rMp3Display, TFT_MAIN_COLOR, false);
    tft.drawString("Display", rMp3Display.x + 25, rMp3Display.y + 5, TFT_BIG_FONT);
  } else {
    printRect(&rMp3Display, TFT_MAIN_COLOR, true);
    tft.setTextColor(TFT_BLACK);
    tft.drawString("Display", rMp3Display.x + 25, rMp3Display.y + 5, TFT_BIG_FONT);
    tft.setTextColor(TFT_MAIN_COLOR);
  }

  if(mp3LightBulbOn) {
    printRect(&rMp3LightBulb, TFT_MAIN_COLOR, true);
    tft.setTextColor(TFT_BLACK);
    tft.drawString("Light", rMp3LightBulb.x + 40, rMp3LightBulb.y + 5, TFT_BIG_FONT);
    tft.setTextColor(TFT_MAIN_COLOR);
  } else {
    printRect(&rMp3LightBulb, TFT_MAIN_COLOR, false);
    tft.drawString("Light", rMp3LightBulb.x + 40, rMp3LightBulb.y + 5, TFT_BIG_FONT);
  }
}

void drawMp3PlayerVoltage() {
  DateTime now = rtc.now();
  if(mp3VoltageLastPrinted.unixtime() + gVoltageReprintAfterSeconds < now.unixtime()) {
    mp3VoltageLastPrinted = now;
    printRect(&rMp3Voltage, TFT_BLACK, true); //reset
    if(isOnUsbPower()) {
      tft.drawString("(USB)", rMp3Voltage.x, rMp3Voltage.y, TFT_SMALL_FONT);
    } else {
      float batVoltage = getBatVoltage();
      if(batVoltage > gLowVoltage) {
        tft.setTextColor(TFT_MAIN_COLOR);
      } else {
        tft.setTextColor(TFT_RED);
      }

      String output = String(batVoltage, 2) + "V";
      output = output + "   @" + String(padByZeroThreeDigits(getBatCurrentMillis())) + "mA";
      tft.drawString(output, rMp3Voltage.x, rMp3Voltage.y, TFT_SMALL_FONT);

      tft.setTextColor(TFT_MAIN_COLOR);
    }
  }
}

void mp3PlayerLoop() {
  while(true) {
    //Alarm test - stop the MP3 player on alarm
    if(isAlarmClockTriggered()) {
      return;
    }

    //Voltage test
    drawMp3PlayerVoltage();
    if(isBatteryLowVoltage()) {
      return;
    }

    //display auto-off test
    mp3PlayerDisplayAutoOffTest();

    //Light Bulb auto-shutdown Test
    boolean wasLightbulbOn = mp3LightBulbOn;
    mp3LightBulbOn = testLightBulbStillActive();
    if(wasLightbulbOn != mp3LightBulbOn) {
      drawMp3PlayerAddonButtons();
    }

    //Play next MP3 if the previous MP3 ended
    mp3PlayerTestCurrentlyPlaying();

    if(handleMp3PlayerTouchAndIsExit()) {
      return;
    }
  }
}


boolean handleMp3PlayerTouchAndIsExit() {
    uint16_t x,y;
    boolean touched = tft.getTouch(&x, &y);
    if(! touched) {
      return false;
    }

    mp3LastTouched = rtc.now();
    if(touched && !mp3DisplayIsOn) {
      //turn on the display if it was off
      Serial.println("Touched turn on dispaly");
      mp3PlayerTurnOnDisplay();
      delay(400);
      return false;
    }

    if(touchedRect(x, y, &rMp3Exit)) {
      Serial.println("Touched exit button");
      return true;
    }

    if(touchedRect(x, y, &rMp3Display)) {
      Serial.println("Touched toggle display mode");
      mp3PlayerToggleDisplayMode();
      delay(120);
      return false;
    }

    if(touchedRect(x, y, &rMp3LightBulb)) {
      Serial.println("Touched toggle light bulb");
      mp3PlayerToggleLightBulb();
      delay(80);
      return false;
    }

    if(mp3TrackCount <= 0) {
      return false;
    }

    //----------------------------
    //MP3 Touch events here:
    //----------------------------
    if(touchedRect(x, y, &rMp3BackTouch)) {
      Serial.println("Touched play previous MP3");
      mp3PlayerPlayPrevious();
      delay(120);
      return false;
    }

    if(touchedRect(x, y, &rMp3ForwardTouch)) {
      Serial.println("Touched play next MP3");
      mp3PlayerPlayNext();
      delay(120);
      return false;
    }

    if(touchedRect(x, y, &rMp3PlayStop)) {
      Serial.println("Touched play / stop MP3");
      mp3PlayerTogglePlayback();
      delay(120);
      return false;
    }

    if(touchedRect(x, y, &rMp3VolumeMinus) && mp3CurrentVolume > 1) {
      Serial.println("Touched decrement volume");
      mp3PlayerDecrementVolume();
      delay(120);
      return false;
    }

    if(touchedRect(x, y, &rMp3VolumePlus) && mp3CurrentVolume <= getMp3MaxPossibleVolume()) {
      Serial.println("Touched increment volume");
      mp3PlayerIncrementVolume();
      delay(120);
      return false;
    }

    return false;
}

/**
* Every two seconds, test if MP3 is still playing and possibly advance to the next MP3
*/
void mp3PlayerTestCurrentlyPlaying() {
  if(mp3PlaybackLastChecked.unixtime() + 2 < rtc.now().unixtime()) {
    //Play next MP3 if the previous MP3 ended
    boolean isCurrentlyPlaying = isMp3Playing();
    if(mp3IsPlaying && ! isCurrentlyPlaying) {
      Serial.println("Forwarding to next MP3");
      mp3PlayerPlayNext();
    }
    mp3PlaybackLastChecked = rtc.now();
  }
}

void mp3PlayerPlayNext() {
  mp3TitlePlayed++;
  if(mp3TitlePlayed > mp3TrackCount) {
    mp3TitlePlayed = 1;
  }
  mp3IsPlaying = true;
  drawMp3PlayerMain();
  playMp3File(mp3TitlePlayed);
  mp3PlaybackLastChecked = rtc.now();
}

void mp3PlayerPlayPrevious() {
  mp3TitlePlayed--;
  if(mp3TitlePlayed <= 0) {
    mp3TitlePlayed = mp3TrackCount;
  }
  mp3IsPlaying = true;
  drawMp3PlayerMain();
  playMp3File(mp3TitlePlayed);
  mp3PlaybackLastChecked = rtc.now();
}

void mp3PlayerTogglePlayback() {
  if(mp3IsPlaying) {
    mp3IsPlaying = false;
    drawMp3PlayerMain();
    stopMp3Playback();
  } else {
    mp3IsPlaying = true;
    drawMp3PlayerMain();
    playMp3File(mp3TitlePlayed);
  }
  mp3PlaybackLastChecked = rtc.now();
}

void mp3PlayerIncrementVolume() {
  mp3CurrentVolume++;
  if(mp3CurrentVolume > getMp3MaxPossibleVolume()) {
    mp3CurrentVolume = getMp3MaxPossibleVolume();
    return;
  }
  drawMp3PlayerVolume();
  setMp3Volume(mp3CurrentVolume);
}

void mp3PlayerDecrementVolume() {
  mp3CurrentVolume--;
  if(mp3CurrentVolume <= 0) {
    mp3CurrentVolume = 1;
    return;
  }
  drawMp3PlayerVolume();
  setMp3Volume(mp3CurrentVolume);
}

void mp3PlayerToggleLightBulb() {
  mp3LightBulbOn = !mp3LightBulbOn;
  if(mp3LightBulbOn) {
    //Note that due to low batery voltage the light bulb might stay off
    mp3LightBulbOn = startLightBulb();
  } else {
    disableLightBulb();
  }
  drawMp3PlayerAddonButtons();
}

void mp3PlayerDisplayAutoOffTest() {
  if(!mp3DisplayAuto) {
    return;
  }

  if(!mp3DisplayIsOn) {
    //display already offline
    return;
  }

  if(mp3LastTouched.unixtime() + gDisplayAutoShutdownSeconds < rtc.now().unixtime()) {
      turnOffDisplay();
      mp3DisplayIsOn = false;
  }
}

void mp3PlayerToggleDisplayMode() {
  mp3DisplayAuto = !mp3DisplayAuto;
  drawMp3PlayerAddonButtons();
}

void mp3PlayerTurnOnDisplay() {
  turnOnDisplay();
  mp3DisplayIsOn = true;
}

