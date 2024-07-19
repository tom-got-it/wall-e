//Buttons
const rect rAlarmOk = {10, 170, 140, 50};
const rect rAlarmSnooze1 = {170, 170, 65, 50};
const rect rAlarmSnooze2 = {245, 170, 65, 50};
const int xAlarmOk = 50;
const int yAlarmOk = 185;
const int xAlarmSnooze1 = 180;
const int yAlarmSnooze1 = 185;
const int xAlarmSnooze2 = 255;
const int yAlarmSnooze2 = 185;

//Alarm Text
const int xAlarmRangeText[] = {10, 200};
const int yAlarmRangeText[] = {10, 130};
const rect rAlarmResetWakeup = {0, 0, 320, 170};
const int alarmRefreshMessageSeconds = 5;
DateTime alarmStart;
DateTime alarmLastWakeupPrint;

//Clock while snoozing
const rect rAlarmClock = {85, 20, 235, 60};
DateTime alarmClockLastUpdate;
int alarmClockColonXPos = -1;

//Animation and text while snoozing
const rect rAlarmSnoozeText = {10, 112, 60, 16};
const rect rAlarmSnoozeAnimation = {70, 111, 240, 18};
int alarmLastAnimationWidth = 0;

//Display control
boolean alarmDisplayIsOn = true;
DateTime alarmLastTouched;


void showAlarmScreenClock() {
  Serial.println("----Entering alarm-Screen (clock) - Alarm has fired----");
  clearAlarmClockFlag();
  missedAlarmNotification = handleAlarmAndIsMissedAlarm();
  showEmptyDelayScreen(50); //prohibit accidental touch events on exit
  Serial.println("----Leaving alarm screen----");
}

void showAlarmScreenTimer() {
  Serial.println("----Entering alarm-Screen (timer)----");
  handleAlarmAndIsMissedAlarm();
  showEmptyDelayScreen(50); //prohibit accidental touch events on exit
  Serial.println("----Leaving alarm screen----");
}

boolean handleAlarmAndIsMissedAlarm() {
  boolean showSnoozeScreen = false;
  int snoozeMinutes;
  while(true) {
    int exitCode = ALARM_EXIT_NONE;
    if(showSnoozeScreen) {
      exitCode = handleAlarmSnoozeScreen(snoozeMinutes);
      handleSnoozeScreenExit();
    } else {
      exitCode = handleAlarmScreen();
      handleAlarmScreenExit();
    }
    
    switch(exitCode) {
      case ALARM_EXIT_TOUCH:
        return false;
      case ALARM_EXIT_MISSED:
        return true;
      case ALARM_EXIT_SNOOZE1:
        snoozeMinutes = pSnoozeMinutes1;
        showSnoozeScreen = true;
        break;
      case ALARM_EXIT_SNOOZE2:
        snoozeMinutes = pSnoozeMinutes2;
        showSnoozeScreen = true;
        break;
      case ALARM_EXIT_SNOOZE_END:
        showSnoozeScreen = false;
        break;
    }
  }

  //should never happen
  return false;
}

int handleAlarmScreen() {
  Serial.println("showing active alarm...");
  alarmStart = rtc.now();
  clearScreen();

  boolean lightBulbWasActive = false;
  if(pLightBulbDelaySecondsOnAlarm == 0) {
    lightBulbWasActive = startLightBulb();
  }

  playMp3RandomAlarm();
  drawWalleGifMain();
  delay(1000);

  clearScreen();
  printAlarmOkButton();
  printAlarmSnoozeButtons();
  printAlarmText();

  alarmLastTouched = rtc.now();
  while(true) {
    DateTime now = rtc.now();

    testLightBulbStillActive();
    if(! lightBulbWasActive && pLightBulbDelaySecondsOnAlarm > 0 && alarmStart.unixtime() + pLightBulbDelaySecondsOnAlarm <= now.unixtime()) {
      lightBulbWasActive = startLightBulb();
    }

    if(alarmStart.unixtime() + (pAlarmTimeoutMinutes * 60) < now.unixtime()) {
      Serial.println("Stop the alarm due to timeout - user did not react.");
      return ALARM_EXIT_MISSED;
    } else {
      //We only test if MP3 is still playing when the MP3 module is ready to receive the next command.
      //This makes the alarm-loop more responsive for touch events
      if(waitMp3Ready(true)) {
        if(!isMp3Playing()) {
          playMp3RandomAlarm();
        }
      }
    }

    int exitMode = handleAlarmTouchEvents(true);
    if(exitMode != ALARM_EXIT_NONE) {
      return exitMode;
    }

    if(alarmLastWakeupPrint.unixtime() + alarmRefreshMessageSeconds < rtc.now().unixtime()) {
      Serial.println("print wake-up message");
      printAlarmText();
    }
  }

  //should never happen
  return ALARM_EXIT_TOUCH;
}

int handleAlarmSnoozeScreen(int minutes) {
  Serial.print("start snoozing for ");
  Serial.print(minutes);
  Serial.println(" minutes...");

  clearScreen();
  printAlarmSnoozeBase();
  printAlarmOkButton();
  printAlarmSnoozeButtons();
  printAlarmClock(true);

  DateTime snoozeStart = rtc.now();
  DateTime snoozeEnd = DateTime(snoozeStart.unixtime() + (minutes * 60));
  alarmLastTouched = rtc.now();

  delay(50);
  while(rtc.now().unixtime() < snoozeEnd.unixtime()) {
    int exitMode = handleAlarmTouchEvents(false);
    if(exitMode != ALARM_EXIT_NONE) {
      return exitMode;
    }

    updateAlarmSnoozeAnimation(snoozeStart, snoozeEnd);
    printAlarmClock(false);

    alarmTestDisplayShutdown();
    delay(120);
  }

  return ALARM_EXIT_SNOOZE_END;
}

void handleAlarmScreenExit() {
  stopMp3Playback();
  disableLightBulb();
}

void handleSnoozeScreenExit() {
  turnOnDisplay();
  alarmDisplayIsOn = true;
}

int handleAlarmTouchEvents(boolean isAlarmScreen) {
  uint16_t x, y;
  boolean touched = tft.getTouch(&x, &y);
  if(touched) {
    alarmLastTouched = rtc.now();
  }

  if(touched && !alarmDisplayIsOn) {
    alarmDisplayIsOn = true;
    turnOnDisplay();
    delay(120);
    return ALARM_EXIT_NONE;
  }

  if(touched && touchedRect(x, y, &rAlarmOk)) {
    Serial.println("close touched");
    return ALARM_EXIT_TOUCH;
  }
  if(touched && touchedRect(x, y, &rAlarmSnooze1)) {
    Serial.println("snooze 1 touched");
    return ALARM_EXIT_SNOOZE1;
  }
  if(touched && touchedRect(x, y, &rAlarmSnooze2)) {
    Serial.println("snooze 2 touched");
    return ALARM_EXIT_SNOOZE2;
  }

  //No button touched
  if(touched && isAlarmScreen) {
    return pAlarmTouchAction;
  }

  return ALARM_EXIT_NONE;
}

void printAlarmText() {
  printRect(&rAlarmResetWakeup, TFT_BLACK, true);
  int wx = random(xAlarmRangeText[0], xAlarmRangeText[1]);
  int wy = random(yAlarmRangeText[0], yAlarmRangeText[1]);
  tft.drawString("Wall-eee", wx, wy, TFT_BIG_FONT);
  alarmLastWakeupPrint = rtc.now();
}

void printAlarmOkButton() {
  printRect(&rAlarmOk, TFT_MAIN_COLOR, true);
  tft.setTextColor(TFT_BLACK);
  tft.drawString("Close", xAlarmOk, yAlarmOk, TFT_BIG_FONT);
  tft.setTextColor(TFT_MAIN_COLOR);
}

void printAlarmSnoozeButtons() {
  printRect(&rAlarmSnooze1, TFT_MAIN_COLOR, false);
  tft.drawString("+" + padByZeroTwoDigits(pSnoozeMinutes1), xAlarmSnooze1, yAlarmSnooze1, TFT_BIG_FONT);

  printRect(&rAlarmSnooze2, TFT_MAIN_COLOR, false);
  tft.drawString("+" + padByZeroTwoDigits(pSnoozeMinutes2), xAlarmSnooze2, yAlarmSnooze2, TFT_BIG_FONT);
}

void printAlarmSnoozeBase() {
   tft.drawString("zzzzZZZ", rAlarmSnoozeText.x, rAlarmSnoozeText.y, TFT_SMALL_FONT);
   printRect(&rAlarmSnoozeAnimation, TFT_MAIN_COLOR, false);
   alarmLastAnimationWidth = 0;
}

void updateAlarmSnoozeAnimation(DateTime snoozeStart, DateTime snoozeEnd) {
  DateTime now = rtc.now();
  int width = map(
    now.unixtime(), 
    snoozeStart.unixtime(), 
    snoozeEnd.unixtime(), 
    0,
    rAlarmSnoozeAnimation.width);
  if(width != alarmLastAnimationWidth) {
    rect r = {rAlarmSnoozeAnimation.x, rAlarmSnoozeAnimation.y, width, rAlarmSnoozeAnimation.height};
    printRect(&r, TFT_MAIN_COLOR, true);
    alarmLastAnimationWidth = width;
  }
}

void printAlarmClock(boolean force) {
    DateTime now = rtc.now();

    uint8_t hour = now.hour();
    uint8_t minute = now.minute();
    uint8_t second = now.second();

    int xPos = rAlarmClock.x;
    int yPos = rAlarmClock.y;

    if(force || alarmClockLastUpdate.second() != second) {
        boolean reprintAll = force || hour != alarmClockLastUpdate.hour();
        boolean reprintMinutes = reprintAll || minute != alarmClockLastUpdate.minute();

        if(reprintAll) {
          printRect(&rAlarmClock, TFT_BLACK, true);
          xPos = drawTwoDigitsAndGetXPos(hour, xPos, yPos, TFT_MAIN_FONT);
          alarmClockColonXPos = xPos;
        } else {
          xPos = alarmClockColonXPos;
        }

        //Draw flashing colon (separator of hour and minutes)
        if(second % 2) {
          tft.setTextColor(TFT_BLACK, TFT_BLACK);
          xPos += tft.drawChar(':', xPos, yPos, TFT_MAIN_FONT);
          tft.setTextColor(TFT_MAIN_COLOR, TFT_BLACK);
        } else {
          xPos += tft.drawChar(':', xPos, yPos, TFT_MAIN_FONT);
        }

        if(reprintMinutes) {
          const rect clear = {xPos, rAlarmClock.y, 320 - xPos, rAlarmClock.height};
          printRect(&clear, TFT_BLACK, true);
          drawTwoDigitsAndGetXPos(minute, xPos, yPos, TFT_MAIN_FONT);
        }
        
        alarmClockLastUpdate = now;
    }
}

void alarmTestDisplayShutdown() {
  if(!alarmDisplayIsOn) {
    return;
  }

  if(alarmLastTouched.unixtime() + gSnoozeDisplayAutoShutdownSeconds <= rtc.now().unixtime()) {
    turnOffDisplay();
    alarmDisplayIsOn = false;
  }
}