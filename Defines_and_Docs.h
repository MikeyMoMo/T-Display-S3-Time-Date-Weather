// Start of user customizations ------------------

//const char* ssid     = "Your_SSID";
//const char* password = "Your_PW";

// POSIX TimeZone rule for where you are.
//const char* time_zone = "CST6CDT,M3.2.0/2:00:00,M11.1.0/2:00:00";  // for Shreveport
//String latstr = "32.523659";   // Your latitude for Shreveport
//String lonstr = "-93.763504";  // Your longitude for Shreveport

// POSIX TimeZone rule for where you are.
const char* time_zone = "PHT-8";  // for Bangui
String latstr = "18.5376";   // Your latitude for Bangui
String lonstr = "120.7671";  // Your longitude for Bangui

// POSIX TimeZone rule for where you are.
//const char* time_zone = "EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00";  // for Buffalo (snow testing)
//String latstr = "42.886447";   // Your latitude for Shreveport
//String lonstr = "-78.878369";  // Your longitude for Shreveport

// Get OneCall API key.
const String api_key = "YOUR_OWM_API_KEY";
//const String api_key = "Your_OWM_API_key_here";  // Get OneCall API key.
// Note: I stay up late and sleep late!
const int lowFetchRateStart =  2;  // 2 am, starts one data fetch per hour.
const int lowFetchRateEnd   = 12;  //12 (noon) starts 4 data fetches per hour.
// Use 'M' (or anything else) for Metric or I for Imperial
String    Units             = "M";

// The buttons are used to bring the display back to life.  The display will
//  remain on for the number of seconds of "blankSecs".  Then press a button
//  to see the display again.  Easy, peasy, peachy keen.
#define button1     0  // These 3 may need to be changed. Maybe...
#define button2    14
#define blankSecs 240  // 4 minutes on, then go to black to save the display.
#define FILL_GRAPH     // If defined, fill color under the graph line.
// End   of user customizations ------------------

#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "esp_sntp.h"
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
struct tm * timeinfo;
time_t workTime, lastOn, testOn;
char fullTimeDate[100];
String sVer;

const String oneCallEndpoint = "https://api.openweathermap.org/data/2.5/"
                               "onecall?lat=" + latstr + "&lon=" + lonstr +
                               "&units=metric&exclude=minutely&appid=" +
                               api_key;
const String ForecastEndpoint = "https://api.openweathermap.org/data/2.5/"
                                "forecast?lat=" + latstr + "&lon=" + lonstr
                                + "&units=metric&exclude=minutely&appid=" +
                                api_key;
bool lowRateFetch;  // True when in low rate (once per hour) fetch time.

//char inp[50000];  // JSON work area. Multiply used.
int prev_hour = -1, prev_sec = -1;
#define WX_CONDITIONS 55
float highRainForecast = -1.;  // Unlikely to be lower than 0.  Will fix, later.
float highSnowForecast = -1.;  // Unlikely to be lower than 0.  Will fix, later.
float highTempForecast = -300.;  // A very low temp to ensure correction.
float lowTempForecast = 1000.;   // A very high temp to ensure correction.
const int graphFloorMargin = 20;   // X axis here.
const int graphLeftMargin  = 32;   // Y axis here from the left.
float pixelsPerHundredthV;
int   timePush;

#include <ArduinoJson.h>
// Now done in the two parsing routines as per suggestion from the author.
//JsonDocument doc;  
DeserializationError error;

//#include <JSON_Decoder.h> // https://github.com/Bodmer/JSON_Decoder
String payload = ""; // Whole json packet returned
float tmpC;          // Temperature from the JSON file in Celsius.
float tmpF;          // Temperature converted to Farenheit.
int humidity;        // Humidity from the JSON file.
String windDegStr;   // Wind direction String
int windDegInt;      // Wind degrees integer
int windSpd;         // Wind speed in mtrs/sec
//char wxMain[100];    // Main, abbreviated conditions
String sTown;
String moonPrefix;   // R for moonrise, S for moonset.
int dataFetchHour, dataFetchMin;
char cMinTmp[10], cMaxTmp[10];

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

#include "fonts.h"

#define TOP_BUTTON 0
#define BOTTOM_BUTTON 14
String buttonClockString;

#define displayOn     true
#define displayOff    false
bool    displayStatus = displayOn;

#define RGB565(r,g,b) ((((r>>3)<<11) | ((g>>2)<<5) | (b>>3)))
#define myBlue   0x0AAD
#define myGray   0xB5B6
#define snowGraphLineColor TFT_WHITE
#define rainGraphLineColor RGB565(100,100,255)
#define tempGraphLineColor RGB565(200,075,075)
//                           0    1    2    3    4    5
int ihourlyBrilliance[] = { 80,  80,  80,  80,  80,  80,      //  0- 5
                            //6   7    8    9   10   11
                            80,  80,  80,  80,  80, 100,      //  6-11
                            //12  13   14   15   16   17
                            120, 130, 140, 150, 160, 160,     // 12-17
                            //18  19   20   21   22   23
                            160, 160, 140, 120,  80,  80      // 18-23
                          };
String IP;
char   timeHour[3], timeMin[3], timeSec[3];
char   myMonth[10], myYear[5], myDay[5];
char   timeWeekDay[10];  // Full week day name (%A)
char   timeWkDay[4];  // Short week day (%a)
char   timeDOM[3];    // Day of the month (%d)
int    intHour, intMin, intSec, intMonth, intYear, intDOM, intDOW;
int    dayInWeek;
int    xCenter, yCenter;
float  totRain = 0., totSnow = 0.;

int    yPos;

char   workChar[100];

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
                    };
String SMonths[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
                     };
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

int   mm[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
int   dayCounter = 0;
int   dayInMonth = 0;
int   daysInMonth = 0;
int   firstDay = 0;

int   brightness = 200;  // Current status initial screen brightness.

// Calendar build values
int   cax, cay, cah, caw;

int   seg = 0;  // How many segments to show on the right for brightness.

int city_timezone;  // Actually time offset from UTC.  Badly named, OWM!

// Setting PWM properties, do not change this!
const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;
const int ledBacklightFull = 250;
const int extraScreensBrightness = 200;

const int iDisplayLine1 =  15;
const int iDisplayLine2 =  45;
const int iDisplayLine3 =  75;
const int iDisplayLine4 = 105;
const int iDisplayLine5 = 135;

#define forecastsLength 48  // How many 3-hourly forcasts there are.
//                             (actually 40 at this time).
#define intermediatesCt  6  // How many graph points per 3-hour forecast.

// OneCall variables
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
const char* current_weather_0_main;
const char* current_weather_0_description;
const char* current_weather_0_icon;
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

long  daily_item_dt[10];
long  daily_item_sunrise[10];
long  daily_item_sunset[10];
long  daily_item_moonrise[10];
long  daily_item_moonset[10];
float daily_item_moon_phase[10];
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
int   daily_item_weather_0_id[10];
const char* daily_item_weather_0_main[10];
const char* daily_item_weather_0_description[10];
const char* daily_item_weather_0_icon[10];
int   daily_item_clouds[10];
float daily_item_pop[10];
float daily_item_uvi[10];

int   ThreeHourSamples;

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
};
// Forecast variables
typedef struct { // For current Day and Day 1, 2, 3, 4 and 5
  // in 3-hour forecast intervals.
  int    dt;
  //  String Description;
  //  String Icon;
  //  String Trend;
  //  float  DewPoint;
  //  float  FeelsLike;
  //  float  High;
  //  float  Humidity;
  //  float  Low;
  //  float  Pressure;
  float  Rainfall;
  float  Snowfall;
  float  Temperature;
  float  TempMin;
  float  TempMax;
  //  float  UVI;
  //  float  Winddir;
  //  float  Windspeed;
  //  int    Cloudcover;
  //  int    Dt;
  int    Moonrise;
  int    Moonset;
  int    Sunrise;
  int    Sunset;
  //  int    Visibility;
} Forecast_record_type;
// As of this writing, 40 is the actual count.  I just left some slack.
Forecast_record_type WxForecast[forecastsLength];

float smoothedData[forecastsLength * intermediatesCt];
