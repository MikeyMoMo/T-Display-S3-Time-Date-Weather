/***************************************************************************/
void graphTheTemp()
/***************************************************************************/
{
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
  int yRange = ceil(highTempForecast) - floor(lowTempForecast);

#if defined DO_TEMP_FORECAST_PRINTS
  Serial.printf("3-Hour high temp %.2f, low temp %.2f\r\n", highTempForecast,
                lowTempForecast);
  Serial.printf("The range of that for graphing is %i\r\n", int(yRange));
#endif
  pixelsPerHundredthV = (tft.height() - graphFloorMargin) /
                        (yRange * 100.);
  float pixelsPerSampleH = (tft.width() - graphLeftMargin) / ThreeHourSamples;

  for (int i = 0; i < ThreeHourSamples; i++) {
    expandedData[i * 6] = WxForecast[i].Temperature;
#if defined DO_TEMP_FORECAST_PRINTS
    Serial.printf("%i: %.2f\r\n", i + 1, WxForecast[i].Temperature);
#endif
  }
  // Now, fill in the gaps with linear numbers between the endpoint.
  calcIntermediates(expandedData, false);
  //--------------
  // Now, start to draw
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextColor(TFT_YELLOW, TFT_BLACK);
  // Decorations part 1
  sprite.setTextDatum(TC_DATUM);  // Temp legend centered in graph area.
  sprite.drawString(legend, (tft.width() + graphLeftMargin) / 2, 0, 2);

  //  lastX = graphLeftMargin;

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
  lastY = (highTempForecast - expandedData[0]) * 100 * pixelsPerHundredthV;
  for (int i = 0; i < (ThreeHourSamples * intermediatesCt) - 5; i++) {
    thisX = (pixelsPerSampleH / intermediatesCt) * (i + 1) + graphLeftMargin;
    thisY = highTempForecast - expandedData[i];
    thisY = thisY * 100. * pixelsPerHundredthV;
    sprite.drawLine(lastX, lastY, thisX, thisY, tempGraphLineColor);
#if defined FILL_GRAPH
    sprite.drawLine(thisX, tft.height() - graphFloorMargin, thisX, thisY,
                    tempGraphLineColor);
    if (thisX - lastX > 1) {
      // The reason I do this twice is that it was leaving a pixel or two out of
      //  being colored when either or these was used, alone.  And they were different
      //  pixels.  So, by using them both, I get all pixels filled in.
      sprite.drawLine(thisX - 1, tft.height() - graphFloorMargin,
                      thisX - 1, lastY, tempGraphLineColor);
      sprite.drawLine(thisX - 1, tft.height() - graphFloorMargin,
                      thisX - 1, thisY, tempGraphLineColor);
    }
#endif
    // Remember starting position for next line segment.
    lastX = thisX, lastY = thisY;
  }
  // Decorations part 2
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
