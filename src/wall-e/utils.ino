int drawTwoDigitsAndGetXPos(uint8_t number, int xPos, int yPos, const int font) {
    if (number < 10) xPos += tft.drawChar('0', xPos, yPos, font);
    xPos += tft.drawNumber(number, xPos, yPos, font);      
    return xPos;
}

void drawWaitingScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_MAIN_COLOR);
  tft.drawString("Just one moment please...", 10, 90, TFT_BIG_FONT);
}

void drawVoltageWarningScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED);
  tft.drawString("Battery voltage is low...", 25, 90, TFT_BIG_FONT);
  tft.setTextColor(TFT_MAIN_COLOR);
}

void drawNoWiFiWarningScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED);
  tft.drawString("WiFi is not configured...", 25, 90, TFT_BIG_FONT);
  tft.setTextColor(TFT_MAIN_COLOR);
}

void drawEmptyMainScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.drawBitmap(0, 0, epd_bitmap_Interface_2, 320, 240, TFT_MAIN_COLOR);
  tft.setTextColor(TFT_MAIN_COLOR, TFT_BLACK);
}


void showEmptyDelayScreen(int delayMs) {
  clearScreen();
  delay(delayMs);
}

void turnOnDisplay() {
  digitalWrite(PIN_LCD_LED, HIGH);
}

void turnOffDisplay() {
  digitalWrite(PIN_LCD_LED, LOW);
}

boolean checkTouch(uint16_t x, uint16_t y, const int xRange[], const int yRange[]) {
    if(x > xRange[0] && x < xRange[1]) {
      if(y > yRange[0] && y < yRange[1]) {
        return true;
      }
    }
    return false;
}

void clearScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_MAIN_COLOR);
}

void printTriangleUpDown(const rect *t, int color) {
  int tx = t->x;
  int ty = t->y;
  int tw = t->width;
  int th = t->height;
  tft.fillTriangle(tx, ty, tx + (tw / 2), ty + th, tx + tw, ty, color);
}

void printTriangleLeftRight(const rect *t, int color) {
  int tx = t->x;
  int ty = t->y;
  int tw = t->width;
  int th = t->height;
  tft.fillTriangle(tx, ty, tx + tw, ty + (th / 2), tx, ty + th, color);
}

void printRect(const rect *t, int color, boolean fill) {
  if(fill) {
    tft.fillRect(t->x, t->y, t->width, t->height, color);
  } else {
    tft.drawRect(t->x, t->y, t->width, t->height, color);
  }
}

boolean touchedRect(int x, int y, const rect *t) {
  int tx = t->x;
  int ty = t->y;
  int tw = t->width;
  int th = t->height;

  if(th < 0) {
    th = -th;
    ty = ty - th;
  }

  if(tw < 0) {
    tw = -tw;
    tx = tx - tw;
  }

  if(x > tx && x < (tx + tw)) {
    if(y > ty && (y < ty + th)) {
      return true;
    }
  }
  return false;
}

void fillArc(int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour)
{

  byte seg = 6; // Segments are 3 degrees wide = 120 segments for 360 degrees
  byte inc = 6; // Draw segments every 3 degrees, increase to 6 for segmented ring

  // Calculate first pair of coordinates for segment start
  float sx = cos((start_angle - 90) * DEG2RAD);
  float sy = sin((start_angle - 90) * DEG2RAD);
  uint16_t x0 = sx * (rx - w) + x;
  uint16_t y0 = sy * (ry - w) + y;
  uint16_t x1 = sx * rx + x;
  uint16_t y1 = sy * ry + y;

  // Draw colour blocks every inc degrees
  for (int i = start_angle; i < start_angle + seg * seg_count; i += inc) {

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * DEG2RAD);
    float sy2 = sin((i + seg - 90) * DEG2RAD);
    int x2 = sx2 * (rx - w) + x;
    int y2 = sy2 * (ry - w) + y;
    int x3 = sx2 * rx + x;
    int y3 = sy2 * ry + y;

    tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
    tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);

    // Copy segment end to segment start for next segment
    x0 = x2;
    y0 = y2;
    x1 = x3;
    y1 = y3;
  }
}

String padByZeroTwoDigits(int in) {
  String out = "";
  if(in < 10) {
    out = out + "0";
  }
  out = out + String(in);
  return out;
}

String padByZeroThreeDigits(int in) {
  String out = "";
  if(in < 10) {
    out = out + "00";
  } else if (in < 100) {
    out = out + "0";
  }
  out = out + String(in);
  return out;
}

String getFormattedTime(DateTime dt, boolean printWeekday) {
    String result = "";
    result = result + dt.year() + "/" + dt.month() + "/" + dt.day() + " ";
    if(printWeekday) {
      result = result + "(" + daysOfTheWeek[dt.dayOfTheWeek()] + ")";
    }
    result = result + padByZeroTwoDigits(dt.hour()) + ":" + padByZeroTwoDigits(dt.minute()) + ":" + padByZeroTwoDigits(dt.second());
    return result;
}

String getFormattedTimeWithoutDate(DateTime dt) {
    String result = "";
    result = result + padByZeroTwoDigits(dt.hour()) + ":" + padByZeroTwoDigits(dt.minute()) + ":" + padByZeroTwoDigits(dt.second());
    return result;
}

void serialPrintTime(DateTime dt) {
    Serial.println(getFormattedTime(dt, true));
}