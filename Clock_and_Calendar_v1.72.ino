// This code is offered with no licencing restrictions but please leave the
//  following in place so others can find the latest code:
//  Downloaded from:
//  https://github.com/MikeyMoMo/T-Display-S3-Time-Date-Weather

//#define DO_ONECALL_PRINTS  // Do debug prints of various data from OneCall
//#define DO_RAIN_FORECAST_PRINTS  // Do debug prints of various data from Forecast
//#define DO_TEMP_FORECAST_PRINTS  // Do debug prints of various data from Forecast

#include "Defines_and_Docs.h"

/***************************************************************************/
void setup()
/***************************************************************************/
{
  int iYear;
  int lastDot, lastV;
  String sFile;

  Serial.begin(115200); delay(3000);
  Serial.println("Running from:");
  Serial.println(__FILE__);

  // Be sure to have a little v in front of the version number
  //  and it is last in the filename.
  // Like this: Clock_and_Calendar_v1.56.ino
  // Then this code picks up what is between the v and the ".".
  // Here's something silly.  This must be in the main .ino tab, else
  //  it adds on the name of the tab to the end and you will have to peel
  //  that off, too.  Never knew that!
  sFile = String(__FILE__);
  lastDot = sFile.lastIndexOf(".");
  if (lastDot > -1) {  // Found a dot.  Thank goodness!
    lastV = sFile.lastIndexOf('v');  // Find start of version number
    if (lastV > -1) {  // Oh, good, found version number, too
      sVer = sFile.substring(lastV + 1, lastDot); // Pick up version number
    } else {
      sVer = "0.00";  // Unknown version.
    }
  } else {
    sVer = "n/a";  // Something badly wrong here!
  }

  Serial.printf("ESP32 Chip model = %s Rev %d\n",
                ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("This chip has %d cores\n", ESP.getChipCores());
  uint64_t chipId = ESP.getEfuseMac();
  Serial.printf("ESP32ChipID2=%04X\r\n", (uint16_t)(chipId >> 32)); //print High 2bytes
  Serial.printf("ESP32ChipID4=%08X\r\n", (uint32_t)chipId); //print Low 4bytes.  Serial.print("Chip ID 2: ");
  Serial.print("EfuseMac: "); Serial.println(ESP.getEfuseMac());  // These two "MAC"s don't match at all!
  String myMACAddress = WiFi.macAddress();
  Serial.print("MAC Address = "); Serial.println(myMACAddress);  // These two "MAC"s don't match at all!

  pinMode( 0, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);

  tft.init();
  tft.setRotation(3);
  tft.setSwapBytes(true);

  tft.setFreeFont(&FreeSans12pt7b);
  tft.showFont(10000);

  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin(TFT_BL, pwmLedChannelTFT);
  ledcWrite(pwmLedChannelTFT, ledBacklightFull);

  xCenter = tft.width() / 2;
  yCenter = tft.height() / 2;

  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextWrap(false);
  tft.setTextDatum(TC_DATUM);

  //  yPos = 10;
  tft.unloadFont(); tft.loadFont(BritanicBold36); // Load a different font
  tft.drawString("WeatherClock", xCenter, iDisplayLine1);
  tft.drawString("Initializing", xCenter, iDisplayLine3);
  tft.drawString("Version " + sVer, xCenter, iDisplayLine5);
  //  tft.drawString("Version " + String(sVer), xCenter, iDisplayLine5);
  delay(3000);

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
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextWrap(false);
  tft.setTextDatum(TC_DATUM);
  //  yPos = 10;
  tft.fillScreen(TFT_WHITE);
  tft.drawString("WiFi", xCenter, iDisplayLine1);
  tft.drawString("connected to", xCenter, iDisplayLine2);
  IPAddress ip = WiFi.localIP();
  sprintf(workChar, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  tft.drawString(String(workChar), xCenter, iDisplayLine3);
  tft.drawString(ssid, xCenter, iDisplayLine4);
  for (int loopWait = 0; loopWait < 3000; loopWait++) delay(1);

  tft.fillScreen(TFT_WHITE);
  tft.drawString("Fetching time", xCenter, iDisplayLine2);
  tft.drawString("from NTP Server.", xCenter, iDisplayLine4);

  configTzTime(time_zone, ntpServer1, ntpServer2);  // This does it all!
  sntp_set_sync_interval(86400000);  // 1 day in ms.
  time(&workTime);
  timeinfo = localtime(&workTime);  // Get correct local time including DST.
  iYear = timeinfo->tm_year + 1900;
  while (iYear < 2024) {
    Serial.println("Waiting for valid time.");
    delay(2000);
    time(&workTime);
    timeinfo = localtime(&workTime);
    iYear = timeinfo->tm_year + 1900;
    if (iYear > 2024) break;
  }
  getMyTime();  // Update the global time variables.
  Serial.println("The time is correct.");
  Serial.println("Let's take a look at the weather, "
                 "with KSLA's own Al Bolton.");

  tft.fillScreen(TFT_WHITE);
  tft.drawString("Fetching WX data", xCenter, iDisplayLine2);
  tft.drawString("from OWM Server.", xCenter, iDisplayLine4);
  fetch_and_decode_Forecast();
  fetch_and_decode_OneCall();

  ledcSetup(0, 10000, 8);
  ledcAttachPin(38, 0);
  brightness = ihourlyBrilliance[intHour];
  ledcWrite(0, brightness);
  showCurrentCondx();
  time(&lastOn);
  displayStatus = displayOn;
}
/***************************************************************************/
void loop()
/***************************************************************************/
{
  getMyTime();  // Update the time variables used all over the place.

  // The following fetch the current UTC epoch into lastOn on a button press.
  // Pressing it again, while on, does not add time but does restart the
  //  on-time timer.
  // Turn on upon button press or on the top of the hour.
  if (digitalRead(button1) == 0 || digitalRead(button2) == 0 ||
      (intMin == 0 && intSec == 0)) {
    time(&testOn);
    if (lastOn + blankSecs < testOn) {
      tft.fillScreen(TFT_WHITE);
      tft.setTextColor(TFT_BLACK, TFT_WHITE);
      tft.setTextDatum(TC_DATUM);
      tft.drawString("Time is: " + String(timeHour) + ":" + String(timeMin),
                     xCenter,  iDisplayLine2);
      //      Serial.println(buttonClockString);
      tft.drawString(buttonClockString, xCenter,  iDisplayLine4);
      ledcWrite(0, extraScreensBrightness);  // Make it bright.
      delay(4000);
    }
    time(&lastOn);  // Restart the blank timer.
    brightness = ihourlyBrilliance[intHour];
    ledcWrite(0, brightness);
  }

  if (prev_sec == intSec) return;
  prev_sec = intSec;  // Reset for next second waiting.
  time(&workTime);  // Get UTC now to compare with lastOn value for blanking.
  if ((workTime - lastOn) > blankSecs) {
    if (displayStatus == displayOn) {
      displayStatus = displayOff;
      //      Serial.print("Turning the screen off at ");
      //      printMyTime
      sprite.fillSprite(TFT_BLACK);
      sprite.pushSprite(0, 0);  // Effectively turns the display off.
    }
  } else {
    //    if (displayStatus == displayOff)
    //      Serial.println("Turning screen back on.");
    displayStatus = displayOn;

    // Rotate the screens.
    if (intSec >  4 && intSec < 13) {
      //      Serial.println("ShowForecast");
      ShowForecast();  // 05-12
    }

    if (intSec > 12 && intSec < 21) {
      //      Serial.println("graphTheTemp");
      graphTheTemp();  // 13-20
    }

    if (intSec > 20 && intSec < 29) {
      //      Serial.println("graphTheRain");
      graphTheRain();  // 21-28
    }
    if (totSnow > 0.)  // Snow is rare in many places.  Skipping the graph if not needed.
      timePush = 8;  // Make a (time) hole for the snow graph.
    else
      timePush = 0;  // No snow, no need for the hole.  Don't display the snow graph.

    if (totSnow > 0. && intSec > (20 + timePush) && intSec < (29 + timePush)) {
      //      Serial.println("graphTheSnow");
      graphTheSnow();  // 21-28
    }
    
    if (intSec > (28 + timePush) && intSec < (37 + timePush)) {
      //      Serial.println("ShowSunMoon");
      ShowSunMoon();  // 29-36
    }

    if (intSec > (36 + timePush) || intSec < 5) { // 37-04
      brightness = ihourlyBrilliance[intHour];
      ledcWrite(0, brightness);
      //      Serial.println("showCurrentCondx");
      showCurrentCondx();
    }
    // People tend to grab data on even times like on the hour.  This
    //  little fix moves the fetch to every 15 minutes starting at 6 minutes
    //  after the hour.  6, 21, 36 and 51 minutes, thereby skipping over
    //  the one hour congestion.  BIG difference!  1 second instead of 10!
  }
  if (intSec == 0) timePush = 0;
  getMyTime();  // Serial.println(fullTimeDate);
  if (lowFetchRateStart > lowFetchRateEnd)
    lowRateFetch = (intHour >= lowFetchRateStart || intHour < lowFetchRateEnd);
  else
    lowRateFetch = (intHour >= lowFetchRateStart && intHour < lowFetchRateEnd);

  // Changed to second 21 so that the fetch is done during one of the
  //  static screens instead of the active current stats screen.  It hides
  //  the action as long as it is fast enough (usually is).

  if (lowRateFetch) {  // in high rate fetch time.
    if (intMin == 21 && intSec == 21) {  // 6, 21, 36, 51 minutes.
      fetch_and_decode_Forecast(); fetch_and_decode_OneCall();
      //      Serial.println("Doing lowRateFetch"); delay(500);
    }
  } else {  // in low rate fetch time.
    if ((intMin - 6) % 15 == 0 && intSec == 21) {  // 6, 21, 36, 51 minutes.
      fetch_and_decode_Forecast(); fetch_and_decode_OneCall();
      //      Serial.println("Doing highRateFetch"); delay(500);
    }
  }

  if (prev_hour != intHour) {
    prev_hour  = intHour;
    getMyTime(); Serial.print(fullTimeDate);
    Serial.printf(" - In %s rate fetch time.\r\n",
                  lowRateFetch ? "low" : "high");
    brightness = ihourlyBrilliance[intHour];
    Serial.printf("Setting screen brightness for hour %02i to %i\r\n",
                  intHour, ihourlyBrilliance[intHour]);
  }
}
