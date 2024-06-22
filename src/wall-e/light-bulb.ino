DateTime lightBulbStartedAt;

boolean startLightBulb() {
  if(getBatVoltage() < gLowVoltage && !isOnUsbPower()) {
    Serial.println("Voltage is too low. don't turn on light bulb");
    return false;
  }

  Serial.println("Starting light bulb");
  lightBulbStartedAt = rtc.now();
  digitalWrite(PIN_LIGHT_BULB, HIGH);
  delay(500);
  return true;
}

boolean testWhileLightBulbActive() {
  DateTime now = rtc.now();
  if(lightBulbStartedAt.unixtime() + gLightBulbAutoShutdownSeconds < now.unixtime()) {
    Serial.println("Auto-Shutdown light bulb due to timeout seconds");
    disableLightBulb();
    return false;
  }

  if(getBatVoltage() < gLowVoltage && !isOnUsbPower()) {
    Serial.println("Voltage is too low. Shutting down light bulb");
    disableLightBulb();
    return false;
  }

  return true;
}

void disableLightBulb() {
  Serial.println("Disabling light bulb now (force).");
  digitalWrite(PIN_LIGHT_BULB, LOW);
  delay(500);
}
