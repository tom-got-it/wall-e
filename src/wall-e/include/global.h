#include "hal/gpio_types.h"
#include "FS.h"
#include <Tom_RTClib.h>
#include "TFT_eSPI.h"
#include "driver/rtc_io.h"
#include <DFPlayerMini_Fast.h>
#include <Adafruit_INA260.h>
#include <AnimatedGIF.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <Preferences.h>
#include "timezone.h"
#include "types.h"
#include "./img/bitmap.h"
#include "./gif/walle-gif-main.h"

//---------------------------------------------------------------
//-------------------Modify these for your needs-----------------
//---------------------------------------------------------------

//================= GLOBAL DEFINITIONS ==========================
#define FIRMWARE_VERSION "2024.07.1"          //Put the firmware version here - just for info
#define CALIBRATION_FILE "/calibrationData11" //Internal filename for the calibrated display data - no need to modify this
#define REPEAT_CAL false                      //Repeat display calibration
#define INIT_CLOCK false                      //Usually this not required, because the clock will detect when it lost power
#define WIFI_SSID ""                          //Required optionally for over-the-air updates
#define WIFI_PWD ""                           //Required optionally for over-the-air updates
//===============================================================

//================= OTHER CONSTANTS =============================
const int gSleepSecondsAfterTouched = 40;             //-1 = Disable. After the last touch event the time is monitored. If x seconds past and no further touch event occured the ESP32 will go to sleep mode (=standby)
const int gVoltageReprintAfterSeconds = 2;            //interval to read and print battery voltage (if no battery is connected, will reprint "USB")
const int gLightBulbAutoShutdownSeconds = 30;         //when the light bulb is turned on, it will shutdown after the given seconds (safety feature)
const float gLowVoltage = 3.3;                        //Sereval things will be disabled on low voltage: Light Bulb, MP3 Player will exit, Timer will exit when not running...
const int gDisplayAutoShutdownSeconds = 15;           //When the Display is set to "Auto" mode, the MP3 player and timer will turn off the display when no interaction occured for x seconds
const int gSnoozeDisplayAutoShutdownSeconds = 15;     //Display auto-shutdown when snoozing. Display will turn back on when touched or snoozing ends.
const int gMp3CountInitSound = 2;                     //the first two MP3s are played only at first boot

const wifi_auth_mode_t gWifiSecurityMode = WIFI_AUTH_WPA2_WPA3_PSK;   //WiFi Security Mode

const char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
//===============================================================

//================= STORED PREFERENCES ==================================================
//==This settings can be modified by the user by touch and will be restored after power-loss==
#define PREFERENCES_NAME "wall-eee"                       //Some settings will be stored to the onboard flash memory. This is the namespace for these settings.
#define CLEAR_PREFERENCES false                           //Set to true to clear alle previously stored preferences. This will restore the default values.
const int gDefaultNotificationVolume = 17;                //Default MP3 playback volume on alarm
const int gDefaultLightBulbDelaySeconds = 20;             //x seconds after the alarm starts, the light bulb will turn on (-1 = light always off at alarm, 0 = immediate on at alarm)
const int gDefaultAlarmTimeoutMinutes = 2;                //An active alarm will be turned off after this time has passed and the user did not interact
const int gDefaultSnoozeMinutes1 = 5;                     //Snooze option 1 (in minutes)
const int gDefaultSnoozeMinutes2 = 10;                    //Snooze option 2 (in minutes)
const int gDefaultAlarmTouchAction = ALARM_EXIT_SNOOZE1;  //Default action when touching the display during an alarm and not hitting a button. Vaild options: ALARM_EXIT_NONE, ALARM_EXIT_TOUCH, ALARM_EXIT_SNOOZE1, ALARM_EXIT_SNOOZE2
//=======================================================================================



//---------------------------------------------------------------
//-------------------Be carefull when modifying here-------------
//---------------------------------------------------------------

//================= GLOBAL DEFINITIONS ==========================
#define FPSerial Serial1
#define DEG2RAD 0.0174532925
#define DATE_TIME_MIN 946684800
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
const gpio_num_t PIN_BATTERY_SENSOR_SDA = GPIO_NUM_13;  //Adafruit BAT-Sensor (will automaticcaly be ignored if the sensor is not connected - do not modify)
const gpio_num_t PIN_BATTERY_SENSOR_SCL = GPIO_NUM_26;  //Adafruit BAT-Sensor (will automaticcaly be ignored if the sensor is not connected - do not modify)
const uint64_t WAKEUP_BITMASK_RTC = 0x8000000; // PIN27
//===============================================================


//= ESP32 RTC KEPT VARS (will be preserved in Deep-Sleep mode) ===
RTC_DATA_ATTR uint32_t firstBootTimeUxt;
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int mp3TrackCount = 0;
RTC_DATA_ATTR int showBatteryStatistics = 0;  //1=show voltage, 2=show current

RTC_DATA_ATTR uint32_t pLastTimezoneChangeUxt;
RTC_DATA_ATTR int pLightBulbDelaySecondsOnAlarm = gDefaultLightBulbDelaySeconds;
RTC_DATA_ATTR int pNotificationVolume = gDefaultNotificationVolume;
RTC_DATA_ATTR int pAlarmTimeoutMinutes = gDefaultAlarmTimeoutMinutes;
RTC_DATA_ATTR int pSnoozeMinutes1 = gDefaultSnoozeMinutes1;
RTC_DATA_ATTR int pSnoozeMinutes2 = gDefaultSnoozeMinutes2;
RTC_DATA_ATTR int pAlarmTouchAction = gDefaultAlarmTouchAction;

RTC_DATA_ATTR int grTimerFavoriteMinutes = 2;
RTC_DATA_ATTR int grTimerFavoriteSeconds = 0;

RTC_DATA_ATTR boolean rtcLostPowerNotification = false;
RTC_DATA_ATTR boolean missedAlarmNotification = false;
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

Preferences preferences;
//================================================================
