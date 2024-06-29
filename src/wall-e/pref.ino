const char* PERF_NOTIFICATION_VOLUME = "P_VOL";   //max 15 chars
const char* PERF_LIGHT_BULB_DELAY = "P_LIGHT_DEL";
const char* PERF_LAST_TIMEZONE_CHANGE = "P_LTC";

void clearPrefs() {
  preferences.begin(PREFERENCES_NAME, false);
  preferences.clear();
  preferences.end();
}


void initPrefs() {
  preferences.begin(PREFERENCES_NAME, false);
  pNotificationVolume = preferences.getInt(PERF_NOTIFICATION_VOLUME, gDefaultNotificationVolume);
  pLightBulbDelaySecondsOnAlarm = preferences.getInt(PERF_LIGHT_BULB_DELAY, gDefaultLightBulbDelaySeconds);
  pLastTimezoneChangeUxt = preferences.getUInt(PERF_LAST_TIMEZONE_CHANGE, DATE_TIME_MIN);
  preferences.end();

  //debug output
  String out;
  out = "";
  out = out + "Restored Notification-Volume: " + pNotificationVolume;
  Serial.println(out);

  out = "";
  out = out + "Restored Light-Bulb delay: " + pLightBulbDelaySecondsOnAlarm;
  Serial.println(out);

  if(pLastTimezoneChangeUxt > DATE_TIME_MIN) {
    out = "";
    out = out + "Restored last timezone-change: " + getFormattedTime(DateTime(pLastTimezoneChangeUxt), true);
    Serial.println(out);
  }

  //since we the keys might not yet exist, save all prefs
  savePrefs();
}

void savePrefs() {
  preferences.begin(PREFERENCES_NAME, false);
  preferences.putInt(PERF_NOTIFICATION_VOLUME, pNotificationVolume);
  preferences.putInt(PERF_LIGHT_BULB_DELAY, pLightBulbDelaySecondsOnAlarm);
  preferences.putUInt(PERF_LAST_TIMEZONE_CHANGE, pLastTimezoneChangeUxt);
  preferences.end();
}