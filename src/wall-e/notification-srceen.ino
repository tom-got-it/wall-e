const rect rNotificationOk = {80, 170, 160, 50};
const int xNotificationOk = 143;
const int yNotificationOk = 185;

void showNotificationScreen() {
  clearScreen();
  printNotificationOkButton();

  tft.setTextColor(TFT_RED);
  if(missedAlarmNotification) {
    tft.drawString("You missed an alarm.", 40, 10, TFT_SMALL_FONT);
  }

  if(rtcLostPowerNotification) {
    tft.drawString("The clock did not have power.", 40, 30, TFT_SMALL_FONT);
  }

  if(mp3TrackCount <= 0) {
    tft.drawString("No MP3 files were found.", 40, 50, TFT_SMALL_FONT);
  }
  tft.setTextColor(TFT_MAIN_COLOR);

  String startStr = "Turned on at " + getFormattedTime(firstBootTime, false) + ".";
  tft.drawString(startStr, 40, 70, TFT_SMALL_FONT);

  String bootStr = String(bootCount) + " times woke up from standby.";
  tft.drawString(bootStr, 40, 90, TFT_SMALL_FONT);

  timezone_change nextTsChange = getNextTimezoneChangeElseFirst();
  String timezoneStr = "";
  if(rtc.now().unixtime() < nextTsChange.when.unixtime()) {
    timezoneStr = "Next time-change at " + getFormattedTime(nextTsChange.when, false) + ".";
  } else {
    timezoneStr = "No time-changes by timezone configured";
  }
  tft.drawString(timezoneStr, 40, 110, TFT_SMALL_FONT);

  String version = FIRMWARE_VERSION;
  String fwr = "Firmware Version: " + version;
  tft.drawString(fwr, 40, 130, TFT_SMALL_FONT);


  while(true) {
    uint16_t x,y;
    boolean touched = tft.getTouch(&x, &y);

    if (touched) {
      if(touchedRect(x, y, &rNotificationOk)) {
        missedAlarmNotification = false;
        rtcLostPowerNotification = false;
        return;
      }
      delay(80);
    }
  }
}

void printNotificationOkButton() {
  printRect(&rNotificationOk, TFT_MAIN_COLOR, true);
  tft.setTextColor(TFT_BLACK);
  tft.drawString("OK", xNotificationOk, yNotificationOk, TFT_BIG_FONT);
  tft.setTextColor(TFT_MAIN_COLOR);
}