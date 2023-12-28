//#define TEMPF  // Uncomment for degrees in Farenheit
//#define DO_ONECALL_PRINTS  // Do debug prints of various data from OneCall
//#define DO_FORECAST_PRINTS  // Do debug prints of various data from Forecast

#include "Defines_and_Docs.h"

/***************************************************************************/
void setup()
/***************************************************************************/
{
  int iYear;

  Serial.begin(115200); delay(3000);  // S3 can be slow, sometimes.  WAIT!

  Serial.println("Running from:");
  Serial.println(__FILE__);

  tft.init();
  tft.setRotation(3);  // Power on the left.  Use 1 for power on right.
  tft.setSwapBytes(true);  // No, I don't know why, either!

  tft.setFreeFont(&FreeSans12pt7b);
  //  tft.showFont(10000);

  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin(TFT_BL, pwmLedChannelTFT);
  ledcWrite(pwmLedChannelTFT, ledBacklightFull);

  xCenter = tft.width() / 2;
  yCenter = tft.height() / 2;

  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);  // Foreground color, Background color
  tft.setTextWrap(false);
  tft.setTextDatum(TC_DATUM);
  yPos = 25;
  tft.unloadFont(); tft.loadFont(BritanicBold36); // Load a different font
  tft.drawString("Little Clock", xCenter, yPos);
  tft.drawString("Initializing", xCenter, yPos + 65);

  tft.fillScreen(TFT_BLACK);
  int *a;  // Sprite61 (the small one)
  a = (int*)sprite.createSprite(320, 170);
  if (a == 0) {
    Serial.println("sprite creation failed.  Cannot continue.");
    while (1);
  }

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\r\nConnected. Fetching time...");
  tft.setTextColor(TFT_BLACK, TFT_WHITE);  // Foreground color, Background color
  tft.setTextWrap(false);
  tft.setTextDatum(TC_DATUM);
  yPos = 10;
  tft.fillScreen(TFT_WHITE);
  tft.drawString("WiFi", xCenter, yPos);
  tft.drawString("connected to", xCenter, yPos + 38);
  IPAddress ip = WiFi.localIP();
  sprintf(workChar, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  tft.drawString(String(workChar), xCenter, yPos + 76);
  tft.drawString(ssid, xCenter, yPos + 114);
  for (int loopWait = 0; loopWait < 3000; loopWait++) delay(1);

  tft.fillScreen(TFT_WHITE);
  tft.drawString("Fetching time", xCenter, iDisplayLine1);
  tft.drawString("from NTP Server.", xCenter, iDisplayLine3);

  configTzTime(time_zone, ntpServer1, ntpServer2);  // This does it all!
  sntp_set_sync_interval(86400000);  // 1 day in ms.
  timeinfo = localtime(&workTime); delay(2000);
  time(&workTime);
  timeinfo = localtime(&workTime);  // Get correct local time including DST.
  iYear = timeinfo->tm_year + 1900;
  while (iYear < 2020) {
    Serial.println("Waiting for valid time.");
    delay(2000);
    time(&workTime);
    timeinfo = localtime(&workTime);
    iYear = timeinfo->tm_year + 1900;
    if (iYear > 2020) break;
  }
  getMyTime();  // Update the global time variables.
  Serial.println("The time is correct. Let's take a look at the weather, Al Bolton.");

  tft.fillScreen(TFT_WHITE);
  tft.drawString("Fetching WX data", xCenter, iDisplayLine1);
  tft.drawString("from OWM Server.", xCenter, iDisplayLine3);
  fetch_and_decode_Forecast();
  fetch_and_decode_OneCall();

  ledcSetup(0, 10000, 8);
  ledcAttachPin(38, 0);
  brightness = ihourlyBrilliance[intHour];
  ledcWrite(0, brightness);
  showCurrentCondx();
}
/***************************************************************************/
void loop()
/***************************************************************************/
{
  getMyTime();
  if (prev_sec == intSec) return;
  prev_sec = intSec;  // Reset for next second.

  // Rotate the screens.
  if (intSec >  4 && intSec < 13) {
    ShowForecast(); delay(500);  // 05-12
  }
  if (intSec > 12 && intSec < 21) {
    graphTheTemp(); delay(500);  // 13-20
  }
  if (intSec > 20 && intSec < 29) {
    graphTheRain(); delay(500);  // 21-28
  }
  if (intSec > 28 && intSec < 37) {
    ShowSunMoon(); delay(500);  // 29-36
  }
  if (intSec > 36 || intSec < 5) { // 37-04
    brightness = ihourlyBrilliance[intHour];
    ledcWrite(0, brightness);
    showCurrentCondx();
  }
  // People tend to grab data on even times like on the hour.  This
  //  little fix moves the fetch to every 15 minutes starting at 6 minutes
  //  after the hour.  6, 21, 36 and 51 minutes, thereby skipping over
  //  the one hour congestion.  BIG difference!  1 second instead of 10!

  if (lowFetchRateStart > lowFetchRateEnd)
    lowRateFetch = (intHour >= lowFetchRateStart || intHour < lowFetchRateEnd);
  else
    lowRateFetch = (intHour >= lowFetchRateStart && intHour < lowFetchRateEnd);

  // Changed to second 21 so that the fetch is done during one of the
  //  static screens instead of the active current stats screen.  It hides
  //  the action as long as it is fast enough (usually is).
  if (!lowRateFetch) {  // in high rate fetch time.
    if ((intMin - 6) % 15 == 0 && intSec == 21) {  // 6, 21, 36, 51 minutes.
      fetch_and_decode_Forecast(); fetch_and_decode_OneCall(); delay(1000);
    }
  } else {  // in low rate fetch time.
    if (intMin == 21 && intSec == 21) {  // 6, 21, 36, 51 minutes.
      fetch_and_decode_Forecast(); fetch_and_decode_OneCall(); delay(1000);
    }
  }
  if (prev_hour != intHour) {
    prev_hour  = intHour;
    Serial.print(fullTimeDate);
    Serial.printf(" - In %s rate fetch time.\r\n", lowRateFetch ? "low" : "high");
    brightness = ihourlyBrilliance[intHour];
    Serial.printf("Setting screen brightness for hour %02i to %i\r\n",
                  intHour, ihourlyBrilliance[intHour]);
  }
}
/***************************************************************************/
void getMyTime()
/***************************************************************************/
{
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) return;

  strftime(fullTimeDate, 100, "%c", &timeinfo);
  strftime(myMonth, 10, "%B", &timeinfo);
  strftime(myYear,   5, "%Y", &timeinfo);
  strftime(timeHour, 3, "%H", &timeinfo);  intHour = timeinfo.tm_hour;
  strftime(timeMin,  3, "%M", &timeinfo);  intMin  = timeinfo.tm_min;
  strftime(timeSec,  3, "%S", &timeinfo);  intSec  = timeinfo.tm_sec;
  strftime(timeWeekDay, 10, "%A", &timeinfo);

  dayInWeek   = timeinfo.tm_wday;  // Sunday = 0
  dayInMonth  = timeinfo.tm_mday;
  daysInMonth = mm[timeinfo.tm_mon];

  firstDay = dayInWeek + 1 - (dayInMonth % 7);
  if (firstDay < 0)firstDay += 7;

  if (timeinfo.tm_mon == 1 && isLeapYear(timeinfo.tm_year + 1900))
    daysInMonth++;  // Bump it up a notch for leap year February.
}
/***************************************************************************/
bool isLeapYear(int testYear)
/***************************************************************************/
{
  if (testYear % 400 == 0)
    return true;
  else if (testYear % 100 != 0 && testYear % 4 == 0)  //
    return true;
  else
    return false;
}
