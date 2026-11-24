/***************************************************************************/
void graphTheTemp()
/***************************************************************************/
//  SCREEN (seconds 13-20): 5-day temperature line graph.
//  Data: WxForecast[].Temperature (filled by fetch_and_decode_Forecast).
//  Y axis runs from floor(lowTempForecast) at the bottom to
//  ceil(highTempForecast) at the top; X axis = 40 three-hour samples.
//
//  >>> REVIEW (biggest maintainability issue in the project):
//  >>>   graphTheTemp, graphTheRain and graphTheSnow are ~170 lines each
//  >>>   and ~90% identical.  Every fix (like the ones noted here) has to
//  >>>   be made three times.  One function would do all three:
//  >>>     void drawGraph(float values[], float low, float high,
//  >>>                    const String& legend, uint16_t lineColor,
//  >>>                    uint16_t fillTopColor, const char* emptyMessage);
//  >>>   and graphTheRain() becomes a 5-line wrapper that copies Rainfall
//  >>>   into an array and calls it.
/***************************************************************************/
{
 if (ThreeHourSamples < 2) {
  Serial.println("No 3-hour sample data"); 
  return; 
  }
 
  //  unsigned long startMillis = millis();
  // The display is 170 x 320.  We will allow some margin at the bottom so
  //  less than 170 will be utilized.  Let's guess at 150 for now.
  // I multiply the highest rainfall amount by 100 and then divide into the
  //  number of pixels that will be used.  All in float.
  float thisX, thisY;
  float lastX, lastY;
  int prevDay = -1, intMyDay, intBackup, dayX;
  String legend = "3-Hourly Temperatures (5 Days)";
  int halfLegend = sprite.textWidth(legend, 2) / 2;
  // >>> NOTE: halfLegend is never used.
  int yRange = ceil(highTempForecast) - floor(lowTempForecast);
  // >>> BUG (rare): if the temperature never changes by a whole degree over
  // >>>   5 days (e.g. all exactly 27.0), yRange is 0 and the division below
  // >>>   gives infinity -> nothing sensible is drawn.  if (yRange < 1) yRange = 1;

#if defined DO_TEMP_FORECAST_PRINTS
  Serial.printf("3-Hour high temp %.2f, low temp %.2f\r\n", highTempForecast,
                lowTempForecast);
  Serial.printf("The range of that for graphing is %i\r\n", int(yRange));
#endif
  // Vertical scale, in pixels per 0.01 degree.
  // (The "x100" here and below cancels out - pixels per degree would do.)
  pixelsPerHundredthV = (tft.height() - graphFloorMargin) /
                        (yRange * 100.);
  // Horizontal scale: graph width / number of 3-hour samples.
  // >>> REVIEW (BUG - can crash): if the forecast fetch failed at boot,
  // >>>   ThreeHourSamples is still 0 -> integer divide by zero -> the ESP32
  // >>>   panics and reboots, and keeps doing so every time this screen comes
  // >>>   up until OWM answers.  Guard at the top of this function:
  // >>>     if (ThreeHourSamples < 2) { /* draw "No forecast data"; */ return; }
  // >>> NOTE: both sides are int, so this is integer division: 288/40 = 7,
  // >>>   not 7.2.  The graph is 8 px narrower than intended.  Write
  // >>>   (tft.width() - graphLeftMargin) / float(ThreeHourSamples).
  float pixelsPerSampleH = (tft.width() - graphLeftMargin) / ThreeHourSamples;

  for (int i = 0; i < ThreeHourSamples; i++) {
    expandedData[i * 6] = WxForecast[i].Temperature;
#if defined DO_TEMP_FORECAST_PRINTS
    Serial.printf("%i: %.2f\r\n", i + 1, WxForecast[i].Temperature);
#endif
  }
  // Now, fill in the gaps with linear numbers between the endpoint.
  // Fill the 5 in-between points of every 3-hour step (straight lines).
  calcIntermediates(expandedData, false);
  //--------------
  // Now, start to draw
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextColor(TFT_YELLOW, TFT_BLACK);
  // Decorations part 1
  sprite.setTextDatum(TC_DATUM);  // Temp legend centered in graph area.
  sprite.drawString(legend, (tft.width() + graphLeftMargin) / 2, 0, 2);

  //  lastX = graphLeftMargin;

  //--- Pass 1: day names along the X axis + dotted day separators ----------
  // A separator is drawn at the first 3-hour sample that falls on a new
  // weekday.
  // >>> NOTE: that sample is usually 01:00-03:00, not midnight, so the
  // >>>   separator is up to 3 h late.  Exact midnight x position =
  // >>>   graphLeftMargin + (midnightEpoch - WxForecast[0].dt) / 10800.0 * pixelsPerSampleH
  for (int i = 0; i < ThreeHourSamples - 1; i++) {
    time_t tm = WxForecast[i].dt;
    struct tm *now_tm = localtime(&tm);
    intMyDay = now_tm->tm_wday;

    // Now, when the day changes, put in the new day name.
    sprite.setTextDatum(TL_DATUM);
    if (prevDay != intMyDay) {
      intBackup = now_tm->tm_hour / 3 * pixelsPerSampleH;
      dayX = pixelsPerSampleH * i + graphLeftMargin - intBackup + 18;
      if (dayX >= graphLeftMargin)  // Only show the day if fully visible
        sprite.drawString(SDays[intMyDay], dayX,
                          tft.height() - graphFloorMargin + 3, 2);
      drawDashLineV(pixelsPerSampleH * i + graphLeftMargin,
                    tft.height() - graphFloorMargin,
                    20, 3, 6, TFT_DARKGREY, TFT_BLACK);  // 3 on, 6 off.
      prevDay = intMyDay;
    }
  }
  lastX = graphLeftMargin;
  // Y is measured DOWN from the top of the screen, so the hottest value is
  // at y = 0 (the very top - it can touch the legend text).
  // >>> REVIEW (BUG - graph vs labels): the line is positioned from the raw
  // >>>   highTempForecast, but the labels assume the scale runs from
  // >>>   floor(low) to ceil(high).  Example: high 31.2, low 24.6 -> labels
  // >>>   say 24..32, but the line is drawn as if the scale were 23.2..31.2,
  // >>>   so every point (and every scale-line label) is ~0.8 degree off.
  // >>>   Fix: use ceil(highTempForecast) in place of highTempForecast in
  // >>>   this line and in the "thisY =" line below.
  lastY = (ceil(highTempForecast) - expandedData[0]) * 100 * pixelsPerHundredthV;
  //--- Pass 2: the graph line, plus (optionally) the fill under it ---------
  // Each of the ~235 small segments goes from (lastX,lastY) to (thisX,thisY).
  // The fill is done with vertical lines from the X axis up to the graph.
  // >>> NOTE: point i is drawn at x for i+1, and the first segment goes from
  // >>>   point 0 to point 0 again - the whole curve is shifted right by one
  // >>>   sub-step (~1 px).  thisX = step * i + graphLeftMargin would fix it.
  for (int i = 0; i < (ThreeHourSamples * intermediatesCt) - 5; i++) {
    thisX = (pixelsPerSampleH / intermediatesCt) * (i + 1) + graphLeftMargin;
    thisY = ceil(highTempForecast) - expandedData[i];
    thisY = thisY * 100. * pixelsPerHundredthV;
    sprite.drawLine(lastX, lastY, thisX, thisY, tempGraphLineColor);
#if defined FILL_GRAPH
#if defined DO_GRADIENT
    drawGradientLine(&sprite,
                     thisX, tft.height() - graphFloorMargin, thisX, thisY,
                     0, TFT_PINK, tempGraphLineColor);

    if (thisX - lastX > 1) {
      // The reason I do this twice is that it was leaving a pixel or two out of
      //  being colored when either or these was used, alone.  And they were different
      //  pixels.  So, by using them both, I get all pixels filled in.
      drawGradientLine(&sprite,
                       thisX - 1, tft.height() - graphFloorMargin, thisX - 1, lastY,
                       0, TFT_PINK, tempGraphLineColor);
      drawGradientLine(&sprite,
                       thisX - 1, tft.height() - graphFloorMargin, thisX - 1, thisY,
                       0, TFT_PINK, tempGraphLineColor);
    }
#else
    sprite.drawLine(thisX, tft.height() - graphFloorMargin, thisX, thisY,
                    tempGraphLineColor);
    if (thisX - lastX > 1) {
      //      The reason I do this twice is that it was leaving a pixel or two out of
      //        being colored when either or these was used, alone.  And they were different
      //        pixels.  So, by using them both, I get all pixels filled in.
      sprite.drawLine(thisX - 1, tft.height() - graphFloorMargin,
                      thisX - 1, lastY, tempGraphLineColor);
      sprite.drawLine(thisX - 1, tft.height() - graphFloorMargin,
                      thisX - 1, thisY, tempGraphLineColor);
    }
#endif
#endif
    // Remember starting position for next line segment.
    lastX = thisX; lastY = thisY;
  }

  // Decorations part 2
  // Top and bottom Y-axis labels, the two axes, then 0-4 scale lines.
  sprite.setTextDatum(BR_DATUM);
  sprite.drawString(String(ceil(highTempForecast), 0),
                    graphLeftMargin - 2, 15, 2);
  sprite.drawString(String(floor(lowTempForecast), 0), graphLeftMargin - 2,
                    tft.height() - graphFloorMargin + 3, 2);
  // Two axes
  sprite.drawFastVLine(graphLeftMargin, 0,
                       tft.height() - graphFloorMargin, TFT_YELLOW);
  sprite.drawFastHLine(graphLeftMargin, tft.height() - graphFloorMargin,
                       (tft.width() - graphFloorMargin * 2) - 5, TFT_YELLOW);

  sprite.setTextDatum(MR_DATUM);

  //--- Extra horizontal scale lines depending on the range ----------------
  // >>> NOTE: several case comments don't match the code (case 4 says
  // >>>   "1 line at 2" but draws 3 lines; case 6 says "1 line at 3" but
  // >>>   draws 2).  And for "default" with an odd range (7, 9, 11...) the one
  // >>>   line at 50% gets a label like 3.5.
  // >>> TIP: one loop replaces the whole switch, and is shared by all graphs:
  // >>>   int step = niceStep(range);         // 1, 2, 5, 10, 20 ...
  // >>>   for (v = low + step; v < low + range; v += step)
  // >>>     drawYAxisScaleLines(1.0 - (v - low) / range, low, range);
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
  if (!screenOn) {
    tft.writecommand(ST7789_DISPON);  // Turn on display hardware
    screenOn = true;
  }
  ledcWrite(TFT_BL, screensExtraBright);  // Turn the display on bigly for init messages.
  sprite.pushSprite(0, 0);
  //  Serial.printf("Time for Temp graph was %lu ms.\r\n", millis() - startMillis);
}
