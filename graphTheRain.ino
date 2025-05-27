/***************************************************************************/
void graphTheRain()
/***************************************************************************/
{
  //  unsigned long startMillis = millis();
  // The display is 170 x 320.  We will allow some margin at the bottom so
  //  less than 170 will be utilized.  Let's guess at 150 for now.
  // I multiply the highest rainfall amount by 100 and then divide into the
  //  number of pixels that will be used.  All in float.
  float thisX, thisY;
  float lastX, lastY;
  String legend;

  int prevDay = -1, intMyDay, intBackup, dayX;
  if (multiCity[whichCity].units == "imperial")
    legend = "3-Hourly Rainfall amount (in) (5 Days)";
  else
    legend = "3-Hourly Rainfall amount (mm) (5 Days)";
  int halfLegend = sprite.textWidth(legend, 2) / 2;

  // Get ready for smoothing. First expand the array by 6x.
  for (int i = 0; i < ThreeHourSamples; i++) {
    expandedData[i * 6] = WxForecast[i].Rainfall;
  }
  int yRange = ceil(highRainForecast);
  if (yRange == 0)  // Avoid that pesky, upcoming division by 0!
    pixelsPerHundredthV = 0;  // Since no graph, no biggy. Shows message.
  else
    pixelsPerHundredthV = (tft.height() - graphFloorMargin) /
                          (yRange * 100.);
  float pixelsPerSampleH = (tft.width() - graphLeftMargin) / ThreeHourSamples;

  // Now, fill in the gaps with linear interpelated numbers
  //  between the endpoint.
  calcIntermediates(expandedData, false);
  //--------------
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
                       (tft.width() - graphFloorMargin * 2) - 5, TFT_YELLOW);

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
  for (int i = 0; i < (ThreeHourSamples * intermediatesCt) - 5; i++) {
    thisX = (pixelsPerSampleH / intermediatesCt) * (i + 1) + graphLeftMargin;
    // Get rain value I am to plot next.
    thisY = tft.height() - graphFloorMargin -
            (expandedData[i] * 100 * pixelsPerHundredthV);
    sprite.drawLine(lastX, lastY, thisX, thisY, rainGraphLineColor);
    //    Serial.printf("lastX %.2f, lastY %.2f, thisX %.2f, thisY %.2f\r\n",
    //                  lastX, lastY, thisX, thisY);
#if defined FILL_GRAPH
    sprite.drawLine(thisX, tft.height() - graphFloorMargin, thisX, thisY,
                    rainGraphLineColor);
    if (thisX - lastX > 1) {
      sprite.drawLine(thisX - 1, tft.height() - graphFloorMargin,
                      thisX - 1, lastY, rainGraphLineColor);
      sprite.drawLine(thisX - 1, tft.height() - graphFloorMargin,
                      thisX - 1, thisY, rainGraphLineColor);
    }
#endif
    // Remember starting position for next line segment.
    lastX = thisX, lastY = thisY;
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
                       (tft.width() - graphFloorMargin * 2) - 5, TFT_YELLOW);

  sprite.setTextDatum(MR_DATUM);

  switch (int(ceil(highRainForecast)))
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
  if (totRain < .05) {
    sprite.setTextDatum(MC_DATUM);
    sprite.drawString("No significant Rain predicted", xCenter, yCenter - 25, 2);
    sprite.drawString("for the next 5 days", xCenter, yCenter + 20, 2);
  }
  if (!screenOn) {
    tft.writecommand(ST7789_DISPON);  // Turn on display hardware
    screenOn = true;
  }
  ledcWrite(TFT_BL, screensExtraBright);  // Turn the display on bigly for init messages.
  sprite.pushSprite(0, 0);
  //  Serial.printf("Time for Rain graph was %lu ms.\r\n", millis() - startMillis);
}
