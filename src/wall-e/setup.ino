void doSetup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("Running setup...");

    ++bootCount;
    Serial.printf("\nBoot number: %d\n", bootCount);

    String version = FIRMWARE_VERSION;
    String fwr = "Firmware Version: " + version;
    Serial.println(fwr);

    printWakeupReason();

    setupRTC();
    setupBatterySensor();
    setupLightBulb();
    setupFilesystem();
    setupTFT();
    setupMp3();
    prepareWebServer();

    firstBootTime = rtc.now();

    registerDeepSleepWakeups();
    checkGoBackToDeepSleep();

    //Turn on the display, calibrate, print warnings and boot animations
    turnOnDisplay();
    setupCalibrateTFT();
    setupDrawMp3Warning();
    if(bootCount == 1) {
      playBootSound();
      drawWalleGifMain();
      delay(1000);
    }

    //On boot, set the last time touched as now
    lastToched = rtc.now();
    drawEmptyMainScreen();

    Serial.println("Ending setup...");
}

void setupTFT() {
  Serial.println("----------Setup TFT--------------");

  //Backlight - by default off during the setup - we don't want to disturb the user
  pinMode(PIN_LCD_LED, OUTPUT);
  turnOffDisplay();

  //TFT IRQ used in order to wake-up
  pinMode(PIN_TFT_IRQ, INPUT_PULLUP);

  //init
  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);

  //Fill the screen with black
  tft.fillScreen(TFT_BLACK);
}

void setupFilesystem() {
  if (!SPIFFS.begin()) {
    Serial.println("Formatting file system");
    SPIFFS.format();
    SPIFFS.begin();
  }
}

void setupCalibrateTFT() {
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL) {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    } else {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 10) == 10)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    Serial.println("TFT Calibration is ok");
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    Serial.println("TFT Calibration is required");
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("TFT Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 10);
      f.close();
    }
    Serial.println("TFT Calibration finished");
  }
  tft.fillScreen(TFT_BLACK);
}

void setupRTC() {
  Serial.println("----------Setup RTC--------------");

  pinMode(PIN_RTC_IRQ, INPUT_PULLUP);

  rtc.begin();
  boolean lostPower = rtc.lostPower();

  if(INIT_CLOCK || lostPower) {
    Serial.println("RTC lost power or forced to adjust");

    rtc.writeSqwPinMode(DS3231_OFF);    // stop oscillating signals at SQW Pin, otherwise setAlarm1 will fail
    rtc.disable32K();                   //we don't need the 32K Pin, so disable it

    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.print("New Time: ");
    serialPrintTime(rtc.now());

    rtc.clearAlarm(1);                // set alarm 1, 2 flag to false (so alarm 1, 2 didn't happen so far)
    rtc.disableAlarm(1);

    lastTimezoneChange = rtc.now();
    setupNextTimezoneAlarm();

    //keep the lost power flag in memory until the user opens the notifications
    rtcLostPowerNotification = rtcLostPowerNotification || lostPower;
  } else if(bootCount == 1) {
    Serial.println("Setup RTC on first boot - clear alarms - setup timezone handling");
    clearAlarmClockFlag();

    lastTimezoneChange = rtc.now();
    setupNextTimezoneAlarm();
  }

  if(isAlarmClockEnabled()) {
    Serial.println("Note: Alarm (clock) is active and waiting");
  } else {
    Serial.println("Note: Alarm (clock) is disabled");
  }
  Serial.println("Alarm (clock) mode is: " + parseAlarmClockModeString(getAlarmClockMode()));
}

void setupMp3() {
  Serial.println("--------Setup MP3 Module----------");

  initMp3Utils();

  FPSerial.begin(9600, SERIAL_8N1, PIN_MP3_RX, PIN_MP3_TX);
  myMP3.begin(FPSerial, false, 500);

  mp3TrackCount = getMp3TrackCount();
  Serial.print("MP3 track count: ");
  Serial.println(mp3TrackCount);

  if(bootCount == 1) {
    setMp3Volume(notificationVolume);
  }

  //No option to wakeup :/
  //myMP3.standbyMode();
}

void playBootSound() {
  playMp3RandomBootSound();
}

void setupDrawMp3Warning() {
  if(mp3TrackCount <= 0) {
      tft.fillScreen(TFT_BLACK);
      tft.drawString("No MP3s found - playback will be disabled.", 20, 112, 2);
      delay(8000);
      tft.fillScreen(TFT_BLACK);
  }
}

void setupLightBulb() {
  Serial.println("--------Setup Light Bulb relay----------");
  pinMode(PIN_LIGHT_BULB, OUTPUT);
}

void setupBatterySensor() {
  Serial.println("--------Setup battery sensor----------");

  if(! batterySensorI2c.begin(PIN_BATTERY_SENSOR_SDA, PIN_BATTERY_SENSOR_SCL)) {
    Serial.println("Battery sensor not found - skipping");
    isBatterySensorConnected = false;
    return;
  }

  if(! batterySensor.begin(INA260_I2CADDR_DEFAULT, &batterySensorI2c)) {
    Serial.println("Battery sensor not found - skipping");
    isBatterySensorConnected = false;
    return;
  }

  isBatterySensorConnected = true;

  Serial.print("Battery Voltage: ");
  Serial.print(getBatVoltage());
  Serial.println(" V");

  Serial.print("Battery output current: ");
  Serial.print(getBatCurrentMillis());
  Serial.println(" mA");
}

void prepareWebServer() {
  Serial.println("--------Prepare Web-Server----------");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "Wall-E is ready to receive firmware updates.");
  });
  ElegantOTA.begin(&server);
  ElegantOTA.setAutoReboot(false);
}

void printWakeupReason()
{
  esp_sleep_wakeup_cause_t wakeupReason;
  wakeupReason = esp_sleep_get_wakeup_cause();

  switch (wakeupReason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by TFT IRQ (touch event)"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by RTC IRQ (alarm)"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeupReason); break;
  }
}

void checkGoBackToDeepSleep() {
  esp_sleep_wakeup_cause_t wakeupReason;
  wakeupReason = esp_sleep_get_wakeup_cause();

  if(wakeupReason != ESP_SLEEP_WAKEUP_EXT1) {
    //continue booting
    return;
  }

  //handle a possible time zone alarm
  handleTimezoneAlarm();

  //Wake-up triggered by RTC
  if(isAlarmClockTriggered()) {
    //continue booting
    return;
  }

  //go back to deep-sleep
  Serial.println("Going back to deep sleep - woke up by RTC and nothing more to do");
  Serial.flush();
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void registerDeepSleepWakeups() {
    esp_sleep_enable_ext0_wakeup(PIN_TFT_IRQ, 0);
    
    rtc_gpio_pullup_en(PIN_RTC_IRQ);
    esp_sleep_enable_ext1_wakeup(WAKEUP_BITMASK_RTC, ESP_EXT1_WAKEUP_ALL_LOW);
}

