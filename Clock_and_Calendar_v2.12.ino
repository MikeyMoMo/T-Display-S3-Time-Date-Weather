// This code is offered with no licencing restrictions but please leave the
//  following in place so others can find the latest code:
//  Downloaded from:
//  https://github.com/MikeyMoMo/T-Display-S3-Time-Date-Weather

//
//#define DO_ONECALL_PRINTS   // Do debug prints of various data from OneCall
//#define DO_FORECAST_PRINTS  // Do all prints from Forecast decode
//#define DO_RAIN_FORECAST_PRINTS  // Do debug prints of rain data from Forecast
//#define DO_TEMP_FORECAST_PRINTS  // Do debug prints of temp data from Forecast

#include "Defines_and_Docs.h"
/*
   This code is designed to run on a T-Display S3.
*/
/***************************************************************************/
void setup()
/***************************************************************************/
{
  int iYear, looper;
  int lastDot, lastV;
  String sFile;
  Serial.begin(115200); delay(4000);
  Serial.println("Running from:");
  Serial.println(__FILE__);

  // Be sure to have a little v in front of the version number
  //  and it is last in the filename.
  // Like this: Clock_and_Calendar_v1.56.ino
  // Then this code picks up what is between the v and the ".".

  sFile = String(__FILE__);
  lastDot = sFile.lastIndexOf(".");
  if (lastDot > -1) {  // Found a dot.  Thank goodness!
    lastV = sFile.lastIndexOf("v");  // Find start of version number
    if (lastV > -1) {  // Oh, good, found version number, too
      sVer = sFile.substring(lastV + 1, lastDot); // Pick up version number
      lastV = sVer.lastIndexOf("\\");
      if (lastV > -1) sVer = sVer.substring(0, lastV);
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
  //print High 2bytes
  Serial.printf("ESP32ChipID2=%04X\r\n", (uint16_t)(chipId >> 32));
  //print Low 4bytes.  Serial.print("Chip ID 2: ");
  Serial.printf("ESP32ChipID4=%08X\r\n", (uint32_t)chipId);
  // These two "MAC"s don't match at all!
  Serial.print("EfuseMac: "); Serial.println(ESP.getEfuseMac());
  String myMACAddress = WiFi.macAddress();
  // These two "MAC"s don't match at all!
  Serial.print("MAC Address = "); Serial.println(myMACAddress);

  pinMode(buttonCityChange, INPUT_PULLUP);  // Press for city changing
  pinMode(buttonShow,       INPUT_PULLUP);  // Press to turn on display

  tft.init();
  tft.setRotation(myOrientation);
  if (myOrientation == ORIENT_POWER_LEFT) {  // Asjust pins based on display orientation.
    buttonCityChange = 0; buttonShow = 14;  // Increase brightness is always on top, and...
  } else {
    buttonCityChange = 14; buttonShow = 0;  // Decrease brightness is always on the bottom button.
  }

  tft.setSwapBytes(true);

  tft.setFreeFont(&FreeSans12pt7b);

  ledcAttach(TFT_BL, 5000, 8);  // PWM timer automatically assigned.
  // Turn the display on bigly for init messages.
  ledcWrite(TFT_BL, screensExtraBright);

  xCenter = tft.width() / 2;   // Used for centering text
  yCenter = tft.height() / 2;  // Same in the horizontal

  // Utility messages are Black text on a white background.
  // Data screens have more color.
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextWrap(false);
  tft.setTextDatum(TC_DATUM);  // Top center baseline.

  tft.unloadFont(); tft.loadFont(BritanicBold36); // Load a different font
  tft.drawString("WeatherClock", xCenter, iDisplayLine1);
  tft.drawString("Initializing", xCenter, iDisplayLine3);
  tft.drawString("Version " + sVer, xCenter, iDisplayLine5);
  delay(3000);

  int *a;  // Temporary pointer holder
  a = (int*)sprite.createSprite(320, 170);
  if (a == 0) {
    Serial.println("Sprite creation failed.  Cannot continue.");
    tft.fillScreen(TFT_WHITE); tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString("PSRAM not available", xCenter, iDisplayLine2);
    tft.drawString("Enable it or use WROVER", xCenter, iDisplayLine4);
    while (1);  // Wait until 1 does not equal 1.
  }
  Serial.printf("Connecting with WifiManager.");

  //  WiFi.begin(ssid, password);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setTimeout(60);  // 60 second connect timeout then reboot.
  wifiManager.disconnect();
  //  wifiManager.mode(WIFI_STA); // switch off AP
  if (!wifiManager.autoConnect(myPortalName, myPortalName)) {
    Serial.println("failed to connect and hit timeout");
    ESP.restart();
  }
  looper = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
    if (looper++ > 20) ESP.restart();  // WiFi fail after 10 sec., retry.
  }
  Serial.println("\r\nConnected. Fetching time...");
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextWrap(false);
  tft.setTextDatum(TC_DATUM);
  tft.fillScreen(TFT_WHITE);
  tft.drawString("WiFi connected", xCenter, iDisplayLine1);
  tft.drawString("to " + WiFi.SSID(), xCenter, iDisplayLine2);
    tft.drawString("at IP address", xCenter, iDisplayLine4);
  IPAddress ip = WiFi.localIP();
  sprintf(workChar, "IP: %u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  tft.drawString(String(workChar), xCenter, iDisplayLine5);
  for (int loopWait = 0; loopWait < 3000; loopWait++) delay(1);

  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString("Fetching time", xCenter, iDisplayLine2);
  tft.drawString("from NTP Server.", xCenter, iDisplayLine4);
  sntp_set_sync_interval(86400000);  // 1 day in ms.
  sntp_set_time_sync_notification_cb(timeSyncCallback);
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  // City 0 is should be your local.  That's the default.
  setenv("TZ", multiCity[0].ENV.c_str(), 1); tzset();

  time(&workTime);
  timeinfo = localtime(&workTime);  // Get correct local time including DST.
  iYear = timeinfo->tm_year + 1900;
  looper = 0;
  int maxLoop = 60;
  while (iYear < 2025) {
    Serial.println("Waiting for valid time.");
    delay(2000);
    if (looper++ > 60) ESP.restart();  // If 2 minutes, reboot.
    time(&workTime);
    timeinfo = localtime(&workTime);
    iYear = timeinfo->tm_year + 1900;
    if (iYear > 2025) break;
  }
  getMyTime();  // Update the global time variables.
  Serial.println("The time is correct.");
  Serial.println("Let's take a look at the weather, "  // Inside joke.
                 "with KSLA's own Al Bolton.");  // He is surely dead now.
  tft.fillScreen(TFT_WHITE);
  tft.drawString("Fetching WX data", xCenter, iDisplayLine2);
  tft.drawString("from OWM Server.", xCenter, iDisplayLine4);

  preferences.begin("WCC", RO_MODE);
  whichCity = preferences.getInt("city", 0);
  Serial.printf("Will fetch city %i: ", whichCity);
  Serial.println(multiCity[whichCity].CityName);
  //  whichCity = 0;

  //  Serial.println("Memory before any JSON usage.");
  showMemoryStats();
  fetch_and_decode_Forecast();
  showMemoryStats();
  fetch_and_decode_OneCall();
  showMemoryStats();
  uiNextFetchTime = millis() + uiFetchInterval;  // Remember when to fetch

  //  ledcSetup(0, 10000, 8);  // Prepare the display for dimming, timer 0.
  ledcAttach(TFT_BL, 5000, 8);  // PWM timer automatically assigned.
  //  ledcAttachPin(DISPLAY_DIM_PIN, 0);  // Pin 38 is the display flicker control pin.

  // This block gets the time offset from UTC for the home time.  It is only
  //  used to find the hour and then the brightness value for each hour.
  //  It runs fast and makes a consistent dimming when changing cities.  It
  //  uses the home hour instead of the selected city hour for consistency.
  //  This was made redundant by the setBrightness routine using setenv.
  //  time(&UTC);
  //  strftime (cCharWork, sizeof(cCharWork), "%z", localtime(&UTC));
  //  iHomeOffset = atoi(cCharWork);
  //  iHomeOffset = (iHomeOffset / 100) * 3600 + iHomeOffset % 100 * 60;
  //  Serial.printf("Home offset %i\r\n", iHomeOffset);

  setBrightness();  // Find the hour and set the display brightness.
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
  if (!cityChangeInProgress) {  // Nothing during city change delay
    if (digitalRead(buttonShow) == buttonPressed ||
        (intMin == 0 && intSec == 0) || forceDisplayOn) {
      delay(50);
      if (digitalRead(buttonShow) == buttonPressed ||
          (intMin == 0 && intSec == 0) || forceDisplayOn) {
        while (digitalRead(buttonShow) == buttonPressed);  // wait for unpress
        time(&testOn);
        if (!forceDisplayOn) {
          if (lastOn + BLANK_SECS < testOn) {
            if (!screenOn) {
              tft.writecommand(ST7789_DISPON);  // Turn on display hardware
              screenOn = true;
            }
            ledcWrite(TFT_BL, screensExtraBright); // Turn the display on bigly for init messages.
            tft.fillScreen(TFT_WHITE);
            tft.setTextColor(TFT_BLACK, TFT_WHITE);
            tft.setTextDatum(TC_DATUM);
            tft.drawString("Time is: " + String(timeHour) + ":" + String(timeMin),
                           xCenter,  iDisplayLine1);
            tft.drawString(clockString, xCenter,  iDisplayLine3);

            if (multiCity[whichCity].units == "imperial") {
              tft.drawString("T: " + String(int(current_temp + .5)) + "F, RH: " +
                             String(current_humidity) + "%",
                             xCenter,  iDisplayLine5);
            } else {  // metric
              tft.drawString("T: " + String(int(current_temp + .5)) + "C, RH: " +
                             String(current_humidity) + "%",
                             xCenter, iDisplayLine5);
            }
            delay(5000);
          }
        }
        forceDisplayOn = false;
      }
      time(&lastOn);  // Restart display the blank timer.
      setBrightness();
    }
  }
  // For each press and release, cycle through the cities defined in the
  //  array of structs.  If the button is not pressed in 4 seconds do the
  //  requested change, if any.  If the same city is selected as was
  //  selected, only show a message and do not hit up OWM again right now.
  if (digitalRead(buttonCityChange) == buttonPressed) {
    delay(50);
    if (digitalRead(buttonCityChange) == buttonPressed) {
      while (digitalRead(buttonCityChange) == buttonPressed); // wait for unpress
      cityChangeInProgress = true; cityChangeTimer = millis() + 4000;
      whichCity++; if (whichCity == MAX_CITY) whichCity = 0;
      Serial.printf("Requesting city #%i - %s\r\n",
                    whichCity, multiCity[whichCity].CityName.c_str());
      if (!screenOn) {
        tft.writecommand(ST7789_DISPON);  // Turn on display hardware
        screenOn = true;
      }
      ledcWrite(TFT_BL, screensExtraBright);  // Turn the display on bigly for init messages.
      sprite.unloadFont(); sprite.loadFont(BritanicBold36); // Load a different font
      sprite.setTextColor(TFT_BLACK, TFT_WHITE); sprite.setTextWrap(false);
      sprite.setTextDatum(TC_DATUM); sprite.fillSprite(TFT_WHITE);
      sprite.drawString("Current city:", xCenter, iDisplayLine1);
      sprite.drawString(multiCity[prevCity].CityName, xCenter, iDisplayLine2);
      sprite.drawString("Requested city:", xCenter, iDisplayLine4);
      sprite.drawString(multiCity[whichCity].CityName, xCenter, iDisplayLine5);
      sprite.pushSprite(0, 0);
      forceDisplayOn = true;
      return;
    }
  }
  // If the city change button was pressed, the boolean is set and the timer
  //  is set.  Wait for the timer to expire then do the requested operation.
  if (cityChangeInProgress && millis() > cityChangeTimer) {  // Time to fetch
    cityChangeInProgress = false;
    if (prevCity != whichCity) {  // Don't bother OWM for the same as current.
      prevCity = whichCity;
      Serial.printf("Fetching city #%i - %s\r\n",
                    whichCity, multiCity[whichCity].CityName.c_str());
      sprite.setTextColor(TFT_BLACK, TFT_WHITE); sprite.setTextWrap(false);
      sprite.setTextDatum(TC_DATUM); sprite.fillSprite(TFT_WHITE);
      sprite.drawString("Fetching", xCenter, iDisplayLine2);
      sprite.drawString(multiCity[whichCity].CityName, xCenter, iDisplayLine4);
      sprite.pushSprite(0, 0);

      // Set the time zone for the newly selected city from the structs array.
      setenv("TZ", multiCity[whichCity].ENV.c_str(), 1); tzset();

      fetch_and_decode_Forecast(); fetch_and_decode_OneCall();
      uiNextFetchTime = millis() + uiFetchInterval;  // Remember when to fetch

      preferences.begin("WCC", RW_MODE);
      preferences.putInt("city", whichCity);
      preferences.end();
    } else {
      Serial.println("No city change.");
      sprite.setTextColor(TFT_BLACK, TFT_WHITE); sprite.setTextWrap(false);
      sprite.setTextDatum(TC_DATUM); sprite.fillSprite(TFT_WHITE);
      sprite.drawString("City not changed.", xCenter, iDisplayLine2);
      sprite.drawString("No fetch needed.", xCenter, iDisplayLine4);
      sprite.pushSprite(0, 0);
      delay(2000);
    }
    sprite.unloadFont();
  }
  if (cityChangeInProgress) return;

  if (prev_sec == intSec) return;
  prev_sec = intSec;  // Reset for next second waiting.

  time(&workTime);  // Get UTC now to compare with lastOn value for blanking.
  if ((workTime - lastOn) > BLANK_SECS) {
    if (displayStatus == displayOn) {
      displayStatus = displayOff;
      sprite.fillSprite(TFT_BLACK);
      sprite.pushSprite(0, 0);  // Effectively turns the display off.
      brightness = 0;
      ledcWrite(TFT_BL, 0);       // Turn the display off.
      tft.writecommand(ST7789_DISPOFF);  // Turn off display hardware.
      screenOn = false;
    }
  } else {
    displayStatus = displayOn;  // Remember screen on

    // Rotate the screens.
    if (intSec >  4 && intSec < 13) ShowForecast();  // 05-12
    if (intSec > 12 && intSec < 21) graphTheTemp();  // 13-20
    if (intSec > 20 && intSec < 29) graphTheRain();  // 21-28
    if (totSnow > 0.)  // Snow is rare.  Skip the graph if not needed.
      timePush = 8;    // Make a (time) hole for the snow graph.
    else
      timePush = 0;    // No snow, no need for the hole,
    //                     don't display the snow graph.
    if (totSnow > 0. && intSec > (20 + timePush) && intSec < (29 + timePush)) {
      graphTheSnow();  // 21-28
    }

    if (intSec > (28 + timePush) && intSec < (37 + timePush)) {
      ShowSunMoon();  // 29-36
    }

    if (intSec > (36 + timePush) || intSec < 5) { // 37-04
      showCurrentCondx();
    }
    // People tend to grab data on even times like on the hour.  This
    //  little fix moves the fetch to every 15 minutes starting at 6 minutes
    //  after the hour.  6, 21, 36 and 51 minutes, thereby skipping over
    //  the one hour congestion.  BIG difference!  1 second instead of 10!
  }
  if (intSec == 0) timePush = 0;
  getMyTime();  // Serial.println(fullTimeDate);

  if (millis() > uiNextFetchTime) {
    showMemoryStats();
    fetch_and_decode_Forecast();
    showMemoryStats();
    fetch_and_decode_OneCall();
    showMemoryStats();
    uiNextFetchTime = millis() + uiFetchInterval;  // Remember when to fetch
  }
  setBrightness();
}
/***************************************************************************/
void setBrightness()
/***************************************************************************/
{
  // By using setenv, DST is supported.  This routine finds the current hour
  //  then sets the display brightness for that hour of home time.
  //  For the purists among you, this can be seen as a little bit wasteful
  //  for if the home city is selected, there is a setenv that are not
  //  needed.  But this whole thing runs in less than 10ms so I did not worry
  //  about that time loss each hour.  If I was writing an operating system, I
  //  would worry about it.  For this application, nope!
  setenv("TZ", multiCity[0].ENV.c_str(), 1); tzset();  // Change to home time.
  time(&workTime);  // Get the local time epoch.
  timeinfo = localtime(&workTime);  // Get correct local time including DST.
  localHour = timeinfo->tm_hour;  // Get the home time hour.
  if (prev_hour != localHour) {  // If hour changed, maybe new brightness.
    prev_hour  = localHour;
    brightness = ihourlyBrilliance[localHour];
    Serial.printf("Setting screen brightness for hour %02i to %i\r\n",
                  localHour, ihourlyBrilliance[localHour]);
    if (!screenOn) {
      tft.writecommand(ST7789_DISPON);  // Turn on display hardware
      screenOn = true;
    }
    ledcWrite(TFT_BL, brightness);       // Turn the display on bigly for init messages.
  }
  // All is done here, go back to previous time zone (if needed).
  if (whichCity != 0)  // 0 was set, above.  That may be redundant.
    setenv("TZ", multiCity[whichCity].ENV.c_str(), 1); tzset();
}
/***************************************************************************/
void timeSyncCallback(struct timeval * tv)
/***************************************************************************/
{
  //  struct timeval {  // Instantiated as "*tv"
  //   time_t      tv_sec;   // Number of whole seconds of elapsed time
  //   Number of microseconds of rest of elapsed time minus tv_sec.
  //   long int    tv_usec;
  //                             Always less than one million
  //};
  Serial.println("\n----Time Sync-----"); Serial.flush();
  Serial.printf("Time sync at %u ms.\r\nUTC Epoch: ", millis()); Serial.flush();
  Serial.println(tv->tv_sec); Serial.flush();
  Serial.println(ctime(&tv->tv_sec)); Serial.flush();
  delay(1000);
}
/***************************************************************************/
void configModeCallback (WiFiManager * myWiFiManager)
/***************************************************************************/
{
  tft.setTextDatum(TC_DATUM);  // Top center baseline.
  Serial.println(F("Entered config mode..."));
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  yPos = 10;
  tft.fillScreen(TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);  // Foreground color, Background color
  tft.drawString("Connect to", xCenter, iDisplayLine1);
  tft.drawString(myWiFiManager->getConfigPortalSSID(), xCenter, iDisplayLine2);
  tft.drawString("(Password the same)", xCenter, iDisplayLine3);
  tft.drawString("then 192.168.4.1", xCenter, iDisplayLine4);
  tft.drawString("and select a WAP", xCenter, iDisplayLine5);
}
