/***************************************************************************/
void fetch_and_decode_Forecast()
/***************************************************************************/
{
  JsonDocument doc;
  int looper = 0;
  int thisHour = -1;
  int httpCode = -1;

  if ((WiFi.status() == WL_CONNECTED)) //Check the current connection status
  {
    HTTPClient http;
    http.setTimeout(10000);
    while (httpCode != 200) {
      Serial.print("\r\nOWM Forecast Request at ");
      printMyTime();
      Serial.println(ForecastEndpoint);
      http.begin(ForecastEndpoint); //Specify the URL
      httpCode = http.GET();      //Make the request
      if (httpCode < 0) {
        Serial.printf("Error %i on Forecast HTTP request. "
                      "Retrying in 10 seconds.\r\n", httpCode);
        badLastFetchF = true;
        if (looper++ > 5) return;
        delay(10000);
      }
    }
    badLastFetchF = false;
    payload = http.getString();
    Serial.println("OWM Forecast Return Packet");
    Serial.println(payload);

    dataFetchHour = intHour; dataFetchMin = intMin;

    error = deserializeJson(doc, payload);
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    totRain = 0.; totSnow = 0.;

#if defined DO_FORECAST_PRINTS
    Serial.println("From Forecast");
#endif
    const char* cod = doc["cod"]; // "200", we hope!
    int message = doc["message"]; // 0
    ThreeHourSamples = doc["cnt"]; // 40 is 5 days of 3-hourly predictions.

    JsonArray list = doc["list"];
    JsonObject city = doc["city"];
    const char* cTown = city["name"];  // Echo back of the city name.
    sTown = String(cTown);
    //    Serial.print("Report for "); Serial.println(sTown);
    // city_timezone = list["city"]["timezone"];  // Should work.  Doesn't. ???
    // Serial.printf("Time offset from UTC %i\r\n", city_timezone);
    city_timezone = city["timezone"];
#if defined DO_FORECAST_PRINTS
    Serial.printf("Time offset from UTC %i\r\n", city_timezone);
#endif

    highRainForecast = -1.;
    highSnowForecast = -1.;

    highTempForecast = -300.;  // A very low temp to ensure correction.
    lowTempForecast = 1000.;   // A very high temp to ensure correction.
    for (thisHour = 0; thisHour < ThreeHourSamples; thisHour++) {

      JsonObject today = list[thisHour];  // Pick up each 3 hour rain prediction (0-cnt)
      WxForecast[thisHour].dt = today["dt"];
#if defined DO_FORECAST_PRINTS
      Serial.printf("3H epoch %i ", WxForecast[thisHour].dt);
      Serial.print(ConvertToLocalTime(WxForecast[thisHour].dt));
#endif

      WxForecast[thisHour].Rainfall = today["rain"]["3h"];
      WxForecast[thisHour].Snowfall = today["snow"]["3h"];
#if defined DO_FORECAST_PRINTS
      Serial.printf("Rain %i: %.2f\r\n",
                    thisHour, WxForecast[thisHour].Rainfall);
      Serial.printf("Snow %i: %.2f\r\n",
                    thisHour, WxForecast[thisHour].Snowfall);
#endif
      if (WxForecast[thisHour].Rainfall > highRainForecast)
        highRainForecast = WxForecast[thisHour].Rainfall;
      if (WxForecast[thisHour].Snowfall > highSnowForecast)
        highSnowForecast = WxForecast[thisHour].Snowfall;

      totRain += WxForecast[thisHour].Rainfall;
      totSnow += WxForecast[thisHour].Snowfall;

      WxForecast[thisHour].Temperature = today["main"]["temp"];
      //      if (Units == "I")
      if (multiCity[whichCity].Units == "imperial")
        WxForecast[thisHour].Temperature =
          (WxForecast[thisHour].Temperature * 9 / 5) + 32;
      if (WxForecast[thisHour].Temperature < lowTempForecast)
        lowTempForecast = WxForecast[thisHour].Temperature;
      if (WxForecast[thisHour].Temperature > highTempForecast)
        highTempForecast = WxForecast[thisHour].Temperature;
#if defined DO_FORECAST_PRINTS
      Serial.printf("Temperature %i: %.2f, High %.2f, Low %.2f\r\n",
                    thisHour, WxForecast[thisHour].Temperature,
                    highTempForecast, lowTempForecast);
#endif
    }
#if defined DO_FORECAST_PRINTS
    Serial.printf("3-hour rainfall max for the next 5 days: %.2f\r\n",
                  highRainForecast);
    Serial.printf("Lowest temp found: %.2f, Highest temp found %.2f\r\n",
                  lowTempForecast, highTempForecast);
#endif
    //    JsonArray list = doc["list"];

    //    JsonObject list_0 = list[0];
    //    long list_0_dt = list_0["dt"]; // 1702414800
    //
    //    JsonObject list_0_main = list_0["main"];
    //    float list_0_main_temp = list_0_main["temp"]; // 25.08
    //    float list_0_main_feels_like = list_0_main["feels_like"]; // 25.99
    //    float list_0_main_temp_min = list_0_main["temp_min"]; // 25.08
    //    float list_0_main_temp_max = list_0_main["temp_max"]; // 25.29
    //    int list_0_main_pressure = list_0_main["pressure"]; // 1014
    //    int list_0_main_sea_level = list_0_main["sea_level"]; // 1014
    //    int list_0_main_grnd_level = list_0_main["grnd_level"]; // 1013
    //    int list_0_main_humidity = list_0_main["humidity"]; // 90
    //    float list_0_main_temp_kf = list_0_main["temp_kf"]; // -0.21
    //
    //    JsonObject list_0_weather_0 = list_0["weather"][0];
    //    int list_0_weather_0_id = list_0_weather_0["id"]; // 500
    //    const char* list_0_weather_0_main = list_0_weather_0["main"]; // "Rain"
    //    const char* list_0_weather_0_description = list_0_weather_0["description"]; // "light rain"
    //    const char* list_0_weather_0_icon = list_0_weather_0["icon"]; // "10n"
    //
    //    int list_0_clouds_all = list_0["clouds"]["all"]; // 50
    //
    //    JsonObject list_0_wind = list_0["wind"];
    //    float list_0_wind_speed = list_0_wind["speed"]; // 5.88
    //    int list_0_wind_deg = list_0_wind["deg"]; // 74
    //    float list_0_wind_gust = list_0_wind["gust"]; // 8.56
    //
    //    int list_0_visibility = list_0["visibility"]; // 10000
    //    float list_0_pop = list_0["pop"]; // 0.44
    //
    //    float list_0_rain_3h = list_0["rain"]["3h"]; // 0.14
    //    const char* list_0_sys_pod = list_0["sys"]["pod"]; // "n"
    //
    //    const char* list_0_dt_txt = list_0["dt_txt"]; // "2023-12-12 21:00:00"
    //
    // ...
    // ...and a bunch of repeats of the above commented lines for each day.
    // ...
    //    JsonObject city = doc["city"];
    //    long city_id = city["id"]; // 1727211
    //    const char* city_name = city["name"]; // "Bangui"
    //
    //    float city_coord_lat = city["coord"]["lat"]; // 18.5376
    //    float city_coord_lon = city["coord"]["lon"]; // 120.7671
    //
    //    const char* city_country = city["country"]; // "PH"
    //    int city_population = city["population"]; // 2458
    //    int city_timezone = city["timezone"]; // 28800
    //    long city_sunrise = city["sunrise"]; // 1702419595
    //    long city_sunset = city["sunset"]; // 1702459309

    Serial.print("OWM Forecast Complete at "); getMyTime();
    printMyTime();
  }
}
