/*===========================================================================
  Defines_and_Docs.h  -  SETTINGS, LIBRARIES AND EVERY GLOBAL VARIABLE
  ===========================================================================
  Layout of this file:
    1. User settings       (which person's config, cities, brightness table)
    2. General globals 1   (buttons, fetch interval)
    3. Library includes    (Preferences, TFT_eSPI, WiFiManager, OFR, JSON)
    4. General globals 2   (time strings, flags, colours, screen positions)
    5. OneCall globals     (current + hourly + daily arrays from OWM 3.0)
    6. Weather-code table  (OWM condition id -> short text + colours)
    7. Forecast globals    (3-hourly records from OWM 2.5 for the graphs)

  >>> REVIEW (structure): The Mike and Joe blocks below are ~45 lines each
  >>>   and only differ in: api_key, MAX_CITY, the city list and the NTP
  >>>   servers.  Everything else (orientation, FILL_GRAPH, BLANK_SECS, the
  >>>   brightness table) is a copy.  Put only the differences inside the
  >>>   #if / #else and the shared settings once after #endif - otherwise a
  >>>   change made in one block is easily forgotten in the other.
  >>>
  >>> REVIEW: Use  #define MAX_CITY (sizeof(multiCity)/sizeof(multiCity[0]))
  >>>   after the array instead of typing the count by hand.  Same trick
  >>>   for WX_CONDITIONS further down.
  ===========================================================================*/

//#define CONFIG_FOR_JOE        // Un-comment to build Joe's version.
#include <MyCredentials.h>  // Get private credentials & API Keys.
// MyCredentials.h lives in your Arduino "libraries" folder (NOT in this
// sketch folder) so the API keys never end up on GitHub.  It must define
// OWM_API_Mike and OWM_API_Joe as text strings.
/*
   This code is designed to run on a T-Display S3.

  #include <Custom_Setups/Setup206_LilyGo_T_Display_S3.h>  // Setup file for ESP32-S3

*/
//===========================================================================
// 1. USER SETTINGS
//===========================================================================
// Start of user customizations ------------------

int whichCity = 0;  // Put your preferred city in 0.  (Index into multiCity[])
int prevCity = whichCity;           // City whose data is currently shown.
bool forceDisplayOn = false;        // Set by city-change to keep screen lit.
bool cityChangeInProgress = false;  // True during the 4 s "pick a city" window.
time_t cityChangeTimer;             // millis() value when that window ends.
//                                     >>> should be uint32_t, not time_t.

// Values for tft.setRotation().  "Power" = side the USB-C socket is on.
#define ORIENT_POWER_RIGHT 1
#define ORIENT_POWER_LEFT  3

// One entry per city you can cycle through with the city button.
struct multiCityStruct {
  String CityName;  // Shown on the city-change screen only.
  String lat;     // Latitude   (text because it is pasted into the URL)
  String lon;     // Longitude
  String units;   // metric or imperial  (exact spelling matters!)
  String ENV;     // POSIX Environmental Time string
  //  ENV examples:  "PHT-8"  = Philippine time, UTC+8, no DST.
  //                 (The sign is backwards in POSIX: -8 means AHEAD of UTC.)
  //                 "PST8PDT,M3.2.0/2:00:00,M11.1.0/2:00:00" = US Pacific,
  //                 DST from 2nd Sunday of March to 1st Sunday of November.
};


#if defined CONFIG_FOR_JOE
//----------------------//
const String api_key  = OWM_API_Joe;  // for joe

#define MAX_CITY 4  // The number of cities you have defined.
multiCityStruct multiCity[MAX_CITY] = {

  { "Bangui, RP",  // City
    //Latitude    Longitude      Units       POSIX ENV Time string
    "18.5376",   "120.7671",    "metric",   "PHT-8                                 "
  },
  { "Benicia, CA",
    "38.053926", "-122.155566", "imperial", "PST8PDT,M3.2.0/2:00:00,M11.1.0/2:00:00"
  },
  { "Cuyahoga Falls, OH",
    "41.136394", "-81.484254",  "imperial", "EST5EDT,M3.2.0,M11.1.0                "
  },
  { "San Clemente, PH",
    "15.7081",   "120.3692",    "imperial", "PHT-8                                 "
  }
};
int homeCity = 1;

const char* ntpServer1 = "pool.ntp.org";    // >>> Not actually used - see
const char* ntpServer2 = "time.nist.gov";   // >>> configTime() in setup().

int myOrientation = ORIENT_POWER_LEFT;

#define FILL_GRAPH   // If defined, fill color under the graph line.
#define DO_GRADIENT  // If defined, use gradient fill for graphs.

// To make the display stay on forever, just make this 3601
#define BLANK_SECS 240  // 4 minutes on, then go to black to save the display.
// (Any value over 1800 works, because the screen auto-wakes every 30 min.)

// Backlight PWM value (0 = off ... 255 = full) for each HOME-city hour.
// >>> See setBrightness() - most screens currently override this with 200.
//                           0    1    2    3    4    5
int ihourlyBrilliance[] = { 80,  80,  80,  80,  80,  80,      //  0- 5
                            //6   7    8    9   10   11
                            80,  80,  80,  80,  80, 100,      //  6-11
                            //12  13   14   15   16   17
                            120, 130, 140, 150, 160, 160,     // 12-17
                            //18  19   20   21   22   23
                            160, 160, 140, 120,  80,  80      // 18-23
                          };
//----------------------//
#else  // for Mike
//----------------------//
const String api_key = OWM_API_Mike;

#define MAX_CITY 5  // The number of cities you have defined.
multiCityStruct multiCity[MAX_CITY] = {
  { "Bangui, RP",  // City
    //Latitude    Longitude      Units       POSIX ENV Time string
    "18.5376",   "120.7671",    "metric",   "PHT-8                                 "
  },
  { "Benicia, CA",
    "38.053926", "-122.155566", "imperial", "PST8PDT,M3.2.0/2:00:00,M11.1.0/2:00:00"
  },
  { "Dayton, OH",
    "39.75895",  "-84.19161",   "imperial", "EST5EDT,M3.2.0,M11.1.0                "
  },
  { "Shreveport, LA",
    "32.523659", "-93.763504",  "imperial", "CST6CDT,M3.2.0/2:00:00,M11.1.0/2:00:00"
  },
  { "San Clemente, PH",
    "15.7081",   "120.3692",    "metric",   "PHT-8                                 "
  },
};
int homeCity = 0;

const char* ntpServer1 = "oceania.pool.ntp.org";   // >>> Not used either.
const char* ntpServer2 = "time.nist.gov";
// >>> NOTE: For the Philippines "asia.pool.ntp.org" (or "ph.pool.ntp.org")
// >>>   is geographically closer than oceania.

//const char degree_symbol[] = "\u00B0";

int myOrientation = ORIENT_POWER_LEFT;

#define FILL_GRAPH   // If defined, fill color under the graph line.
#define DO_GRADIENT  // If defined, use gradient fill for graphs.

// To make the display stay on forever, just make this 3601
#define BLANK_SECS 240  // 4 minutes on, then go to black to save the display.

//                           0    1    2    3    4    5
int ihourlyBrilliance[] = { 80,  80,  80,  80,  80,  80,      //  0- 5
                            //6   7    8    9   10   11
                            80,  80,  80,  80,  80, 100,      //  6-11
                            //12  13   14   15   16   17
                            120, 130, 140, 150, 160, 160,     // 12-17
                            //18  19   20   21   22   23
                            160, 160, 140, 120,  80,  80      // 18-23
                          };
//----------------------//
#endif  // End for Mike //

//===========================================================================
// 2. GENERAL GLOBALS (part 1)
//===========================================================================
int localHour = -1;   // Force first pass update
char cCharWork[200];  // Character string buildup area   (>>> unused now)
time_t UTC, workTime; // workTime = scratch epoch used all over.  UTC unused.
bool bRestartPending = false;  // Set Monday 02:12 by getMyTime(); acted on
//                                at the next fetch (weekly self-reboot).

// The button is used to bring the display back to life.  The display will
//  remain on for the number of seconds of "BLANK_SECS".  Then press a button
//  to see the display again.  Easy, peasy, peachy keen.
// These 2 need to be changed for different board.
int buttonCityChange;  // On T-Display  0 with orientation 3 (power on left)
int buttonShow;        // On T-Display 14 with orientation 3 (power on left)
// (Both are filled in by setup() - see the orientation bug noted there.)

#define buttonPressed 0   // Buttons pull the pin to GND (LOW) when pressed.

// End of user customizations ------------------

// Note: If there is a city change, the fetch will be done immediately then
//       every uiFetchInterval minutes aferwards unless there is another
//       city change.
unsigned int uiFetchInterval = 31 * 60 * 1000;  // Fetch every 31 minutes
unsigned int uiNextFetchTime;   // millis() value of the next fetch.
// >>> OWM free One Call 3.0 tier = 1,000 calls/day.  Every 31 min = ~47/day
// >>>   plus city changes, so plenty of headroom.

//===========================================================================
// 3. LIBRARIES
//===========================================================================
#define RO_MODE true       // Preferences Read-only mode
#define RW_MODE false      // Preferences Read-write mode
#include "Preferences.h"   // Remember stuff that's worth remembering.
Preferences preferences;   // Stores the last chosen city in flash (NVS).

#include <TFT_eSPI.h>      // Display driver.  Needs the S3 User_Setup.
#include <JPEGDecoder.h>   // Decodes the colour-bar JPEG in colorbar.h.
#include "colorbar.h"
// >>> REVIEW: the file on disk is "colorbar.h" (lower case c).  Windows
// >>>   doesn't care, but the build fails on Linux/macOS.  Match the case.

//#include <WiFi.h>
#include "WiFiManager.h"    // https://github.com/tzapu/WiFiManager
WiFiManager wifiManager;
#include <HTTPClient.h>
const char* myPortalName = "WeatherCC";  // Setup-portal SSID AND password.

#include "OpenFontRender.h"  // Include after M5Stack.h / M5Core2.h
OpenFontRender ofr;          // TrueType renderer, used for the wake splash.
// Area to return version information (from library)
char vers[ofr.FT_VERSION_STRING_SIZE];
// Areas to return credits information (from library)
char cred[ofr.CREDIT_STRING_SIZE];   // >>> unused

//C:\Users\bangu\AppData\Local\Arduino15\packages\esp32\hardware\esp32\
  x.x.x\tools\sdk\esp32s3\include\lwip\include\apps
#include "esp_sntp.h"        // sntp_set_sync_interval / sync callback.

//===========================================================================
// 4. GENERAL GLOBALS (part 2)
//===========================================================================
struct tm * timeinfo;       // Points into localtime()'s static buffer.
time_t lastOn, testOn;      // lastOn = epoch when the screen was last woken.
char fullTimeDate[100];     // "%c" formatted date/time for Serial logs.
String sVer;                // Version text, built from the file name.
// The default start city is the 0 element of the structs array.  It is
//  usually the home city of the one you want to see the most. The selected city 
//   is now saved in Preferences.

String oneCallEndpoint, ForecastEndpoint;   // Full request URLs (incl. key).

int   prev_hour = -1, prev_sec = -1;  // "Has it changed?" trackers.
float highRainForecast = -1.;  // Unlikely to be lower than 0.  Will adjust, later.
float highSnowForecast = -1.;  // Unlikely to be lower than 0.  Will adjust, later.
float highTempForecast = -300.;  // A very low temp to ensure correction.
float lowTempForecast = 1000.;   // A very high temp to ensure correction.
// Graph frame: X axis is graphFloorMargin px above the bottom, Y axis is
//  graphLeftMargin px in from the left (room for the scale numbers).
const int graphFloorMargin = 20;   // X axis here.
const int graphLeftMargin  = 32;   // Y axis here from the left.
float pixelsPerHundredthV;  // Vertical scale of the graph being drawn.
int   timePush;             // 8 when the snow screen is inserted, else 0.

// ArduinoJson: use 32-bit float/long - smaller and fine for weather data.
// These MUST come before the #include to have any effect.
#define ARDUINOJSON_USE_DOUBLE 0
#define ARDUINOJSON_USE_LONG_LONG 0

// This needs dumbass Benoits JSON version 7
#include <ArduinoJson.h>
// >>> REVIEW: This sketch is published on GitHub; the "dumbass" remarks
// >>>   about the ArduinoJson author (here and below) don't help anyone
// >>>   reading it and reflect on you.  Worth deleting.

// Now done in the two parsing routines as per suggestion from the
//  dumbass author.
//JsonDocument doc;
//DeserializationError error;

String payload = ""; // Whole json packet returned
// >>> REVIEW: OneCall replies are ~15-20 KB.  Holding them in a String,
// >>>   then a JsonDocument as well, doubles the memory.  See the
// >>>   "filter + stream" suggestion in OneCall_Decode.ino.

// >>> The next block (tmpC ... windSpd, moonPrefix) is left over from an
// >>>   older version and is not used any more - safe to delete.
float tmpC;          // Temperature from the JSON file in Celsius.
float tmpF;          // Temperature converted to Farenheit.
int humidity;        // Humidity from the JSON file.
String windDegStr;   // Wind direction String  (USED by showCurrentCondx)
int windDegInt;      // Wind degrees integer
int windSpd;         // Wind speed in mtrs/sec
//char wxMain[100];    // Main, abbreviated conditions
String sTown;        // City name as OWM echoes it back (forecast reply).
String moonPrefix;   // R for moonrise, S for moonset.
int dataFetchHour, dataFetchMin;   // Shown as "Updated at: HH:MM".
char cMinTmp[10], cMaxTmp[10];     // Scratch for the 7-day forecast temps.

TFT_eSPI tft = TFT_eSPI();               // The physical display.
TFT_eSprite sprite = TFT_eSprite(&tft);  // Off-screen 320x170 frame buffer.

#include "fonts.h"   // Embedded fonts (BritanicBold36, EldoradoBoldTTF...).

String clockString;  // "Mon, Nov 23, 2026" - built in getMyTime() (unused on screen)
// Return the minimum of two values a and b
#define minimum(a,b)     (((a) < (b)) ? (a) : (b))

#define displayOn     true
#define displayOff    false
bool    displayStatus = displayOn;   // Is the screen currently lit?

// Colours.  The display uses 16-bit RGB565: 5 bits red, 6 green, 5 blue.
#define RGB565(r,g,b) ((((r>>3)<<11) | ((g>>2)<<5) | (b>>3)))
#define myBlue   0x0AAD    // Header-bar background on text screens.
#define myGray   0xB5B6    // Line under the clock.
#define Red      RGB565(255,0,0)
#define snowGraphLineColor TFT_WHITE
#define rainGraphLineColor RGB565(50,50,255)  // Also fill under the graph color
#define tempGraphLineColor RGB565(200,75,75)  // Also fill under the graph color
// >>> NOTE: "075" is an OCTAL number in C (= 61 decimal), not 75!  It just
// >>>   happens to look fine.  Write 75.

String IP;   // >>> unused
// Time pieces as text (for drawing) and as ints (for maths).  Filled by
// getMyTime() several times per second.
char   timeHour[3], timeMin[3], timeSec[3];
char   myMonth[10], myYear[5], myDay[5];
char   timeWeekDay[10];  // Full week day name (%A)
char   timeWkDay[4];     // Short week day (%a)
char   timeDOM[3];       // Day of the month (%d)
int    intHour, intMin, intSec, intMonth, intYear, intDOM, intDOW;
int    dayInWeek;        // 0 = Sunday
int    xCenter, yCenter; // Middle of the screen (160, 85).
float  totRain = 0., totSnow = 0.;   // 5-day totals (snow > 0 shows snow graph).
float  pctBlend;         // Scratch for drawGradientLine().
// uint16_t blendedColor;
int    yPos;             // >>> Mostly shadowed by local yPos variables.

char   workChar[100];    // General sprintf() scratch buffer.

// SSDays (Short Short Day) is used for the calendar column headers only.
String SSDays[7] = {"SU", "MO", "TU", "WE", "TH", "FR", "SA"};
//String LDays[7] = {"Sunday", "Monday", "TueSDays", "WedneSDays", "ThurSDays",
//                  "Friday", "Saturday"
//                 };
String SDays[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
                  };
String Months[12] = {"January", "February", "March", "April", "May", "June",
                     "July", "August", "September", "October", "November",
                     "December"
                    };   // >>> unused (strftime "%B" is used instead)
String SMonths[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
                     };
// 16-point compass names for windDegIntToOrdinalDirection().
const String TXT_N   = "N";
const String TXT_NNE = "NNE";
const String TXT_NE  = "NE";
const String TXT_ENE = "ENE";
const String TXT_E   = "E";
const String TXT_ESE = "ESE";
const String TXT_SE  = "SE";
const String TXT_SSE = "SSE";
const String TXT_S   = "S";
const String TXT_SSW = "SSW";
const String TXT_SW  = "SW";
const String TXT_WSW = "WSW";
const String TXT_W   = "W";
const String TXT_WNW = "WNW";
const String TXT_NW  = "NW";
const String TXT_NNW = "NNW";
// >>> TIP: const char* dirs[16] = {"N","NNE",...}; and
// >>>   dirs[(int)((deg + 11.25) / 22.5) % 16]  replaces 16 Strings and the
// >>>   16 if-statements in Screen_Utilities.ino.

// Calendar helpers (filled by getMyTime()).
int   mm[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // days/month
int   dayCounter = 0;    // >>> unused
int   dayInMonth = 0;    // Today's date (1-31).
int   daysInMonth = 0;   // Length of this month (leap years handled).
int   firstDay = 0;      // Weekday of the 1st (0 = Sunday).

int   brightness = 200;  // Current status initial screen brightness.

// Calendar build values: x, y of first cell, cell height, cell width.
int   cax, cay, cah, caw;

int   seg = 0;  // How many segments to show on the right for brightness. (>>> unused)

int city_timezone;  // Actually time offset from UTC.  Badly named, OWM!

// Setting PWM properties, do not change this!
// >>> NOTE: pwmFreq/pwmResolution/pwmLedChannelTFT/ledBacklightFull are not
// >>>   used - setup() passes 5000 and 8 directly to ledcAttach().
const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;
const int ledBacklightFull = 250;
const int screensExtraBright = 200;   // Backlight for message screens.
bool screenOn = true;  // Starts up on.  (Display controller powered?)

// Y positions for the 5 lines of the white "message" screens.
const int iDisplayLine1 =  15;
const int iDisplayLine2 =  45;
const int iDisplayLine3 =  75;
const int iDisplayLine4 = 105;
const int iDisplayLine5 = 135;

// Y positions for the 4 lines of the wake-up "info" splash.
#define BL_SPACING 40
const int buttonLine1 = 5;
const int buttonLine2 = buttonLine1 + BL_SPACING;
const int buttonLine3 = buttonLine2 + BL_SPACING;
const int buttonLine4 = buttonLine3 + BL_SPACING;

#define forecastsLength 48  // How many 3-hourly forcasts there are.
//                             (actually 40 at this time).
#define intermediatesCt  6  // How many graph points per 3-hour forecast.
// (Each 3-hour value is stretched into 6 points so the graph lines are
//  smooth; see calcIntermediates().)

//===========================================================================
// 5. ONECALL (OWM 3.0) GLOBALS - filled by fetch_and_decode_OneCall()
//===========================================================================
// >>> REVIEW: Of everything below, the screens only use:
// >>>   current_temp, current_humidity, current_wind_speed, current_wind_deg,
// >>>   current_clouds, current_weather_0_main, and from daily[]:
// >>>   dt, sunrise, sunset, moonrise, moonset, moon_phase, temp_min,
// >>>   temp_max, weather_0_id, pop.
// >>>   The hourly_* arrays (~3.5 KB) and the other daily_* fields are
// >>>   filled and never read.  Deleting them shortens OneCall_Decode.ino
// >>>   by ~150 lines.
// >>>
// >>> REVIEW (BUG, latent): every "const char*" below (timezone and the
// >>>   *_weather_0_main/description/icon arrays) is set to point INSIDE
// >>>   the JsonDocument.  That document is destroyed when the fetch
// >>>   function returns, so these pointers then point at freed memory.
// >>>   Nothing reads them after the fetch today, so no crash - but the day
// >>>   you use one on screen you will get garbage.  Use String (as you
// >>>   already did for current_weather_0_*) or delete them.
float lat, lon;
const char* timezone;
int   timezone_offset;
long  current_dt;
long  current_sunrise;
long  current_sunset;
float current_temp;
float current_feels_like;
int   current_pressure;
int   current_humidity;
float current_dew_point;
float current_uvi;
int   current_clouds;
int   current_visibility;
float current_wind_speed;
int   current_wind_deg;
float current_wind_gust;
int   current_weather_0_id;
//const char* current_weather_0_main;
String current_weather_0_main;
//const char* current_weather_0_description;
String current_weather_0_description;
//const char* current_weather_0_icon;
//String current_weather_0_icon;
long  minutely_item_dt;
int   minutely_item_precipitation;

long  hourly_item_dt[48];
float hourly_item_temp[48];
float hourly_item_feels_like[48];
int   hourly_item_pressure[48];
int   hourly_item_humidity[48];
float hourly_item_dew_point[48];
int   hourly_item_uvi[48];
int   hourly_item_clouds[48];
int   hourly_item_visibility[48];
float hourly_item_wind_speed[48];
int   hourly_item_wind_deg[48];
float hourly_item_wind_gust[48];
int   hourly_item_weather_0_id[48];
const char* hourly_item_weather_0_main[48];
const char* hourly_item_weather_0_description[48];
const char* hourly_item_weather_0_icon[48];
float hourly_item_pop[48];

// daily[0] = today, daily[1] = tomorrow ... OWM sends 8 days.
long  daily_item_dt[10];
long  daily_item_sunrise[10];
long  daily_item_sunset[10];
long  daily_item_moonrise[10];   // 0 = no moonrise that day.
long  daily_item_moonset[10];    // 0 = no moonset that day.
float daily_item_moon_phase[10]; // 0/1 new, .25 first qtr, .5 full, .75 last.
float daily_item_temp_day[10];
float daily_item_temp_min[10];
float daily_item_temp_max[10];
float daily_item_temp_night[10];
float daily_item_temp_eve[10];
float daily_item_temp_morn[10];
float daily_item_feels_like_day[10];
float daily_item_feels_like_night[10];
float daily_item_feels_like_eve[10];
float daily_item_feels_like_morn[10];
int   daily_item_pressure[10];
int   daily_item_humidity[10];
float daily_item_dew_point[10];
float daily_item_wind_speed[10];
int   daily_item_wind_deg[10];
float daily_item_wind_gust[10];
int   daily_item_weather_0_id[10];   // Looked up in Conditions[] below.
const char* daily_item_weather_0_main[10];
const char* daily_item_weather_0_description[10];
const char* daily_item_weather_0_icon[10];
int   daily_item_clouds[10];
float daily_item_pop[10];          // Probability of precipitation 0.0-1.0.
float daily_item_uvi[10];

int   ThreeHourSamples;   // How many 3-hour records the forecast returned (40).

//===========================================================================
// 6. WEATHER CONDITION TABLE
//===========================================================================
// One row per OWM condition id: id, short text, text colour, background.
struct Condx_S
{
  int Code;
  String Condition;
  int foreground;
  int background;
};
// I had to do this since I did not have enough space in the
//  area to be able to use the condition texts from OWM.  I used
//  most of them as the same but some had to be shortened a bit.
// This is a sort of dictionary lookup.  I search for the code (1st element),
//  then return the text and colors, elements 2-4.
#define WX_CONDITIONS 56
const struct Condx_S Conditions[WX_CONDITIONS] =
{
  // Put the more frequent ones at the top for speed.
  {800, "Clear Sky", TFT_YELLOW, TFT_BLACK},
  {801, "Few Clouds: 11-25%", TFT_YELLOW, TFT_BLACK},
  {802, "Scattered Clouds: 25-50%", TFT_YELLOW, TFT_BLACK},
  {803, "Broken Clouds: 51-84%", TFT_YELLOW, TFT_BLACK},
  {804, "Overcast Clouds: 85-100%", TFT_YELLOW, TFT_BLACK},

  {500, "Light Rain", TFT_YELLOW, TFT_BLACK},
  {501, "Moderate Rain", TFT_YELLOW, TFT_BLACK},
  {502, "Heavy Intensity Rain", TFT_YELLOW, TFT_BLACK},
  {503, "Very Heavy Rain", TFT_YELLOW, TFT_BLACK},
  {504, "Extreme Rain", TFT_WHITE, TFT_RED},
  {511, "Freezing Rain", TFT_WHITE, TFT_BLUE},
  {520, "Light Intensity Shower Rain", TFT_YELLOW, TFT_BLACK},
  {521, "Shower Rain", TFT_YELLOW, TFT_BLACK},
  {522, "Heavy Intensity Shower Rain", TFT_YELLOW, TFT_BLACK},
  {531, "Ragged Shower Rain", TFT_YELLOW, TFT_BLACK},

  {200, "Thunderstorm with Light Rain", TFT_YELLOW, TFT_BLACK},
  {201, "Thunderstorm with Rain", TFT_YELLOW, TFT_BLACK},
  {202, "Thunderstorm with Heavy Rain", TFT_WHITE, TFT_RED},
  {210, "Light Thunderstorm", TFT_YELLOW, TFT_BLACK},
  {211, "Thunderstorm", TFT_YELLOW, TFT_BLACK},
  {212, "Heavy Thunderstorm", TFT_WHITE, TFT_RED},
  {221, "Ragged Thunderstorm", TFT_YELLOW, TFT_BLACK},
  {230, "Thunderstorm w/Light Drizzle", TFT_YELLOW, TFT_BLACK},
  {231, "Thunderstorm w/Drizzle", TFT_YELLOW, TFT_BLACK},
  {232, "Thunderstorm w/Heavy Drizzle", TFT_YELLOW, TFT_BLACK},

  {300, "Light Intensity Drizzle", TFT_YELLOW, TFT_BLACK},
  {301, "Drizzle", TFT_YELLOW, TFT_BLACK},
  {302, "Heavy Intensity Drizzle", TFT_YELLOW, TFT_BLACK},
  {310, "Light Intensity Drizzle Rain", TFT_YELLOW, TFT_BLACK},
  {311, "Drizzle Rain", TFT_YELLOW, TFT_BLACK},
  {312, "Heavy Intensity Drizzle Rain", TFT_YELLOW, TFT_BLACK},
  {313, "Shower Rain And Drizzle", TFT_YELLOW, TFT_BLACK},
  {314, "Heavy Shower Rain & Drizzle", TFT_YELLOW, TFT_BLACK},
  {321, "Shower Drizzle", TFT_YELLOW, TFT_BLACK},

  {600, "Light Snow", TFT_YELLOW, TFT_BLACK},
  {601, "Snow", TFT_YELLOW, TFT_BLACK},
  {602, "Heavy Snow", TFT_BLACK, TFT_WHITE},
  {611, "Sleet", TFT_YELLOW, TFT_BLACK},
  {612, "Light Shower Sleet", TFT_YELLOW, TFT_BLACK},
  {613, "Shower Sleet", TFT_YELLOW, TFT_BLACK},
  {615, "Light Rain And Snow", TFT_YELLOW, TFT_BLACK},
  {616, "Rain And Snow", TFT_YELLOW, TFT_BLACK},
  {620, "Light Shower Snow", TFT_YELLOW, TFT_BLACK},
  {621, "Shower Snow", TFT_YELLOW, TFT_BLACK},
  {622, "Heavy Shower Snow", TFT_YELLOW, TFT_BLACK},

  {701, "Mist", TFT_YELLOW, TFT_BLACK},
  {711, "Smoke", TFT_YELLOW, TFT_DARKGREY},
  {721, "Haze", TFT_YELLOW, TFT_BLACK},
  {731, "Sand / Dust Whirls", TFT_YELLOW, TFT_BLACK},
  {741, "Fog", TFT_YELLOW, TFT_DARKGREY},
  {751, "Sand", TFT_YELLOW, TFT_BLACK},
  {761, "Dust", TFT_YELLOW, TFT_BLACK},
  {762, "Volcanic Ash", TFT_YELLOW, TFT_DARKGREY},
  {771, "Squalls", TFT_YELLOW, TFT_BLACK},
  {781, "Tornado", TFT_WHITE, TFT_RED},

  {0,   "No data", TFT_WHITE, TFT_BLACK}
};

//===========================================================================
// 7. FORECAST (OWM 2.5, 5 day / 3 hour) GLOBALS - used by the 3 graphs
//===========================================================================
// Forecast variables
typedef struct { // For current Day and Day 1, 2, 3, 4 and 5
  // in 3-hour forecast intervals.
  int    dt;            // UTC epoch of this 3-hour slot.
  //  String Description;
  //  String Icon;
  //  String Trend;
  //  float  DewPoint;
  //  float  FeelsLike;
  //  float  High;
  //  float  Humidity;
  //  float  Low;
  //  float  Pressure;
  float  Rainfall;      // mm (or inches if the city is imperial).
  float  Snowfall;      // mm (or inches if the city is imperial).
  float  Temperature;   // In the city's units.
  float  TempMin;       // >>> TempMin..Sunset are never filled - removable.
  float  TempMax;
  //  float  UVI;
  //  float  Winddir;
  //  float  Windspeed;
  //  int    Cloudcover;
  //  int    Dt;
  //  int    Visibility;
  int    Moonrise;
  int    Moonset;
  int    Sunrise;
  int    Sunset;
} Forecast_record_type;
// As of this writing, 40 is the actual count.  I just left some slack.
Forecast_record_type WxForecast[forecastsLength];

// Working buffer for the graphs: 6 points per 3-hour sample (40 x 6 = 240).
float expandedData[forecastsLength * intermediatesCt];
