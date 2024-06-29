const rect rAlarmOk = {80, 170, 160, 50};
const int xAlarmOk = 143;
const int yAlarmOk = 185;
const int xAlarmRangeText[] = {10, 200};
const int yAlarmRangeText[] = {10, 130};
const rect rAlarmResetWakeup = {0, 0, 320, 170};
const int alarmRefreshMessageSeconds = 5;

DateTime alarmStart;
DateTime alarmLastWakeupPrint;

void showAlarmScreenClock() {
  Serial.println("----Entering alarm-Screen (clock) - Alarm has fired----");

  clearAlarmClockFlag();
  missedAlarmNotification = handleAlarmScreenAndIsMissedAlarm();
  handleExit();
}

void showAlarmScreenTimer() {
  Serial.println("----Entering alarm-Screen (timer)----");
  handleAlarmScreenAndIsMissedAlarm();
  handleExit();
}


boolean handleAlarmScreenAndIsMissedAlarm() {
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
  printAlarmText();

  while(true) {
    if(isExitByTouch()) {
      return false;
    }

    boolean isPlaying = isMp3Playing();
    DateTime now = rtc.now();

    testLightBulbStillActive();
    if(! lightBulbWasActive && lightBulbDelaySecondsOnAlarm > 0 && alarmStart.unixtime() + lightBulbDelaySecondsOnAlarm <= now.unixtime()) {
      lightBulbWasActive = startLightBulb();
    }

    if(isExitByTouch()) {
      return false;
    }

    if(alarmStart.unixtime() + gAlarmAutoShutdownSeconds < now.unixtime()) {
      Serial.println("Stop the alarm due to timeout - user did not react.");
      return true;
    } else {
      if(! isPlaying) {
        playMp3RandomAlarm();
      }
    }

    if(isExitByTouch()) {
      return false;
    }

    if(alarmLastWakeupPrint.unixtime() + alarmRefreshMessageSeconds < rtc.now().unixtime()) {
      Serial.println("print wake-up message");
      printAlarmText();
    }
  }

  //should never happen
  return true;
}

void printAlarmText() {
  printRect(&rAlarmResetWakeup, TFT_BLACK, true);
  int wx = random(xAlarmRangeText[0], xAlarmRangeText[1]);
  int wy = random(yAlarmRangeText[0], yAlarmRangeText[1]);
  tft.drawString("Wall-eee", wx, wy, TFT_BIG_FONT);
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