/***************************************************************************/
void showMemoryStats()
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
{
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo, 60000)) return;  // Fix this!

  strftime(fullTimeDate, 100, "%c", &timeinfo);
  strftime(myMonth, 10, "%B", &timeinfo);  intMonth = timeinfo.tm_mon;
  strftime(myYear,   5, "%Y", &timeinfo);  intYear  = timeinfo.tm_year + 1900;
  strftime(timeHour, 3, "%H", &timeinfo);  intHour  = timeinfo.tm_hour;
  strftime(timeMin,  3, "%M", &timeinfo);  intMin   = timeinfo.tm_min;
  strftime(timeSec,  3, "%S", &timeinfo);  intSec   = timeinfo.tm_sec;
  strftime(timeWeekDay, 10, "%A", &timeinfo);
  strftime(timeWkDay, 4, "%a", &timeinfo); intDOW   = timeinfo.tm_wday;
  strftime(timeDOM, 4, "%d", &timeinfo);   intDOM   = timeinfo.tm_mday;
  clockString = SDays[intDOW]  + ", " + SMonths[intMonth] + " " +
                String(intDOM) + ", " + String(intYear);

  dayInWeek   = timeinfo.tm_wday;  // Sunday = 0
  dayInMonth  = timeinfo.tm_mday;  // Day of the month (rationally starting at 1)
  daysInMonth = mm[timeinfo.tm_mon];  // How many days in this month.
  // If it is February and this is a Leap Year, add one day, as expected.
  if (timeinfo.tm_mon == 1 && isLeapYear(timeinfo.tm_year + 1900))
    daysInMonth++;  // Bump it up a notch for leap year February.

  // This is my little hack to quickly find the day of the 1st or the month.
  // You are welcome.  Mike Morrow.
  firstDay = dayInWeek + 1 - (dayInMonth % 7);  // Sunday = 0 for all, here.
  // This uses Sunday as 0 as TimeLib does.  Others do not.
  // If the 19th is a Friday, then that's 6 (Friday is the 6th day) minus
  //  5 (19 mod 7) and that gives a 1 and that's Monday. (Sunday is 0)
  if (firstDay < 0) firstDay += 7;  // Looks like Sunday is 0 here, too.

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
String ConvertToLocalTime(time_t epoch)
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
  sprite.drawString("Day       ",              0,  0, 2);
  sprite.drawString("Temps     ",             38,  0, 2);
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
    //    if (Units == "I") {
    //    if (multiCity[whichCity].units == "imperial") {
    //      sprintf(cMinTmp, "%3i", int((daily_item_temp_min[i] * 9. / 5.) + 32.));
    //      sprintf(cMaxTmp, "%3i", int((daily_item_temp_max[i] * 9. / 5.) + 32.));
    //    } else {
    sprintf(cMinTmp, "%3i", int(daily_item_temp_min[i]));
    sprintf(cMaxTmp, "%3i", int(daily_item_temp_max[i]));
    //    }
    forecast = String(cMinTmp) + "`/" + String(cMaxTmp) + "`";
    sprite.drawString(forecast, 28, yPos, 2);

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
    if (condX == 5)  // If one of the rain entries, add pop
      sprite.drawString(" pop " + String(int(daily_item_pop[i] * 100)) +
                        "%", textCol + txtW, yPos, 2);
    yPos += 22;
  }
  sprite.pushSprite(0, 0);
  if (!screenOn) {
    tft.writecommand(ST7789_DISPON);  // Turn on display hardware
    screenOn = true;
  }
  ledcWrite(TFT_BL, screensExtraBright);       // Turn the display on bigly for init messages.
#if defined DO_FORECAST_PRINTS
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
  }
  ledcWrite(TFT_BL, screensExtraBright);  // Turn the display on bigly for init messages.
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
{
  getMyTime(); Serial.println(fullTimeDate);
}
/***************************************************************************/
void drawGradientLine(TFT_eSprite *targetSprite,
                      int x0, int y0, int x1, int y1,
                      uint16_t colorStart, uint16_t colorEnd)
/***************************************************************************/
{
  int steps = abs(x1 - x0) > abs(y1 - y0) ? abs(x1 - x0) : abs(y1 - y0);
  int x = x0, y = y0;
  for (int i = 0; i <= steps; i++) {
    pctBlend = (float)i / steps;
    blendedColor = alphaBlend((uint8_t)(pctBlend * 255), colorEnd, colorStart);
    targetSprite->drawPixel(x, y, blendedColor);
    if ((x0 - x1) == 0) y1 > y0 ? y++ : y--;
    if ((y0 - y1) == 0) x1 > x0 ? x++ : x--;
  }
}
/***************************************************************************/
uint16_t alphaBlend(uint8_t alpha, uint16_t fgc, uint16_t bgc)
/***************************************************************************/
{
  uint32_t rxb = bgc & 0xF81F;
  rxb += ((fgc & 0xF81F) - rxb) * (alpha >> 2) >> 6;
  uint32_t xgx = bgc & 0x07E0;
  xgx += ((fgc & 0x07E0) - xgx) * alpha >> 8;
  return (rxb & 0xF81F) | (xgx & 0x07E0);
}
