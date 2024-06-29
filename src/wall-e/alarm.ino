int getAlarmClockMode() {
  int mode = getAlarmClock().day();
  if(mode > getAlarmClockModeMaxValue() || mode < getAlarmClockModeMinValue()) {
    return ALARM_CLOCK_EACH_DAY;
  }
  return mode;
}

int getAlarmClockModeMinValue() {
  return 1;
}

int getAlarmClockModeMaxValue() {
  return ALARM_CLOCK_MODES_LENGTH;
}

String parseAlarmClockModeString(int mode) {
  switch (mode) {
    case ALARM_CLOCK_EACH_DAY: return "each day";
    case ALARM_CLOCK_WEEKDAYS: return "weekdays";
    case ALARM_CLOCK_WEEKEND: return "weekend";
    default: return "each day";
  }
}

boolean isAlarmClockModeMatchingNow() {
  DateTime now = rtc.now();
  int mode = getAlarmClockMode();
  switch (mode) {
    case ALARM_CLOCK_EACH_DAY:
      //alarm rings each day
      return true;
    case ALARM_CLOCK_WEEKDAYS:
      return now.dayOfTheWeek() >= 1 && now.dayOfTheWeek() <= 5;
    case ALARM_CLOCK_WEEKEND:
      return now.dayOfTheWeek() == 6 || now.dayOfTheWeek() == 0;
    default:
      //alarm rings each day
      return true;
  }
}

void setAlarmClockModeAndEnable(int mode) {
  //note that we use the year to store the alarm mode
  DateTime alarm = getAlarmClock();
  boolean wasActive = isAlarmClockEnabled();
  if(mode > getAlarmClockModeMaxValue() || mode < getAlarmClockModeMinValue()) {
    mode = ALARM_CLOCK_EACH_DAY;
  }
  DateTime newAlarm = DateTime(alarm.year(), alarm.month(), mode, alarm.hour(), alarm.minute(), alarm.second());

  String stateStr = "enabled";
  Serial.print("Setup alarm: ");
  Serial.print(getFormattedTimeWithoutDate(newAlarm));
  Serial.print(" - mode: ");
  Serial.print(parseAlarmClockModeString(mode));
  Serial.print(" - state: ");
  Serial.println(stateStr);

  rtc.setAlarm1(newAlarm, DS3231_A1_Hour);
}

void setAlarmClock(DateTime dt) {
  //note that we use the day to store the alarm mode. Since we do not want to change the mode, we must preserve the year of the current alarm
  DateTime alarm = getAlarmClock();
  DateTime newAlarm = DateTime(dt.year(), dt.month(), alarm.day(), dt.hour(), dt.minute(), dt.second());

  String stateStr = "enabled";
  Serial.print("Setup alarm: ");
  Serial.print(getFormattedTimeWithoutDate(newAlarm));
  Serial.print(" - mode: ");
  Serial.print(parseAlarmClockModeString(newAlarm.day()));
  Serial.print(" - state: ");
  Serial.println(stateStr);

  rtc.setAlarm1(newAlarm, DS3231_A1_Hour);
}

boolean isAlarmClockEnabled() {
  return rtc.isAlarm1InterruptEnabled();
}

boolean isAlarmClockTriggered() {
  boolean fired = rtc.alarmFired(1);
  boolean enabled = isAlarmClockEnabled();
  boolean modeMatchingNow = isAlarmClockModeMatchingNow();
  if(fired && enabled && modeMatchingNow) {
    return true;
  }

  if(fired && ! enabled) {
    Serial.println("Alarm 1 flag is active but alarm is disabled - skipping alarm");
  }

  if(fired && ! modeMatchingNow) {
    Serial.println("Alarm 1 flag is active the alarm-mode does not fit the current day of week - skipping alarm");
  }

  if(fired) {
    //We clear the alarm flag on false-positive alarm events
    rtc.clearAlarm(1);
  }
  return false;
}

void clearAlarmClockFlag() {
  if(rtc.alarmFired(1)) {
    rtc.clearAlarm(1);
  }
}

void toggleAlarmClockEnabled() {
  DateTime alarm = rtc.getAlarm1();
  if(isAlarmClockEnabled()) {
    rtc.disableAlarm(1);
  } else {
    setAlarmClock(alarm);
  }
}

DateTime getAlarmClock() {
  return rtc.getAlarm1();
}

void adjustClockAndSetupTimezoneHandling(DateTime dt) {
  rtc.adjust(dt);
  Serial.print("Adjusted clock - new time: ");
  serialPrintTime(dt);

  pLastTimezoneChangeUxt = DATE_TIME_MIN;
  applyPendingTimezoneChanges();
}

boolean handleTimezoneAlarmAndIsClockAdjusted() {
  if(rtc.alarmFired(2)) {
    Serial.println("Timezone alarm fired");
    return applyPendingTimezoneChanges();
  }
  return false;
}

boolean applyPendingTimezoneChanges() {
  boolean adjustedClock = false;
  DateTime lastChangeBefore = DateTime(pLastTimezoneChangeUxt);
  Serial.println("Last timezone time change was: " + getFormattedTime(lastChangeBefore, true));

  if(lastChangeBefore.unixtime() == DATE_TIME_MIN) {
    //this happens, when the system first booted and did not store the last time zone change before
    lastChangeBefore = rtc.now();
  }
  int lastTimezoneAppliedIndex = -1;
  const int tsLength = sizeof(TIMEZONE_CHANGES) / sizeof(TIMEZONE_CHANGES[0]);

  for(int i = 0; i < tsLength; i++) {
    timezone_change timezone = TIMEZONE_CHANGES[i];
    if(lastChangeBefore.unixtime() >= timezone.when.unixtime()) {
      //timezone-change already applied - continue searching
      lastTimezoneAppliedIndex = i;
      continue;
    }

    //--------------
    //Now, handle all timezone changes not yet applied.
    //Note that when the system was offline, multiple clock adjustments might be necessary in a row.
    //--------------

    DateTime now = rtc.now();
    if(now.unixtime() >= timezone.when.unixtime()) {
      now = rtc.now();
      uint32_t ntu = now.unixtime() + (timezone.offset * 60 * 60);

      DateTime changed = DateTime(ntu);
      rtc.adjust(changed);

      Serial.println("Clock-adjustment (timezone): ");
      Serial.print("old time: ");
      serialPrintTime(now);
      Serial.print("new time: ");
      serialPrintTime(changed);

      adjustedClock = true;
      lastTimezoneAppliedIndex = i;
    }
  }
  setupNextTimezoneAlarm(lastTimezoneAppliedIndex);

  //it is importent that we do not stroe rtc.now() as last timezone change - if the time was set back, the same change would be applied forever
  if(lastTimezoneAppliedIndex >= 0) {
    pLastTimezoneChangeUxt = TIMEZONE_CHANGES[lastTimezoneAppliedIndex].when.unixtime();
  } else {
    pLastTimezoneChangeUxt = DATE_TIME_MIN;
  }
  Serial.println("Last timezone time change stored: " + getFormattedTime(DateTime(pLastTimezoneChangeUxt), true));
  savePrefs();

  return adjustedClock;
}

void setupNextTimezoneAlarm(int lastTimezoneAppliedIndex) {
  if(rtc.alarmFired(2)) {
    rtc.clearAlarm(2);
  }

  int nextIndex = lastTimezoneAppliedIndex + 1;
  const int tsLength = sizeof(TIMEZONE_CHANGES) / sizeof(TIMEZONE_CHANGES[0]);
  if(tsLength <= nextIndex) {
    Serial.print("No next timezone change found - alarm2 disabled");
    rtc.disableAlarm(2);
  } else {
    timezone_change next = TIMEZONE_CHANGES[nextIndex];
    rtc.setAlarm2(next.when, DS3231_A2_Date);

    Serial.print("Next change in timezone: ");
    serialPrintTime(next.when);

    Serial.print("On next DST, clock will be corrected by offset: ");
    Serial.print(next.offset, DEC);
    Serial.println();
  }
}

uint32_t getNextTimeChangeUnixtime() {
  DateTime lastChange = DateTime(pLastTimezoneChangeUxt);

  const int tsLength = sizeof(TIMEZONE_CHANGES) / sizeof(TIMEZONE_CHANGES[0]);
  for(int i = 0; i < tsLength; i++) {
    timezone_change timezone = TIMEZONE_CHANGES[i];
    if(lastChange.unixtime() >= timezone.when.unixtime()) {
      //timezone-change already applied - continue searching
      continue;
    }

    //--------------
    //Now, handle all timezone changes not yet applied.
    //Note that when the system was offline, multiple clock adjustments might be necessary in a row.
    //--------------

    DateTime now = rtc.now();
    if(now.unixtime() >= timezone.when.unixtime()) {
      return timezone.when.unixtime();
    }
  }
  return 0;
}