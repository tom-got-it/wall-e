const rect rAlarmOk = {80, 170, 160, 50};
const int xAlarmOk = 143;
const int yAlarmOk = 185;
const int xAlarmRangeWakeUp[] = {10, 200};
const int yAlarmRangeWakeUp[] = {10, 130};
const rect rAlarmResetWakeup = {0, 0, 320, 170};
const int alarmRefreshMessageSeconds = 5;

DateTime alarmStart;
DateTime alarmLastWakeupPrint;

boolean testAndShowAlarmScreen() {
  if(rtc.alarmFired(1)) {
      rtc.clearAlarm(1);

      if(isAlarmActive) {
        //There seems to be a bug within the rtc.
        //When alarm is disabled and the system wakes up, alarmFired is true when the alarm occurred in sleep mode.
        missedAlarmNotification = false;
        showAlarmScreen();
        return true;
      }
  }
  return false;
}

void showAlarmScreen() {
  Serial.println("----Entering alarm-Screen - Alarm has fired----");
  alarmStart = rtc.now();

  clearScreen();

  boolean lightBulbActive = false;
  boolean lightForceOff = false;
  if(lightBulbDelaySecondsOnAlarm == 0) {
    lightBulbActive = startLightBulb();
  }

  playAlarmMp3();
  drawWalleGifMain();
  delay(1000);

  clearScreen();
  printOkButton();
  printWakeUp();

  while(true) {
    if(isExitByTouch()) {
      handleExit();
      return;
    }

    boolean isPlaying = myMP3.isPlaying();
    delay(1000);

    DateTime now = rtc.now();

    if(lightBulbActive) {
      lightBulbActive = testWhileLightBulbActive();
      if(! lightBulbActive) {
        lightForceOff = true;  //make sure to not turn on the light bulb again
      }
    } else if(! lightForceOff && lightBulbDelaySecondsOnAlarm > 0 && alarmStart.unixtime() + lightBulbDelaySecondsOnAlarm <= now.unixtime()) {
      lightBulbActive = startLightBulb();
    }

    if(isExitByTouch()) {
      handleExit();
      return;
    }

    if(alarmStart.unixtime() + gAlarmAutoShutdownSeconds < now.unixtime()) {
      Serial.println("Stop the alarm due to timeout - user did not react.");
      missedAlarmNotification = true;
      handleExit();
      return;
    } else {
      if(! isPlaying) {
        playAlarmMp3();
        delay(1000);
      }
    }

    if(isExitByTouch()) {
      handleExit();
      return;
    }

    if(alarmLastWakeupPrint.unixtime() + alarmRefreshMessageSeconds < rtc.now().unixtime()) {
      Serial.println("print wake-up message");
      printWakeUp();
    }
  }
}

void playAlarmMp3() {
  if(mp3TrackCount > gMp3CountInitSound) {
      myMP3.playFolder(1, random(gMp3CountInitSound + 1, mp3TrackCount + 1));
      delay(1000);
  }
}

void printWakeUp() {
  printRect(&rAlarmResetWakeup, TFT_BLACK, true);
  int wx = random(xAlarmRangeWakeUp[0], xAlarmRangeWakeUp[1]);
  int wy = random(yAlarmRangeWakeUp[0], yAlarmRangeWakeUp[1]);
  tft.drawString("Wake up!", wx, wy, TFT_BIG_FONT);
  alarmLastWakeupPrint = rtc.now();
}

void printOkButton() {
  printRect(&rAlarmOk, TFT_MAIN_COLOR, true);
  tft.setTextColor(TFT_BLACK);
  tft.drawString("OK", xAlarmOk, yAlarmOk, TFT_BIG_FONT);
  tft.setTextColor(TFT_MAIN_COLOR);
  alarmLastWakeupPrint = rtc.now();
}

void handleExit() {
  myMP3.stop();
  disableLightBulb();
}

boolean isExitByTouch() {
  uint16_t x, y;
  boolean touched = tft.getTouch(&x, &y);
  if(touched && touchedRect(x, y, &rAlarmOk)) {
    Serial.print("ok touched");
    return true;
  }
  return false;
}