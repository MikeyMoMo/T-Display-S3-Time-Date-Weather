/***************************************************************************/
void showCurrentCondx()
/***************************************************************************/
{
  int strLength;

  sprite.fillSprite(TFT_BLACK);

  cax = 167;  // was 142
  cay = 70; cah = 15; caw = 24;
  sprite.setTextDatum(MC_DATUM);
  sprite.setTextColor(TFT_WHITE, TFT_BLACK);
  //  drawArrayJpeg(Colorbar10, sizeof(Colorbar10), 132, 0);
  //  drawArrayJpeg(Colorbar5, sizeof(Colorbar5), 135, 0);
  drawArrayJpeg(Colorbar2, sizeof(Colorbar2), 136, 0);

  for (int j = 0; j < 7; j++)
    sprite.drawString(SSDays[j], cax + (j * caw), cay, 2);

  int DOM = 1;
  int DOW = 0;
  bool started = false;

  for (int i = 0; i < 6; i++) {
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
  }
  sprite.setTextDatum(TL_DATUM);
  sprite.setTextColor(TFT_WHITE, TFT_BLACK);
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

  sprite.drawLine(cax - 10, cay - 10, cax + 177, cay - 10, myGray);  // was 152

  sprite.setTextColor(0x35F9, TFT_BLACK);
  sprite.setFreeFont(&FreeSans9pt7b);
  int yPos = 10;
  sprite.drawString("Temp", 0, yPos);
  sprite.drawString("Hum",  0, yPos + 30);
  sprite.drawString("Wind", 0, yPos + 60);
  sprite.drawString("From", 0, yPos + 88);
  
  // Clouds, Clear, etc.
  sprite.drawString(current_weather_0_main, 0, yPos + 120);
  sprite.setTextColor(TFT_DARKGREY, TFT_BLACK);
  // This is so odd.  You have to negate to get a go signal on a match.
  if (!strcmp(current_weather_0_main, "Clouds")) {
    sprite.setTextDatum(TR_DATUM);
    sprite.drawString(String(current_clouds) + "%", 115, yPos + 120);  // was 110
    sprite.setTextDatum(TL_DATUM);
  }
  sprite.setTextColor(TFT_DARKGREY, TFT_BLACK);
  sprintf(workChar, "Updated at: %02i:%02i", dataFetchHour, dataFetchMin);
  sprite.drawString(workChar, 0, 150, 2);
  sprite.setFreeFont(&FreeSans12pt7b);
  yPos = 31;
  sprite.drawFastHLine(0, yPos, 114, TFT_WHITE);
  sprite.drawFastHLine(0, yPos + 29, 114, TFT_WHITE);
  sprite.drawFastHLine(0, yPos + 89, 114, TFT_WHITE);

  yPos = 5;
  // Temperature
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

  float wWind;
  //  if (Units == "I")
  if (multiCity[whichCity].units == "metric")  // Convert m/s to kph
    // if (unitsStrings[whichCity] == "metric")  // Convert m/s to kph
    wWind = current_wind_speed * 3.6;       // Convert m/s to kph
  //  else  // If imperial, it is already m/h
  //    wWind = current_wind_speed * 2.237;     // Convert m/s to mph
  if (wWind < 0.01) wWind = 0.;  // Silly thing is putting up -0.00 m/h!
  if (wWind > 9.99) {
    sprintf(workChar, "%.0f %s",
            wWind, multiCity[whichCity].units == "imperial" ? "m/h" : "k/h");
    //      wWind, unitsStrings[whichCity] == "imperial" ? "m/h" : "k/h");
  } else {
    sprintf(workChar, "%.1f %s",
            wWind, multiCity[whichCity].units == "imperial" ? "m/h" : "k/h");
    //      wWind, unitsStrings[whichCity] == "imperial" ? "m/h" : "k/h");
  }
  sprite.drawString(workChar, 55, yPos + 62);
  windDegStr = windDegIntToOrdinalDirection(current_wind_deg);
  sprite.drawString(windDegStr, 55, 96);
  //  sprite.pushImage(297, 0, 26, 26, snowFlake);  // Brightness indicator in upper right corner.
  sprite.pushSprite(0, 0);
}
