// This code is offered with no licencing restrictions but please leave the
//  following in place so others can find the latest code:
//  Downloaded from:
//  https://github.com/MikeyMoMo/T-Display-S3-Time-Date-Weather

/*===========================================================================
  WEATHER CLOCK & CALENDAR (WCC) - MAIN SKETCH
  ===========================================================================
  HOW THE PROJECT IS ORGANISED (Arduino glues all .ino tabs into one file,
  in alphabetical order after this main tab, so every function can see
  every global declared in Defines_and_Docs.h):

    Weather_Clock_and_Calendar_WCC_vX.XX.ino  <- this file
         setup()            boot, WiFi, NTP, first weather fetch
         loop()             buttons, screen rotation, blanking, refetch
         setBrightness()    hourly backlight level (home-city time)
         timeSyncCallback() called by SNTP when the clock is corrected
         configModeCallback() shown when WiFiManager opens its portal
         HandleSerialInput()/showInputOptionsFull()  tiny serial menu

    Defines_and_Docs.h        user settings (cities, brightness table),
                              every global variable, OWM condition table
    Forecast_Decode.ino       OWM 2.5 "forecast" (5 day / 3 hour) -> graphs
    OneCall_Decode.ino        OWM 3.0 "onecall" (current + daily)  -> text
    Screen_Utilities.ino      time helpers, forecast & sun/moon screens,
                              JPEG drawing, gradient lines, font loading
    graphTheTemp/Rain/Snow    the three line graphs
    showCurrentConditions     the clock + calendar "home" screen
    colorbar.h                small JPEG drawn as the calendar background
    fonts.h                   embedded fonts (large!)

  SCREEN ROTATION (driven by the seconds value of the clock, see loop()):
    sec 05-12  ShowForecast()      7-day text forecast
    sec 13-20  graphTheTemp()      5-day temperature graph
    sec 21-28  graphTheRain()      5-day rain graph
    sec 29-36  graphTheSnow()      ONLY if snow is forecast (else skipped)
    next 8 s   ShowSunMoon()       sunrise/set, moonrise/set, moon phase
    rest       showCurrentCondx()  clock, calendar, current conditions

  BUTTONS (T-Display S3 has two, GPIO 0 and GPIO 14, active LOW):
    "Show" button        wake the display for BLANK_SECS seconds
    "City change" button step through multiCity[]; 4 s after the last
                         press the new city is fetched and saved to flash
  ===========================================================================*/

/*  Program Notes:

     This code needs ArduinoJSON version 7.  Will not compile with v6.
     This code is designed to run on a T-Display S3.  I may work on other
     devices but there may have to be changes, especially screen size.

  In return packets from OWM:

   🌡 Temperature → respects units (°C or °F) -- returns what you asked for.

   💨 Wind speed → always metric (m/s), unless you explicitly convert.
      >>> REVIEW: NOT correct.  OWM docs: metric = metre/sec,
      >>>         imperial = miles/hour.  With units=imperial you already
      >>>         get mph.  See the wind bug in showCurrentConditions.ino.

   📊 Pressure → always hPa.

   💧 Precipitation → millimeters.  (True: rain.3h / snow.3h are always mm.)

  Use: Setup206_LilyGo_T_Display_S3.h with tft_espi library.

  >>> REVIEW: ledcAttach()/ledcWrite(pin,..) are the ESP32 Arduino core 3.x
  >>>         API.  This sketch will NOT compile on core 2.x.  Worth stating
  >>>         the core version you build with here, next to the library list.
*/

//---------------------------------------------------------------------------
// DEBUG SWITCHES - remove the // to get lots of Serial Monitor output.
//---------------------------------------------------------------------------
// Do debug prints of various data from OneCall
//#define DO_ONECALL_PRINTS

// Do all prints from Forecast decode
//#define DO_FORECAST_PRINTS

// Do debug prints of rain data from Forecast
//#define DO_RAIN_FORECAST_PRINTS
// >>> REVIEW: DO_RAIN_FORECAST_PRINTS is never tested anywhere.  Dead switch.

// Do debug prints of temp data from Forecast
//#define DO_TEMP_FORECAST_PRINTS

#include "Defines_and_Docs.h"   // All settings, globals and libraries.

/***************************************************************************/
void setup()
/***************************************************************************/
//  Runs once at power-on / reset.  Order of events:
//   1. Serial banner, work out version number from the file name
//   2. Print chip info and OpenFontRender (OFR) version
//   3. Init display, backlight, show "Initializing" splash
//   4. Create the full-screen sprite (needs PSRAM) and load the OFR font
//   5. Connect WiFi via WiFiManager (opens a setup portal if needed)
//   6. Start NTP and wait until the clock is valid
//   7. Read the last-used city from flash, fetch both OWM data sets
//   8. Set brightness and draw the home screen
/***************************************************************************/
{
  int iYear, looper;
  int lastDot, lastV;
  String sFile;
  Serial.begin(115200); delay(2000);
  Serial.println("This is Weather Clock Calendar (WCC) for T-Display S3.");
  Serial.println("Running from:");
  Serial.println(__FILE__);

  //--- Version number taken from the file name ----------------------------
  // Be sure to have a little v in front of the version number
  //  and it is last in the filename.
  // Like this: Clock_and_Calendar_v1.56.ino
  // Then this code picks up what is between the v and the ".".
  // >>> REVIEW: Works, but fragile (renaming the folder/file breaks it, and
  // >>>         on Linux/macOS the path separator is '/', not '\\').  Simpler
  // >>>         and bullet-proof:   #define WCC_VERSION "2.15"
  // >>>         in Defines_and_Docs.h and drop this block.

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

  //--- Chip information (informational only) ------------------------------
  Serial.printf("ESP32 Chip model = %s Rev %d\n",
                ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("This chip has %d cores\n", ESP.getChipCores());
  uint64_t chipId = ESP.getEfuseMac();
  //print High 2bytes
  Serial.printf("ESP32ChipID2=%04X\r\n", (uint16_t)(chipId >> 32));
  //print Low 4bytes.  Serial.print("Chip ID 2: ");
  Serial.printf("ESP32ChipID4=%08X\r\n", (uint32_t)chipId);
  // These two "MAC"s don't match at all!
  // >>> NOTE: They do match - they are just printed differently.  EfuseMac is
  // >>>       a 64-bit number printed in DECIMAL with the bytes in reverse
  // >>>       order; WiFi.macAddress() is the same 6 bytes as hex "AA:BB:..".
  // >>>       (Before WiFi is started, macAddress() can also come back as
  // >>>       all zeros on some core versions.)
  Serial.print("EfuseMac: "); Serial.println(ESP.getEfuseMac());
  String myMACAddress = WiFi.macAddress();
  // These two "MAC"s don't match at all!
  Serial.print("MAC Address = "); Serial.println(myMACAddress);

  //--- OpenFontRender (TrueType font engine) ------------------------------
  ofr.setSerial(Serial);
  delay(1000);
  ofr.getFreeTypeVersion(vers);
  // A different way where you get the text in hand
  Serial.print("OFR version: ");
  printWrapped(vers, 54);

  //--- Display ------------------------------------------------------------
  tft.init();
  tft.setRotation(myOrientation);

  //--- Buttons ------------------------------------------------------------
  // Asjust pins based on display orientation.
  if (myOrientation == ORIENT_POWER_LEFT) {
    // City Change is always on top, and...
    buttonCityChange = 0; buttonShow = 14;
  } else {
    // 4 minute show kickoff is always on the bottom button.
    buttonCityChange = 14; buttonShow = 0;
  }
  pinMode(buttonCityChange, INPUT_PULLUP);  // Press for city changing
  pinMode(buttonShow,       INPUT_PULLUP);  // Press to turn on display

  tft.setSwapBytes(true);   // JPEG decoder gives byte-swapped RGB565.

  tft.setFreeFont(&FreeSans12pt7b);

  //--- Backlight (PWM on the TFT_BL pin, 5 kHz, 8 bit = 0..255) -----------
  ledcAttach(TFT_BL, 5000, 8);  // PWM timer automatically assigned.
  // Turn the display on bigly for init messages.
  ledcWrite(TFT_BL, screensExtraBright);

  xCenter = tft.width() / 2;   // Used for centering text
  yCenter = tft.height() / 2;  // Same in the horizontal

  //--- Splash screen ------------------------------------------------------
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

  //--- Full-screen sprite (off-screen frame buffer) -----------------------
  // 320 x 170 x 2 bytes = ~109 KB, so it lives in PSRAM.  Every data screen
  // is drawn into this sprite and then pushed to the display in one go,
  // which is why there is no flicker.
  int *a;  // Temporary pointer holder
  a = (int*)sprite.createSprite(320, 170);
  if (a == 0) {
    Serial.println("Sprite creation failed.  Cannot continue.");
    tft.fillScreen(TFT_WHITE); tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString("PSRAM not available", xCenter, iDisplayLine2);
    tft.drawString("Enable it or use WROVER", xCenter, iDisplayLine4);
    while (1);  // Wait until 1 does not equal 1.
  }
  ofr.setDrawer(tft);  // Link ofr output to base display.
  if (!safeLoadFont("EldoradoBoldTTF", EldoradoBoldTTF,
                    sizeof(EldoradoBoldTTF), __LINE__))
    while (1);
  // >>> REVIEW: The two while(1) "hang forever" traps leave the clock dead
  // >>>   until someone power-cycles it.  For an unattended device, a
  // >>>   delay(30000); ESP.restart(); is usually friendlier.

  //--- WiFi via WiFiManager -----------------------------------------------
  // If no saved network (or it can't connect), WiFiManager starts its own
  // access point "WeatherCC" (password the same) with a web page at
  // 192.168.4.1 where you pick your network.  configModeCallback() shows
  // those instructions on screen.  After 60 s with no setup -> reboot.
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
  // >>> REVIEW: Nothing handles WiFi dropping LATER (router reboot, power
  // >>>   blip).  The fetch routines just skip silently while disconnected.
  // >>>   ESP32 normally auto-reconnects, but adding
  // >>>   WiFi.setAutoReconnect(true); here makes it explicit, and showing
  // >>>   "data is N hours old" on screen would tell you when it hasn't.
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
  // >>> NOTE: the loop above is simply delay(3000).

  //--- Time from NTP ------------------------------------------------------
  // configTime(0,0,...) keeps the system clock in UTC; the TZ string then
  // tells localtime() how to convert to the selected city, including DST.
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString("Fetching time", xCenter, iDisplayLine2);
  tft.drawString("from NTP Server.", xCenter, iDisplayLine4);
  sntp_set_sync_interval(86400000);  // 1 day in ms.
  sntp_set_time_sync_notification_cb(timeSyncCallback);
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  // >>> REVIEW: ntpServer1/ntpServer2 are defined per user in
  // >>>   Defines_and_Docs.h ("oceania.pool.ntp.org" for Mike) but are never
  // >>>   used - the names are hard-coded here.  Use:
  // >>>   configTime(0, 0, ntpServer1, ntpServer2);
  // City 0 is should be your local.  That's the default.
  setenv("TZ", multiCity[0].ENV.c_str(), 1); tzset();

  // Wait (up to ~2 minutes) for NTP to give us a real date.  Before sync
  // the ESP32 clock reads 1970, so "year < 2025" means "not synced yet".
  time(&workTime);
  timeinfo = localtime(&workTime);  // Get correct local time including DST.
  iYear = timeinfo->tm_year + 1900;
  looper = 0;
  int maxLoop = 60;   // >>> REVIEW: unused - the 60 below is hard-coded.
  while (iYear < 2025) {
    Serial.println("Waiting for valid time.");
    delay(2000);
    if (looper++ > 60) ESP.restart();  // If 2 minutes, reboot.
    time(&workTime);
    timeinfo = localtime(&workTime);
    iYear = timeinfo->tm_year + 1900;
    if (iYear > 2025) break;   // >>> NOTE: redundant, the while() does this.
  }
  getMyTime();  // Update the global time variables.
  Serial.println("The time is correct.");
  Serial.println("Let's take a look at the weather, "  // Inside joke.
                 "with KSLA's own Al Bolton.");  // He is surely dead now.
  tft.fillScreen(TFT_WHITE);
  tft.drawString("Fetching WX data", xCenter, iDisplayLine2);
  tft.drawString("from OWM Server.", xCenter, iDisplayLine4);

  //--- Restore last selected city from flash (Preferences / NVS) ----------
  preferences.begin("WCC", RO_MODE);
  whichCity = preferences.getInt("city", 0);
  preferences.end();
  if (whichCity < 0 || whichCity >= MAX_CITY) whichCity = 0;
  prevCity = whichCity;
  Serial.printf("Will fetch city %i: ", whichCity);
  Serial.println(multiCity[whichCity].CityName);
  setenv("TZ", multiCity[0].ENV.c_str(), 1); tzset();

  //--- First weather fetch ------------------------------------------------
  //  Serial.println("Memory before any JSON usage.");
  //  showMemoryStats();
  fetch_and_decode_Forecast();
  //  showMemoryStats();
  fetch_and_decode_OneCall();
  //  showMemoryStats();
  uiNextFetchTime = millis() + uiFetchInterval;  // Remember when to fetch

  //  ledcSetup(0, 10000, 8);  // Prepare the display for dimming, timer 0.
  //  ledcAttach(TFT_BL, 5000, 8);  // PWM timer automatically assigned.
  // >>> REVIEW: second ledcAttach() on the same pin (first one is near the
  // >>>   top of setup).  Core 3.x logs an error / re-attaches.  Delete this.
  // Pin 38 is the display flicker control pin.
  //  ledcAttachPin(DISPLAY_DIM_PIN, 0);

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

  setBrightness(false);  // Find the hour and set the display brightness.
  // Serial.printf("4-Set brightness to %d\r\n", brightness);
  showCurrentCondx();
  time(&lastOn);              // Start the "screen on" timer now.
  displayStatus = displayOn;
}
/***************************************************************************/
void loop()
/***************************************************************************/
//  Runs over and over, many times per second.  Each pass:
//   A. Refresh the time globals (getMyTime) and check the serial menu
//   B. "Show" button / every :00 and :30 -> wake display (+ info splash)
//   C. "City change" button -> show current/requested city, start 4 s timer
//   D. 4 s timer expired -> fetch new city (or say "not changed")
//   E. Once per second: blank the display if its time is up, otherwise
//      draw whichever screen belongs to this second
//   F. Every uiFetchInterval -> refetch weather
//   G. Re-check hourly brightness
/***************************************************************************/
{
  getMyTime();  // Update the time variables used all over the place.

  while (Serial.available() > 0)  // Check if data is available to read
    HandleSerialInput();

  //=== B. Wake-up: button press, or automatically every half hour =========
  // The following fetch the current UTC epoch into lastOn on a button
  // press.  Pressing it again, while on, does not add time but does
  //  restart the on-time timer.
  // Turn on upon button press or on the hour and half hour.
  // >>> NOTE: the code now does it at :00 AND :30 (intMin % 30), not just
  // >>>   the top of the hour.
  if (!cityChangeInProgress) {  // Nothing during city change delay
    if (digitalRead(buttonShow) == buttonPressed ||
        ((intMin % 30) == 0 && intSec == 0) || forceDisplayOn) {
      // (intMin == 0 && intSec == 0) || forceDisplayOn) {
      delay(50);   // Simple debounce: wait, then test again.
      if (digitalRead(buttonShow) == buttonPressed ||
          ((intMin % 30) == 0 && intSec == 0) || forceDisplayOn) {
        // wait for unpress
        while (digitalRead(buttonShow) == buttonPressed);
        // >>> REVIEW: no timeout here (or in the city button below).  A
        // >>>   button that sticks, or a crumb under it, freezes the whole
        // >>>   clock.  e.g.  uint32_t t0 = millis();
        // >>>   while (digitalRead(..) == buttonPressed && millis()-t0 < 3000);
        time(&testOn);
        if (!forceDisplayOn) {
          // Only show the big "info" splash if the screen had been dark
          // (i.e. its BLANK_SECS on-time has run out).
          if (lastOn + BLANK_SECS < testOn) {
            if (!screenOn) {
              Serial.println("Turning display on via button press.");
              tft.writecommand(ST7789_DISPON);  // Turn on display hardware
              screenOn = true;
              setBrightness(true);
            }
            // Turn the display on bigly for init messages.
            ledcWrite(TFT_BL, screensExtraBright);
            tft.fillScreen(TFT_WHITE);
            tft.setTextColor(TFT_BLACK, TFT_WHITE);
            tft.setTextDatum(TC_DATUM);
            ofr.setFontColor(TFT_BLACK, TFT_WHITE);
            ofr.setFontSize(38);

            ofr.setCursor(xCenter,  buttonLine1);
            ofr.cprintf("Time is: %s:%s", timeHour, timeMin);

            ofr.setCursor(xCenter,  buttonLine2);
            ofr.cprintf("in %s", sTown.c_str());

            ofr.setFontSize(34);
            ofr.setCursor(xCenter, buttonLine3);
            ofr.cprintf("Temperature: %i%s",
                        int(lroundf(current_temp)),
                        (multiCity[whichCity].units == "imperial" ? "°F" : "°C"));
            // >>> NOTE: int(x + 0.5) rounds wrong below zero (-2.7 -> -2).
            // >>>   lroundf(current_temp) handles negatives correctly.

            ofr.setCursor(xCenter,  buttonLine4);
            ofr.cprintf("Humidity: %d%%", current_humidity);
            delay(5000);   // Leave the splash up 5 s (clock pauses).
          }
        }
        forceDisplayOn = false;
      }
      time(&lastOn);  // Restart display the blank timer.
      setBrightness(true);
      // Serial.printf("5-Set brightness to %d\r\n", brightness);
    }
  }

  //=== C. City-change button ==============================================
  // For each press and release, cycle through the cities defined in the
  //  array of structs.  If the button is not pressed in 4 seconds do the
  //  requested change, if any.  If the same city is selected as was
  //  selected, only show a message and do not hit up OWM again right now.
  //  prevCity  = the city whose data is on screen now
  //  whichCity = the city being requested
  if (digitalRead(buttonCityChange) == buttonPressed) {
    delay(50);
    if (digitalRead(buttonCityChange) == buttonPressed) {
      // wait for unpress
      while (digitalRead(buttonCityChange) == buttonPressed);
      cityChangeInProgress = true; cityChangeTimer = millis() + 4000;
      // >>> NOTE: cityChangeTimer is declared time_t but holds a millis()
      // >>>   value.  Make it uint32_t to match.
      whichCity++; if (whichCity == MAX_CITY) whichCity = 0;
      Serial.printf("Requesting city #%i - %s\r\n",
                    whichCity, multiCity[whichCity].CityName.c_str());
      if (!screenOn) {
        tft.writecommand(ST7789_DISPON);  // Turn on display hardware
        screenOn = true;
        setBrightness(true);
      }
      // Turn the display on bigly for init messages.
      ledcWrite(TFT_BL, screensExtraBright);
      // Load a different font
      sprite.unloadFont(); sprite.loadFont(BritanicBold36);
      sprite.setTextColor(TFT_BLACK, TFT_WHITE);
      sprite.setTextWrap(false);
      sprite.setTextDatum(TC_DATUM); sprite.fillSprite(TFT_WHITE);
      sprite.drawString("Current city:", xCenter, iDisplayLine1);
      sprite.drawString(multiCity[prevCity].CityName, xCenter,
                        iDisplayLine2);
      sprite.drawString("Requested city:", xCenter, iDisplayLine4);
      sprite.drawString(multiCity[whichCity].CityName, xCenter,
                        iDisplayLine5);
      sprite.pushSprite(0, 0);
      forceDisplayOn = true;   // Keep the screen on once we are done.
      return;
    }
  }

  //=== D. City-change timer ran out -> act on it ==========================
  // If the city change button was pressed, the boolean is set and the timer
  //  is set.  Wait for the timer to expire then do the requested operation.
  // Time to fetch
  if (cityChangeInProgress && millis() > cityChangeTimer) {
    cityChangeInProgress = false;
    // Don't bother OWM for the same as current.
    if (prevCity != whichCity) {
      prevCity = whichCity;
      Serial.printf("Fetching city #%i - %s\r\n",
                    whichCity, multiCity[whichCity].CityName.c_str());
      sprite.setTextColor(TFT_BLACK, TFT_WHITE);
      sprite.setTextWrap(false);
      sprite.setTextDatum(TC_DATUM); sprite.fillSprite(TFT_WHITE);
      sprite.drawString("Fetching", xCenter, iDisplayLine2);
      sprite.drawString(multiCity[whichCity].CityName, xCenter,
                        iDisplayLine4);
      sprite.pushSprite(0, 0);

      // Set the time zone for the newly selected city from
      //  the structs array.
      setenv("TZ", multiCity[whichCity].ENV.c_str(), 1); tzset();

      fetch_and_decode_Forecast(); fetch_and_decode_OneCall();
      // Remember when to fetch
      uiNextFetchTime = millis() + uiFetchInterval;

      // Save the new city so it survives a reboot.
      preferences.begin("WCC", RW_MODE);
      preferences.putInt("city", whichCity);
      preferences.end();
      Serial.printf("Saved city #%i.\r\n", whichCity);
      // Read it back just to prove it was saved (debug - can be removed).
      preferences.begin("WCC", RO_MODE);
      int asdf = preferences.getInt("city", 0);
      preferences.end();
      Serial.printf("Checking saved city is %i\r\n", asdf);
    } else {
      Serial.printf("No city change, staying with %i, %s.\r\n",
                    whichCity, multiCity[whichCity].CityName.c_str());
      sprite.setTextColor(TFT_BLACK, TFT_WHITE);
      sprite.setTextWrap(false);
      sprite.setTextDatum(TC_DATUM); sprite.fillSprite(TFT_WHITE);
      sprite.drawString("City not changed.", xCenter, iDisplayLine2);
      sprite.drawString("No fetch needed.", xCenter, iDisplayLine4);
      sprite.pushSprite(0, 0);
      delay(2000);
    }
    sprite.unloadFont();   // Back to the built-in / GFX fonts.
  }
  if (cityChangeInProgress) return;   // Still inside the 4 s window.

  //=== E. Once-per-second work ============================================
  if (prev_sec == intSec) return;   // Same second as last pass - nothing to do.
  prev_sec = intSec;  // Reset for next second waiting.

  // Get UTC now to compare with lastOn value for blanking.
  time(&workTime);
  if ((workTime - lastOn) > BLANK_SECS) {
    //--- On-time is over: blank the screen (only once) ---
    if (displayStatus == displayOn) {
      displayStatus = displayOff;
      sprite.fillSprite(TFT_BLACK);
      sprite.pushSprite(0, 0);  // Effectively turns the display off.
      brightness = 0;
      // Serial.printf("2-Set brightness to %d\r\n", brightness);
      ledcWrite(TFT_BL, 0);       // Turn the display off.
      tft.writecommand(ST7789_DISPOFF);  // Power off display hardware.
      screenOn = false;
    }
  } else {
    //--- Screen is on: draw the screen that belongs to this second ---
    displayStatus = displayOn;  // Remember screen on

    // >>> REVIEW (efficiency): each graph/text screen is completely redrawn
    // >>>   every second for 8 seconds, although its data has not changed.
    // >>>   Only the clock screen needs a per-second redraw.  Remember the
    // >>>   last screen drawn and only redraw when the screen changes (or
    // >>>   new data arrives).  Less CPU, less heat, smoother clock.

    // Serial.printf("Curr Second %d\r\n", intSec);
    // Rotate the screens.
    if (intSec >  4 && intSec < 13) ShowForecast();  // 05-12
    if (intSec > 12 && intSec < 21) graphTheTemp();  // 13-20
    if (intSec > 20 && intSec < 29) graphTheRain();  // 21-28
    if (totSnow > 0.)  // Snow is rare.  Skip the graph if not needed.
      timePush = 8;    // Make a (time) hole for the snow graph.
    else
      timePush = 0;    // No snow, no need for the hole,
    //                     don't display the snow graph.
    if (totSnow > 0. && intSec > (20 + timePush) &&
        intSec < (29 + timePush)) {
      graphTheSnow();  // 21-28   (>>> NOTE: actually 29-36 when shown)
    }

    if (intSec > (28 + timePush) && intSec < (37 + timePush)) {
      ShowSunMoon();  // 29-36   (37-44 when snow is shown)
    }

    if (intSec > (36 + timePush) || intSec < 5) { // 37-04
      showCurrentCondx();
    }
    // People tend to grab data on even times like on the hour.  This
    //  little fix moves the fetch to every 15 minutes starting at 6 minutes
    //  after the hour.  The code below fetches every 31 minutes counted from
    //  boot (uiFetchInterval), not at fixed minutes past the hour.
    //  BIG difference!  1 second instead of 10!
  }
  //  if (intSec == 0) timePush = 0;   // >>> NOTE: no effect, recalculated above.
  getMyTime();  // Serial.println(fullTimeDate);

  //=== F. Periodic weather refresh ========================================
  if (millis() > uiNextFetchTime) {
    // >>> REVIEW (BUG, latent): millis() wraps to 0 after 49.7 days.  Near
    // >>>   the wrap, millis()+interval overflows to a small number, so this
    // >>>   test is true on EVERY pass for up to 31 minutes - hammering OWM.
    // >>>   Your weekly Monday-02:12 reboot hides this today.  The wrap-safe
    // >>>   pattern is:   if (millis() - lastFetchMs >= uiFetchInterval)
    //    showMemoryStats();
    fetch_and_decode_Forecast();
    //    showMemoryStats();
    fetch_and_decode_OneCall();
    //    showMemoryStats();
    uiNextFetchTime = millis() + uiFetchInterval;  // Remember when to fetch
  }
  // Serial.printf("3-Exit Loop, brightness set to %d\r\n", brightness);
}
/***************************************************************************/
void timeSyncCallback(struct timeval * tv)
/***************************************************************************/
//  Called by the ESP32 SNTP client every time it sets/corrects the clock
//  (at boot and then once a day - see sntp_set_sync_interval in setup()).
//  >>> REVIEW: This runs inside the SNTP/network task, not in loop().  The
//  >>>   two delay(1000)s and the flushes block that task.  Keep callbacks
//  >>>   short: set a flag here and print from loop() if you want the log.
/***************************************************************************/
{
  //  struct timeval {  // Instantiated as "*tv"
  //   time_t      tv_sec;   // Number of whole seconds of elapsed time
  //   Number of microseconds of rest of elapsed time minus tv_sec.
  //   long int    tv_usec;
  //                             Always less than one million
  //};
  Serial.flush(); delay(1000);
  Serial.println("\n----Time Sync-----"); Serial.flush();
  Serial.printf("Time sync at %u ms.\r\nUTC Epoch: ", millis());
  Serial.flush(); Serial.println(tv->tv_sec); Serial.flush();
  Serial.println(ctime(&tv->tv_sec)); Serial.flush();
  delay(1000);
}
/***************************************************************************/
void configModeCallback (WiFiManager * myWiFiManager)
/***************************************************************************/
//  Called by WiFiManager when it could NOT join a saved network and has
//  started its own access point.  Shows red instructions on the screen.
/***************************************************************************/
{
  tft.setTextDatum(TC_DATUM);  // Top center baseline.
  Serial.println(F("Entered config mode..."));
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  yPos = 10;
  tft.fillScreen(TFT_RED);
  // Foreground color, Background color
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.drawString("Connect to", xCenter, iDisplayLine1);
  tft.drawString(myWiFiManager->getConfigPortalSSID(),
                 xCenter, iDisplayLine2);
  tft.drawString("(Password the same)", xCenter, iDisplayLine3);
  tft.drawString("then 192.168.4.1", xCenter, iDisplayLine4);
  tft.drawString("and select a WAP", xCenter, iDisplayLine5);
  // >>> NOTE: 60 s (setTimeout in setup) is short for someone to find
  // >>>   their phone, join the AP and type a password.  120-180 s is kinder.
}
/*******************************************************************************************/
void HandleSerialInput()
/*******************************************************************************************/
//  Very small serial-monitor menu.  Reads ONE character per call.
//  Currently only '?' (help) is implemented - room to add commands such as
//  'F' = fetch now, 'C' = next city, 'B' = toggle backlight, 'M' = memory.
/*******************************************************************************************/
{
  if (!Serial.available()) return;

  int i, iHTemp;   // >>> NOTE: unused.

  char input = Serial.read();
  input = toupper(input);

  // Ignore carriage return and newline entirely
  if (input == '\r' || input == '\n') return;

  Serial.printf("User input %c\r\n", input);

  switch (input) {
    case '?':
      showInputOptionsFull();
      break;

    default:
      Serial.printf("*>> Unknown input '%c'!\r\n", input);
      showInputOptionsFull();
      break;
  }
}
/*******************************************************************************************/
void showInputOptionsFull()
/*******************************************************************************************/
//  Prints the version/build info and the (short) list of serial commands.
/*******************************************************************************************/
{
  Serial.println("Weather Clock Calendar (WCC), v" + sVer);
  Serial.println("Running from:"); Serial.println(__FILE__);
  Serial.print("Compiled on "); Serial.print(__DATE__);
  Serial.print(" at "); Serial.println(__TIME__);
  Serial.println("Enter ? for this list.  Upper or Lower case OK.");
  Serial.println("If you make an unknown entry, this list is printed on the monitor.\r\n");
}
