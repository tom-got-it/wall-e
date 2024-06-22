const rect rNotificationOk = {80, 170, 160, 50};
const int xNotificationOk = 143;
const int yNotificationOk = 185;

void showNotificationScreen() {
  clearScreen();
  printNotificationOkButton();

  tft.setTextColor(TFT_RED);
  if(missedAlarmNotification) {
    tft.drawString("Sie haben einen Weck-Alarm verpasst.", 40, 10, TFT_SMALL_FONT);
  }

  if(rtcLostPowerNotification) {
    tft.drawString("Die Uhr hatte keinen Strom.", 40, 30, TFT_SMALL_FONT);
  }

  if(mp3TrackCount <= 0) {
    tft.drawString("Keine MP3s zum Abspielen gefunden.", 40, 50, TFT_SMALL_FONT);
  }
  tft.setTextColor(TFT_MAIN_COLOR);

  String startStr = "Eingeschaltet am " + getFormattedTime(firstBootTime, false) + ".";
  tft.drawString(startStr, 40, 70, TFT_SMALL_FONT);

  String bootStr = String(bootCount) + " Mal aus Standby aufgewacht.";
  tft.drawString(bootStr, 40, 90, TFT_SMALL_FONT);

  timezone_change nextTsChange = getNextTimezoneChangeElseFirst();
  String timezoneStr = "";
  if(rtc.now().unixtime() < nextTsChange.when.unixtime()) {
    timezoneStr = "Zeitumstellung am " + getFormattedTime(nextTsChange.when, false) + ".";
  } else {
    timezoneStr = "Nächste Zeitumstellung nicht gepflegt";
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