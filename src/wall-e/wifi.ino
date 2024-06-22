boolean isWiFiConfigured() {
  return strlen(WIFI_SSID) > 0;
}

void enableWifi(boolean waitUntilConnected) {
  WiFi.setMinSecurity(gWifiSecurityMode);
  WiFi.disconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PWD);

  while (WiFi.status() != WL_CONNECTED && waitUntilConnected) {
      delay(500);
      Serial.println("Waiting for Wifi...");
  }

  if(waitUntilConnected) {
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
  }
}

void disableWifi() {
  WiFi.disconnect(true);  // Disconnect from the network
  WiFi.mode(WIFI_OFF);    // Switch WiFi off
}
