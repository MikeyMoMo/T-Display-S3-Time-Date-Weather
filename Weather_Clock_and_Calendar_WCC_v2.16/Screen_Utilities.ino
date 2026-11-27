/*===========================================================================
  Screen_Utilities.ino  -  HELPERS + TWO OF THE TEXT SCREENS
  ===========================================================================
  Contents (in order):
    showMemoryStats()        print heap / PSRAM to Serial (debug)
    getMyTime()              refresh ALL the time/date globals + calendar maths
    isLeapYear()             true for leap years
    findLastIndex()          >>> unused (String::lastIndexOf does this)
    ConvertToLocalTime()     epoch -> "%c" text (debug prints only)
    windDegIntToOrdinalDirection()  degrees -> "NNE" etc.
    drawDashLineV()          dotted vertical line (day separators on graphs)
    ShowForecast()           SCREEN: 7-day text forecast
    ShowSunMoon()            SCREEN: sun & moon times, moon phase
    moonPhase()              0..1 phase number -> name
    calcIntermediates()      stretch 3-hour data to 6 points each (graphs)
    drawYAxisScaleLines()    horizontal scale line + label on a graph
    drawArrayJpeg()/renderJPEG()/jpegInfo()   draw the colour bar JPEG
    printMyTime()            print the date/time to Serial
    drawGradientLine()       vertical line fading from one colour to another
    alphaBlend()             mix two RGB565 colours
    printWrapped()           word-wrap long text on Serial
    printFontError()/safeLoadFont()   load the OFR TrueType font safely

  >>> REVIEW: "Utilities" has become a catch-all.  ShowForecast and
  >>>   ShowSunMoon are screens like graphTheTemp and would sit more
  >>>   naturally in their own tab (e.g. showForecastScreens.ino).
  ===========================================================================*/
/***************************************************************************/
void showMemoryStats()
/***************************************************************************/
//  Debug: how much heap and PSRAM is free.  Calls are commented out in
//  setup()/loop(); un-comment them to hunt memory leaks.
/***************************************************************************/
{
  Serial.printf("Total heap:\t%u\r\n", ESP.getHeapSize());
  Serial.printf("Free heap:\t%u\r\n", ESP.getFreeHeap());
  Serial.printf("Total PSRAM:\t%u\r\n", ESP.getPsramSize());
  Serial.printf("Free PSRAM:\t%d\r\n", ESP.getFreePsram());
  //  Serial.printf("spiram size\t%u\r\n", esp_spiram_get_size());
  //  Serial.printf("himem free\t%u\r\n", esp_himem_get_free_size());
  //  Serial.printf("himem phys\t%u\r\n", esp_himem_get_phys_size());
  //  Serial.printf("himem reserved\t%u\r\n", esp_himem_reserved_area_size());
}
/***************************************************************************/
void getMyTime()
/***************************************************************************/
//  Reads the clock (in whatever TZ is currently set) and fills:
//    text:  fullTimeDate, myMonth, myYear, timeHour, timeMin, timeSec,
//           timeWeekDay, timeWkDay, timeDOM, clockString
//    ints:  intMonth (0-11), intYear, intHour, intMin, intSec, intDOW,
//           intDOM, dayInWeek, dayInMonth, daysInMonth, firstDay
//  Also sets bRestartPending at Monday 02:12:00 (weekly reboot).
//  Called several times per loop() pass - it is quick.
/***************************************************************************/
{
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo, 60000)) return;  // Fix this!
  // >>> NOTE: getLocalTime() waits up to 60 s for a valid time if the
  // >>>   clock isn't set yet.  After NTP sync it returns instantly, so in
  // >>>   practice this only matters before the first sync.  A 10-100 ms
  // >>>   timeout is plenty once setup() has confirmed the time.

  strftime(fullTimeDate, 100, "%c", &timeinfo);
  strftime(myMonth, 10, "%B", &timeinfo);  intMonth = timeinfo.tm_mon;
  strftime(myYear,   5, "%Y", &timeinfo);  intYear  = timeinfo.tm_year + 1900;
  strftime(timeHour, 3, "%H", &timeinfo);  intHour  = timeinfo.tm_hour;
  strftime(timeMin,  3, "%M", &timeinfo);  intMin   = timeinfo.tm_min;
  strftime(timeSec,  3, "%S", &timeinfo);  intSec   = timeinfo.tm_sec;
  strftime(timeWeekDay, 10, "%A", &timeinfo);
  strftime(timeWkDay, 4, "%a", &timeinfo); intDOW   = timeinfo.tm_wday;
  // >>> NOTE: timeDOM is only 3 bytes but 4 is passed as its size.  It works
  // >>>   because "%d" never makes more than 2 chars + the end marker, but
  // >>>   always pass sizeof(buffer) so the size can never lie:
  // >>>   strftime(timeDOM, sizeof(timeDOM), "%d", &timeinfo);
  strftime(timeDOM, 4, "%d", &timeinfo);   intDOM   = timeinfo.tm_mday;
  clockString = SDays[intDOW]  + ", " + SMonths[intMonth] + " " +
                String(intDOM) + ", " + String(intYear);

  // Weekly restart trigger: Monday (wday 1) at 02:12:00 in the CURRENT
  // city's time zone.  The restart itself happens at the next forecast fetch.
  if (timeinfo.tm_wday == 1)
    if (timeinfo.tm_hour == 2)
      if (timeinfo.tm_min == 12)
        if (timeinfo.tm_sec == 0) bRestartPending = true;

  dayInWeek   = timeinfo.tm_wday;  // Sunday = 0
  dayInMonth  = timeinfo.tm_mday;  // Day of the month (rationally starting at 1)
  daysInMonth = mm[timeinfo.tm_mon];  // How many days in this month.
  // If it is February and this is a Leap Year, add one day, as expected.
  if (timeinfo.tm_mon == 1 && isLeapYear(timeinfo.tm_year + 1900))
    daysInMonth++;  // Bump it up a notch for leap year February.

  // This is my little hack to quickly find the day of the 1st or the month.
  // You are welcome.  Mike Morrow.
  // >>> REVIEW (BUG - visible on the calendar!):
  // >>>   This formula can give 7, and "if (firstDay < 0)" doesn't fix 7.
  // >>>   It happens on the 7th, 14th, 21st and 28th of any month that starts
  // >>>   on a Sunday (those days are Saturdays: 6 + 1 - 0 = 7).  The calendar
  // >>>   then starts in the SECOND row and its top row is blank.
  // >>>   November 2026 started on a Sunday, so Sat 28 Nov 2026 will show it.
  // >>>   Correct, always 0-6:
  // >>>     firstDay = ((dayInWeek - (dayInMonth - 1)) % 7 + 7) % 7;
  // >>>   (Even simpler: copy timeinfo, set tm_mday = 1, call mktime(), and
  // >>>   read tm_wday.)
  // Calendar might be wrong here.  Claude's proposed solution, above.
  firstDay = ((dayInWeek - (dayInMonth - 1)) % 7 + 7) % 7;  // Sunday = 0 here.
  // This uses Sunday as 0 as TimeLib does.  Others do not.
  // If the 19th is a Friday, then that's 6 (Friday is the 6th day) minus 1
  // is 5 (19 mod 7) and that gives a 1 and that's Monday. (Sunday is 0)
  if (firstDay < 0) firstDay += 7;  // Looks like Sunday is 0 here, too.
}
/***************************************************************************/
bool isLeapYear(int testYear)
/***************************************************************************/
//  Gregorian rule: every 4th year, except centuries, except every 400th.
/***************************************************************************/
{
  if (testYear % 400 == 0)
    return true;
  else if (testYear % 100 != 0 && testYear % 4 == 0)  //
    return true;
  else
    return false;
}
/***************************************************************************/
int findLastIndex(String str, char x)
/***************************************************************************/
//  >>> UNUSED - String already has lastIndexOf(), which setup() uses.
/***************************************************************************/
{
  int index = -1;
  for (int i = 0; i < str.length(); i++)
    if (str[i] == x) index = i;
  return index;
}
/***************************************************************************/
String ConvertToLocalTime(time_t epoch)
/***************************************************************************/
//  Turns a UTC epoch into a local date/time string.  Only used by the
//  forecast debug prints.
//  >>> NOTE: the comment says 'HH:MMpm', but the "%c" format gives
//  >>>   "Mon Nov 23 09:59:00 2026" - which never starts with '0', so the
//  >>>   "remove leading zero" block below never runs.
//  >>> BUG (if it ever did run): strncpy() with overlapping source and
//  >>>   destination is undefined behaviour - use memmove() for that.
/***************************************************************************/
{
  // Returns 'HH:MMpm'
  //  time_t tm = epoch;
  struct tm *now_tm = localtime(&epoch);
  char output[40];
  //  strftime(output, sizeof(output), "%I:%M%p %m/%d/%y", now_tm);
  strftime(output, sizeof(output), "%c", now_tm);
  // Remove leading zero on AM/PM format. They should not put it in with %I!
  //  Serial.printf("Output length: %i\r\n", strlen(output));
  if (output[0] == '0') {  // Not null, but a integer zero.
    strncpy(output, output + 1, strlen(output) - 1);
    // One byte shorter now, loop off the duplicate left by the copy left.
    output[strlen(output) - 1] = '\0';  // This one is the null.
  }
  return output;
}
/***************************************************************************/
String windDegIntToOrdinalDirection(float winddirection)
/***************************************************************************/
//  Wind direction in degrees (0 = from North, 90 = from East) -> one of
//  16 compass names.  Each name covers 22.5 degrees.
//  >>> TIP: one line does the same:
//  >>>   static const char* d[16] = {"N","NNE","NE","ENE","E","ESE","SE",
//  >>>       "SSE","S","SSW","SW","WSW","W","WNW","NW","NNW"};
//  >>>   return d[(int)((winddirection + 11.25) / 22.5) % 16];
/***************************************************************************/
{
  if (winddirection >= 348.75 || winddirection < 11.25)  return TXT_N;
  if (winddirection >=  11.25 && winddirection < 33.75)  return TXT_NNE;
  if (winddirection >=  33.75 && winddirection < 56.25)  return TXT_NE;
  if (winddirection >=  56.25 && winddirection < 78.75)  return TXT_ENE;
  if (winddirection >=  78.75 && winddirection < 101.25) return TXT_E;
  if (winddirection >= 101.25 && winddirection < 123.75) return TXT_ESE;
  if (winddirection >= 123.75 && winddirection < 146.25) return TXT_SE;
  if (winddirection >= 146.25 && winddirection < 168.75) return TXT_SSE;
  if (winddirection >= 168.75 && winddirection < 191.25) return TXT_S;
  if (winddirection >= 191.25 && winddirection < 213.75) return TXT_SSW;
  if (winddirection >= 213.75 && winddirection < 236.25) return TXT_SW;
  if (winddirection >= 236.25 && winddirection < 258.75) return TXT_WSW;
  if (winddirection >= 258.75 && winddirection < 281.25) return TXT_W;
  if (winddirection >= 281.25 && winddirection < 303.75) return TXT_WNW;
  if (winddirection >= 303.75 && winddirection < 326.25) return TXT_NW;
  if (winddirection >= 326.25 && winddirection < 348.75) return TXT_NNW;
  return "?";
}
/***************************************************************************/
/***************************************************************************/
//  Dotted vertical line on the sprite: onPix pixels drawn, offPix skipped,
//  from startY to endY (either direction).  Used for the day separators.
//  >>> NOTE: the "drawing up" branch (signus < 0, which is the one the
//  >>>   graphs use) still draws the offColor dash, while the comment in the
//  >>>   other branch says that was removed.  Pick one behaviour for both.
void drawDashLineV(int xValue, int startY, int endY,
                   int onPix, int offPix, int onColor, int offColor)
/***************************************************************************/
{
  int signus = endY - startY;
  if (signus == 0) {
    Serial.println("signus is 0, exiting.");
    return;
  }
  // Serial.printf("signus %i\r\n", signus);
  if (signus > 0) {
    while (startY < endY) {
      // if signus is -, we are drawing down on the display,
      //  if +, we are drawing up.  Can't be 0.
      sprite.drawLine(xValue, startY, xValue, startY + onPix, onColor);
      startY += onPix;
      // This one will overlay the graph line so it was taken out.  Left
      //  for documentation, not function.  You can put it back if you want.
      // sprite.drawLine(xValue, startY, xValue, startY + onPix, offColor);
      startY += offPix;
    }
  }
  if (signus < 0) {
    while (startY > endY) {
      // Serial.printf("1- xValue %i, startY %i, startY - onPix %i\r\n",
      //               xValue, startY, startY - onPix);
      sprite.drawLine(xValue, startY, xValue, startY - onPix, onColor);
      startY -= onPix;
      // Serial.printf("2- xValue %i, startY %i, startY - onPix %i\r\n",
      //               xValue, startY, startY - onPix);
      sprite.drawLine(xValue, startY, xValue, startY - onPix, offColor);
      startY -= offPix;
    }
  }
}
/***************************************************************************/
void ShowForecast()
/***************************************************************************/
//  SCREEN (seconds 05-12): 7-day text forecast from OneCall daily[].
//    Day | min/max temp | condition text (+ "pop NN%" for rain codes)
//  The ` (back-tick) character is drawn as a degree sign: TFT_eSPI's
//  built-in font 2 has a little circle in that slot.
//  >>> REVIEW: int(daily_item_temp_min[i]) chops decimals (24.9 -> 24).
//  >>>   lroundf() rounds properly, also below zero.
//  >>> REVIEW: "pop" is only shown for 5xx (rain) codes.  Thunderstorm (2xx)
//  >>>   and drizzle (3xx) days also have a useful pop.  e.g.
//  >>>   if (condX == 2 || condX == 3 || condX == 5)
/***************************************************************************/
{
  String forecast;
  int i, j, condX, txtW;
  char output[40];
  int yPos = 20;
  struct tm * now_tm;

#define textCol 95
  // There are exactly 40 characters in font 2 across the screen.  Good luck!
  // Day Txx/yy 29 chars based on ID number.
#if defined DO_FORECAST_PRINTS
  Serial.println("Here's the Forecast.");
#endif
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextDatum(TL_DATUM);
  sprite.setTextColor(TFT_WHITE, myBlue);
  // I know, this is stupid.  The extra spaces are to get the background
  //  color.  Should be a drawFilledRect.  Just lazy.
  sprite.drawString("Day       ",  0,  0, 2);
  sprite.drawString("Temps     ", 38,  0, 2);
  // sTown is pulled from the return packet to verify we got the
  //  right place.  It does not come from the struct arrays CityName field.
  forecast = "WX Forecast for " + sTown;
  sprite.drawString("                                    ", textCol,  0, 2);
  sprite.drawString(forecast, textCol,  0, 2);
  for (i = 0; i < 7; i++) {
    sprite.setTextColor(TFT_YELLOW, TFT_BLACK);
    workTime = daily_item_dt[i];
    now_tm = localtime(&workTime);

    // The short Day name
    sprite.drawString(SDays[now_tm->tm_wday], 0, yPos, 2);

    // The high and low temps expected for the day
    // tmpF = (current_temp * 9. / 5.) + 32.;  // Convert ºF to ºC.
    sprintf(cMinTmp, "%3i", int(lroundf(daily_item_temp_min[i])));
    sprintf(cMaxTmp, "%3i", int(lroundf(daily_item_temp_max[i])));
    forecast = String(cMinTmp) + "`/" + String(cMaxTmp) + "`";
    sprite.drawString(forecast, 28, yPos, 2);

    // Find this day's weather id in the Conditions[] table.
    // >>> BUG: if not found, j ends up == WX_CONDITIONS and Conditions[j]
    // >>>   below reads past the end of the table (crash).  Happens when
    // >>>   OneCall failed and the id is still 0.  Add after the loop:
    // >>>   if (j == WX_CONDITIONS) { /* draw "No data" and continue; */ }
    for (j = 0; j < WX_CONDITIONS; j++) {
      if (Conditions[j].Code == daily_item_weather_0_id[i]) break;
    }
#if defined DO_FORECAST_PRINTS
    forecast.replace("`", "º");
    Serial.print(forecast);
    Serial.printf(" %s\r\n", Conditions[j].Condition.c_str());
#endif
    // The expected weather condition for the day
    sprite.setTextColor(Conditions[j].foreground,  Conditions[j].background);
    txtW = sprite.drawString(Conditions[j].Condition, textCol, yPos, 2);
    condX = daily_item_weather_0_id[i] / 100;
    if (condX == 2 || condX == 3 || condX == 5)  // If one of the rain entries, add pop
      sprite.drawString(" pop " + String(int(daily_item_pop[i] * 100)) +
                        "%", textCol + txtW, yPos, 2);
    yPos += 22;
  }
  if (!screenOn) {
    tft.writecommand(ST7789_DISPON);  // Turn on display hardware
    screenOn = true;
    setBrightness(true);
  }
  // Serial.println("Pushing sprite");
  sprite.pushSprite(0, 0);
  // Serial.printf("Brightness %d\r\n", brightness);
  ledcWrite(TFT_BL, brightness);       // Turn the display on bigly for init messages.
#if defined DO_FORECAST_PRINTS
  Serial.println("-------------------");
#endif
}
/***************************************************************************/
void ShowSunMoon()
/***************************************************************************/
//  SCREEN (8 s after the rain/snow graph): for the next 7 days
//    Day | sunrise/sunset | moonrise/moonset | moon phase name
//  Times are local to the selected city (TZ was set for it).
//  A moonrise/moonset of 0 means "none that day" and is shown as ------.
//  >>> TIP: the 4 copies of the "pad with 0, add hour, ':', pad, minute"
//  >>>   code can each be replaced with:
//  >>>     strftime(buf, sizeof(buf), "%H:%M", localtime(&workTime));
/***************************************************************************/
{
  int i;
  //  time_t myTime;
  String forecast, hourString, minString;
  struct tm * now_tm;
  int yPos = 20;

  // There are exactly 40 characters in font 2 across the screen.  Good luck!
  //  Serial.println("Here's the Sun and Moon information.");
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextColor(TFT_WHITE, myBlue);
  sprite.setTextDatum(TL_DATUM);
  sprite.drawString("Day    ",           0, 0, 2);
  sprite.drawString("Sunrise/Set    ",  33, 0, 2);
  sprite.drawString("Moonrise/Set   ", 122, 0, 2);
  sprite.drawString("Moon phase     ", 215, 0, 2);
  sprite.setTextColor(TFT_YELLOW, TFT_BLACK);

  // Testing
  //  daily_item_moonrise[3] = 0;
  //  daily_item_moonset[5] = 0;
  // End Testing

  for (i = 0; i < 7; i++) {

    // Sunrise and Sunset

    workTime = daily_item_dt[i]; now_tm = localtime(&workTime);

    // Surprised to find the following prints: Sat Dec 16 11:00:00 2023
    // Serial.println(now_tm);

    // The short Day name
    sprite.drawString(SDays[now_tm->tm_wday], 0, yPos, 2);

    // Sunrise and Sunset
    workTime = daily_item_sunrise[i]; now_tm = localtime(&workTime);

    forecast = "";
    if (now_tm->tm_hour < 10) forecast += "0";
    forecast += String(now_tm->tm_hour) + ":";

    if (now_tm->tm_min < 10) forecast += "0";

    forecast += String(now_tm->tm_min) + "/";

    workTime = daily_item_sunset[i]; now_tm = localtime(&workTime);

    if (now_tm->tm_hour < 10) forecast += "0";;
    forecast += String(now_tm->tm_hour) + ":";

    if (now_tm->tm_min < 10) forecast += "0";;
    forecast += String(now_tm->tm_min);

    sprite.drawString(forecast, 33, yPos, 2);

    // Moonrise and Moonset

    forecast = "";  // The buildup area.
    workTime = daily_item_moonrise[i];
    if (workTime == 0) {
      forecast += "------";
    } else {
      now_tm = localtime(&workTime);
      if (now_tm->tm_hour < 10) forecast += "0";
      forecast += String(now_tm->tm_hour) + ":";
      if (now_tm->tm_min < 10) forecast += "0";
      forecast += String(now_tm->tm_min);
    }

    forecast += "/";

    workTime = daily_item_moonset[i];
    if (workTime == 0) {
      forecast += "------";
    } else {
      now_tm = localtime(&workTime);
      if (now_tm->tm_hour < 10) forecast += "0";
      forecast += String(now_tm->tm_hour) + ":";
      if (now_tm->tm_min < 10) forecast += "0";
      forecast += String(now_tm->tm_min);
    }

    sprite.drawString(forecast, 122, yPos, 2);

    // Moon phase text
    sprite.drawString(moonPhase(daily_item_moon_phase[i]), 215, yPos, 2);
    yPos += 22;
  }
  sprite.pushSprite(0, 0);
  if (!screenOn) {
    tft.writecommand(ST7789_DISPON);  // Turn on display hardware
    screenOn = true;
    setBrightness(true);
  }
  ledcWrite(TFT_BL, screensExtraBright);  // Turn the display on bigly for init messages.
}
/***************************************************************************/
String moonPhase(float phase)
/***************************************************************************/
//  >>> TYPO on screen: "Gibous" should be "Gibbous" (twice below).
//  >>> NOTE: phase == .25 etc. only matches if OWM sends exactly 0.25.  It
//  >>>   usually does on the quarter-moon day, but a small window
//  >>>   (e.g. phase within 0.02 of 0.25 -> "First Quarter") is more robust.
/***************************************************************************/
// Moon phase. 0 and 1 are 'new moon', 0.25 is 'first quarter moon',
//  0.5 is 'full moon' and 0.75 is 'last quarter moon'.
//  The periods in between are called 'waxing crescent', 'waxing gibous',
//  'waning gibous', and 'waning crescent', respectively.

// A tolerance of .02 was added to this table to allow for small variances in the
//  returned numbers so it won't totally miss a phase.
{
  if (phase >= 0.98 || phase <= 0.02) return "New Moon";
  if (phase >  0.02 && phase <  0.23) return "Waxing Crescent";
  if (phase >= 0.23 && phase <= 0.27) return "First Quarter";
  if (phase >  0.27 && phase <  0.48) return "Waxing Gibbous";
  if (phase >= 0.48 && phase <= 0.52) return "Full Moon";
  if (phase >  0.52 && phase <  0.73) return "Waning Gibbous";
  if (phase >= 0.73 && phase <= 0.77) return "Last Quarter";
  if (phase >  0.77 && phase <  0.98) return "Waning Crescent";
  return " ";
}
/***************************************************************************/
void calcIntermediates(float myData[], bool doPrint)
/***************************************************************************/
//  The caller puts the real 3-hour values in myData[0], [6], [12] ...
//  This fills the 5 gaps between each pair with evenly spaced values
//  (straight-line interpolation), so the graph is drawn with 6x as many
//  short segments and looks smooth.  doPrint = dump the table to Serial.
//  (There is no later "smoothing" step - the comment below is a plan.)
/***************************************************************************/
{
  int i;
  float diff;
  // This makes a linear interpolation between the data points.
  // Later on, there will be a smoothing adjustment of these values.
  for (i = 0; i < (ThreeHourSamples - 1) * intermediatesCt;
       i += intermediatesCt) {
    diff = myData[i
                  + intermediatesCt] - myData[i];
    myData[i + 1] = myData[i] + diff * .166666;
    myData[i + 2] = myData[i] + diff * .333333;
    myData[i + 3] = myData[i] + diff * .5;
    myData[i + 4] = myData[i] + diff * .666666;
    myData[i + 5] = myData[i] + diff * .833333;
  }
  if (doPrint) {
    //    Serial.println("Y values array with intermediates.");
    for (i = 0; i < (ThreeHourSamples - 1) * intermediatesCt;
         i += intermediatesCt) {
      Serial.printf("%3i: %2.2f / %2.2f / %2.2f / %2.2f / %2.2f / "
                    "%2.2f / %2.2f\r\n",
                    i, myData[i], myData[i + 1], myData[i + 2],
                    myData[i + 3], myData[i + 4], myData[i + 5],
                    myData[i + 6]);
    }
  }
}
/***************************************************************************/
void drawYAxisScaleLines(float myPctHeight, int myLowEnd, int myRange)
/***************************************************************************/
//  Draws one yellow horizontal scale line at myPctHeight of the graph
//  height (0 = top, 1 = X axis) and writes its value at the left.
//  Value = myLowEnd + (1 - myPctHeight) * myRange.
//  >>> NOTE: myLowEnd is an int, so lowTempForecast (a float) is chopped
//  >>>   when passed in.  The temp graph axis uses floor(low), which is
//  >>>   what you want for positive numbers but not for e.g. -2.5.
/***************************************************************************/
{ // These are the 1 or more lines across the graph with a label on the left.
  sprite.drawFastHLine(graphLeftMargin,
                       (tft.height() - graphFloorMargin) * myPctHeight,
                       (tft.width() - graphFloorMargin * 2) - 5, TFT_YELLOW);
  float a = float(1. - myPctHeight) * myRange;
  float c = a + myLowEnd;
  sprite.drawString(String(c, 1), graphLeftMargin - 2,
                    (tft.height() - graphFloorMargin) * myPctHeight, 2);
}
/***************************************************************************/
// Draw a JPEG on the TFT pulled from a program memory array
/***************************************************************************/
//  (Standard JPEGDecoder example code.)  Decodes a JPEG held in a const
//  array (colorbar.h) and draws it into the SPRITE at xpos, ypos.
void drawArrayJpeg(const uint8_t arrayname[],
                   uint32_t array_size, int xpos, int ypos)
{

  int x = xpos;
  int y = ypos;

  JpegDec.decodeArray(arrayname, array_size);

  // jpegInfo(); // Print information from the JPEG file. For Testing only.

  renderJPEG(x, y);

  // Serial.println("#########################");
}
/***************************************************************************/
// Draw a JPEG on the TFT.
// Images will be cropped on the right/bottom sides if they do not fit
/***************************************************************************/
// This function assumes xpos,ypos is a valid screen coordinate.
// For convenience images that do not fit totally on the screen
//  are cropped to the nearest MCU size and may leave right/bottom borders.
/***************************************************************************/
void renderJPEG(int xpos, int ypos) {

  // retrieve information about the image
  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  // Jpeg images are draw as a set of image block (tiles)
  //  called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = minimum(mcu_w, max_x % mcu_w);
  uint32_t min_h = minimum(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  // record the current time so we can measure how long it takes
  //  to draw an image.
  //  uint32_t drawTime = millis();

  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;

  // read each MCU block until there are no more
  while (JpegDec.read()) {

    // save a pointer to the image block
    pImg = JpegDec.pImage ;

    // calculate where the image block should be drawn on the screen
    // Calculate coordinates of top left corner of current MCU
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++)
      {
        p += mcu_w;
        for (int w = 0; w < win_w; w++)
        {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    // calculate how many pixels must be drawn
    uint32_t mcu_pixels = win_w * win_h;

    sprite.startWrite();

    // draw image MCU block only if it will fit on the screen
    if (( mcu_x + win_w ) <= sprite.width() && ( mcu_y + win_h ) <= sprite.height())
    {

      // Now set a MCU bounding window on the sprite
      //  to push pixels into (x, y, x + width - 1, y + height - 1)
      sprite.setAddrWindow(mcu_x, mcu_y, win_w, win_h);

      // Write all MCU pixels to the sprite window
      while (mcu_pixels--) {
        // Push each pixel to the sprite MCU area
        sprite.pushColor(*pImg++);
      }

    }
    // Image has run off bottom of screen so abort decoding
    else if ( (mcu_y + win_h) >= sprite.height()) JpegDec.abort();

    sprite.endWrite();
  }

  // calculate how long it took to draw the image
  //  drawTime = millis() - drawTime;

  // print the results to the serial port
  //  Serial.print(F(  "Total render time was    : "));
  //  Serial.print(drawTime); Serial.println(F(" ms"));
  //  Serial.println(F(""));
}
/***************************************************************************/
// Print image information to the serial port (optional)
/***************************************************************************/
void jpegInfo() {
  Serial.println(F("==============="));
  Serial.println(F("JPEG image info"));
  Serial.println(F("==============="));
  Serial.print(F(  "Width      :")); Serial.println(JpegDec.width);
  Serial.print(F(  "Height     :")); Serial.println(JpegDec.height);
  Serial.print(F(  "Components :")); Serial.println(JpegDec.comps);
  Serial.print(F(  "MCU / row  :")); Serial.println(JpegDec.MCUSPerRow);
  Serial.print(F(  "MCU / col  :")); Serial.println(JpegDec.MCUSPerCol);
  Serial.print(F(  "Scan type  :")); Serial.println(JpegDec.scanType);
  Serial.print(F(  "MCU width  :")); Serial.println(JpegDec.MCUWidth);
  Serial.print(F(  "MCU height :")); Serial.println(JpegDec.MCUHeight);
  Serial.println(F("==============="));
}
/***************************************************************************/
void printMyTime()
/***************************************************************************/
//  Refresh the time globals and print the full date/time to Serial.
/***************************************************************************/
{
  getMyTime(); Serial.println(fullTimeDate);
}
///***************************************************************************/
//void drawGradientLine(TFT_eSprite *targetSprite,
//                      int x0, int y0, int x1, int y1,
//                      uint16_t colorStart, uint16_t colorEnd)
///***************************************************************************/
//{
//  int steps = abs(x1 - x0) > abs(y1 - y0) ? abs(x1 - x0) : abs(y1 - y0);
//  int x = x0, y = y0;
//  for (int i = 0; i <= steps; i++) {
//    pctBlend = (float)i / steps;
//    blendedColor = alphaBlend((uint8_t)(pctBlend * 255), colorEnd, colorStart);
//    targetSprite->drawPixel(x, y, blendedColor);
//    if ((x0 - x1) == 0) y1 > y0 ? y++ : y--;
//    if ((y0 - y1) == 0) x1 > x0 ? x++ : x--;
//  }
//}
/***************************************************************************/
void drawGradientLine(TFT_eSprite *targetSprite,
                      //  Pixel-by-pixel line that fades from colorStart (at x1,y1) to colorEnd
                      //  (at x2,y2).  Used to fill under the graph lines.
                      //  blendLvls: 0 = fade over the whole length of this line (what all callers
                      //             use); N = fade over N pixels regardless of line length.
                      //  >>> LIMITATION: only horizontal or vertical lines work.  For a sloping
                      //  >>>   line neither x nor y is stepped, so one pixel is drawn repeatedly.
                      //  >>> BUG (if blendLvls > 0 is ever used): when the line is longer than
                      //  >>>   blendLvls, pctBlend goes above 1.0 and (uint8_t)(pctBlend*255)
                      //  >>>   wraps round -> colour jumps.  Clamp pctBlend to 1.0.
                      //  >>> NOTE: "static" on x, y, steps is not needed (and makes the function
                      //  >>>   non-reentrant); plain locals are fine.
                      //  >>> SPEED: for a vertical fade, drawFastVLine() of a few colour bands,
                      //  >>>   or pre-computing a colour table once per graph, is much faster
                      //  >>>   than ~150 drawPixel + alphaBlend calls per column.
                      int x1, int y1, int x2, int y2,
                      int blendLvls, uint16_t colorStart, uint16_t colorEnd)
/***************************************************************************/
{
  static int x, y, steps;

  // The 6th value (steps) changes the look of the gradient line.
  //  If it is 0, the gradient is completed along the length of the
  //  line no matter how long or short.  If there is a value here, its
  //  value is used for the line. This gives a more regular look
  //  to the gradient lines.  Try both to see the difference.

  // steps is the length of the line in pixels
  steps = abs(x2 - x1) > abs(y2 - y1) ? abs(x2 - x1) : abs(y2 - y1);
  // if blandLvls came in as 0, use the line length for step count
  if (blendLvls == 0) blendLvls = steps;
  //  Serial.printf("x1 %i, y1 %i, x2 %i, y2 %i, steps %i, blendLvls %i\r\n",
  //                x1, y2, x2, y1, steps, blendLvls);
  x = x1; y = y1;
  for (int i = 0; i <= steps; i++) {
    pctBlend = (float)i / (float)blendLvls;
    //    blendedColor = alphaBlend((uint8_t)(pctBlend * 255),
    //                              colorEnd, colorStart);
    // targetSprite->drawPixel(x, y, blendedColor);
    targetSprite->drawPixel(
      x, y, alphaBlend((uint8_t)(pctBlend * 255), colorEnd, colorStart));
    if ((x1 - x2) == 0) y2 > y1 ? y++ : y--;
    if ((y1 - y2) == 0) x2 > x1 ? x++ : x--;
  }
}
/***************************************************************************/
uint16_t alphaBlend(uint8_t alpha, uint16_t fgc, uint16_t bgc)
/***************************************************************************/
//  Mix two RGB565 colours: alpha 0 = all bgc, 255 = all fgc.
//  (Same as TFT_eSPI's own tft.alphaBlend() - you could call that instead.)
/***************************************************************************/
{
  uint32_t rxb = bgc & 0xF81F;
  rxb += ((fgc & 0xF81F) - rxb) * (alpha >> 2) >> 6;
  uint32_t xgx = bgc & 0x07E0;
  xgx += ((fgc & 0x07E0) - xgx) * alpha >> 8;
  return (rxb & 0xF81F) | (xgx & 0x07E0);
}
/*****************************************************************************/
void printWrapped(const char *str, int width)
/*****************************************************************************/
//  Print text to Serial, breaking at the first space after "width" columns.
/*****************************************************************************/
{
  int col = 0;
  const char *p = str;

  while (*p) {
    // If we hit a space and we're past the wrap width, break line here
    if (*p == ' ' && col >= width) {
      Serial.print("\r\n");
      col = 0;
      p++; // skip the space
      continue;
    }

    // Print the current character
    Serial.print(*p);
    col++;

    // If we hit newline in the source, reset column count
    if (*p == '\n') {
      col = 0;
    }

    p++;
  }
  Serial.print("\r\n"); // final line break
}
//template <size_t N>
//bool safeLoadFont(const char* fontName, const uint8_t (&fontData)[N],
//                  int line)
//{
//  int rc = 0;
//
//  try {
//    ofr.unloadFont();
//  }
//  catch (...) {
//    Serial.println("unloadFont failed, continuing anyway...");
//  }
//
//  rc = ofr.loadFont(fontData, N);  // compiler knows N at compile time
//  if (rc) {
//    printFontError(fontName, rc, __func__, line);
//    return false;  // failure
//  }
//  return true;  // success
//}
/*****************************************************************************/
void printFontError(const char* fontName, int rc, const char* func, int line)
/*****************************************************************************/
//  Report an OpenFontRender loadFont() failure on Serial and the screen.
//  >>> NOTE: the error-code meanings below are guesses ("add more as you
//  >>>   discover them").  OFR returns FreeType error codes; check
//  >>>   fterrdef.h before trusting the text.
/*****************************************************************************/
{
  Serial.printf("Render loadFont error for %s\n", fontName);
  Serial.printf("In routine %s, near line %d\n", func, line - 2);
  Serial.printf("Error code %i\r\n", rc);
  // Optional: map known error codes to text
  switch (rc) {
    case 1: Serial.println("Error: invalid font data"); break;
    case 2: Serial.println("Error: allocation failed"); break;
    case 3: Serial.println("Error: font already loaded"); break;
    // Add more codes as you discover them
    default: Serial.println("Error: unknown code"); break;
  }
  tft.setCursor(0, 0);
  tft.print("OFR Font load failed for: ");
  tft.print(fontName);
}
/*****************************************************************************/
//  Unload whatever OFR font is loaded, load the new one from a byte array.
//  Returns true on success.
//  >>> NOTE: the try/catch does nothing useful - unloadFont() never throws,
//  >>>   and a real crash on an ESP32 is not a C++ exception.  Can go.
bool safeLoadFont(const char* fontName, const uint8_t* fontData,
                  size_t dataSize, int line)
/*****************************************************************************/
{
  int rc = 0;

  try {
    ofr.unloadFont();
  }
  catch (...) {
    Serial.println("unloadFont failed, continuing anyway...");
  }

  rc = ofr.loadFont(fontData, dataSize);
  if (rc) {
    printFontError(fontName, rc, __func__, line);
    return false;  // failure
  }
  return true;  // success
}
/***************************************************************************/
void setBrightness(bool forceTime)
/***************************************************************************/
//  PURPOSE: Once per (home-city) hour, look up ihourlyBrilliance[hour] and
//           apply it to the backlight.
//  HOW:     Temporarily switch TZ to the home city (multiCity[0]), read the
//           hour, then switch TZ back to the selected city.
//
//  >>> REVIEW (BUG - the brightness table is mostly ignored):
//  >>>   ShowForecast, ShowSunMoon and all three graphs call
//  >>>   ledcWrite(TFT_BL, screensExtraBright) = 200 every time they draw.
//  >>>   So within ~5 s of the hour change your 80/100/160 value is replaced
//  >>>   by 200, and it stays 200 until the next hour (showCurrentCondx
//  >>>   doesn't reset it).  Night dimming therefore never really happens.
//  >>>   Fix: in those screens use ledcWrite(TFT_BL, brightness), and keep
//  >>>   screensExtraBright only for the white "message" screens.
//  >>>
//  >>>   Also: on an hour change while the screen is blanked, this turns the
//  >>>   display hardware and backlight back on (showing black).  It
//  >>>   coincides with the :00 auto-wake so you don't notice, but it should
//  >>>   only touch the hardware if displayStatus == displayOn.
//  >>>
//  >>>   Also: this runs every loop pass (and does two setenv/tzset calls
//  >>>   each time).  Cheap, but it could just compare the hour from a
//  >>>   single localtime() using the home offset.
/***************************************************************************/
{
  // By using setenv, DST is supported.  This routine finds the current hour
  //  then sets the display brightness for that hour of home time.
  //  For the purists among you, this can be seen as a little bit wasteful
  //  for if the home city is selected, there is a setenv that are not
  //  needed.  But this whole thing runs in less than 10ms so I did not
  //  worry about that time loss each hour.  If I was writing an operating
  //  system, I would worry about it.  For this application, nope!

  // Change to home time.
  if (whichCity != homeCity) {
    setenv("TZ", multiCity[homeCity].ENV.c_str(), 1); tzset();
  }
  time(&workTime);  // Get the local time epoch.
  timeinfo = localtime(&workTime);  // Get correct local time including DST.
  localHour = timeinfo->tm_hour;  // Get the home time hour.
  if ((prev_hour != localHour) || forceTime) {  // If hour changed, maybe new brightness.
    // Serial.printf("setBrightness() local hour %d\r\n", localHour);
    prev_hour  = localHour;
    // Serial.printf("Setting brightness to %d\r\n", ihourlyBrilliance[localHour]);
    brightness = ihourlyBrilliance[localHour];
    // Serial.printf("1-Set brightness to %d\r\n", brightness);
    // Serial.printf("Setting screen brightness for hour %02i to %i\r\n",
    //               localHour, ihourlyBrilliance[localHour]);
    if (!screenOn) {
      tft.writecommand(ST7789_DISPON);  // Turn on display hardware
      screenOn = true;
    }
    ledcWrite(TFT_BL, brightness);
  }
  // All is done here, go back to previous time zone (if needed).
  if (whichCity != homeCity) {  // Fix it back if changed.
    setenv("TZ", multiCity[whichCity].ENV.c_str(), 1); tzset();
  }
}
