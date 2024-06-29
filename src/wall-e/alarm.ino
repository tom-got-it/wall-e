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

void handleTimezoneAlarm() {
    const int tsLength = sizeof(TIMEZONE_CHANGES) / sizeof(TIMEZONE_CHANGES[0]);

    //Alarm 2 = TimeZone correction fired?
    if(rtc.alarmFired(2)) {
      Serial.println("Timezone alarm fired");

      Serial.println("Clock needs correction due to TimeZone");
      DateTime tz = rtc.getAlarm2();
      DateTime now = rtc.now();

      //note that the alarm will wake us up every month at the same day
      //we need to correct the year + month
      tz = DateTime(now.year(), now.month(), tz.day(), tz.hour(), tz.minute(), 0);

      for(int i = 0; i < tsLength; i++) {
        timezone_change elem = TIMEZONE_CHANGES[i];
        if(elem.when.unixtime() == tz.unixtime()) {
          Serial.print("Correct timezone by offset: ");
          Serial.print(elem.offset, DEC);
          Serial.println();

          DateTime dt = rtc.now();
          DateTime changed = DateTime(dt.year(), dt.month(), dt.day(), dt.hour() + elem.offset, dt.minute(), dt.second());
          rtc.adjust(changed);
          lastTimezoneChange = elem.when;

          Serial.print("old time: ");
          serialPrintTime(dt);
          Serial.print("new time: ");
          serialPrintTime(changed);
        }
      }
      rtc.clearAlarm(2);
      setupNextTimezoneAlarm();
    }
}

timezone_change getNextTimezoneChangeElseFirst() {
  const int tsLength = sizeof(TIMEZONE_CHANGES) / sizeof(TIMEZONE_CHANGES[0]);

  DateTime now = rtc.now();
  timezone_change candidate = TIMEZONE_CHANGES[0];
  int i = 1;
  while(i < tsLength && 
      (now.unixtime() > candidate.when.unixtime() || lastTimezoneChange.unixtime() == candidate.when.unixtime())
    ) {
    //when setting the clock an hour back, we must make sure not to trigger the same timezone change again

    candidate = TIMEZONE_CHANGES[i];
    i++;
  }
  return candidate;
}

void setupNextTimezoneAlarm() {
  rtc.clearAlarm(2);

  timezone_change next = getNextTimezoneChangeElseFirst();
  DateTime now = rtc.now();
  if(now.unixtime() < next.when.unixtime()) {
    Serial.print("Next change in timezone: ");
    serialPrintTime(next.when);

    Serial.print("On next DST, clock will be corrected by offset: ");
    Serial.print(next.offset, DEC);
    Serial.println();

    rtc.setAlarm2(next.when, DS3231_A2_Date);
  } else {
    Serial.print("No next timezone change found - alarm2 disabled");
    rtc.disableAlarm(2);
  }
}