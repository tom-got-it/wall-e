#include "./include/global.h"

void setup() {
    doSetup();
}

void loop() {
    handleTimezoneAlarm();

    if(isFirmwareUpdateInProgress) {
      showFirmwareUpdateScreen();

      //Prohibit to enter sleep-mode directly after exiting the update-screen.
      lastToched = rtc.now();
    } else {
      printMainScreen();
      handleMainAlarm();
      handleMainTouch();
    }

    exitLoop();
}

void exitLoop() {
  DateTime now = rtc.now();

  //Note that we don't go to sleep when updates are in progress
  //We must leave the main-loop in order for the update-server to handle incoming HTTP-requests
  if(now.unixtime() >= lastToched.unixtime() + gSleepSecondsAfterTouched && ! isFirmwareUpdateInProgress) {
    Serial.println("Going to sleep now");
    Serial.flush();
    esp_deep_sleep_start();
    Serial.println("This will never be printed");
  }
}