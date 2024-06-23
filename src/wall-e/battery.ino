float getBatVoltage() {
  if(isBatterySensorConnected) {
    return getBatVoltageBySensor();
  }
  return 0;
}

float getBatVoltageBySensor() {
  float batVoltage = batterySensor.readBusVoltage() / 1000.0;
  return batVoltage;
}

float getBatCurrentMillis() {
  if(isBatterySensorConnected) {
    return batterySensor.readCurrent();
  }
  return -1;
}

boolean isBatteryLowVoltage() {
  return !isOnUsbPower() && getBatVoltage() <= gLowVoltage;
}

boolean isOnUsbPower() {
  return getBatCurrentMillis() <= 5;
}

boolean isChargingBattery() {
  return getBatCurrentMillis() <= -5;
}