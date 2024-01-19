/***************************************************************************/
void showCurrentCondx()
/***************************************************************************/
{
  sprite.fillSprite(TFT_BLACK);

  cax = 142; cay = 70; cah = 15; caw = 24;
  sprite.setTextDatum(MC_DATUM);
  sprite.setTextColor(TFT_WHITE, TFT_BLACK);
  for (int j = 0; j < 7; j++)
    sprite.drawString(SSDays[j], cax + (j * caw), cay, 2);

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
  //  sprite.setTextFont(0);  // Really small

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
  sprite.drawString("Wind", 0, yPos + 61);
  sprite.drawString("From", 0, yPos + 89);
  sprite.drawString(current_weather_0_main, 0, yPos + 120);
  sprite.setTextColor(TFT_DARKGREY, TFT_BLACK);
  // This is so odd.  You have to negate to get a go signal on a match.
  if (!strcmp(current_weather_0_main, "Clouds")) {
    //  if (!strcmp(hourly_item_weather_0_main[0], "Clouds")) {
    sprite.setTextDatum(TR_DATUM);
    sprite.drawString(String(current_clouds) + "%", 110, yPos + 120);
    sprite.setTextDatum(TL_DATUM);
  }
  sprite.setTextColor(TFT_DARKGREY, TFT_BLACK);
  sprintf(workChar, "Updated at: %02i:%02i", dataFetchHour, dataFetchMin);
  sprite.drawString(workChar, 0, 150, 2);
  sprite.setFreeFont(&FreeSans12pt7b);
  yPos = 30;
  sprite.drawFastHLine(0, yPos, 110, TFT_WHITE);
  sprite.drawFastHLine(0, yPos + 30, 110, TFT_WHITE);
  sprite.drawFastHLine(0, yPos + 90, 110, TFT_WHITE);

  yPos = 5;
  // Temperature
  if (Units == "I") {
    // F Temperature
    tmpF = (current_temp * 9. / 5.) + 32.;  // Convert ºF to ºC.
    sprite.drawString(String(int(tmpF + .5)) + "F", 50, yPos);
  } else {
    // C Temperature
    tmpC = current_temp;  // Convert String representation to float
    sprite.drawString(String(int(tmpC + .5)) + "c", 50, yPos);
  }
  // Humidity
  sprite.drawString(String(current_humidity) + "%", 50, yPos + 30);

  // Wind info
  //  Serial.print("current_wind_speed/");
  //  Serial.print(String(current_wind_speed, 0));
  //  Serial.println("/");
  //  Serial.print(String(current_wind_speed, 1));
  //  Serial.println("/");

  float wTemp;
  if (Units == "I")
    wTemp = current_wind_speed * 2.237;
  else
    wTemp = current_wind_speed;
  if (wTemp >= 10.0) {
    sprintf(workChar, "%.0f %s", wTemp, Units == "I" ? "m/h" : "m/s");
  } else {
    sprintf(workChar, "%.1f %s", wTemp, Units == "I" ? "m/h" : "m/s");
  }
  sprite.drawString(workChar, 50, yPos + 61);
  windDegStr = windDegIntToOrdinalDirection(current_wind_deg);
  sprite.drawString(windDegStr, 50, 94);

  sprite.pushImage(297, 0, 26, 26, snowFlake);  // Brightness indicator in upper right corner.
  sprite.pushSprite(0, 0);
}
