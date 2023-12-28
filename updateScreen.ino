/***************************************************************************/
void showCurrentCondx()
/***************************************************************************/
{
  sprite.fillSprite(TFT_BLACK);

  cax = 142; cay = 70; cah = 15; caw = 24;
  sprite.setTextDatum(MC_DATUM);
  sprite.setTextColor(TFT_WHITE, TFT_BLACK);
  for (int j = 0; j < 7; j++)
    sprite.drawString(SSDay[j], cax + (j * caw), cay, 2);

  int DOM = 1;
  int DOW = 0;
  bool started = false;

  for (int i = 0; i < 6; i++)
    for (int j = 0; j < 7; j++) {
      if (DOW == firstDay)
        started = true;
      if (started && DOM <= daysInMonth) {
        if (DOM == dayInMonth)
          sprite.setTextColor(TFT_BLACK, TFT_WHITE);
        else
          sprite.setTextColor(TFT_ORANGE, TFT_BLACK);
        sprite.drawString(String(DOM), cax + (j * caw),
                          cay + cah + (cah * i), 2);
        DOM++;
      }
      DOW++;
    }
  sprite.setTextDatum(TL_DATUM);
  sprite.setTextColor(TFT_WHITE, TFT_BLACK);
  sprite.setFreeFont(&Orbitron_Light_32);
  sprite.drawString(String(timeHour) + ":" + String(timeMin), 130, -6);
  sprite.setFreeFont(&Orbitron_Light_24);

  sprite.setTextColor(0xD399, TFT_BLACK);
  sprite.drawString(String(timeSec), 250, -4);

  sprite.setTextColor(0x35F9, TFT_BLACK);
  sprite.setFreeFont(&FreeSans9pt7b);
  sprite.drawString(String(myMonth) + " " + String(dayInMonth) + ", " +
                    String(myYear), 132, 37);

  sprite.setTextColor(TFT_DARKGREY, TFT_BLACK);

  seg = brightness / 25;
  for (int i = 0; i < seg; i++)
    sprite.fillRect(308, 150 - (i * 13), 4, 11, 0x35F9);
  ledcWrite(0, brightness);

  sprite.drawLine(cax - 10, cay - 10, cax + 152, cay - 10, myGray);

  sprite.setTextColor(0x35F9, TFT_BLACK);
  sprite.setFreeFont(&FreeSans9pt7b);
  int yPos = 10;
  sprite.drawString("Temp", 0, yPos);
  sprite.drawString("Hum",  0, yPos + 30);
  sprite.drawString("Wind", 0, yPos + 60);
  sprite.drawString("From", 0, yPos + 90);
  sprite.drawString(current_weather_0_main, 0, yPos + 120);
  sprite.setTextColor(TFT_DARKGREY, TFT_BLACK);
  // This is so odd.  You have to negate to get a go signal on a match.
  //  if (!strcmp(current_weather_0_main, "Clouds")) {
  if (!strcmp(hourly_item_weather_0_main[0], "Clouds")) {
    sprite.setTextDatum(TR_DATUM);
    sprite.drawString(String(current_clouds) + "%", 110, yPos + 120);
    sprite.setTextDatum(TL_DATUM);
  }
  sprite.setTextColor(TFT_DARKGREY, TFT_BLACK);
  sprintf(workChar, "Updated: %02i:%02i", dataFetchHour, dataFetchMin);
  sprite.drawString(workChar, 0, 150, 2);
  sprite.setFreeFont(&FreeSans12pt7b);
  yPos = 30;
  sprite.drawFastHLine(0, yPos, 110, TFT_WHITE);
  sprite.drawFastHLine(0, yPos + 30, 110, TFT_WHITE);
  sprite.drawFastHLine(0, yPos + 90, 110, TFT_WHITE);

  yPos = 5;
  // Temperature
#if defined TEMPF
  // F Temperature
  tmpF = (current_temp * 9. / 5.) + 32.;  // Convert ºF to ºC.
  sprite.drawString(String(int(tmpF + .5)) + "F", 50, yPos);
#else
  // C Temperature
  tmpC = current_temp;  // Convert String representation to float
  sprite.drawString(String(int(tmpC + .5)) + "c", 50, yPos);
#endif
  // Humidity
  sprite.drawString(String(current_humidity) + "%", 50, yPos + 30);

  // Wind info
  //  Serial.print("current_wind_speed/");
  //  Serial.print(String(current_wind_speed, 0));
  //  Serial.println("/");
  //  Serial.print(String(current_wind_speed, 1));
  //  Serial.println("/");

  if (current_wind_speed >= 10) {
    sprintf(workChar, "%.0f m/s", current_wind_speed);
    sprite.drawString(workChar, 50, yPos + 60);
  } else {
    sprintf(workChar, "%.1f m/s", current_wind_speed);
    sprite.drawString(workChar, 50, yPos + 60);
  }
  windDegStr = windDegIntToOrdinalDirection(current_wind_deg);
  sprite.drawString(windDegStr, 50, 95);

  sprite.pushImage(297, 0, 26, 26, snowFlake);
  sprite.pushSprite(0, 0);
}
/***************************************************************************/
String ConvertToLocalTime(long epoch)
/***************************************************************************/
{
  // Returns 'HH:MMpm'
  //  time_t tm = epoch;
  struct tm *now_tm = localtime(&epoch);
  char output[40];
  strftime(output, sizeof(output), "%c", now_tm);
  // Remove leading zero on AM/PM format. They should not put it in with %I!
  //  Serial.printf("Output length: %i\r\n", strlen(output));
  if (output[0] == '0') {
    strncpy(output, output + 1, strlen(output) - 1);
    output[strlen(output) - 1] = '\0';  // Erase the left over digit.
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
void graphTheRain()
/***************************************************************************/
{
  // The display is 170 x 320.  We will allow some margin at the bottom so
  //  less than 170 will be utilized.
  // I multiply the highest rainfall amount by 100 and then divide into the
  //  number of pixels that will be used giving pix/degree.  All in float.
  float thisX, thisY;
  float lastX, lastY;
  int prevDay = -1, intMyDay;
  String legend = "3-Hourly Rainfall amount (mm) (5 Days)";
  int halfLegend = sprite.textWidth(legend, 2) / 2;  // Text width in font 2.
  int yRange = ceil(highRainForecast);
  float pixelsPerHundredthV = (tft.height() - graphFloorMargin) /
                              (yRange * 100.);
  // So, the minimum plot point line will be graphFloor and
  //  the top will be the top of the display.
  // The display is 320 wide.  Again, we leave a margin at the right.  There
  //  will be 40 (currently) plots of rainfall amount, each hour, for 5 days.
  float pixelsPerSampleH = (tft.width() - graphLeftMargin) / ThreeHourSamples;

  // Now, start to draw
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextColor(TFT_YELLOW, TFT_BLACK);
  // Decorations part 1
  sprite.setTextDatum(TC_DATUM);  // Rain legend
  sprite.drawString(legend,
                    (tft.width() + graphLeftMargin) / 2, 0, 2); // 180

  sprite.drawFastVLine(graphLeftMargin, 0,
                       tft.height() - graphFloorMargin, TFT_YELLOW);
  sprite.drawFastHLine(graphLeftMargin, tft.height() - graphFloorMargin,
                       tft.width() - graphFloorMargin * 2, TFT_YELLOW);

  lastX = graphLeftMargin;
  lastY = tft.height() - graphFloorMargin -
          (WxForecast[0].Rainfall * 100 * pixelsPerHundredthV);

  for (int i = 0; i < ThreeHourSamples - 1; i++) {
    thisX = ((pixelsPerSampleH * (i + 1)) + float(graphLeftMargin));
    thisY = float(tft.height()) - float(graphFloorMargin) -
            (WxForecast[i + 1].Rainfall * 100 * pixelsPerHundredthV);

    sprite.drawLine(int(lastX), int(lastY), int(thisX), int(thisY), rainGraphLineColor);

    // Remember starting position for next line segment or curve.
    lastX = thisX, lastY = thisY;

    time_t tm = WxForecast[i].dt;  // Get start time/date for 3 hour forecast.
    struct tm *now_tm = localtime(&tm);
    intMyDay = now_tm->tm_wday;
    // Now, when the day changes, put in the new day name and dashed line.
    sprite.setTextDatum(TL_DATUM);
    if (prevDay != intMyDay) {
      sprite.drawString(" " + SDay[intMyDay],
                        pixelsPerSampleH * i + graphLeftMargin + 12,
                        tft.height() - graphFloorMargin + 3, 2);
      drawDashLineV(pixelsPerSampleH * i + graphLeftMargin,
                    tft.height() - graphFloorMargin,
                    20, 3, 6, TFT_DARKGREY, TFT_BLACK);  // 3 on, 6 off.
      prevDay = intMyDay;
    }
  }
  // Decorations part 2
  sprite.setTextDatum(BR_DATUM);
  sprite.drawString(String(ceil(highRainForecast), 0),
                    graphLeftMargin - 2, 15, 2);
  sprite.drawString("0", graphLeftMargin - 2,
                    tft.height() - graphFloorMargin + 3, 2);

  sprite.drawFastVLine(graphLeftMargin, 0,
                       tft.height() - graphFloorMargin, TFT_YELLOW);
  sprite.drawFastHLine(graphLeftMargin, tft.height() - graphFloorMargin,
                       tft.width() - graphFloorMargin * 2, TFT_YELLOW);

  sprite.setTextDatum(MR_DATUM);

  switch (int(ceil(highRainForecast)))
  {
    case 0: case 1:  // Do nothing
      break;
    case 2:  // Draw 1 line at 1 and label it
      drawYAxisScaleLines(.5, 0, yRange);
      break;
    case 3:  // Draw 2 lines, one at 1, one at 2
      drawYAxisScaleLines(.66,  0, yRange);
      drawYAxisScaleLines(.33, 0, yRange);
      break;
    case 4:  // Draw 1 line at 2 and label it
      drawYAxisScaleLines(.75, 0, yRange);
      drawYAxisScaleLines(.5, 0, yRange);
      drawYAxisScaleLines(.25, 0, yRange);
      break;
    case 5:
      drawYAxisScaleLines(.8, 0, yRange);
      drawYAxisScaleLines(.6, 0, yRange);
      drawYAxisScaleLines(.4, 0, yRange);
      drawYAxisScaleLines(.2, 0, yRange);
      break;
    case 6:  // Draw 1 line at 3 and label it
      drawYAxisScaleLines(.66, 0, yRange);
      drawYAxisScaleLines(.33, 0, yRange);
      break;
    default:
      if (yRange % 2)
        drawYAxisScaleLines(.5, 0, yRange);
      else {
        drawYAxisScaleLines(.6666, 0, yRange);
        drawYAxisScaleLines(.3333, 0, yRange);
      }
      break;
  }
  ledcWrite(0, extraScreenBrightness);  // Make it bright.
  sprite.pushSprite(0, 0);
}
/***************************************************************************/
void graphTheTemp()
/***************************************************************************/
{
  // The display is 170 x 320.  We will allow some margin at the bottom so
  //  less than 170 will be utilized.
  // I multiply the highest rainfall amount by 100 and then divide into the
  //  number of pixels that will be used.  All in float.
  float thisX, thisY;
  float lastX, lastY;
  int prevDay = -1, intMyDay;
  String legend = "3-Hourly Temperatures (5 Days)";
  int halfLegend = sprite.textWidth(legend, 2) / 2;
  int yRange = ceil(highTempForecast) - floor(lowTempForecast);

#if defined DO_FORECAST_PRINTS
  Serial.printf("3-Hour high temp %.2f, low temp %.2f\r\n", highTempForecast,
                lowTempForecast);
  Serial.printf("The range of that for graphing is %i\r\n", int(yRange));
#endif
  float pixelsPerHundredthV = (tft.height() - graphFloorMargin) /
                              (yRange * 100.);
  // So, the minimum plot point line will be graphFloor and
  //  the top will be the top of the display.
  // The display is 320 wide.  Again, we leave a margin at the right.  There
  //  will be 40 (currently) plots of rainfall amount, each hour, for 5 days.
  float pixelsPerSampleH = (tft.width() - graphLeftMargin) / ThreeHourSamples;

  // Now, start to draw
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextColor(TFT_YELLOW, TFT_BLACK);
  // Decorations part 1
  sprite.setTextDatum(TC_DATUM);  // Temp legend centered in graph area.
  sprite.drawString(legend, (tft.width() + graphLeftMargin) / 2, 0, 2);

  // The two axes
  sprite.drawFastVLine(graphLeftMargin, 0,
                       tft.height() - graphFloorMargin, TFT_YELLOW);
  sprite.drawFastHLine(graphLeftMargin, tft.height() - graphFloorMargin,
                       tft.width() - graphFloorMargin * 2, TFT_YELLOW);

  lastX = graphLeftMargin;
  lastY = (highTempForecast - WxForecast[0].Temperature) * 100 *
          pixelsPerHundredthV;
  for (int i = 0; i < ThreeHourSamples - 1; i++) {
    thisX = ((pixelsPerSampleH * (i + 1)) + float(graphLeftMargin));
    // This time, plotting from the top.
    thisY = (highTempForecast - WxForecast[i + 1].Temperature) * 100. *
            pixelsPerHundredthV;
    sprite.drawLine(int(lastX), int(lastY),
                    int(thisX), int(thisY), tempGraphLineColor);

    // Remember starting position for next line segment.
    lastX = thisX, lastY = thisY;
    time_t tm = WxForecast[i].dt;
    struct tm *now_tm = localtime(&tm);
    intMyDay = now_tm->tm_wday;

    // Now, when the day changes, put in the new day name and a dashed line.
    sprite.setTextDatum(TL_DATUM);
    if (prevDay != intMyDay) {
      sprite.drawString(" " + SDay[intMyDay],
                        pixelsPerSampleH * i + graphLeftMargin + 12,
                        tft.height() - graphFloorMargin + 3, 2);
      drawDashLineV(pixelsPerSampleH * i + graphLeftMargin,
                    tft.height() - graphFloorMargin,
                    20, 3, 6, TFT_DARKGREY, TFT_BLACK);  // 3 on, 6 off.
      prevDay = intMyDay;
    }
  }
  // Decorations part 2
  sprite.setTextDatum(BR_DATUM);
  sprite.drawString(String(ceil(highTempForecast), 0),
                    graphLeftMargin - 2, 15, 2);
  sprite.drawString(String(floor(lowTempForecast), 0), graphLeftMargin - 2,
                    tft.height() - graphFloorMargin + 3, 2);

  sprite.drawFastVLine(graphLeftMargin, 0,
                       tft.height() - graphFloorMargin, TFT_YELLOW);
  sprite.drawFastHLine(graphLeftMargin, tft.height() - graphFloorMargin,
                       tft.width() - graphFloorMargin * 2, TFT_YELLOW);

  sprite.setTextDatum(MR_DATUM);

  switch (int(yRange))
  {
    case 0: case 1:  // Do nothing
      break;
    case 2:  // Draw 1 line at 1 and label it
      drawYAxisScaleLines(.5, lowTempForecast, yRange);
      break;
    case 3:  // Draw 2 lines, one at 1, one at 2
      drawYAxisScaleLines(.66,  lowTempForecast, yRange);
      drawYAxisScaleLines(.33, lowTempForecast, yRange);
      break;
    case 4:  // Draw 1 line at 2 and label it
      drawYAxisScaleLines(.75, lowTempForecast, yRange);
      drawYAxisScaleLines(.5, lowTempForecast, yRange);
      drawYAxisScaleLines(.25, lowTempForecast, yRange);
      break;
    case 5:
      drawYAxisScaleLines(.8, lowTempForecast, yRange);
      drawYAxisScaleLines(.6, lowTempForecast, yRange);
      drawYAxisScaleLines(.4, lowTempForecast, yRange);
      drawYAxisScaleLines(.2, lowTempForecast, yRange);
      break;
    case 6:  // Draw 1 line at 3 and label it
      drawYAxisScaleLines(.66, lowTempForecast, yRange);
      drawYAxisScaleLines(.33, lowTempForecast, yRange);
      break;
    default:
      if (yRange % 2)
        drawYAxisScaleLines(.5, lowTempForecast, yRange);
      else {
        drawYAxisScaleLines(.6666, lowTempForecast, yRange);
        drawYAxisScaleLines(.3333, lowTempForecast, yRange);
      }
      break;
  }
  ledcWrite(0, extraScreenBrightness);  // Make it bright.
  sprite.pushSprite(0, 0);
}
/***************************************************************************/
void drawYAxisScaleLines(float myPctHeight, int myLowEnd, int myRange)
/***************************************************************************/
{ // These are the 1 or more lines across the graph with a label on the left.
  sprite.drawFastHLine(graphLeftMargin,
                       (tft.height() - graphFloorMargin) * myPctHeight,
                       tft.width() - graphFloorMargin * 2, TFT_YELLOW);
  float a = float(1. - myPctHeight) * myRange;
  float c = a + myLowEnd;
  sprite.drawString(String(c, 1), graphLeftMargin - 2,
                    (tft.height() - graphFloorMargin) * myPctHeight, 2);
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
      sprite.drawLine(xValue, startY, xValue, startY + onPix, offColor);
      startY += offPix;
    }
  }
  if (signus < 0) {
    while (startY > endY) {
      sprite.drawLine(xValue, startY, xValue, startY - onPix, onColor);
      startY -= onPix;
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
  sprite.drawString("Day       ",              0,  0, 2);
  sprite.drawString("Temps     ",             28,  0, 2);
  sprite.drawString("WX Forecast                         ", 75,  0, 2);
  for (i = 0; i < 7; i++) {
    sprite.setTextColor(TFT_YELLOW, TFT_BLACK);
    workTime = daily_item_dt[i];
    now_tm = localtime(&workTime);

    // The short Day name
    sprite.drawString(SDay[now_tm->tm_wday], 0, yPos, 2);

    // The high and low temps expected for the day
    forecast = String(int(daily_item_temp_min[i])) + "-" +
               String(int(daily_item_temp_max[i]));
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
    txtW = sprite.drawString(Conditions[j].Condition, 76, yPos, 2);
    condX = daily_item_weather_0_id[i] / 100;
    if (condX == 5) sprite.drawString(" pop " +
                                        String(int(daily_item_pop[i] * 100)) +
                                        "%", 76 + txtW, yPos, 2);
    yPos += 22;
  }
  sprite.pushSprite(0, 0);
  ledcWrite(0, extraScreenBrightness);
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

  for (i = 0; i < 7; i++) {

    workTime = daily_item_dt[i]; now_tm = localtime(&workTime);

    // Surprised to find the following prints: Sat Dec 16 11:00:00 2023
    // Serial.println(now_tm);

    // The short Day name
    sprite.drawString(SDay[now_tm->tm_wday], 0, yPos, 2);

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
  ledcWrite(0, extraScreenBrightness);
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
