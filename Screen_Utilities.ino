/***************************************************************************/
void smoothData(float myData[], int parts)
/***************************************************************************/
{
  // When I get here, I receive an array of readings, not converted to
  //  pixels.  So if the graph will be going up, (i+1) - i will be positive.
  //  If the graph is going downward, (i+1) - i will be negative.
  //  Adjust the very first 1/2 of the graph, between 0 and 2, then go into
  //  loop waiting for the main spot, which will be an intermetiate middle
  //  spot to reach 40*6-3.  That's 239 backed up to the middle spot.
  //  Then adjust the final 3 and exit.

  // The trick comes with how to adjust the two spots preceeding and
  //  succeeding the mid spot.
  //
  // If the initial mid spot is higher than the first data point, move the
  //  two spots down by the percentages in the segments array.  Then, if the
  //  next set is going up, there is no change.  If the next spot is not
  //  going up, then adjust the succeeding spots upwards.
  // If the initial mid spot is the same, then there will be no change of the
  //  two preceeding and succeeding spots.  Check the next set and proceed
  //  as mentioned elsewhere.  If the next mid spot is going up,
  //  then adjust the preceeding spots in the second set downwards. If going
  //  down, adjust the preceeding spots upwards.
  // If the initial mid spot is lower than the first data point, move the
  //  two spots up.  If the next midspot is not going up, adjust the
  //  succeeding two spots in the first group downward.
  //
  // Or something like that.
  //
  // These are the fractions to move the spots down or up between the
  //  last spot (lastX/Y) and the mid spot or the mid spot and the
  //  next (thisX/Y) spot.
  float segments[3] = {0.166666, 0.333333};

}
/***************************************************************************/
void getMyTime()
/***************************************************************************/
{
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) return;

  strftime(fullTimeDate, 100, "%c", &timeinfo);
  strftime(myMonth, 10, "%B", &timeinfo);  intMonth = timeinfo.tm_mon;
  strftime(myYear,   5, "%Y", &timeinfo);  intYear  = timeinfo.tm_year + 1900;
  strftime(timeHour, 3, "%H", &timeinfo);  intHour = timeinfo.tm_hour;
  strftime(timeMin,  3, "%M", &timeinfo);  intMin  = timeinfo.tm_min;
  strftime(timeSec,  3, "%S", &timeinfo);  intSec  = timeinfo.tm_sec;
  strftime(timeWeekDay, 10, "%A", &timeinfo);
  strftime(timeWkDay, 4, "%a", &timeinfo); intDOW  = timeinfo.tm_wday;
  strftime(timeDOM, 4, "%d", &timeinfo);   intDOM  = timeinfo.tm_mday;
  buttonClockString = SDays[intDOW]  + ", " + SMonths[intMonth] + " " +
                      String(intDOM) + ", " + String(intYear);

  dayInWeek   = timeinfo.tm_wday;  // Sunday = 0
  dayInMonth  = timeinfo.tm_mday;
  daysInMonth = mm[timeinfo.tm_mon];

  // This is my little hack to quickly find the day of the 1st or the month.
  // You are welcome.  Mike Morrow.
  firstDay = dayInWeek + 1 - (dayInMonth % 7);
  // This uses Sunday as 0 as TimeLib does.  Others do not.
  // If the 19th is a Friday, then that's 6 (Friday is the 6th day) minus
  //  5 (19 mod 7) and that gives a 1 and that's Monday. (Sunday is 0)
  if (firstDay < 0) firstDay += 7;  // Looks like Sunday is 0 here, too.

  if (timeinfo.tm_mon == 1 && isLeapYear(timeinfo.tm_year + 1900))
    daysInMonth++;  // Bump it up a notch for leap year February.
}
/***************************************************************************/
void printMyTime()
/***************************************************************************/
{
  getMyTime(); Serial.println(fullTimeDate);
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

/***************************************************************************/
int findLastIndex(String str, char x)
/***************************************************************************/
{
  int index = -1;
  for (int i = 0; i < str.length(); i++)
    if (str[i] == x) index = i;
  return index;
}
/***************************************************************************/
String ConvertToLocalTime(long epoch)
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
{
  String forecast;
  int i, j, condX, txtW;
  char output[40];
  int yPos = 20;
  struct tm * now_tm;

  // There are exactly 40 characters in font 2 across the screen.  Good luck!
  // Day Txx/yy 29 chars based on ID number.
#if defined DO_ONECALL_PRINTS
  Serial.println("Here's the Forecast.");
#endif
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextDatum(TL_DATUM);
  sprite.setTextColor(TFT_WHITE, myBlue);
  // I know, this is stupid.  The extra spaces are to get the background
  //  color.  Should be a drawFilledRect.  Just lazy.  Letting
  sprite.drawString("Day       ",              0,  0, 2);
  sprite.drawString("Temps     ",             38,  0, 2);
  forecast = "WX Forecast for " + sTown;
  //  Serial.println(forecast);
  sprite.drawString("                                    ", 85,  0, 2);
  sprite.drawString(forecast, 85,  0, 2);
  for (i = 0; i < 7; i++) {
    sprite.setTextColor(TFT_YELLOW, TFT_BLACK);
    workTime = daily_item_dt[i];
    now_tm = localtime(&workTime);

    // The short Day name
    sprite.drawString(SDays[now_tm->tm_wday], 0, yPos, 2);

    // The high and low temps expected for the day
    // tmpF = (current_temp * 9. / 5.) + 32.;  // Convert ºF to ºC.
    if (Units == "I") {
      sprintf(cMinTmp, "%3i", int((daily_item_temp_min[i] * 9. / 5.) + 32.));
      sprintf(cMaxTmp, "%3i", int((daily_item_temp_max[i] * 9. / 5.) + 32.));
    } else {
      sprintf(cMinTmp, "%3i", int(daily_item_temp_min[i]));
      sprintf(cMaxTmp, "%3i", int(daily_item_temp_max[i]));
    }
    forecast = String(cMinTmp) + "/" + String(cMaxTmp);
    sprite.drawString(forecast, 28, yPos, 2);

    for (j = 0; j < WX_CONDITIONS; j++) {
      if (Conditions[j].Code == daily_item_weather_0_id[i]) break;
    }
#if defined DO_ONECALL_PRINTS
    Serial.print(forecast)
#endif
    // The expected weather condition for the day
    sprite.setTextColor(Conditions[j].foreground,  Conditions[j].background);
#if defined DO_ONECALL_PRINTS
    Serial.println(Conditions[j].Condition);
#endif
    txtW = sprite.drawString(Conditions[j].Condition, 85, yPos, 2);
    condX = daily_item_weather_0_id[i] / 100;
    if (condX == 5)  // If one of the rain entries, add pop
      sprite.drawString(" pop " + String(int(daily_item_pop[i] * 100)) +
                        "%", 85 + txtW, yPos, 2);
    yPos += 22;
  }
  sprite.pushSprite(0, 0);
  ledcWrite(0, extraScreensBrightness);
#if defined DO_ONECALL_PRINTS
  Serial.println("-------------------");
#endif
}
/***************************************************************************/
void ShowSunMoon()
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
  sprite.drawString("Sunrise/Set    ",  30, 0, 2);
  sprite.drawString("Moon Rise/Set   ", 114, 0, 2);
  sprite.drawString("Moon phase     ", 215, 0, 2);
  sprite.setTextColor(TFT_YELLOW, TFT_BLACK);

  // Testing
  //  daily_item_moonrise[3] = 0;
  //  daily_item_moonset[5] = 0;
  // End Testing

  for (i = 0; i < 7; i++) {

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
    forecast += String(now_tm->tm_min) + "-";

    workTime = daily_item_sunset[i]; now_tm = localtime(&workTime);

    if (now_tm->tm_hour < 10) forecast += "0";;
    forecast += String(now_tm->tm_hour) + ":";

    if (now_tm->tm_min < 10) forecast += "0";;
    forecast += String(now_tm->tm_min);

    sprite.drawString(forecast, 30, yPos, 2);

    // Moonrise and Moonset
    forecast = "";
    if (daily_item_moonset[i] < daily_item_moonrise[i]) {
      workTime = daily_item_moonset[i];
      moonPrefix = "S";
    } else {
      workTime = daily_item_moonrise[i];
      moonPrefix = "R";
    }
    if (workTime == 0) {
      forecast += " ------";
    } else {
      forecast += moonPrefix;
      now_tm = localtime(&workTime);
      if (now_tm->tm_hour < 10) forecast += "0";
      forecast += String(now_tm->tm_hour) + ":";
      if (now_tm->tm_min < 10) forecast += "0";
      forecast += String(now_tm->tm_min);
    }
    forecast += "/";

    if (daily_item_moonset[i] < daily_item_moonrise[i]) {
      workTime = daily_item_moonrise[i];
      moonPrefix = "R";
    } else {
      workTime = daily_item_moonset[i];
      moonPrefix = "S";
    }
    if (workTime == 0) {
      forecast += "------";
    } else {
      forecast += moonPrefix;
      now_tm = localtime(&workTime);
      if (now_tm->tm_hour < 10) forecast += "0";
      forecast += String(now_tm->tm_hour) + ":";
      if (now_tm->tm_min < 10) forecast += "0";
      forecast += String(now_tm->tm_min);
    }
    sprite.drawString(forecast, 115, yPos, 2);

    // Moon phase text
    sprite.drawString(moonPhase(daily_item_moon_phase[i]), 215, yPos, 2);
    yPos += 22;
  }
  sprite.pushSprite(0, 0);
  ledcWrite(0, extraScreensBrightness);
}
/***************************************************************************/
String moonPhase(float phase)
/***************************************************************************/
// Moon phase. 0 and 1 are 'new moon', 0.25 is 'first quarter moon',
//  0.5 is 'full moon' and 0.75 is 'last quarter moon'.
//  The periods in between are called 'waxing crescent', 'waxing gibous',
//  'waning gibous', and 'waning crescent', respectively.
{
  if (phase == 0. || phase == 1.) return "New Moon";
  if (phase > 0.  && phase < .25) return "Waxing Crescent";
  if (phase == .25)               return "First Quarter";
  if (phase > .25 && phase < .5)  return "Waxing Gibous";
  if (phase == .5)                return "Full Moon";
  if (phase > .5  && phase < .75) return "Waning Gibous";
  if (phase == .75)               return "Last Quarter";
  if (phase > .75)                return "Waning Crescent";
}
/***************************************************************************/
void calcIntermediates(float myData[], bool doPrint)
/***************************************************************************/
{
  int i;
  float diff;
  // This makes a linear interpolation between the data points.
  // Later on, there will be a smoothing adjustment of these values.
  for (i = 0; i < (ThreeHourSamples - 1) * intermediatesCt; i += intermediatesCt) {
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
{ // These are the 1 or more lines across the graph with a label on the left.
  sprite.drawFastHLine(graphLeftMargin,
                       (tft.height() - graphFloorMargin) * myPctHeight,
                       (tft.width() - graphFloorMargin * 2) - 5, TFT_YELLOW);
  float a = float(1. - myPctHeight) * myRange;
  float c = a + myLowEnd;
  sprite.drawString(String(c, 1), graphLeftMargin - 2,
                    (tft.height() - graphFloorMargin) * myPctHeight, 2);
}
