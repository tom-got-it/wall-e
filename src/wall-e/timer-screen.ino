const int TIMER_STATUS_STOPPED = 0;
const int TIMER_STATUS_PAUSED = 1;
const int TIMER_STATUS_RUNNING = 2;

const rect rTimerExit = {30, 5, -20, 25};

const rect rTimerTop1 = {40, 75, 30, -25};
const rect rTimerTop2 = {112, 75, 30, -25};

const rect rTimerBottom1 = {40, 163, 30, 25};
const rect rTimerBottom2 = {112, 163, 30, 25};

const rect rTimerDigits = {20, 95, 175, 50};

const rect rTimerStop = {180, 99, 40, 40};
const rect rTimerPlayPause = {235, 99, 40, 40};

const rect rTimerAnimation = {290, 49, 25, 141};

const rect rTimerAddonButtonsReset = {0, 205, 320, 31};
const rect rTimerDisplay = {5, 205, 140, 30};
const rect rTimerLightBulb = {175, 205, 140, 30};

const rect rTimerClock = {245, 10, 75, 25};

int timerStatus = TIMER_STATUS_STOPPED;
int timerTimeSpanOrigSeconds = -1;
DateTime timerStartTime;
DateTime timerAlarmTime;

int timerMinutesRest = 0;
int timerSecondsRest = 0;
int timerPrintedMinutes = -1;
int timerPrintedSeconds = -1;

int timerMinutesXPos = -1;
int timerMinutesEndXPos = -1;
int timerSecondsXPos = -1;
int timerSecondsEndXPos = -1;
rect rTimerDigitMinutes;
rect rTimerDigitSeconds;

int timerAnimationCurrentPixelHeight = -1;

boolean timerDisplayIsOn = true;
boolean timerDisplayAuto = false;
boolean timerLightBulbOn = false;

DateTime timerClockLastUpdate;
int timerClockColonXPos = -1;


DateTime timerLastTouched;

void showTimerScreen() {
  Serial.println("Entered Timer Screen");
  if(isBatteryLowVoltage()) {
    Serial.println("Exit timer due to low battery voltage");
    drawVoltageWarningScreen();
    delay(3000);
    return;
  }

  setupTimerOnEnter();

  rePrintTimerScreen();
  timerLoop();

  timerTurnOnDisplay();
  disableLightBulb();

  //when the timer is running, we do not listen to clock alarms
  if(isAlarmClockTriggered()) {
    missedAlarmNotification = true;
    clearAlarmClockFlag();
  }
}

void setupTimerOnEnter() {
  //variables
  timerLastTouched = rtc.now();
  timerDisplayIsOn = true;
  timerDisplayAuto = false;
  timerLightBulbOn = false;

  //setup status
  timerMinutesRest = grTimerFavoriteMinutes;
  timerSecondsRest = grTimerFavoriteSeconds;
  timerStatus = TIMER_STATUS_STOPPED;

  //setup position of timer digits
  tft.setTextColor(TFT_BLACK);
  timerMinutesXPos = rTimerDigits.x;
  timerMinutesEndXPos = timerMinutesXPos + tft.drawString("00", timerMinutesXPos, rTimerDigits.y, TFT_MAIN_FONT);
  timerSecondsXPos = timerMinutesEndXPos + tft.drawString(":", timerMinutesEndXPos, rTimerDigits.y, TFT_MAIN_FONT);
  timerSecondsEndXPos = timerSecondsXPos + tft.drawString("00", timerSecondsXPos, rTimerDigits.y, TFT_MAIN_FONT);
  tft.setTextColor(TFT_MAIN_COLOR);

  //setup invisible rectengles for touching minutes and seconds
  rTimerDigitMinutes = {timerMinutesXPos, rTimerDigits.y, timerMinutesEndXPos - timerMinutesXPos, rTimerDigits.height};
  rTimerDigitSeconds = {timerSecondsXPos, rTimerDigits.y, timerSecondsEndXPos - timerSecondsXPos, rTimerDigits.height};
}

void rePrintTimerScreen() {
  timerPrintedMinutes = -1;
  timerPrintedSeconds = -1;

  clearScreen();
  printTimerStaticSymbols();
  printTimerClock(true);
  printTimerControlTriangles();
  printTimerDigits();
  printTimerControlButtons();
  printTimerAddonButtons();
  printTimerAnimationBase();
}

void printTimerStaticSymbols() {
  tft.drawString("Timer", 120, 10, TFT_BIG_FONT);
  printTriangleLeftRight(&rTimerExit, TFT_MAIN_COLOR);

  //print timer colon
  tft.drawString(":", timerMinutesEndXPos, rTimerDigits.y, TFT_MAIN_FONT);
}

void printTimerControlTriangles() {
  int color = TFT_WHITE;
  if(! isShowTimerSetupTriangles()) {
    //hide
    color = TFT_BLACK;
  }

  printTriangleUpDown(&rTimerTop1, color);
  printTriangleUpDown(&rTimerTop2, color);
  printTriangleUpDown(&rTimerBottom1, color);
  printTriangleUpDown(&rTimerBottom2, color);
}

void printTimerDigits() {
  if(timerPrintedMinutes != timerMinutesRest) {
    timerPrintedMinutes = timerMinutesRest;
    tft.setTextColor(TFT_BLACK);
    tft.drawString("88", timerMinutesXPos, rTimerDigits.y, TFT_MAIN_FONT);
    tft.setTextColor(TFT_MAIN_COLOR); 
    tft.drawString(padByZeroTwoDigits(timerMinutesRest), timerMinutesXPos, rTimerDigits.y, TFT_MAIN_FONT);
  }

  if(timerPrintedSeconds != timerSecondsRest) {
    timerPrintedSeconds = timerSecondsRest;
    tft.setTextColor(TFT_BLACK);
    tft.drawString("88", timerSecondsXPos, rTimerDigits.y, TFT_MAIN_FONT);
    tft.setTextColor(TFT_MAIN_COLOR);
    tft.drawString(padByZeroTwoDigits(timerSecondsRest), timerSecondsXPos, rTimerDigits.y, TFT_MAIN_FONT);
  }
}

void printTimerControlButtons() {
  //stop
  fillArc(rTimerStop.x + 20, rTimerStop.y + 20, 0, 60, 20, 20, 20, TFT_BLACK);
  fillArc(rTimerStop.x + 20, rTimerStop.y + 20, 0, 60, 20, 20, 2, TFT_MAIN_COLOR);
  const rect stopRect = {rTimerStop.x + 13, rTimerStop.y + 13, rTimerStop.width - 26, rTimerStop.height - 26};
  printRect(&stopRect, TFT_MAIN_COLOR, true);

  //play / pause
  fillArc(rTimerPlayPause.x + 20, rTimerPlayPause.y + 20, 0, 60, 20, 20, 20, TFT_BLACK);
  if(isTimerShowPauseButton() || isTimerShowPlayButton()) {
    if(isTimerShowPauseButton()) {
      fillArc(rTimerPlayPause.x + 20, rTimerPlayPause.y + 20, 0, 60, 20, 20, 2, TFT_MAIN_COLOR);
      const rect pause1Rect = {rTimerPlayPause.x + 13, rTimerPlayPause.y + 13, 5, rTimerPlayPause.height - 26};
      const rect pause2Rect = {rTimerPlayPause.x + 13 + 9, rTimerPlayPause.y + 13, 5, rTimerPlayPause.height - 26};
      printRect(&pause1Rect, TFT_MAIN_COLOR, true);
      printRect(&pause2Rect, TFT_MAIN_COLOR, true);
    } else {
      fillArc(rTimerPlayPause.x + 20, rTimerPlayPause.y + 20, 0, 60, 20, 20, 2, TFT_MAIN_COLOR);
      const rect playRect = {rTimerPlayPause.x + 13, rTimerPlayPause.y + 13, rTimerPlayPause.width - 26, rTimerPlayPause.height - 26};
      printTriangleLeftRight(&playRect, TFT_MAIN_COLOR);
    }
  }
}

void printTimerAddonButtons() {
  printRect(&rTimerAddonButtonsReset, TFT_BLACK, true);

  if(timerDisplayAuto) {
    printRect(&rTimerDisplay, TFT_MAIN_COLOR, false);
    tft.drawString("Display", rTimerDisplay.x + 25, rTimerDisplay.y + 5, TFT_BIG_FONT);
  } else {
    printRect(&rTimerDisplay, TFT_MAIN_COLOR, true);
    tft.setTextColor(TFT_BLACK);
    tft.drawString("Display", rTimerDisplay.x + 25, rTimerDisplay.y + 5, TFT_BIG_FONT);
    tft.setTextColor(TFT_MAIN_COLOR);
  }

  if(timerLightBulbOn) {
    printRect(&rTimerLightBulb, TFT_MAIN_COLOR, true);
    tft.setTextColor(TFT_BLACK);
    tft.drawString("Light", rTimerLightBulb.x + 40, rTimerLightBulb.y + 5, TFT_BIG_FONT);
    tft.setTextColor(TFT_MAIN_COLOR);
  } else {
    printRect(&rTimerLightBulb, TFT_MAIN_COLOR, false);
    tft.drawString("Light", rTimerLightBulb.x + 40, rTimerLightBulb.y + 5, TFT_BIG_FONT);
  }
}

void printTimerAnimationBase() {
  if(timerStatus == TIMER_STATUS_RUNNING) {
    printRect(&rTimerAnimation, TFT_MAIN_COLOR, false);
  } else {
    printRect(&rTimerAnimation, TFT_BLACK, true);
  }
}

void printTimerAnimationContent(boolean force) {
  if(timerStatus != TIMER_STATUS_RUNNING) {
    return;
  }
  
  DateTime now = rtc.now();
  uint32_t timeSpanRest = timerAlarmTime.unixtime() - now.unixtime();
  uint32_t timeSpanTotal = timerTimeSpanOrigSeconds;
  if(timeSpanRest < 0) {
    timeSpanRest = 0;
  }

  float totalPercentile = (float)timeSpanTotal / 100.0;
  float percentRest = (float)timeSpanRest / totalPercentile;

  float pixelsPercentile = (float)(rTimerAnimation.height - 2) / 100.0;
  int pixelsRest = pixelsPercentile * percentRest;

  if(pixelsRest < 0) {
    pixelsRest = 0;
  }

  if(force || pixelsRest != timerAnimationCurrentPixelHeight) {
    const rect rClear = {rTimerAnimation.x + 1, rTimerAnimation.y + 1, rTimerAnimation.width - 2, rTimerAnimation.height - pixelsRest - 2};
    const rect rAni = {rTimerAnimation.x, rTimerAnimation.y + rTimerAnimation.height - pixelsRest - 1, rTimerAnimation.width, pixelsRest};
    
    if(rClear.height > 0) {
      printRect(&rClear, TFT_BLACK, true);
    }
    if(rAni.height > 0) {
      printRect(&rAni, TFT_MAIN_COLOR, true);
    }
  }
  timerAnimationCurrentPixelHeight = pixelsRest;
}

void printTimerClock(boolean force) {
    DateTime now = rtc.now();

    uint8_t hour = now.hour();
    uint8_t minute = now.minute();
    uint8_t second = now.second();

    int xPos = rTimerClock.x;
    int yPos = rTimerClock.y;

    if(force || timerClockLastUpdate.second() != second) {
        boolean reprintAll = force || hour != timerClockLastUpdate.hour();
        boolean reprintMinutes = reprintAll || minute != timerClockLastUpdate.minute();

        if(reprintAll) {
          printRect(&rTimerClock, TFT_BLACK, true);
          xPos = drawTwoDigitsAndGetXPos(hour, xPos, yPos, TFT_BIG_FONT);
          timerClockColonXPos = xPos;
        } else {
          xPos = timerClockColonXPos;
        }

        //Draw flashing colon (separator of hour and minutes)
        if(second % 2) {
          tft.setTextColor(TFT_BLACK, TFT_BLACK);
          xPos += tft.drawChar(':', xPos, yPos, TFT_BIG_FONT);
          tft.setTextColor(TFT_MAIN_COLOR, TFT_BLACK);
        } else {
          xPos += tft.drawChar(':', xPos, yPos, TFT_BIG_FONT);
        }

        if(reprintMinutes) {
          const rect clear = {xPos, rTimerClock.y, 320 - xPos, rTimerClock.height};
          printRect(&clear, TFT_BLACK, true);
          drawTwoDigitsAndGetXPos(minute, xPos, yPos, TFT_BIG_FONT);
        }
        
        timerClockLastUpdate = now;
    }
}

boolean isShowTimerSetupTriangles() {
  return timerStatus == TIMER_STATUS_STOPPED;
}

boolean isTimerShowPlayButton() {
  return timerStatus != TIMER_STATUS_RUNNING && 
          (timerMinutesRest > 0 || timerSecondsRest > 0);
}

boolean isTimerShowPauseButton() {
  return timerStatus == TIMER_STATUS_RUNNING;
}

void startTimer() {
  //save as favorite
  if(timerStatus != TIMER_STATUS_PAUSED) {
    grTimerFavoriteMinutes = timerMinutesRest;
    grTimerFavoriteSeconds = timerSecondsRest;
    timerTimeSpanOrigSeconds = (timerMinutesRest * 60) + timerSecondsRest;
  }

  //start timer
  timerStatus = TIMER_STATUS_RUNNING;
  timerStartTime = rtc.now();
  timerAlarmTime = DateTime(timerStartTime.unixtime() + (timerMinutesRest * 60) + timerSecondsRest);
  Serial.print("timer will trigger an alarm at: ");
  serialPrintTime(timerAlarmTime);

  //re-print
  printTimerControlTriangles();
  printTimerControlButtons();
  printTimerAnimationBase();
  printTimerAnimationContent(true);
}

void pauseTimer() {
  timerStatus = TIMER_STATUS_PAUSED;
  printTimerControlTriangles();
  printTimerControlButtons();
  printTimerAnimationBase();
}

void stopTimer() {
  timerStatus = TIMER_STATUS_STOPPED;
  timerMinutesRest = grTimerFavoriteMinutes;
  timerSecondsRest = grTimerFavoriteSeconds;
  printTimerControlTriangles();
  printTimerControlButtons();
  printTimerAnimationBase();
}

void computeTimerRestWhileRunning() {
  uint32_t secondsDiff = timerAlarmTime.unixtime() - rtc.now().unixtime();
  if(secondsDiff < 0) {
    timerMinutesRest = 0;
    timerSecondsRest = 0;
  } else {
    timerMinutesRest = secondsDiff / 60;
    timerSecondsRest = secondsDiff % 60;
  }
}

void testTimerAlarmWhileRunning() {
  if(rtc.now().unixtime() >= timerAlarmTime.unixtime()) {
    timerTurnOnDisplay();
    if(timerLightBulbOn) {
      timerToggleLightBulb();
    }
    
    showAlarmScreenTimer();
    clearScreen();
    stopTimer();
    rePrintTimerScreen();
  }
}

void timerDisplayAutoOffTest() {
  if(!timerDisplayAuto) {
    return;
  }

  if(!timerDisplayIsOn) {
    //display already offline
    return;
  }

  if(timerLastTouched.unixtime() + gDisplayAutoShutdownSeconds < rtc.now().unixtime()) {
      turnOffDisplay();
      timerDisplayIsOn = false;
  }
}

void timerToggleDisplayMode() {
  timerDisplayAuto = !timerDisplayAuto;
  printTimerAddonButtons();
}

void timerTurnOnDisplay() {
  turnOnDisplay();
  timerDisplayIsOn = true;
}

void timerToggleLightBulb() {
  timerLightBulbOn = !timerLightBulbOn;
  if(timerLightBulbOn) {
    //Note that due to low batery voltage the light bulb might stay off
    timerLightBulbOn = startLightBulb();
  } else {
    disableLightBulb();
  }
  printTimerAddonButtons();
}

void timerLoop() {
  while(true) {
    if(handleTimerTouchAndIsExit()) {
      return;
    }

    if(isBatteryLowVoltage() && timerStatus != TIMER_STATUS_RUNNING) {
      Serial.println("Exit timer due to low battery voltage");
      drawVoltageWarningScreen();
      delay(3000);
      return;
    }

    //display auto-off test
    timerDisplayAutoOffTest();

    //Light Bulb auto-shutdown Test
    boolean wasLightbulbOn = timerLightBulbOn;
    timerLightBulbOn = testLightBulbStillActive();
    if(wasLightbulbOn != timerLightBulbOn) {
      printTimerAddonButtons();
    }

    if(timerStatus == TIMER_STATUS_RUNNING) {
      computeTimerRestWhileRunning();
    }

    printTimerClock(false);
    printTimerDigits();
    printTimerAnimationContent(false);

    if(timerStatus == TIMER_STATUS_RUNNING) {
      testTimerAlarmWhileRunning();
    }

    delay(140);
  }
}

boolean handleTimerTouchAndIsExit() {
  uint16_t x,y;
  boolean touched = tft.getTouch(&x, &y);
  if(! touched) {
    return false;
  }

  timerLastTouched = rtc.now();
  if(touched && !timerDisplayIsOn) {
    //turn on the display if it was off
    Serial.println("Touched turn on dispaly");
    timerTurnOnDisplay();
    delay(400);
    return false;
  }

  if(touchedRect(x, y, &rTimerExit)) {
    Serial.println("Touched exit button");
    timerStatus = TIMER_STATUS_STOPPED;
    return true;
  }

  if(touchedRect(x, y, &rTimerStop)) {
    Serial.println("stop timer touched");
    stopTimer();
    return false;
  }

  if(touchedRect(x, y, &rTimerPlayPause)) {
    if(isTimerShowPlayButton()) {
      Serial.println("start timer touched");
      startTimer();

    } else if(isTimerShowPauseButton()) {
      Serial.println("pause timer touched");
      pauseTimer();
    }
    return false;
  }

  if(touchedRect(x, y, &rTimerDisplay)) {
    Serial.println("Touched toggle display mode");
    timerToggleDisplayMode();
    return false;
  }

  if(touchedRect(x, y, &rTimerLightBulb)) {
    Serial.println("Touched toggle light bulb");
    timerToggleLightBulb();
    return false;
  }

  if(isShowTimerSetupTriangles()) {
    handleTimerTouchSetup(x, y);
    return false;
  }

  return false;
}

void handleTimerTouchSetup(uint16_t x, uint16_t y) {
  if(touchedRect(x, y, &rTimerTop1)) {
    Serial.println("Touched increase minutes");
    boolean wasZero = timerMinutesRest == 0 && timerSecondsRest == 0;
    timerMinutesRest = timerMinutesRest + 1;
    if(timerMinutesRest > 99) {
      timerMinutesRest = 99;
    }
    if(wasZero) {
      printTimerControlButtons();
    }
    return;
  }

  if(touchedRect(x, y, &rTimerTop2)) {
    Serial.println("Touched increase seconds");
    boolean wasZero = timerMinutesRest == 0 && timerSecondsRest == 0;
    timerSecondsRest = timerSecondsRest + 1;
    if(timerSecondsRest >= 60) {
      timerMinutesRest = timerMinutesRest + 1;
      if(timerMinutesRest > 99) {
        timerMinutesRest = 99;
        timerSecondsRest = 59;
      } else {
        timerSecondsRest = 0;
      }
    }
    if(wasZero) {
      printTimerControlButtons();
    }
    return;
  }

  if(touchedRect(x, y, &rTimerBottom1)) {
    Serial.println("Touched decrease minutes");
    timerMinutesRest = timerMinutesRest - 1;
    if(timerMinutesRest < 0) {
      timerMinutesRest = 0;
    }
    if(timerMinutesRest == 0 && timerSecondsRest == 0) {
      printTimerControlButtons();
    }
    return;
  }

  if(touchedRect(x, y, &rTimerBottom2)) {
    Serial.println("Touched decrease seconds");
    timerSecondsRest = timerSecondsRest - 1;
    if(timerSecondsRest < 0) {
      timerMinutesRest = timerMinutesRest - 1;
      if(timerMinutesRest < 0) {
        timerMinutesRest = 0;
        timerSecondsRest = 0;
      } else {
        timerSecondsRest = 59;
      }
    }
    if(timerMinutesRest == 0 && timerSecondsRest == 0) {
      printTimerControlButtons();
    }
    return;
  }

  if(touchedRect(x, y, &rTimerDigitMinutes)) {
    timerMinutesRest = 0;
    if(timerMinutesRest == 0 && timerSecondsRest == 0) {
      printTimerControlButtons();
    }
    return;
  }

  if(touchedRect(x, y, &rTimerDigitSeconds)) {
    timerSecondsRest = 0;
    if(timerMinutesRest == 0 && timerSecondsRest == 0) {
      printTimerControlButtons();
    }
    return;
  }
}