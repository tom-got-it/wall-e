const char* PERF_NOTIFICATION_VOLUME = "P_VOL";   //max 15 chars
const char* PERF_LIGHT_BULB_DELAY = "P_LIGHT_DEL";
const char* PERF_LAST_TIMEZONE_CHANGE = "P_LTC";
const char* PERF_ALARM_TIMEOUT_MINUTES = "P_ALM_OFF";
const char* PERF_SNOOZE_MINUTES_1 = "P_SNOOZE_1";
const char* PERF_SNOOZE_MINUTES_2 = "P_SNOOZE_2";
const char* PERF_ALARM_TOUCH_ACTION = "P_ALM_ACT";

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
  pAlarmTimeoutMinutes = preferences.getInt(PERF_ALARM_TIMEOUT_MINUTES, gDefaultAlarmTimeoutMinutes);
  pSnoozeMinutes1 = preferences.getInt(PERF_SNOOZE_MINUTES_1, gDefaultSnoozeMinutes1);
  pSnoozeMinutes2 = preferences.getInt(PERF_SNOOZE_MINUTES_2, gDefaultSnoozeMinutes2);
  pAlarmTouchAction = preferences.getInt(PERF_ALARM_TOUCH_ACTION, gDefaultAlarmTouchAction);
  preferences.end();

  //debug output
  String out;
  out = "";
  out = out + "Restored Notification-Volume: " + pNotificationVolume;
  Serial.println(out);

  out = "";
  out = out + "Restored Light-Bulb delay (seconds): " + pLightBulbDelaySecondsOnAlarm;
  Serial.println(out);

  out = "";
  out = out + "Alarm timeout after (minutes): " + pAlarmTimeoutMinutes;
  Serial.println(out);

  out = "";
  out = out + "Restored snooze option 1 (minutes): " + pSnoozeMinutes1;
  Serial.println(out);

  out = "";
  out = out + "Restored snooze option 2 (minutes): " + pSnoozeMinutes2;
  Serial.println(out);

  out = "";
  out = out + "Restored alarm touch action: " + pAlarmTouchAction;
  Serial.println(out);

  if(pLastTimezoneChangeUxt > DATE_TIME_MIN) {
    out = "";
    out = out + "Restored last timezone-change: " + getFormattedTime(DateTime(pLastTimezoneChangeUxt), true);
    Serial.println(out);
  }

  //Ensure that all preferences are stored (necessary after formatting or when new preferences are added)
  savePrefs();
}

void savePrefs() {
  preferences.begin(PREFERENCES_NAME, false);
  preferences.putInt(PERF_NOTIFICATION_VOLUME, pNotificationVolume);
  preferences.putInt(PERF_LIGHT_BULB_DELAY, pLightBulbDelaySecondsOnAlarm);
  preferences.putUInt(PERF_LAST_TIMEZONE_CHANGE, pLastTimezoneChangeUxt);
  preferences.putInt(PERF_ALARM_TIMEOUT_MINUTES, pAlarmTimeoutMinutes);
  preferences.putInt(PERF_SNOOZE_MINUTES_1, pSnoozeMinutes1);
  preferences.putInt(PERF_SNOOZE_MINUTES_2, pSnoozeMinutes2);
  preferences.putInt(PERF_ALARM_TOUCH_ACTION, pAlarmTouchAction);
  preferences.end();
}