#include "hal/gpio_types.h"
#include "FS.h"
#include "RTClib.h"
#include "TFT_eSPI.h"
#include "driver/rtc_io.h"
#include <DFPlayerMini_Fast.h>
#include <Adafruit_INA260.h>
#include <AnimatedGIF.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include "timezone.h"
#include "./img/bitmap.h"
#include "./gif/walle-gif-main.h"

//---------------------------------------------------------------
//-------------------Modify these for your needs-----------------
//---------------------------------------------------------------

//================= GLOBAL DEFINITIONS ==========================
#define FIRMWARE_VERSION "2024.06.4"          //Put the firmware version here - just for info
#define CALIBRATION_FILE "/calibrationData11" //Interal filename for the calibrated display data
#define REPEAT_CAL false                      //Repeat display calibration
#define INIT_CLOCK false                      //Only required when clock-battery is changed or the clock was reset before
#define WIFI_SSID ""
#define WIFI_PWD ""
//===============================================================

//================= OTHER CONSTANTS =============================
const int gSleepSecondsAfterTouched = 40;             //after the last touch event the time is monitored. If x seconds past and no further touch event occured the ESP32 will go to sleep mode (=standby)
const int gVoltageReprintAfterSeconds = 2;            //interval to read and print battery voltage
const int gAlarmAutoShutdownSeconds = 120;            //an active alarm will be turned off after this time has passed
const int gDefaultLightBulbDelaySeconds = 0;          //x seconds after the alarm starts, the light bulb will turn on (-1 = light always off at alarm, 0 = immediate on at alarm)
const int gLightBulbAutoShutdownSeconds = 30;         //when the light bulb is turned on, it will shutdown after the given seconds (safety faeture)
const float gLowVoltage = 3.4;                        //light bulb will be disabled on alarm when the battery voltage drops below this level
const int gMp3CountInitSound = 2;                     //the first two MP3s are played only at first boot

const wifi_auth_mode_t gWifiSecurityMode = WIFI_AUTH_WPA2_WPA3_PSK;   //WiFi Security Mode

const char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
//===============================================================




//---------------------------------------------------------------
//-------------------Be carefull when modifying here-------------
//---------------------------------------------------------------

//================= GLOBAL DEFINITIONS ==========================
#define FPSerial Serial1
#define DEG2RAD 0.0174532925
//===============================================================

//================= OTHER CONSTANTS =============================
const int TFT_MAIN_FONT = 7;        //48 pixels - special chars only
const int TFT_SMALL_FONT = 2;       //16 pixels
const int TFT_BIG_FONT = 4;         //26 pixels
const int TFT_MAIN_COLOR = 0xFDE0;  //Wallee-yellow
//===============================================================

//================= PIN-ASSIGNMENTS =============================
const gpio_num_t PIN_TFT_IRQ = GPIO_NUM_33;
const gpio_num_t PIN_LCD_LED = GPIO_NUM_32;
const gpio_num_t PIN_RTC_IRQ = GPIO_NUM_27;
const gpio_num_t PIN_MP3_RX = GPIO_NUM_16;
const gpio_num_t PIN_MP3_TX = GPIO_NUM_17;
const gpio_num_t PIN_LIGHT_BULB = GPIO_NUM_25;
const gpio_num_t PIN_BATTERY_SENSOR_SDA = GPIO_NUM_13;  //Adafruit BAT-Sensor
const gpio_num_t PIN_BATTERY_SENSOR_SCL = GPIO_NUM_26;  //Adafruit BAT-Sensor
const uint64_t WAKEUP_BITMASK_RTC = 0x8000000; // PIN27
//===============================================================


//================= RTC KEPT VARS ===============================
RTC_DATA_ATTR DateTime firstBootTime;
RTC_DATA_ATTR DateTime lastTimezoneChange;
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR boolean isAlarmActive = true;
RTC_DATA_ATTR int notificationVolume = 17;
RTC_DATA_ATTR int mp3TrackCount = 0;
RTC_DATA_ATTR int showBatteryStatistics = 0;  //1=show voltage, 2=show current
RTC_DATA_ATTR int lightBulbDelaySecondsOnAlarm = gDefaultLightBulbDelaySeconds;

RTC_DATA_ATTR boolean rtcLostPowerNotification = false;
RTC_DATA_ATTR boolean missedAlarmNotification = false;
//===============================================================


//========================== TYPES ===============================
typedef struct {
  int x;
  int y;
  int width;
  int height;
} rect;
//===============================================================


//================ OTHERS =======================================
DateTime lastToched;
boolean isBatterySensorConnected = false;
boolean isFirmwareUpdateInProgress = false;

TFT_eSPI tft = TFT_eSPI(); 
RTC_DS3231 rtc;
DFPlayerMini_Fast myMP3;

TwoWire batterySensorI2c = TwoWire(1);
Adafruit_INA260 batterySensor = Adafruit_INA260();

AnimatedGIF gif;
AsyncWebServer server(80);
//================================================================
