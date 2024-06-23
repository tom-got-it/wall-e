const int xClock = 20;
const int yClock = 150;
const int xRangeSetupAlarm[] = {20, 165};
const int yRangeSetupAlarm[] = {150, 210};

const int xToggleAlarm = 20;
const int yToggleAlarm = 210;
const int xRangeToggleAlarm[] = {20, 165};
const int yRangeToggleAlarm[] = {210, 240};

const int xRangeChangeVolume[] = {190, 320};
const int yRangeChangeVolume[] = {30, 200};

const int xPosVoltage = 220;
const int xPosCurrentMilliAmps = 210;
const int yPosBatteryStatistics = 203;
const rect rResetBatteryStatistics = {200, 205, 100, 18};
const int xRangeBatteryStatistics[] = {190, 320};
const int yRangeBatteryStatistics[] = {201, 240};

const rect rVoltageBar = {195, 185, 113, 7};
const rect rResetVoltageBars = {190, 30, 130, 170};

const int xPosNotificationCenter = 30;
const int yPosNotificationCenter = 27;
const rect rResetNotificationCenter = {0, 0, 50, 50};

const int xRangeLightBulb[] = {100, 170};
const int yRangeLightBulb[] = {50, 120};

const int xPosMenuCenter = 30;
const int yPosMenuCenter = 77;
const rect rMenuTouch = {0, 57, 50, 40};

const rect rMp3Player = {15, 60, 25, 30};

const int xRangeFirmwareupdateInvisible[] = {0, 80};  //this is invisible - updates are triggered after 3x touching this place
const int yRangeFirmwareupdateInvisible[] = {100, 140};


//------------CLOCK and ALARM snf VOLTAGE----------------
//Note that these values will bel ost in deep-sleep
boolean printedClock = false;
DateTime lastClockUpdate;
int lastColonXPos = 0;

boolean printedAlarm = false;
DateTime lastAlarmPrinted;
boolean alarmPrintedActive = false;

boolean printedBatteryData = false;
DateTime lastBatteryDataWhen;
int lastPrintedVoltageBarCount = -3; //-1 = on USB, -2 = charging battery

boolean printedNotification = false;
boolean wasUrgentNotificationBefore = false;

boolean printedMenu = false;
//-------------------------------------------


void rePrintMainScreen() {
  printedClock = false;
  printedAlarm = false;
  printedBatteryData = false;
  printedNotification = false;
  printedMenu = false;

  clearScreen();
  tft.drawBitmap(0, 0, epd_bitmap_Interface_2, 320, 240, TFT_MAIN_COLOR);

  printMainScreen();
}

void printMainScreen() {
  printClock();
  printAlarmToggle();
  printBatteryData();
  printNotification();
  printMenu();
}

void printClock() {
    DateTime now = rtc.now();

    uint8_t hour = now.hour();
    uint8_t minute = now.minute();
    uint8_t second = now.second();

    int xPos = xClock;
    int yPos = yClock;

    if(!printedClock || lastClockUpdate.second() != second) {
        if(!printedClock || hour != lastClockUpdate.hour()) {
          tft.setTextColor(TFT_BLACK);
          drawTwoDigitsAndGetXPos(88, xPos, yPos, TFT_MAIN_FONT); //reset
          tft.setTextColor(TFT_MAIN_COLOR);
          lastColonXPos = drawTwoDigitsAndGetXPos(hour, xPos, yPos, TFT_MAIN_FONT);
        }

        //Draw flashing colon (separator of hour and minutes)
        int xPosMinute = lastColonXPos;
        if(second % 2) {
          tft.setTextColor(TFT_BLACK, TFT_BLACK);
          xPosMinute += tft.drawChar(':', lastColonXPos, yPos, TFT_MAIN_FONT);
          tft.setTextColor(TFT_MAIN_COLOR, TFT_BLACK);
        } else {
          xPosMinute += tft.drawChar(':', lastColonXPos, yPos, TFT_MAIN_FONT);
        }

        if(!printedClock || minute != lastClockUpdate.minute()) {
          tft.setTextColor(TFT_BLACK);
          drawTwoDigitsAndGetXPos(88, xPosMinute, yPos, TFT_MAIN_FONT); //reset
          tft.setTextColor(TFT_MAIN_COLOR);
          drawTwoDigitsAndGetXPos(minute, xPosMinute, yPos, TFT_MAIN_FONT);
        }
        
        lastClockUpdate = now;
        printedClock = true;
    }
}


void printAlarmToggle() {
    int xPos = xToggleAlarm;
    int yPos = yToggleAlarm;

    if(printedAlarm && 
        alarmPrintedActive == isAlarmActive && 
        lastAlarmPrinted.unixtime() == rtc.getAlarm1().unixtime()) {
        //nothing to print
        return;
    }

    //Print wheter the alarm is on or off
    if(isAlarmActive) {
      xPos += tft.drawString("<<ON>> ", xPos, yPos, TFT_SMALL_FONT);
    } else {
      xPos += tft.drawString("<<", xPos, yPos, TFT_SMALL_FONT);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      xPos += tft.drawString("OFF", xPos, yPos, TFT_SMALL_FONT);
      tft.setTextColor(TFT_MAIN_COLOR, TFT_BLACK);
      xPos += tft.drawString(">>", xPos, yPos, TFT_SMALL_FONT);
    }

    //Blanks
    xPos += tft.drawString("   ", xPos, yPos, TFT_SMALL_FONT);

    //Print the alarm-time
    DateTime alarm = rtc.getAlarm1();
    xPos = drawTwoDigitsAndGetXPos(alarm.hour(), xPos, yPos, TFT_SMALL_FONT);
    xPos += tft.drawChar(':', xPos, yPos, TFT_SMALL_FONT);
    xPos = drawTwoDigitsAndGetXPos(alarm.minute(), xPos, yPos, TFT_SMALL_FONT);

    lastAlarmPrinted = alarm;
    alarmPrintedActive = isAlarmActive;
    printedAlarm = true;
}

void printBatteryData() {
  float batVoltage = getBatVoltage();
  float batCurrent = getBatCurrentMillis();
  if(printedBatteryData && lastBatteryDataWhen.unixtime() + gVoltageReprintAfterSeconds > rtc.now().unixtime()) {
    //Voltage is still valid - nothing to print
    return;
  }

  printBatteryStatistics(batVoltage, batCurrent);
  printVoltageBars(batVoltage);

  printedBatteryData = true;
  lastBatteryDataWhen = rtc.now();
}

void printBatteryStatistics(float batVoltage, float batCurrent) {
  if(showBatteryStatistics == 0) {
    return;
  }

  if(batVoltage > gLowVoltage) {
    tft.setTextColor(TFT_BLACK);
  } else {
    tft.setTextColor(TFT_RED);
  }

  printRect(&rResetBatteryStatistics, TFT_MAIN_COLOR, true);

  if(showBatteryStatistics == 1) {
    String output = String(batVoltage, 2) + "v";
    tft.drawString(output, xPosVoltage, yPosBatteryStatistics, 4);
  } else {
    String output = batCurrent < 0 ? "-" : "";
    output += abs(batCurrent) < 10 ? "0" : "";
    output += abs(batCurrent) < 100 ? "0" : "";
    output += String((int)abs(batCurrent)) + "ma";
    tft.drawString(output, xPosCurrentMilliAmps, yPosBatteryStatistics, 4);
  }

  tft.setTextColor(TFT_MAIN_COLOR);
}

void printVoltageBars(float batVoltage) {
  int printBars = 0;
  if(batVoltage >= 4.20) {
    printBars = 11;
  } else if (batVoltage >= 4.10) {
    printBars = 10;
  } else if (batVoltage >= 4.00) {
    printBars = 9;
  } else if (batVoltage >= 3.90) {
    printBars = 8;
  } else if (batVoltage >= 3.85) {
    printBars = 7;
  } else if (batVoltage >= 3.80) {
    printBars = 6;
  } else if (batVoltage >= 3.75) {
    printBars = 5;
  } else if (batVoltage >= 3.70) {
    printBars = 4;
  } else if (batVoltage >= 3.65) {
    printBars = 3;
  } else if (batVoltage >= 3.60) {
    printBars = 2;
  } else if (batVoltage >= 3.40) {
    printBars = 1;
  }

  if(isOnUsbPower()) {
    printBars = -1;
  }

  if(isChargingBattery()) {
    printBars = -2;
  }

  if(printedBatteryData && lastPrintedVoltageBarCount == printBars) {
    return;
  }
  lastPrintedVoltageBarCount = printBars;

  printRect(&rResetVoltageBars, TFT_BLACK, true);
  if(printBars > 0) {
    rect bar = rVoltageBar;
    for(int i = 1; i <= printBars; i++) {
      printRect(&bar, TFT_MAIN_COLOR, true);
      bar.y = bar.y - (2 * bar.height);
    }
    return;
  }

  if(printBars == 0) {
    rect bar = rVoltageBar;
    printRect(&bar, TFT_RED, true);
    return;
  }

  rect bar = rVoltageBar;
  if(printBars == -1) {
    tft.drawString("USB", bar.x + 30, bar.y - 10, TFT_BIG_FONT);
  } else {
    tft.drawString("CHARGE", bar.x + 5, bar.y - 10, TFT_BIG_FONT);
  }
}

void printNotification() {
  boolean showUrgentNotification = missedAlarmNotification || rtcLostPowerNotification || mp3TrackCount <= 0;

  if(! printedNotification || wasUrgentNotificationBefore != showUrgentNotification) {
    printRect(&rResetNotificationCenter, TFT_BLACK, true);
    if(showUrgentNotification) {
      fillArc(xPosNotificationCenter, yPosNotificationCenter, 0, 60, 15, 15, 15, TFT_RED);
      tft.setTextColor(TFT_BLACK);
      tft.drawString("!", xPosNotificationCenter - 4, yPosNotificationCenter - 9, TFT_BIG_FONT);
      tft.setTextColor(TFT_MAIN_COLOR);
    } else {
      fillArc(xPosNotificationCenter, yPosNotificationCenter, 0, 60, 15, 15, 15, TFT_MAIN_COLOR);
      tft.setTextColor(TFT_BLACK);
      tft.drawString("i", xPosNotificationCenter - 4, yPosNotificationCenter - 9, TFT_BIG_FONT);
      tft.setTextColor(TFT_MAIN_COLOR);
    }

    wasUrgentNotificationBefore = showUrgentNotification;
    printedNotification = true;
  }
}

void printMenu() {
  if(! printedMenu) {
    fillArc(xPosMenuCenter, yPosMenuCenter, 0, 60, 15, 15, 1, TFT_MAIN_COLOR);
    tft.drawString("+", xPosMenuCenter - 5, yPosMenuCenter - 12, TFT_BIG_FONT);
    printedMenu = true;
  }
}

//----------Alarm fired on main screen-------
void handleMainAlarm() {
    if(testAndShowAlarmScreen()) {
      rePrintMainScreen();
      lastToched = rtc.now();
      delay(200);
    }
}


//-----------Touch------------------
void handleMainTouch() {
    uint16_t x,y = 0;
    boolean touched = tft.getTouch(&x, &y);

    if(! touched) {
      return;
    }

    if(checkTouch(x, y, xRangeSetupAlarm, yRangeSetupAlarm)) {
      Serial.println("touched alarm setup");
      showSetupScreen();
      rePrintMainScreen();
    }

    if(checkTouch(x, y, xRangeToggleAlarm, yRangeToggleAlarm)) {
      Serial.println("touched alarm toggle");
      toggleAlarm();
      rePrintMainScreen();
    }

    if(checkTouch(x, y, xRangeChangeVolume, yRangeChangeVolume)) {
      Serial.println("touched volume change");
      setupVolume();
      rePrintMainScreen();
    }

    if(checkTouch(x, y, xRangeBatteryStatistics, yRangeBatteryStatistics)) {
      Serial.println("touched toggle show battery stats");
      showBatteryStatistics++;
      if(showBatteryStatistics > 2) {
        showBatteryStatistics = 0;
      }
      rePrintMainScreen();
    }

    if(touchedRect(x, y, &rResetNotificationCenter)) {
      Serial.println("touched reset notifications");
      showNotificationScreen();
      rePrintMainScreen();
    }

    if(checkTouch(x, y, xRangeLightBulb, yRangeLightBulb)) {
      Serial.println("touched light bulb toggle");
      handleLightBulb();
    }

    if(touchedRect(x, y, &rMenuTouch)) {
      Serial.println("touched menu icon");
      showMenuScreen();

      if(! isFirmwareUpdateInProgress) {
        rePrintMainScreen();
      }
    }

    lastToched = rtc.now();
    delay(200);
}

void handleLightBulb() {
  boolean started = startLightBulb();
  if(! started) {
    return;
  }
  
  while(true) {
    uint16_t x,y = 0;
    boolean touched = tft.getTouch(&x, &y);

    if(touched && checkTouch(x, y, xRangeLightBulb, yRangeLightBulb)) {
      disableLightBulb();
      return;
    }

    boolean stillActive = testLightBulbStillActive();
    if(! stillActive) {
      return;
    }

    //Re-Print battery data - allows to monitor current
    printBatteryData();

    delay(80);
  }
}

