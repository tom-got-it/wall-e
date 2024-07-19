const rect setupTop1 = {50, 80, 50, -35};
const rect setupTop2 = {125, 80, 50, -35};
const rect setupTop3 = {200, 80, 50, -35};

const rect setupBottom1 = {50, 170, 50, 35};
const rect setupBottom2 = {125, 170, 50, 35};
const rect setupBottom3 = {200, 170, 50, 35};

const rect rSetupReset = {40, 90, 240, 70};
const rect rSetupOk = {270, 190, 50, 50};
const rect rSetupLeft = {40, 10, -30, 30};
const rect rSetupRight = {280, 10, 30, 30};

const int setupClockX = 80;
const int setupClockY = 100;
const int setupDateX = 42;
const int setupDateY = 100;

const byte SETUP_MODE_EXIT_OK = 0;
const byte SETUP_MODE_ALARM = 1;
const byte SETUP_MODE_ALARM_MODE = 2;
const byte SETUP_MODE_ALARM_TIMEOUT = 3;
const byte SETUP_MODE_LIGHT = 4;
const byte SETUP_MODE_SNOOZE1 = 5;
const byte SETUP_MODE_SNOOZE2 = 6;
const byte SETUP_MODE_ALARM_TOUCH_ACTION = 7;
const byte SETUP_MODE_CLOCK = 8;
const byte SETUP_MODE_DATE = 9;

void showAlarmSetupScreen() {
  showSetupScreen(SETUP_MODE_ALARM);
}

void showClockSetupScreen() {
  showSetupScreen(SETUP_MODE_CLOCK);
}

void showSetupScreen(byte initialMode) {
  Serial.println("----Entering setup screen----");
  Serial.print("Initial mode: ");
  Serial.println(initialMode);

  byte mode = initialMode;
  while(true) {
    if(mode == SETUP_MODE_ALARM) {
      mode = doAlarmSetup();
      delay(150);
      continue;
    }

    if(mode == SETUP_MODE_ALARM_MODE) {
      mode = doAlarmModeSetup();
      delay(150);
      continue;
    }

    if(mode == SETUP_MODE_ALARM_TIMEOUT) {
      mode = doAlarmTimeoutSetup();
      delay(150);
      continue;
    }

    if(mode == SETUP_MODE_LIGHT) {
      mode = doLightSetup();
      delay(150);
      continue;
    }

    if(mode == SETUP_MODE_SNOOZE1) {
      mode = doSnooze1Setup();
      delay(150);
      continue;
    }

    if(mode == SETUP_MODE_SNOOZE2) {
      mode = doSnooze2Setup();
      delay(150);
      continue;
    }

    if(mode == SETUP_MODE_ALARM_TOUCH_ACTION) {
      mode = doAlarmTouchActionSetup();
      delay(150);
      continue;
    }

    if(mode == SETUP_MODE_CLOCK) {
      mode = doClockSetup();
      delay(150);
      continue;
    }

    if(mode == SETUP_MODE_DATE) {
      mode = doDateSetup();
      delay(150);
      continue;
    }

    Serial.println("----Leaving setup screen----");
    return;
  }
}

byte doAlarmSetup() {
  Serial.println("Setup alarm");

  printCommonItems(SETUP_MODE_ALARM);
  tft.drawString("Alarm", 112, 10, TFT_BIG_FONT);

  return executeSetupLoopAlarm();
}

byte doAlarmModeSetup() {
  Serial.println("Setup alarm mode");

  printCommonItems(SETUP_MODE_ALARM_MODE);
  tft.drawString("Alarm-Mode", 90, 10, TFT_BIG_FONT);

  return executeSetupLoopAlarmMode();
}

byte doAlarmTimeoutSetup() {
  Serial.println("Setup alarm timeout");

  printCommonItems(SETUP_MODE_ALARM_TIMEOUT);
  tft.drawString("Alarm-Timeout", 75, 10, TFT_BIG_FONT);

  return executeSetupLoopAlarmTimeout();
}

byte doLightSetup() {
  Serial.println("Setup light bulb on alarm");

  printCommonItems(SETUP_MODE_LIGHT);
  tft.drawString("Alarm-Light", 90, 10, TFT_BIG_FONT);

  return executeSetupLoopLight();
}

byte doSnooze1Setup() {
  Serial.println("Setup snooze option 1");

  printCommonItems(SETUP_MODE_SNOOZE1);
  tft.drawString("Snooze 1", 97, 10, TFT_BIG_FONT);

  return executeSetupLoopSnooze(&pSnoozeMinutes1, true);
}

byte doSnooze2Setup() {
  Serial.println("Setup snooze option 2");

  printCommonItems(SETUP_MODE_SNOOZE2);
  tft.drawString("Snooze 2", 97, 10, TFT_BIG_FONT);

  return executeSetupLoopSnooze(&pSnoozeMinutes2, false);
}

byte doAlarmTouchActionSetup() {
  Serial.println("Setup alarm touch action");

  printCommonItems(SETUP_MODE_ALARM_TOUCH_ACTION);
  tft.drawString("Display touch", 75, 10, TFT_BIG_FONT);

  return executeSetupLoopAlarmTouchAction();
}


byte doClockSetup() {
  Serial.println("Setup clock");

  printCommonItems(SETUP_MODE_CLOCK);
  tft.drawString("Time", 120, 10, TFT_BIG_FONT);

  return executeSetupLoopClock();
}

byte doDateSetup() {
  Serial.println("Setup date");

  printCommonItems(SETUP_MODE_DATE);
  tft.drawString("Date", 120, 10, TFT_BIG_FONT);

  return executeSetupLoopDate();
}

void printCommonItems(byte mode) {
  clearScreen();

  printRect(&rSetupOk, TFT_MAIN_COLOR, false);
  tft.drawString("OK", 276, 205, TFT_BIG_FONT);

  switch(mode) {
    case SETUP_MODE_ALARM:
      printTriangleUpDown(&setupTop1, TFT_MAIN_COLOR);
      printTriangleUpDown(&setupBottom1, TFT_MAIN_COLOR);
      printTriangleUpDown(&setupTop3, TFT_MAIN_COLOR);
      printTriangleUpDown(&setupBottom3, TFT_MAIN_COLOR);
      printTriangleLeftRight(&rSetupRight, TFT_MAIN_COLOR);
      break;
    case SETUP_MODE_ALARM_MODE:
      printTriangleUpDown(&setupTop2, TFT_MAIN_COLOR);
      printTriangleUpDown(&setupBottom2, TFT_MAIN_COLOR);
      printTriangleLeftRight(&rSetupLeft, TFT_MAIN_COLOR);
      printTriangleLeftRight(&rSetupRight, TFT_MAIN_COLOR);
      break;
    case SETUP_MODE_ALARM_TIMEOUT:
    case SETUP_MODE_LIGHT:
    case SETUP_MODE_SNOOZE1:
    case SETUP_MODE_SNOOZE2:
      printTriangleUpDown(&setupTop2, TFT_MAIN_COLOR);
      printTriangleUpDown(&setupBottom2, TFT_MAIN_COLOR);
      printTriangleLeftRight(&rSetupLeft, TFT_MAIN_COLOR);
      printTriangleLeftRight(&rSetupRight, TFT_MAIN_COLOR);
      break;
    case SETUP_MODE_ALARM_TOUCH_ACTION:
      printTriangleUpDown(&setupTop2, TFT_MAIN_COLOR);
      printTriangleUpDown(&setupBottom2, TFT_MAIN_COLOR);
      printTriangleLeftRight(&rSetupLeft, TFT_MAIN_COLOR);
      break;
    case SETUP_MODE_CLOCK:
      printTriangleUpDown(&setupTop1, TFT_MAIN_COLOR);
      printTriangleUpDown(&setupBottom1, TFT_MAIN_COLOR);
      printTriangleUpDown(&setupTop3, TFT_MAIN_COLOR);
      printTriangleUpDown(&setupBottom3, TFT_MAIN_COLOR);
      printTriangleLeftRight(&rSetupRight, TFT_MAIN_COLOR);
      break;
    case SETUP_MODE_DATE:
      printTriangleUpDown(&setupTop1, TFT_MAIN_COLOR);
      printTriangleUpDown(&setupBottom1, TFT_MAIN_COLOR);
      printTriangleUpDown(&setupTop2, TFT_MAIN_COLOR);
      printTriangleUpDown(&setupBottom2, TFT_MAIN_COLOR);
      printTriangleUpDown(&setupTop3, TFT_MAIN_COLOR);
      printTriangleUpDown(&setupBottom3, TFT_MAIN_COLOR);
      printTriangleLeftRight(&rSetupLeft, TFT_MAIN_COLOR);
      break;
  }
}

byte executeSetupLoopAlarm() {
  DateTime dt = getAlarmClock();
  int hour = dt.hour();
  int minute = dt.minute();

  boolean reprint = true;
  boolean clockChanged = false;
  while(true) {
    //Primt time
    if(reprint) {
      printRect(&rSetupReset, TFT_BLACK, true);
      int cx = setupClockX;
      cx = drawTwoDigitsAndGetXPos(hour, cx, setupClockY, TFT_MAIN_FONT);
      cx += tft.drawChar(':', cx, setupClockY, TFT_MAIN_FONT);
      cx = drawTwoDigitsAndGetXPos(minute, cx, setupClockY, TFT_MAIN_FONT);
      reprint = false;
    }

    uint16_t x,y;
    boolean touched = tft.getTouch(&x, &y);
    if(touched && touchedRect(x, y, &setupTop1)) {
      hour = hour + 1;
      if (hour == 24) hour = 0;
      clockChanged = true;
      reprint = true;
    }

    if(touched && touchedRect(x, y, &setupTop3)) {
      minute = minute + 1;
      if (minute == 60) {
        minute = 0;
        hour = hour + 1;
        if (hour == 24) hour = 0;
      }
      clockChanged = true;
      reprint = true;
    }

    if(touched && touchedRect(x, y, &setupBottom1)) {
      hour = hour - 1;
      if (hour < 0) hour = 23;
      clockChanged = true;
      reprint = true;
    }

    if(touched && touchedRect(x, y, &setupBottom3)) {
      minute = minute - 1;
      if (minute < 0) {
        minute = 59;
        hour = hour - 1;
        if (hour < 0) hour = 23;
      }
      clockChanged = true;
      reprint = true;
    }

    if(touched && touchedRect(x, y, &rSetupOk)) {
      if(clockChanged) {
        DateTime alarm = getAlarmClock();
        DateTime dt = DateTime(alarm.year(), alarm.month(), alarm.day(), hour, minute, 0);
        setAlarmClock(dt);
      }
      return SETUP_MODE_EXIT_OK;
    }

    if(touchedRect(x, y, &rSetupRight)) {
      if(clockChanged) {
        DateTime alarm = getAlarmClock();
        DateTime dt = DateTime(alarm.year(), alarm.month(), alarm.day(), hour, minute, 0);
        setAlarmClock(dt);
      }
      return SETUP_MODE_ALARM_MODE;
    }

    if(touched) {
      delay(80);
    }
  }
}

byte executeSetupLoopAlarmMode() {
  boolean reprint = true;
  int mode = getAlarmClockMode();
  boolean changed = false;

  while(true) {
    if(reprint) {
      printRect(&rSetupReset, TFT_BLACK, true);
      tft.drawString(parseAlarmClockModeString(mode), 95, 115, TFT_BIG_FONT);
      reprint = false;
    }

    uint16_t x,y;
    boolean touched = tft.getTouch(&x, &y);

    if(touched && touchedRect(x, y, &setupTop2)) {
      mode = mode + 1;
      if(mode > getAlarmClockModeMaxValue()) {
        mode = getAlarmClockModeMinValue();
      }
      reprint = true;
      changed = true;
    }

    if(touched && touchedRect(x, y, &setupBottom2)) {
      mode = mode -1;
      if(mode < getAlarmClockModeMinValue()) {
        mode = getAlarmClockModeMaxValue();
      }
      reprint = true;
      changed = true;
    }


    if(touched && touchedRect(x, y, &rSetupOk)) {
      if(changed) {
        setAlarmClockModeAndEnable(mode);
      }
      return SETUP_MODE_EXIT_OK;
    }


    if(touched && touchedRect(x, y, &rSetupLeft)) {
      if(changed) {
        setAlarmClockModeAndEnable(mode);
      }
      return SETUP_MODE_ALARM;
    }

    if(touched && touchedRect(x, y, &rSetupRight)) {
      if(changed) {
        setAlarmClockModeAndEnable(mode);
      }
      return SETUP_MODE_ALARM_TIMEOUT;
    }

    if(touched) {
      delay(80);
    }
  }
}

byte executeSetupLoopAlarmTimeout() {
  boolean reprint = true;
  boolean changed = false;
  while(true) {
    if(reprint) {
      printRect(&rSetupReset, TFT_BLACK, true);
      String str = "after ";
      str = str + pAlarmTimeoutMinutes;
      str = str + " minutes";
      tft.drawString(str, 65, 115, TFT_BIG_FONT);
      reprint = false;
    }

    uint16_t x,y;
    boolean touched = tft.getTouch(&x, &y);

    if(touched && touchedRect(x, y, &setupTop2)) {
      pAlarmTimeoutMinutes = pAlarmTimeoutMinutes + 1;
      if(pAlarmTimeoutMinutes > 20) {
        pAlarmTimeoutMinutes = 1;
      }
      changed = true;
      reprint = true;
    }

    if(touched && touchedRect(x, y, &setupBottom2)) {
      pAlarmTimeoutMinutes = pAlarmTimeoutMinutes - 1;
      if(pAlarmTimeoutMinutes < 1) {
        pAlarmTimeoutMinutes = 20;
      }
      changed = true;
      reprint = true;
    }


    if(touched && touchedRect(x, y, &rSetupOk)) {
      if(changed) {
        savePrefs();
      }
      validateAlarmLightSecondsMaxValue(true);
      return SETUP_MODE_EXIT_OK;
    }

    if(touched && touchedRect(x, y, &rSetupLeft)) {
      if(changed) {
        savePrefs();
      }
      validateAlarmLightSecondsMaxValue(true);
      return SETUP_MODE_ALARM_MODE;
    }

    if(touched && touchedRect(x, y, &rSetupRight)) {
      if(changed) {
        savePrefs();
      }
      validateAlarmLightSecondsMaxValue(true);
      return SETUP_MODE_LIGHT;
    }

    if(touched) {
      delay(80);
    }
  }
}

byte executeSetupLoopLight() {
  boolean reprint = true;
  boolean changed = false;
  while(true) {
    if(reprint) {
      printRect(&rSetupReset, TFT_BLACK, true);

      if(pLightBulbDelaySecondsOnAlarm < 0) {
        tft.drawString("off", 130, 115, TFT_BIG_FONT);
      } else if (pLightBulbDelaySecondsOnAlarm == 0) {
        tft.drawString("on", 130, 115, TFT_BIG_FONT);
      } else {
        String str = "after ";
        str = str + pLightBulbDelaySecondsOnAlarm;
        str = str + " seconds";
        tft.drawString(str, 55, 115, TFT_BIG_FONT);
      }
      reprint = false;
    }

    uint16_t x,y;
    boolean touched = tft.getTouch(&x, &y);

    if(touched && touchedRect(x, y, &setupTop2)) {
      pLightBulbDelaySecondsOnAlarm = pLightBulbDelaySecondsOnAlarm + 1;
      if(! validateAlarmLightSecondsMaxValue(false)) {
        pLightBulbDelaySecondsOnAlarm = -1;
      }
      changed = true;
      reprint = true;
    }

    if(touched && touchedRect(x, y, &setupBottom2)) {
      pLightBulbDelaySecondsOnAlarm = pLightBulbDelaySecondsOnAlarm - 1;
      if(pLightBulbDelaySecondsOnAlarm < - 1) {
        pLightBulbDelaySecondsOnAlarm = pAlarmTimeoutMinutes * 60;
        validateAlarmLightSecondsMaxValue(true);
      }
      changed = true;
      reprint = true;
    }


    if(touched && touchedRect(x, y, &rSetupOk)) {
      if(changed) {
        savePrefs();
      }
      return SETUP_MODE_EXIT_OK;
    }

    if(touched && touchedRect(x, y, &rSetupLeft)) {
      if(changed) {
        savePrefs();
      }
      return SETUP_MODE_ALARM_TIMEOUT;
    }

    if(touched && touchedRect(x, y, &rSetupRight)) {
      if(changed) {
        savePrefs();
      }
      return SETUP_MODE_SNOOZE1;
    }

    if(touched) {
      delay(80);
    }
  }
}

byte executeSetupLoopSnooze(int* snoozeMinutesPtr, boolean isSnooze1) {
  boolean reprint = true;
  boolean changed = false;
  while(true) {
    if(reprint) {
      printRect(&rSetupReset, TFT_BLACK, true);
      String str = "";
      str = str + *snoozeMinutesPtr;
      str = str + " minutes";
      tft.drawString(str, 88, 115, TFT_BIG_FONT);
      reprint = false;
    }

    uint16_t x,y;
    boolean touched = tft.getTouch(&x, &y);

    if(touched && touchedRect(x, y, &setupTop2)) {
      *snoozeMinutesPtr = *snoozeMinutesPtr + 1;
      if(*snoozeMinutesPtr > 59) {
        *snoozeMinutesPtr = 1;
      }
      changed = true;
      reprint = true;
    }

    if(touched && touchedRect(x, y, &setupBottom2)) {
      *snoozeMinutesPtr = *snoozeMinutesPtr - 1;
      if(*snoozeMinutesPtr < 1) {
        *snoozeMinutesPtr = 59;
      }
      changed = true;
      reprint = true;
    }


    if(touched && touchedRect(x, y, &rSetupOk)) {
      if(changed) {
        savePrefs();
      }
      return SETUP_MODE_EXIT_OK;
    }

    if(touched && touchedRect(x, y, &rSetupLeft)) {
      if(changed) {
        savePrefs();
      }
      if(isSnooze1) {
        return SETUP_MODE_LIGHT;
      } else {
        return SETUP_MODE_SNOOZE1;
      }
    }

    if(touched && touchedRect(x, y, &rSetupRight)) {
      if(changed) {
        savePrefs();
      }
      if(isSnooze1) {
        return SETUP_MODE_SNOOZE2;
      } else {
        return SETUP_MODE_ALARM_TOUCH_ACTION;
      }
    }

    if(touched) {
      delay(80);
    }
  }
}

byte executeSetupLoopAlarmTouchAction() {
  boolean reprint = true;
  boolean changed = false;

  int touchOptions[] = {
    ALARM_EXIT_NONE,
    ALARM_EXIT_TOUCH,
    ALARM_EXIT_SNOOZE1,
    ALARM_EXIT_SNOOZE2
  };
  int optionsLength = 4;

  int optionsIndex = 0;
  for(int i = 0; i < optionsLength; i++) {
    if(touchOptions[i] == pAlarmTouchAction) {
      optionsIndex = i;
    }
  }

  while(true) {
    if(reprint) {
      printRect(&rSetupReset, TFT_BLACK, true);
      String str = "";
      switch(pAlarmTouchAction) {
        case ALARM_EXIT_NONE:
          str = "do nothing";
          break;
        case ALARM_EXIT_TOUCH:
          str = "exit alarm";
          break;
        case ALARM_EXIT_SNOOZE1:
          str = "snooze (1)";
          break;
        case ALARM_EXIT_SNOOZE2:
          str = "snooze (2)";
          break;
      }
      tft.drawString(str, 88, 115, TFT_BIG_FONT);
      reprint = false;
    }

    uint16_t x,y;
    boolean touched = tft.getTouch(&x, &y);

    if(touched && touchedRect(x, y, &setupTop2)) {
      optionsIndex = optionsIndex + 1;
      if(optionsIndex >= optionsLength) {
        optionsIndex = 0;
      }
      pAlarmTouchAction = touchOptions[optionsIndex];
      changed = true;
      reprint = true;
    }

    if(touched && touchedRect(x, y, &setupBottom2)) {
      optionsIndex = optionsIndex - 1;
      if(optionsIndex < 0) {
        optionsIndex = optionsLength - 1;
      }
      pAlarmTouchAction = touchOptions[optionsIndex];
      changed = true;
      reprint = true;
    }

    if(touched && touchedRect(x, y, &rSetupOk)) {
      if(changed) {
        savePrefs();
      }
      return SETUP_MODE_EXIT_OK;
    }

    if(touched && touchedRect(x, y, &rSetupLeft)) {
      if(changed) {
        savePrefs();
      }
      return SETUP_MODE_SNOOZE2;
    }

    if(touched) {
      delay(80);
    }
  }
}

byte executeSetupLoopClock() {
  DateTime dt = rtc.now();
  int hour = dt.hour();
  int minute = dt.minute();

  boolean reprint = true;
  boolean clockChanged = false;
  while(true) {
    //Primt time
    if(reprint) {
      printRect(&rSetupReset, TFT_BLACK, true);
      int cx = setupClockX;
      cx = drawTwoDigitsAndGetXPos(hour, cx, setupClockY, TFT_MAIN_FONT);
      cx += tft.drawChar(':', cx, setupClockY, TFT_MAIN_FONT);
      cx = drawTwoDigitsAndGetXPos(minute, cx, setupClockY, TFT_MAIN_FONT);
      reprint = false;
    }

    uint16_t x,y;
    boolean touched = tft.getTouch(&x, &y);

    if(touched && touchedRect(x, y, &setupTop1)) {
      hour = hour + 1;
      if (hour == 24) hour = 0;
      clockChanged = true;
      reprint = true;
    }

    if(touched && touchedRect(x, y, &setupTop3)) {
      minute = minute + 1;
      if (minute == 60) {
        minute = 0;
        hour = hour + 1;
        if (hour == 24) hour = 0;
      }
      clockChanged = true;
      reprint = true;
    }

    if(touched && touchedRect(x, y, &setupBottom1)) {
      hour = hour - 1;
      if (hour < 0) hour = 23;
      clockChanged = true;
      reprint = true;
    }

    if(touched && touchedRect(x, y, &setupBottom3)) {
      minute = minute - 1;
      if (minute < 0) {
        minute = 59;
        hour = hour - 1;
        if (hour < 0) hour = 23;
      }
      clockChanged = true;
      reprint = true;
    }

    if(touched && touchedRect(x, y, &rSetupOk)) {
      if(clockChanged) {
        //Adjust RTC
        DateTime now = rtc.now();
        adjustClockAndSetupTimezoneHandling(DateTime(now.year(), now.month(), now.day(), hour, minute, 0));
      }
      return SETUP_MODE_EXIT_OK;
    }

    if(touched && touchedRect(x, y, &rSetupRight)) {
      if(clockChanged) {
        //Adjust RTC
        DateTime now = rtc.now();
        adjustClockAndSetupTimezoneHandling(DateTime(now.year(), now.month(), now.day(), hour, minute, 0));
      }
      return SETUP_MODE_DATE;
    }

    if(touched) {
      delay(80);
    }
  }
}

byte executeSetupLoopDate() {
  DateTime dt = rtc.now();
  int day = dt.day();
  int month = dt.month();
  int year = dt.year() - 2000;

  boolean reprint = true;
  boolean clockChanged = false;
  while(true) {
    //Primt time
    if(reprint) {
      printRect(&rSetupReset, TFT_BLACK, true);
      int cx = setupDateX;
      cx = drawTwoDigitsAndGetXPos(day, cx, setupClockY, TFT_MAIN_FONT);
      cx += tft.drawChar('.', cx, setupClockY, TFT_MAIN_FONT);
      cx = drawTwoDigitsAndGetXPos(month, cx, setupClockY, TFT_MAIN_FONT);
      cx += tft.drawChar('.', cx, setupClockY, TFT_MAIN_FONT);
      cx = drawTwoDigitsAndGetXPos(year, cx, setupClockY, TFT_MAIN_FONT);
      reprint = false;
    }

    uint16_t x,y;
    boolean touched = tft.getTouch(&x, &y);

    if(touched && touchedRect(x, y, &setupTop1)) {
      day = day + 1;
      if (day == 32) day = 1;
      clockChanged = true;
      reprint = true;
    }

    if(touched && touchedRect(x, y, &setupTop2)) {
      month = month + 1;
      if (month == 13) month = 1;
      clockChanged = true;
      reprint = true;
    }

    if(touched && touchedRect(x, y, &setupTop3)) {
      year = year + 1;
      if (year == 100) year = 0;
      clockChanged = true;
      reprint = true;
    }

    if(touched && touchedRect(x, y, &setupBottom1)) {
      day = day - 1;
      if (day <= 0) day = 31;
      clockChanged = true;
      reprint = true;
    }

    if(touched && touchedRect(x, y, &setupBottom2)) {
      month = month - 1;
      if (month <= 0) month = 12;
      clockChanged = true;
      reprint = true;
    }

    if(touched && touchedRect(x, y, &setupBottom3)) {
      year = year - 1;
      if (year < 0) year = 99;
      clockChanged = true;
      reprint = true;
    }

    if(touched && touchedRect(x, y, &rSetupOk)) {
      if(clockChanged) {
        //Adjust RTC
        DateTime now = rtc.now();
        adjustClockAndSetupTimezoneHandling(DateTime(year + 2000, month, day, now.hour(), now.minute(), now.second()));
      }
      return SETUP_MODE_EXIT_OK;
    }


    if(touched && touchedRect(x, y, &rSetupLeft)) {
      if(clockChanged) {
        //Adjust RTC
        DateTime now = rtc.now();
        adjustClockAndSetupTimezoneHandling(DateTime(year + 2000, month, day, now.hour(), now.minute(), now.second()));
      }
      return SETUP_MODE_CLOCK;
    }

    if(touched) {
      delay(80);
    }
  }
}

boolean validateAlarmLightSecondsMaxValue(boolean autoCorrect) {
  int maxSeconds = (pAlarmTimeoutMinutes * 60) - 5;
  if(pLightBulbDelaySecondsOnAlarm > maxSeconds) {
    if(autoCorrect) {
      pLightBulbDelaySecondsOnAlarm = maxSeconds;
    } else {
      return false;
    }
  }
  return true;
}

