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
const byte SETUP_MODE_LIGHT = 2;
const byte SETUP_MODE_CLOCK = 3;
const byte SETUP_MODE_DATE = 4;

void showSetupScreen() {
  Serial.println("----Entering clock setup screen----");
  byte mode = SETUP_MODE_ALARM;
  while(true) {
    if(mode == SETUP_MODE_ALARM) {
      mode = doAlarmSetup();
      delay(150);
      continue;
    }

    if(mode == SETUP_MODE_LIGHT) {
      mode = doLightSetup();
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

    //Clock might have changed -> set alarm for next DST change
    setupNextTimezoneAlarm();
    Serial.println("----Leaving clock setup screen----");
    return;
  }
}

byte doAlarmSetup() {
  Serial.println("Setup alarm");

  printCommonItems(SETUP_MODE_ALARM);
  tft.drawString("Alarm", 112, 10, TFT_BIG_FONT);

  return executeSetupLoopAlarm();
}

byte doLightSetup() {
  Serial.println("Setup light bulb on alarm");

  printCommonItems(SETUP_MODE_LIGHT);
  tft.drawString("Alarm-Light", 90, 10, TFT_BIG_FONT);

  return executeSetupLoopLight();
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
    case SETUP_MODE_LIGHT:
      printTriangleUpDown(&setupTop2, TFT_MAIN_COLOR);
      printTriangleUpDown(&setupBottom2, TFT_MAIN_COLOR);
      printTriangleLeftRight(&rSetupLeft, TFT_MAIN_COLOR);
      printTriangleLeftRight(&rSetupRight, TFT_MAIN_COLOR);
      break;
    case SETUP_MODE_CLOCK:
      printTriangleUpDown(&setupTop1, TFT_MAIN_COLOR);
      printTriangleUpDown(&setupBottom1, TFT_MAIN_COLOR);
      printTriangleUpDown(&setupTop3, TFT_MAIN_COLOR);
      printTriangleUpDown(&setupBottom3, TFT_MAIN_COLOR);
      printTriangleLeftRight(&rSetupLeft, TFT_MAIN_COLOR);
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
  DateTime dt = rtc.getAlarm1();
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

    if(touched && clockChanged) {
      DateTime alarm = rtc.getAlarm1();
      DateTime dt = DateTime(alarm.year(), alarm.month(), alarm.day(), hour, minute, 0);
      rtc.setAlarm1(dt, DS3231_A1_Hour);
      if(! isAlarmActive) {
        toggleAlarm();
      }
    }

    if(touched && touchedRect(x, y, &rSetupOk)) {
      return SETUP_MODE_EXIT_OK;
    }

    if(touchedRect(x, y, &rSetupRight)) {
      return SETUP_MODE_LIGHT;
    }

    if(touched) {
      delay(80);
    }
  }
}

byte executeSetupLoopLight() {
  boolean reprint = true;
  while(true) {
    if(reprint) {
      printRect(&rSetupReset, TFT_BLACK, true);

      if(lightBulbDelaySecondsOnAlarm < 0) {
        tft.drawString("off", 130, 115, TFT_BIG_FONT);
      } else if (lightBulbDelaySecondsOnAlarm == 0) {
        tft.drawString("on", 130, 115, TFT_BIG_FONT);
      } else {
        String str = "after ";
        str = str + lightBulbDelaySecondsOnAlarm;
        str = str + " seconds";
        tft.drawString(str, 55, 115, TFT_BIG_FONT);
      }
      reprint = false;
    }

    uint16_t x,y;
    boolean touched = tft.getTouch(&x, &y);

    if(touched && touchedRect(x, y, &setupTop2)) {
      lightBulbDelaySecondsOnAlarm = lightBulbDelaySecondsOnAlarm + 1;
      if(lightBulbDelaySecondsOnAlarm > gAlarmAutoShutdownSeconds - 5) {
        lightBulbDelaySecondsOnAlarm = -1;
      }
      reprint = true;
    }

    if(touched && touchedRect(x, y, &setupBottom2)) {
      lightBulbDelaySecondsOnAlarm = lightBulbDelaySecondsOnAlarm - 1;
      if(lightBulbDelaySecondsOnAlarm < - 1) {
        lightBulbDelaySecondsOnAlarm = gAlarmAutoShutdownSeconds - 5;
      }
      reprint = true;
    }


    if(touched && touchedRect(x, y, &rSetupOk)) {
      return SETUP_MODE_EXIT_OK;
    }


    if(touched && touchedRect(x, y, &rSetupLeft)) {
      return SETUP_MODE_ALARM;
    }

    if(touched && touchedRect(x, y, &rSetupRight)) {
      return SETUP_MODE_CLOCK;
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
        rtc.adjust(DateTime(now.year(), now.month(), now.day(), hour, minute, 0));

        //we do so as if the last timezone change occurred now such that the next timezone change is correctly determined
        now = rtc.now();
        lastTimezoneChange = now;
      }
      return SETUP_MODE_EXIT_OK;
    }


    if(touched && touchedRect(x, y, &rSetupLeft)) {
      if(clockChanged) {
        //Adjust RTC
        DateTime now = rtc.now();
        rtc.adjust(DateTime(now.year(), now.month(), now.day(), hour, minute, 0));

        //we do so as if the last timezone change occurred now such that the next timezone change is correctly determined
        now = rtc.now();
        lastTimezoneChange = now;
      }
      return SETUP_MODE_LIGHT;
    }

    if(touched && touchedRect(x, y, &rSetupRight)) {
      if(clockChanged) {
        //Adjust RTC
        DateTime now = rtc.now();
        rtc.adjust(DateTime(now.year(), now.month(), now.day(), hour, minute, 0));

        //we do so as if the last timezone change occurred now such that the next timezone change is correctly determined
        now = rtc.now();
        lastTimezoneChange = now;
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
        rtc.adjust(DateTime(year + 2000, month, day, now.hour(), now.minute(), now.second()));

        //we do so as if the last timezone change occurred now such that the next timezone change is correctly determined
        now = rtc.now();
        lastTimezoneChange = now;
      }
      return SETUP_MODE_EXIT_OK;
    }


    if(touched && touchedRect(x, y, &rSetupLeft)) {
      if(clockChanged) {
        //Adjust RTC
        DateTime now = rtc.now();
        rtc.adjust(DateTime(year + 2000, month, day, now.hour(), now.minute(), now.second()));

        //we do so as if the last timezone change occurred now such that the next timezone change is correctly determined
        now = rtc.now();
        lastTimezoneChange = now;
      }
      return SETUP_MODE_CLOCK;
    }

    if(touched) {
      delay(80);
    }
  }
}

