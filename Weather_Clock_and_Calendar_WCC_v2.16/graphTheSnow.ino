/***************************************************************************/
void graphTheSnow()
/***************************************************************************/
//  SCREEN (seconds 29-36, ONLY when totSnow > 0): 5-day snow amount graph.
//  Data: WxForecast[].Snowfall - mm per 3 hours, or inches for imperial.
//  Y axis runs from 0 to ceil(highSnowForecast).
//  >>> See graphTheTemp.ino - the three graph functions are near copies
//  >>>   and should be merged into one.
/***************************************************************************/
{
  //  unsigned long startMillis = millis();
  // The display is 170 x 320.  We will allow some margin at the bottom so
  //  less than 170 will be utilized.  Let's guess at 150 for now.
  // I multiply the highest Snowfall amount by 100 and then divide into the
  //  number of pixels that will be used.  All in float.
  float thisX, thisY;
  float lastX, lastY;
  float tempSnow = 0.;
  String legend;
  int prevDay = -1, intMyDay, intBackup, dayX;

 if (ThreeHourSamples < 2) {
  Serial.println("No 3-hour sample data"); 
  return; 
  }
 
 if (multiCity[whichCity].units == "imperial")
    legend = "3-Hourly Snowfall amount (in) (5 Days)";
  else
    legend = "3-Hourly Snowfall amount (mm) (5 Days)";
  int halfLegend = sprite.textWidth(legend, 2) / 2;
  // >>> NOTE: halfLegend is never used.

  // Get ready for smoothing. First expand the array by 6x.
  for (int i = 0; i < ThreeHourSamples; i++) {
    expandedData[i * 6] = WxForecast[i].Snowfall;
    tempSnow += WxForecast[i].Snowfall;
    // >>> NOTE: tempSnow is the same number as the global totSnow worked out in
    // >>>   Forecast_Decode.ino - this local total can be dropped.
  }
  //  if (tempSnow > 0.) Serial.printf("Snowfall total %.2f, 3H max %.2f mm.\r\n",
  //                                     tempSnow, highSnowForecast);
  int yRange = ceil(highSnowForecast);
  // >>> REVIEW (imperial cities): snow in inches is usually 0.0x per 3 h,
  // >>>   but ceil() makes the scale at least 1 inch.  A 0.05" shower is
  // >>>   then a 7-pixel bump.  Pick a "nice" top instead, e.g. the first of
  // >>>   0.1, 0.25, 0.5, 1, 2, 5, 10 ... that is >= highSnowForecast.
  // >>>   (yRange and the scale-line code would then need to be float.)
  if (yRange == 0)  // Avoid that pesky, upcoming division by 0!
    pixelsPerHundredthV = 0;  // Since no graph, no biggy. Shows message.
  else
    pixelsPerHundredthV = (tft.height() - graphFloorMargin) /
                          (yRange * 100.);
  //  Serial.printf("There'll be %f pixels per 1/100 of a inch, vertical."
  //                "\r\n", pixelsPerHundredthV);
  // So, the minimum plot point line will be graphFloor and
  //  the top will be the top of the display.
  // The display is 320 wide.  Again, we leave a margin at the right.  There
  //  will be 40 (currently) plots of Snowfall amount, each hour, for 5 days.
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
  //  Serial.printf("%.2f pixels per entry (%i) across.\r\n",
  //                pixelsPerSampleH, ThreeHourSamples);

  // Now, fill in the gaps with linear interpelated numbers
  //  between the endpoint.
  // Fill the 5 in-between points of every 3-hour step (straight lines).
  calcIntermediates(expandedData, false);
  // Now, do the smoothing (the hard part).
  //  smoothData(expandedData);
  //--------------
  // Now, start to draw
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextColor(TFT_YELLOW, TFT_BLACK);
  // Decorations part 1
  sprite.setTextDatum(TC_DATUM);  // Snow legend
  sprite.drawString(legend,
                    (tft.width() + graphLeftMargin) / 2, 0, 2); // 180

  // >>> NOTE: the two axes are drawn here AND again in "Decorations part 2".
  // >>>   The second pair (after the graph) is the one that matters; this
  // >>>   pair can go.
  sprite.drawFastVLine(graphLeftMargin, 0,
                       tft.height() - graphFloorMargin, TFT_YELLOW);
  sprite.drawFastHLine(graphLeftMargin, tft.height() - graphFloorMargin,
                       (tft.width() - graphFloorMargin * 2) - 5, TFT_YELLOW);

  // First one puts in days and dashes, no graph line.
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
  // Second pass puts in the graph line.
  lastX = graphLeftMargin;
  lastY = (tft.height() - graphFloorMargin -
           (expandedData[0]) * 100 * pixelsPerHundredthV);
  //--- Pass 2: the graph line, plus (optionally) the fill under it ---------
  // Each of the ~235 small segments goes from (lastX,lastY) to (thisX,thisY).
  // The fill is done with vertical lines from the X axis up to the graph.
  // >>> NOTE: point i is drawn at x for i+1, and the first segment goes from
  // >>>   point 0 to point 0 again - the whole curve is shifted right by one
  // >>>   sub-step (~1 px).  thisX = step * i + graphLeftMargin would fix it.
  for (int i = 0; i < (ThreeHourSamples * intermediatesCt) - 5; i++) {
    thisX = (pixelsPerSampleH / intermediatesCt) * (i + 1) + graphLeftMargin;
    // Get Snow value I am to plot next.
    thisY = tft.height() - graphFloorMargin -
            (expandedData[i] * 100 * pixelsPerHundredthV);
    sprite.drawLine(lastX, lastY, thisX, thisY, snowGraphLineColor);
#if defined FILL_GRAPH
#if defined DO_GRADIENT
    drawGradientLine(&sprite, thisX, tft.height() - graphFloorMargin, thisX, thisY,
                     0, RGB565(25, 25, 25), snowGraphLineColor);
    if (thisX - lastX > 1) {
      drawGradientLine(&sprite, thisX - 1, tft.height() - graphFloorMargin,
                       thisX - 1, lastY,
                       0, RGB565(25, 25, 25), snowGraphLineColor);
      drawGradientLine(&sprite, thisX - 1, tft.height() - graphFloorMargin,
                       thisX - 1, thisY,
                       0, RGB565(25, 25, 25), snowGraphLineColor);
    }
#else
    sprite.drawLine(thisX, tft.height() - graphFloorMargin, thisX, thisY,
                    snowGraphLineColor);
    if (thisX - lastX > 1) {
      sprite.drawLine(thisX - 1, tft.height() - graphFloorMargin,
                      thisX - 1, lastY, snowGraphLineColor);
      sprite.drawLine(thisX - 1, tft.height() - graphFloorMargin,
                      thisX - 1, thisY, snowGraphLineColor);
    }
#endif
#endif
    // Remember starting position for next line segment.
    lastX = thisX, lastY = thisY;
  }

  // Decorations part 2
  // Top and bottom Y-axis labels, the two axes, then 0-4 scale lines.
  sprite.setTextDatum(BR_DATUM);
  sprite.drawString(String(ceil(highSnowForecast), 0),
                    graphLeftMargin - 2, 15, 2);
  sprite.drawString("0", graphLeftMargin - 2,
                    tft.height() - graphFloorMargin + 3, 2);

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
  switch (int(ceil(highSnowForecast)))
  {
    case 0: case 1:  // Do nothing
      break;
    case 2:  // Draw 1 line at 1 and label it
      drawYAxisScaleLines(.5, 0, yRange);
      break;
    case 3:  // Draw 2 lines, one at 1, one at 2
      drawYAxisScaleLines(.6666,  0, yRange);
      drawYAxisScaleLines(.3333, 0, yRange);
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
      drawYAxisScaleLines(.6666, 0, yRange);
      drawYAxisScaleLines(.3333, 0, yRange);
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
  // This screen is only shown when totSnow > 0, so this message only appears
  // for a trace (0 - 0.05) of snow.
  if (tempSnow < .05) {
    sprite.setTextDatum(MC_DATUM);
    sprite.drawString("No significant Snow predicted", xCenter, yCenter - 25, 2);
    sprite.drawString("for the next 5 days", xCenter, yCenter + 20, 2);
  }
  if (!screenOn) {
    tft.writecommand(ST7789_DISPON);  // Turn on display hardware
    screenOn = true;
    setBrightness(true);
  }
  ledcWrite(TFT_BL, screensExtraBright);  // Turn the display on bigly for init messages.
  sprite.pushSprite(0, 0);
  //  Serial.printf("Time for Snow graph was %lu ms.\r\n", millis() - startMillis);
}
