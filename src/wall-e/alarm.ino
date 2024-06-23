boolean hasAlarmFired() {
  return rtc.alarmFired(1) && isAlarmActive;
}

void toggleAlarm() {
  DateTime alarm = rtc.getAlarm1();

  if(isAlarmActive) {
    rtc.disableAlarm(1);
  } else {
    rtc.setAlarm1(alarm, DS3231_A1_Hour);
  }
  isAlarmActive = !isAlarmActive;
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