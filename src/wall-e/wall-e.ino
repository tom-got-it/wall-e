#include "./include/global.h"

void setup() {
    doSetup();
}

void loop() {
    if(handleTimezoneAlarmAndIsClockAdjusted()) {
      lastToched = rtc.now();
    }

    if(isFirmwareUpdateInProgress) {
      showFirmwareUpdateScreen();
      lastToched = rtc.now();
    } else if(isAlarmClockTriggered()) {
      showAlarmScreenClock();
      rePrintMainScreen();
      lastToched = rtc.now();
    } else {
      printMainScreen();
      handleMainTouch();
    }

    exitLoop();
}

void exitLoop() {
  DateTime now = rtc.now();

  if(gSleepSecondsAfterTouched < 0) {
    //do not go to sleep
    return;
  }

  //Note that we don't go to sleep when updates are in progress
  //We must leave the main-loop in order for the update-server to handle incoming HTTP-requests
  if(now.unixtime() >= lastToched.unixtime() + gSleepSecondsAfterTouched && ! isFirmwareUpdateInProgress) {
    Serial.println("Going to sleep now");
    Serial.flush();
    esp_deep_sleep_start();
    Serial.println("This will never be printed");
  }
}