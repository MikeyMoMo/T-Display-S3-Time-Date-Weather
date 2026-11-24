/*===========================================================================
  showCurrentConditions.ino  -  THE "HOME" SCREEN
  ===========================================================================
  Shown for most of each minute (seconds 37/45 .. 04).  Layout, 320 x 170:

    +----------------+----------------------------------+
    | Temp   27`C    |  HH:MM ss        (Orbitron font) |
    |----------------|  November 23, 2026               |
    | Hum    78%     |----------------------------------|
    |----------------|  SU MO TU WE TH FR SA            |
    | Wind   12 k/h  |   1  2  3  4  5  6  7            |
    | From   NNE     |   8  9 ...                       |
    |----------------|            (today highlighted)   |
    | Clouds    40%  |                                  |
    | Updated at: HH:MM                                 |
    +---------------------------------------------------+
  A thin colour-bar JPEG (colorbar.h) is drawn at x = 138 as a divider.
  ===========================================================================*/
/***************************************************************************/
void showCurrentCondx()
/***************************************************************************/
//  Redrawn every second (the seconds change), into the sprite, then pushed.
//  Uses: firstDay, daysInMonth, dayInMonth (from getMyTime) for the
//  calendar, and current_* from OneCall for the left column.
//  >>> NOTE: unlike the other screens this one does NOT call
//  >>>   ledcWrite(TFT_BL, ...), so it inherits whatever the previous
//  >>>   screen set (normally 200).
/***************************************************************************/
{
  int strLength;

  sprite.fillSprite(TFT_BLACK);

  //--- Calendar grid geometry: first column x, header row y, row height,
  //    column width.  (Globals, but only used here - could be locals.)
  cax = 167;  // was 142
  cay = 70; cah = 15; caw = 24;
  sprite.setTextDatum(MC_DATUM);
  sprite.setTextColor(TFT_WHITE, TFT_BLACK);
  //  drawArrayJpeg(Colorbar10, sizeof(Colorbar10), 132, 0);
  //  drawArrayJpeg(Colorbar5, sizeof(Colorbar5), 135, 0);
  // Vertical colour strip between the left column and the clock/calendar.
  drawArrayJpeg(Colorbar2, sizeof(Colorbar2), 138, 0);

  //--- Calendar: weekday headings, then up to 6 rows x 7 columns ----------
  for (int j = 0; j < 7; j++)
    sprite.drawString(SSDays[j], cax + (j * caw), cay, 2);

  // Walk all 42 cells.  Nothing is printed until cell number == firstDay
  // (weekday of the 1st), then 1, 2, 3 ... up to daysInMonth.
  // >>> See the firstDay bug in getMyTime(): when it is 7 the first row
  // >>>   stays empty and the month starts on row 2.
  int DOM = 1;
  int DOW = 0;
  bool started = false;

  for (int i = 0; i < 6; i++) {  // Rows
    for (int j = 0; j < 7; j++) {  // Columns
      if (DOW == firstDay)
        started = true;
      if (started && DOM <= daysInMonth) {
        if (DOM == dayInMonth)
          sprite.setTextColor(TFT_BLACK, TFT_LIGHTGREY);  // Was TFT_WHITE
        else
          sprite.setTextColor(TFT_ORANGE, TFT_BLACK);
        sprite.drawString(String(DOM), cax + (j * caw),
                          cay + cah + (cah * i), 2);
        DOM++;
      }
      DOW++;
    }
  }
  sprite.setTextDatum(TL_DATUM);
  sprite.setTextColor(TFT_WHITE, TFT_BLACK);
  //--- Clock: HH:MM big, seconds smaller, then the date -----------------------
  // Orbitron_Light_32/24 come with TFT_eSPI (needs LOAD_GFXFF / smooth
  // font options in the User_Setup).
  sprite.setFreeFont(&Orbitron_Light_32);
  strLength = sprite.drawString(String(timeHour) + ":" + String(timeMin),
                                150, -6);
  //  strLength2 = sprite.drawString(String(timeMin), 150 + strLength1, -6);

  sprite.setFreeFont(&Orbitron_Light_24);
  sprite.setTextColor(0xD399, TFT_BLACK);
  //  sprite.drawString(String(timeSec), 150 + strLength1 + strLength2 + 6, -4);
  sprite.drawString(String(timeSec), 150 + strLength + 6, -4);

  sprite.setTextColor(0x35F9, TFT_BLACK);
  sprite.setFreeFont(&FreeSans9pt7b);
  sprite.drawString(String(myMonth) + " " + String(dayInMonth) + ", " +
                    String(myYear), 157, 37);  // was 132

  sprite.setTextColor(TFT_DARKGREY, TFT_BLACK);

  // Grey rule between the date and the calendar.
  sprite.drawLine(cax - 10, cay - 10, cax + 177, cay - 10, myGray);  // was 152

  sprite.setTextColor(0x35F9, TFT_BLACK);
  sprite.setFreeFont(&FreeSans9pt7b);
  //--- Left column labels ----------------------------------------------------
  int yPos = 10;
  sprite.drawString("Temp", 0, yPos);
  sprite.drawString("Hum",  0, yPos + 30);
  sprite.drawString("Wind", 0, yPos + 60);
  sprite.drawString("From", 0, yPos + 88);

  // Clouds, Clear, etc.
  //  Serial.print("Repeat: current_weather_0_main: ");
  //  Serial.println(current_weather_0_main);
  // OWM's one-word summary: "Clouds", "Rain", "Clear" ...  For "Clouds" the
  // cloud cover % is added at the right.
  sprite.drawString(current_weather_0_main, 0, yPos + 120);
  sprite.setTextColor(TFT_DARKGREY, TFT_BLACK);
  // This is so odd.  You have to negate to get a go signal on a match.
  //  if (!strcmp(current_weather_0_main, "Clouds")) {
  if (current_weather_0_main == "Clouds") {
    sprite.setTextDatum(TR_DATUM);
    sprite.drawString(String(current_clouds) + "%", 115, yPos + 120);  // was 110
    sprite.setTextDatum(TL_DATUM);
  }
  sprite.setTextColor(TFT_DARKGREY, TFT_BLACK);
  // Time of the last successful forecast fetch (in the city's local time).
  // >>> TIP: if that is more than ~1 hour ago, draw it in red - the only
  // >>>   visible sign that WiFi or OWM has stopped working.
  sprintf(workChar, "Updated at: %02i:%02i", dataFetchHour, dataFetchMin);
  sprite.drawString(workChar, 0, 150, 2);
  sprite.setFreeFont(&FreeSans12pt7b);
  yPos = 31;
  // White separator lines in the left column.
  sprite.drawFastHLine(0, yPos, 114, TFT_WHITE);
  sprite.drawFastHLine(0, yPos + 29, 114, TFT_WHITE);
  sprite.drawFastHLine(0, yPos + 89, 114, TFT_WHITE);

  yPos = 5;
  // Temperature
  //--- Left column values --------------------------------------------------
  // ` (back-tick) is drawn as a degree sign by these fonts.
  // >>> NOTE: int(x + .5) rounds -2.7 to -2.  lroundf() is correct for both
  // >>>   signs (matters for your US cities in winter).
  if (multiCity[whichCity].units == "imperial")
    sprite.drawString(String(int(current_temp + .5)) + "`F", 55, yPos, 4);
  else
    sprite.drawString(String(int(current_temp + .5)) + "`C", 55, yPos, 4);

  // Humidity
  sprite.drawString(String(current_humidity) + "%", 55, yPos + 30);

  // Wind info
  //  Serial.print("current_wind_speed/");
  //  Serial.print(String(current_wind_speed, 0));
  //  Serial.println("/");
  //  Serial.print(String(current_wind_speed, 1));
  //  Serial.println("/");

  //1 m/s = 2.236936 mph
  //1 m/s = 3.6 kph

  // OWM wind_speed is m/s for "metric" and ALREADY mph for "imperial".
  //1 m/s = 2.236936 mph
  //1 m/s = 3.6 kph
  float wWind;
  if (multiCity[whichCity].units == "metric")  // Convert m/s to kph
    wWind = current_wind_speed * 3.6;       // Convert m/s to kph
  else
    wWind = current_wind_speed;   // already mph from OWM
  if (wWind < 0.01) wWind = 0.;  // Silly thing is putting up -0.00 m/h!
  // (The "-0.00" was probably this uninitialised value, not OWM.)
  // >>> NOTE: "m/h" reads as metres per hour; "mph" and "km/h" are the usual
  // >>>   abbreviations.
  if (wWind > 9.99) {
    sprintf(workChar, "%.0f %s",
            wWind, multiCity[whichCity].units == "imperial" ? "mph" : "k/h");
  } else {
    sprintf(workChar, "%.1f %s",
            wWind, multiCity[whichCity].units == "imperial" ? "mph" : "k/h");
  }
  sprite.drawString(workChar, 55, yPos + 62);
  // Direction the wind is blowing FROM, as a compass name.
  windDegStr = windDegIntToOrdinalDirection(current_wind_deg);
  sprite.drawString(windDegStr, 55, 96);
  sprite.pushSprite(0, 0);
}
