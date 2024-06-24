const rect rAlarmOk = {80, 170, 160, 50};
const int xAlarmOk = 143;
const int yAlarmOk = 185;
const int xAlarmRangeWakeUp[] = {10, 200};
const int yAlarmRangeWakeUp[] = {10, 130};
const rect rAlarmResetWakeup = {0, 0, 320, 170};
const int alarmRefreshMessageSeconds = 5;

DateTime alarmStart;
DateTime alarmLastWakeupPrint;

void showAlarmScreen() {
  Serial.println("----Entering alarm-Screen - Alarm has fired----");

  clearAlarmClockFlag();
  missedAlarmNotification = false;

  alarmStart = rtc.now();
  clearScreen();

  boolean lightBulbWasActive = false;
  if(lightBulbDelaySecondsOnAlarm == 0) {
    lightBulbWasActive = startLightBulb();
  }

  playMp3RandomAlarm();
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

    boolean isPlaying = isMp3Playing();
    DateTime now = rtc.now();

    testLightBulbStillActive();
    if(! lightBulbWasActive && lightBulbDelaySecondsOnAlarm > 0 && alarmStart.unixtime() + lightBulbDelaySecondsOnAlarm <= now.unixtime()) {
      lightBulbWasActive = startLightBulb();
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
        playMp3RandomAlarm();
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
    Serial.println("ok touched - exiting alarm screen");
    return true;
  }
  return false;
}