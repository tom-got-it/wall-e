void doSetup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("Running setup...");

    ++bootCount;
    Serial.printf("\nBoot number: %d\n", bootCount);

    String version = FIRMWARE_VERSION;
    String fwr = "Firmware Version: " + version;
    Serial.println(fwr);

    printWaekeupReason();

    setupRTC();
    setupBatterySensor();
    setupLightBulb();
    setupFilesystem();
    setupTFT();
    setupMp3();

    if(bootCount == 1) {
      setupFirstBoot();
    }

    prepareWebServer();

    //On boot, set the last time touched as now
    lastToched = rtc.now();
    drawEmptyMainScreen();
    registerDeepSleepWakeups();
    Serial.println("Ending setup...");
}

void setupTFT() {
  Serial.println("----------Setup TFT--------------");

  //Backlight
  pinMode(PIN_LCD_LED, OUTPUT);
  digitalWrite(PIN_LCD_LED, HIGH);

  //TFT IRQ used in order to wake-up
  pinMode(PIN_TFT_IRQ, INPUT_PULLUP);

  //init
  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);

  //Calibrate if necessary
  calibrateTFT();

  //Fill the screen with black
  tft.fillScreen(TFT_BLACK);
}

void drawEmptyMainScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.drawBitmap(0, 0, epd_bitmap_Interface_2, 320, 240, TFT_MAIN_COLOR);
  tft.setTextColor(0xFDE0, TFT_BLACK);
}

void setupFilesystem() {
  if (!SPIFFS.begin()) {
    Serial.println("Formatting file system");
    SPIFFS.format();
    SPIFFS.begin();
  }
}

void calibrateTFT() {
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
}

void setupRTC() {
  Serial.println("----------Setup RTC--------------");

  pinMode(PIN_RTC_IRQ, INPUT_PULLUP);

  rtc.begin();
  rtc.disable32K();                   //we don't need the 32K Pin, so disable it
  rtc.writeSqwPinMode(DS3231_OFF);    // stop oscillating signals at SQW Pin, otherwise setAlarm1 will fail

  boolean lostPower = rtc.lostPower();

  if(INIT_CLOCK || lostPower) {
    Serial.println("RTC lost power or forced to adjust");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.print("New Time: ");
    serialPrintTime(rtc.now());

    rtc.clearAlarm(1);                // set alarm 1, 2 flag to false (so alarm 1, 2 didn't happen so far)
    rtc.clearAlarm(2);
    rtc.disableAlarm(1);
    rtc.disableAlarm(2);

    lastTimezoneChange = rtc.now();
    setupNextTimezoneAlarm();

    //keep the lost power flag in memory until the user opens the notifications
    rtcLostPowerNotification = rtcLostPowerNotification || lostPower;
  }
}

void setupMp3() {
  Serial.println("--------Setup MP3 Module----------");

  initMp3Utils();

  FPSerial.begin(9600, SERIAL_8N1, PIN_MP3_RX, PIN_MP3_TX);
  myMP3.begin(FPSerial, false, 500);

  //Test the connection (only on first boot)
  if(bootCount == 1) {
    mp3TrackCount = getMp3TrackCount();
    Serial.print("MP3 track count: ");
    Serial.println(mp3TrackCount);
    if(bootCount == 1 && mp3TrackCount <= 0) {
      Serial.println(F("Unable to begin:"));
      Serial.println(F("1.Please recheck the connection!"));
      Serial.println(F("2.Please insert the SD card!"));

      tft.fillScreen(TFT_BLACK);
      tft.drawString("No MP3s found - playback will be disabled.", 20, 112, 2);
      delay(8000);
      drawEmptyMainScreen();
    }
    setMp3Volume(notificationVolume);
    playMp3RandomBootSound();
  }

  //No option to wakeup :/
  //myMP3.standbyMode();
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

void setupFirstBoot() {
  Serial.println("--------Setup first boot----------");

  drawWalleGifMain();
  delay(1000);
  firstBootTime = rtc.now();
  lastTimezoneChange = rtc.now();
}

void prepareWebServer() {
  Serial.println("--------Prepare Web-Server----------");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "Wall-E is ready to receive firmware updates.");
  });
  ElegantOTA.begin(&server);
  ElegantOTA.setAutoReboot(false);
}

void printWaekeupReason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by TFT IRQ (touch event)"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by RTC IRQ (alarm)"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}

void registerDeepSleepWakeups() {
    esp_sleep_enable_ext0_wakeup(PIN_TFT_IRQ, 0);
    
    rtc_gpio_pullup_en(PIN_RTC_IRQ);
    esp_sleep_enable_ext1_wakeup(WAKEUP_BITMASK_RTC, ESP_EXT1_WAKEUP_ALL_LOW);
}

