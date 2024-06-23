const rect rUpdateBack = {80, 170, 160, 50};
const rect rUpdateStatusClear = {0, 0, 320, 150};
const int xUpdateBack = 120;
const int yUpdateBack = 185;

enum update_status {
  INIT,
  STARTING_WIFI,
  WIFI_CONNECTED,
  UPDATE_AVAILABLE,
  UPDATE_INCOMING,
  UPDATE_ERROR,
  UPDATE_SUCCESS,
  DUMMY,
};

update_status updateCurrentStatus = INIT;
update_status updateStatusLastPrinted = DUMMY;

unsigned long otaProgressMillis = 0;

void showFirmwareUpdateScreen() {
  if(! isWiFiConfigured()) {
    drawNoWiFiWarningScreen();
    delay(3000);
    return;
  }

  switch(updateCurrentStatus) {
    case INIT:
      isFirmwareUpdateInProgress = true;
      printStatusInit();
      enableWifi(false);
      startWebServer();
      updateCurrentStatus = STARTING_WIFI;
      break;
    case STARTING_WIFI:
      if(WiFi.isConnected()) {
        printStatusWifiConnected();
        updateCurrentStatus = WIFI_CONNECTED;
      }
      break;
    case WIFI_CONNECTED:
      //nothing
      break;
    case UPDATE_AVAILABLE:
      printStatusUpdateAvailable();
      break;
    case UPDATE_INCOMING:
      printStatusUpdateIncoming();
      break;
    case UPDATE_SUCCESS:
      printStatusUpdateSuccess();
      break;
    case UPDATE_ERROR:
      printStatusUpdateError();
      break;
  }

  uint16_t x,y;
  boolean touched = tft.getTouch(&x, &y);
  if(touched && touchedRect(x, y, &rUpdateBack)) {
    if(updateCurrentStatus == UPDATE_SUCCESS || updateCurrentStatus == UPDATE_ERROR) {
      ESP.restart();
    }

    Serial.println("Exit Firmware Update Process - stopping WLAN and Web-Server...");
    server.end();
    disableWifi();
    rePrintMainScreen();
    isFirmwareUpdateInProgress = false;
    updateCurrentStatus = INIT;
    updateStatusLastPrinted = DUMMY;
    return;
  }

  delay(80);
}


void printStatusInit() {
  if(updateCurrentStatus == updateStatusLastPrinted) {
    return;
  }

  clearScreen();
  tft.drawString("Starting WiFi and Server...", 40, 30, TFT_SMALL_FONT);
  printUpdateExitButton("Abort");
  Serial.println("Init update...");

  updateStatusLastPrinted = INIT;
}

void printStatusWifiConnected() {
  if(updateCurrentStatus == updateStatusLastPrinted) {
    return;
  }

  printRect(&rUpdateStatusClear, TFT_BLACK, true);
  tft.drawString("Ready to receive updates...", 40, 30, TFT_SMALL_FONT);

  String url = "";
  url = "URL: http://";
  url = url + WiFi.localIP().toString();
  url = url + "/update";
  tft.drawString(url, 40, 70, TFT_SMALL_FONT); 

  Serial.println("WiFi connected - URL for updates is: " + url);

  updateStatusLastPrinted = WIFI_CONNECTED;
}

void printStatusUpdateAvailable() {
  if(updateCurrentStatus == updateStatusLastPrinted) {
    return;
  }

  printRect(&rUpdateStatusClear, TFT_BLACK, true);
  tft.drawString("Ready to receive updates...", 40, 30, TFT_SMALL_FONT);

  String url = "";
  url = "URL: http://";
  url = url + WiFi.localIP().toString();
  url = url + "/update";
  tft.drawString(url, 40, 70, TFT_SMALL_FONT); 

  tft.drawString("Update available...", 40, 110, TFT_SMALL_FONT);

  Serial.println("Update is available and will be downloaded...");

  updateStatusLastPrinted = UPDATE_AVAILABLE;
}

void printStatusUpdateIncoming() {
  if(updateCurrentStatus == updateStatusLastPrinted) {
    return;
  }

  printRect(&rUpdateStatusClear, TFT_BLACK, true);
  tft.drawString("Ready to receive updates...", 40, 30, TFT_SMALL_FONT);

  String url = "";
  url = "URL: http://";
  url = url + WiFi.localIP().toString();
  url = url + "/update";
  tft.drawString(url, 40, 70, TFT_SMALL_FONT); 

  tft.drawString("Downloading update...", 40, 110, TFT_SMALL_FONT);

  Serial.println("downloading update...");

  updateStatusLastPrinted = UPDATE_INCOMING;
}

void printStatusUpdateError() {
  if(updateCurrentStatus == updateStatusLastPrinted) {
    return;
  }

  printRect(&rUpdateStatusClear, TFT_BLACK, true);

  tft.drawString("Ready to receive updates...", 40, 30, TFT_SMALL_FONT);

  String url = "";
  url = "URL: http://";
  url = url + WiFi.localIP().toString();
  url = url + "/update";
  tft.drawString(url, 40, 70, TFT_SMALL_FONT); 

  tft.setTextColor(TFT_RED);
  tft.drawString("Update failed", 40, 110, TFT_SMALL_FONT);
  tft.setTextColor(TFT_MAIN_COLOR);
  printUpdateExitButton("Reboot");

  Serial.println("Error downloading update...");

  updateStatusLastPrinted = UPDATE_ERROR;
}

void printStatusUpdateSuccess() {
  if(updateCurrentStatus == updateStatusLastPrinted) {
    return;
  }

  printRect(&rUpdateStatusClear, TFT_BLACK, true);

  tft.drawString("Ready to receive updates...", 40, 30, TFT_SMALL_FONT);

  String url = "";
  url = "URL: http://";
  url = url + WiFi.localIP().toString();
  url = url + "/update";
  tft.drawString(url, 40, 70, TFT_SMALL_FONT); 

  tft.setTextColor(TFT_GREEN);
  tft.drawString("Update received successfully", 40, 110, TFT_SMALL_FONT);
  tft.setTextColor(TFT_MAIN_COLOR);
  printUpdateExitButton("Reboot");

  Serial.println("Success downloading update...");

  updateStatusLastPrinted = UPDATE_SUCCESS;
}


void printUpdateExitButton(String label) {
  printRect(&rUpdateBack, TFT_BLACK, true);  //reset
  printRect(&rUpdateBack, TFT_MAIN_COLOR, false);
  tft.drawString(label, xUpdateBack, yUpdateBack, TFT_BIG_FONT);
}

void startWebServer() {
  Serial.println("--------Setup Web-Server----------");

  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);

  server.begin();
}

void onOTAStart() {
  updateCurrentStatus = UPDATE_AVAILABLE;
  Serial.println("OTA update started!");
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - otaProgressMillis > 1000) {
    otaProgressMillis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
  updateCurrentStatus = UPDATE_INCOMING;
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    updateCurrentStatus = UPDATE_SUCCESS;
    Serial.println("Update success");
  } else {
    updateCurrentStatus = UPDATE_ERROR;
    Serial.println("Update error");
  }
}
