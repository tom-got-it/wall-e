DateTime lightBulbStartedAt;
boolean lightBulbIsActive = false;

boolean startLightBulb() {
  if(isBatteryLowVoltage()) {
    Serial.println("Voltage is too low. Don't turn on light bulb");
    return false;
  }

  Serial.println("Starting light bulb");
  lightBulbStartedAt = rtc.now();
  lightBulbIsActive = true;
  digitalWrite(PIN_LIGHT_BULB, HIGH);
  delay(500);
  return true;
}

/**
* Will do a shudown if the configured amount of time has passed.
*/
boolean testLightBulbStillActive() {
  if(! lightBulbIsActive) {
    return false;
  }

  DateTime now = rtc.now();
  if(lightBulbStartedAt.unixtime() + gLightBulbAutoShutdownSeconds < now.unixtime()) {
    Serial.println("Auto-Shutdown light bulb due to timeout seconds");
    disableLightBulb();
    return false;
  }

  if(isBatteryLowVoltage()) {
    Serial.println("Voltage is too low. Shutting down light bulb");
    disableLightBulb();
    return false;
  }

  return true;
}

void disableLightBulb() {
  Serial.println("Disabling light bulb now if it was active (forcing it).");
  digitalWrite(PIN_LIGHT_BULB, LOW);
  lightBulbIsActive = false;
  delay(500);
}
