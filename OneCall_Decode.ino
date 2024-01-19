/***************************************************************************/
void fetch_and_decode_OneCall()
/***************************************************************************/
{
  JsonDocument doc;
  
  int thisHour = -1;
  int httpCode = -1;

  if ((WiFi.status() == WL_CONNECTED)) //Check the current connection status
  {
    HTTPClient http;
    http.setTimeout(10000);
    while (httpCode < 0) {
      Serial.print("\r\nOWM OneCall Request at ");
      printMyTime();
      Serial.println(oneCallEndpoint);
      http.begin(oneCallEndpoint); //Specify the URL
      httpCode = http.GET();      //Make the request
      if (httpCode < 0) {
        Serial.printf("Error %i on OneCall HTTP request. "
                      "Retrying in 1 minute.\r\n", httpCode);
        delay(60000);
      }
    }
    payload = http.getString();
    Serial.println("OWM OneCall Return Packet");
    Serial.println(payload);

    dataFetchHour = intHour; dataFetchMin = intMin;

    error = deserializeJson(doc, payload);
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

#if defined DO_ONECALL_PRINTS
    Serial.println("From OneCall");
#endif
    lat = doc["lat"]; // 18.5376
    lon = doc["lon"]; // 120.7671
    timezone = doc["timezone"]; // "Asia/Manila"
    timezone_offset = doc["timezone_offset"]; // 28800
#if defined DO_ONECALL_PRINTS
    Serial.printf("Lat %f\r\n", lat);
    Serial.printf("Lon %f\r\n", lon);
    Serial.println(timezone);
    Serial.println(timezone_offset);
#endif

    JsonObject current = doc["current"];

    current_dt = current["dt"];
    current_sunrise = current["sunrise"];
    current_sunset = current["sunset"];
    current_temp = current["temp"];
    current_feels_like = current["feels_like"];
    current_pressure = current["pressure"];
    current_humidity = current["humidity"];
    current_dew_point = current["dew_point"];
    current_uvi = current["uvi"];
    current_clouds = current["clouds"];
    current_visibility = current["visibility"];
    current_wind_speed = current["wind_speed"];
    current_wind_deg = current["wind_deg"];
    current_wind_gust = current["wind_gust"];

    JsonObject current_weather_0 = current["weather"][0];

    current_weather_0_id = current_weather_0["id"];
    current_weather_0_main = current_weather_0["main"];
    current_weather_0_description = current_weather_0["description"];
    current_weather_0_icon = current_weather_0["icon"];

#if defined DO_ONECALL_PRINTS
    Serial.printf("lat\t%f\r\n", lat);
    Serial.printf("lon\t%f\r\n", lon);
    Serial.printf("timezone\t%s\r\n", timezone);
    Serial.printf("timezone_offset\t%i\r\n", timezone_offset);
    Serial.printf("current_uvi\t%.2f\r\n", current_uvi);
    Serial.printf("current_clouds\t%i\r\n", current_clouds);
    Serial.printf("current_visibility\t%i\r\n", current_visibility);
    Serial.printf("current_wind_speed\t%.2f\r\n", current_wind_speed);
    Serial.printf("current_wind_deg\t%i\r\n", current_wind_deg);
    Serial.printf("current_wind_gust\t%.2f\r\n", current_wind_gust);
    Serial.printf("current_dt\t%lu\r\n", current_dt);
    Serial.printf("current_sunrise\t%lu\r\n", current_sunrise);
    Serial.printf("current_sunset\t%lu\r\n", current_sunset);
    Serial.printf("current_temp\t%.2f\r\n", current_temp);
    Serial.printf("current_feels_like\t%.2f\r\n", current_feels_like);
    Serial.printf("current_pressure\t%i\r\n", current_pressure);
    Serial.printf("current_humidity\t%i\r\n", current_humidity);
    Serial.printf("current_dew_point\t%.2f\r\n", current_dew_point);
    Serial.printf("current_weather_0_id\t%i\r\n", current_weather_0_id);
    Serial.printf("current_weather_0_main\t%s\r\n", current_weather_0_main);
    Serial.printf("current_weather_0_description\t%s\r\n",
                  current_weather_0_description);
    Serial.printf("current_weather_0_icon\t%s\r\n", current_weather_0_icon);
#endif

    // "minutely" is excluded in the http call.  It only returns p.o.p.
    //  for (JsonObject minutely_item : doc["minutely"].as<JsonArray>()) {
    //
    //    minutely_item_dt = minutely_item["dt"];
    //    // Serial.printf("minutely_item_dt\t%i\r\n", minutely_item_dt);
    //
    //    minutely_item_precipitation = minutely_item["precipitation"];
    //    // Serial.printf("minutely_item_precipitation\t%i\r\n",
    //                     minutely_item_precipitation);
    //  }

    thisHour = -1;
    for (JsonObject hourly_item : doc["hourly"].as<JsonArray>()) {

      thisHour++;

      hourly_item_dt[thisHour] = hourly_item["dt"];
      hourly_item_temp[thisHour] = hourly_item["temp"];
      hourly_item_feels_like[thisHour] = hourly_item["feels_like"];
      hourly_item_pressure[thisHour] = hourly_item["pressure"];
      hourly_item_humidity[thisHour] = hourly_item["humidity"];
      hourly_item_dew_point[thisHour] = hourly_item["dew_point"];
      hourly_item_uvi[thisHour] = hourly_item["uvi"];
      hourly_item_clouds[thisHour] = hourly_item["clouds"];
      hourly_item_visibility[thisHour] = hourly_item["visibility"];
      hourly_item_wind_speed[thisHour] = hourly_item["wind_speed"];
      hourly_item_wind_deg[thisHour] = hourly_item["wind_deg"];
      hourly_item_wind_gust[thisHour] = hourly_item["wind_gust"];

      JsonObject hourly_item_weather_0 = hourly_item["weather"][0];

      hourly_item_weather_0_id[thisHour] = hourly_item_weather_0["id"];
      hourly_item_weather_0_main[thisHour] = hourly_item_weather_0["main"];
      hourly_item_weather_0_description[thisHour] =
        hourly_item_weather_0["description"];
      hourly_item_weather_0_icon[thisHour] = hourly_item_weather_0["icon"];
      hourly_item_pop[thisHour] = hourly_item["pop"];

#if defined DO_ONECALL_PRINTS
      Serial.printf("hourly_item_dt[%i]\t%i\r\n", thisHour,
                    hourly_item_dt[thisHour]);
      Serial.printf("hourly_item_temp[%i]\t%.2f\r\n", thisHour,
                    hourly_item_temp[thisHour]);
      Serial.printf("hourly_item_feels_like[%i]\t%.2f\r\n", thisHour,
                    hourly_item_feels_like[thisHour]);
      Serial.printf("hourly_item_pressure[%i]\t%i\r\n", thisHour,
                    hourly_item_pressure[thisHour]);
      Serial.printf("hourly_item_humidity[%i]\t%i\r\n", thisHour,
                    hourly_item_humidity[thisHour]);
      Serial.printf("hourly_item_dew_point[%i]\t%.2f\r\n", thisHour,
                    hourly_item_dew_point[thisHour]);
      Serial.printf("hourly_item_uvi[%i]\t%i\r\n", thisHour,
                    hourly_item_uvi[thisHour]);
      Serial.printf("hourly_item_clouds[%i]\t%i\r\n", thisHour,
                    hourly_item_clouds[thisHour]);
      Serial.printf("hourly_item_visibility[%i]\t%i\r\n", thisHour,
                    hourly_item_visibility[thisHour]);
      Serial.printf("hourly_item_wind_speed[%i]\t%.2f\r\n", thisHour,
                    hourly_item_wind_speed[thisHour]);
      Serial.printf("hourly_item_wind_deg[%i]\t%i\r\n", thisHour,
                    hourly_item_wind_deg[thisHour]);
      Serial.printf("hourly_item_wind_gust[%i]\t%.2f\r\n", thisHour,
                    hourly_item_wind_gust[thisHour]);
      Serial.printf("hourly_item_weather_0_id[%i]\t%i\r\n", thisHour,
                    hourly_item_weather_0_id[thisHour]);
      Serial.printf("hourly_item_weather_0_main[%i]\t%s\r\n", thisHour,
                    hourly_item_weather_0_main[thisHour]);
      Serial.printf("hourly_item_weather_0_description[%i]\t%s\r\n", thisHour,
                    hourly_item_weather_0_description[thisHour]);
      Serial.printf("hourly_item_weather_0_icon[%i]\t%s\r\n", thisHour,
                    hourly_item_weather_0_icon[thisHour]);
      Serial.printf("hourly_item_pop[%i]\t%.2f\r\n", thisHour,
                    hourly_item_pop[thisHour]);
#endif
    }

    thisHour = -1;
    for (JsonObject daily_item : doc["daily"].as<JsonArray>()) {

      thisHour++;

      daily_item_dt[thisHour]         = daily_item["dt"];
      daily_item_sunrise[thisHour]    = daily_item["sunrise"];
      daily_item_sunset[thisHour]     = daily_item["sunset"];
      daily_item_moonrise[thisHour]   = daily_item["moonrise"];
      daily_item_moonset[thisHour]    = daily_item["moonset"];

      // If moonset is after midnight, OWM puts in a 0 for that first
      //  day.  On subsequent days, it put in a smaller  epoch for
      //  the moonset than for moonrise.
      // if ((daily_item_moonset[thisHour] < daily_item_moonrise[thisHour])
      //     && (thisHour > 0))
      //     daily_item_moonset[thisHour - 1] = daily_item_moonset[thisHour];
      //      if (thisHour > 0)
      //        if (daily_item_moonset[thisHour - 1] == 0) {
      //          daily_item_moonset[thisHour - 1] = daily_item_moonset[thisHour];
      //          daily_item_moonset[thisHour] = 0;
      //        }
      // Something will have to be done for the reverse...

      daily_item_moon_phase[thisHour] = daily_item["moon_phase"];

      JsonObject daily_item_temp      = daily_item["temp"];

      daily_item_temp_day[thisHour]   = daily_item_temp["day"];
      daily_item_temp_min[thisHour]   = daily_item_temp["min"];
      daily_item_temp_max[thisHour]   = daily_item_temp["max"];
      daily_item_temp_night[thisHour] = daily_item_temp["night"];
      daily_item_temp_eve[thisHour]   = daily_item_temp["eve"];
      daily_item_temp_morn[thisHour]  = daily_item_temp["morn"];

      JsonObject daily_item_feels_like = daily_item["feels_like"];

      daily_item_feels_like_day[thisHour]   = daily_item_feels_like["day"];
      daily_item_feels_like_night[thisHour] = daily_item_feels_like["night"];
      daily_item_feels_like_eve[thisHour]   = daily_item_feels_like["eve"];
      daily_item_feels_like_morn[thisHour]  = daily_item_feels_like["morn"];
      daily_item_pressure[thisHour]   = daily_item["pressure"];
      daily_item_humidity[thisHour]   = daily_item["humidity"];
      daily_item_dew_point[thisHour]  = daily_item["dew_point"];
      daily_item_wind_speed[thisHour] = daily_item["wind_speed"];
      daily_item_wind_deg[thisHour]   = daily_item["wind_deg"];
      daily_item_wind_gust[thisHour]  = daily_item["wind_gust"];

      JsonObject daily_item_weather_0 = daily_item["weather"][0];

      daily_item_weather_0_id[thisHour]          = daily_item_weather_0["id"];
      daily_item_weather_0_main[thisHour]        = daily_item_weather_0["main"];
      daily_item_weather_0_description[thisHour] = daily_item_weather_0["description"];
      daily_item_weather_0_icon[thisHour]        = daily_item_weather_0["icon"];
      daily_item_clouds[thisHour]                = daily_item["clouds"];
      daily_item_pop[thisHour]                   = daily_item["pop"];
      daily_item_uvi[thisHour]                   = daily_item["uvi"];

#if defined DO_ONECALL_PRINTS
      Serial.printf("daily_item_dt[%i]\t%i\r\n", thisHour,
                    daily_item_dt[thisHour]);
      Serial.printf("daily_item_sunrise[%i]\t%i\r\n", thisHour,
                    daily_item_sunrise[thisHour]);
      Serial.printf("daily_item_sunset[%i]\t%i\r\n", thisHour,
                    daily_item_sunset[thisHour]);
      Serial.printf("daily_item_moonrise[%i]\t%i\r\n", thisHour,
                    daily_item_moonrise[thisHour]);
      Serial.printf("daily_item_moonset[%i]\t%i\r\n", thisHour,
                    daily_item_moonset[thisHour]);
      Serial.printf("daily_item_moon_phase[%i]\t%.2f\r\n", thisHour,
                    daily_item_moon_phase[thisHour]);
      Serial.printf("daily_item_temp_day[%i]\t%.2f\r\n", thisHour,
                    daily_item_temp_day[thisHour]);
      Serial.printf("daily_item_temp_min[%i]\t%.2f\r\n", thisHour,
                    daily_item_temp_min[thisHour]);
      Serial.printf("daily_item_temp_max[%i]\t%.2f\r\n", thisHour,
                    daily_item_temp_max[thisHour]);
      Serial.printf("daily_item_temp_night[%i]\t%.2f\r\n", thisHour,
                    daily_item_temp_night[thisHour]);
      Serial.printf("daily_item_temp_eve[%i]\t%.2f\r\n", thisHour,
                    daily_item_temp_eve[thisHour]);
      Serial.printf("daily_item_temp_morn[%i]\t%.2f\r\n", thisHour,
                    daily_item_temp_morn[thisHour]);
      Serial.printf("daily_item_feels_like_day[%i]\t%.2f\r\n", thisHour,
                    daily_item_feels_like_day[thisHour]);
      Serial.printf("daily_item_feels_like_night[%i]\t%.2f\r\n", thisHour,
                    daily_item_feels_like_night[thisHour]);
      Serial.printf("daily_item_feels_like_eve[%i]\t%.2f\r\n", thisHour,
                    daily_item_feels_like_eve[thisHour]);
      Serial.printf("daily_item_feels_like_morn[%i]\t%.2f\r\n", thisHour,
                    daily_item_feels_like_morn[thisHour]);
      Serial.printf("daily_item_pressure[%i]\t%i\r\n", thisHour,
                    daily_item_pressure[thisHour]);
      Serial.printf("daily_item_humidity[%i]\t%i\r\n", thisHour,
                    daily_item_humidity[thisHour]);
      Serial.printf("daily_item_dew_point[%i]\t%.2f\r\n", thisHour,
                    daily_item_dew_point[thisHour]);
      Serial.printf("daily_item_wind_speed[%i]\t%.2f\r\n", thisHour,
                    daily_item_wind_speed[thisHour]);
      Serial.printf("daily_item_wind_deg[%i]\t%i\r\n", thisHour,
                    daily_item_wind_deg[thisHour]);
      Serial.printf("daily_item_wind_gust[%i]\t%.2f\r\n", thisHour,
                    daily_item_wind_gust[thisHour]);
      Serial.printf("daily_item_weather_0_id[%i]\t%i\r\n", thisHour,
                    daily_item_weather_0_id[thisHour]);
      Serial.printf("daily_item_weather_0_main[%i]\t%s\r\n", thisHour,
                    daily_item_weather_0_main[thisHour]);
      Serial.printf("daily_item_weather_0_description[%i]\t%s\r\n", thisHour,
                    daily_item_weather_0_description[thisHour]);
      Serial.printf("daily_item_weather_0_icon[%i]\t%s\r\n", thisHour,
                    daily_item_weather_0_icon[thisHour]);
      Serial.printf("daily_item_clouds[%i]\t%i\r\n", thisHour,
                    daily_item_clouds[thisHour]);
      Serial.printf("daily_item_pop[%i]\t%.2f\r\n", thisHour,
                    daily_item_pop[thisHour]);
      Serial.printf("daily_item_uvi[%i]\t%.2f\r\n", thisHour,
                    daily_item_uvi[thisHour]);
#endif
    }
    //    time_t currentEpoch; time(&currentEpoch);  // Get UTC epoch
    //    unsigned long nextMoonrise, nextMoonset;
    //    if (currentEpoch > daily_item_moonrise[0])
    //      nextMoonrise = daily_item_moonrise[1];
    //    else
    //      nextMoonrise = daily_item_moonrise[0];
    //    if (currentEpoch > daily_item_moonset[0])
    //      nextMoonset = daily_item_moonset[1];
    //    else
    //      nextMoonset = daily_item_moonset[0];
    //    String mr = ConvertToLocalTime(nextMoonrise);
    //    String ms = ConvertToLocalTime(nextMoonset);
    //    Serial.printf("Next Moonrise %i or %s,\r\nNext Moonset %i or %s\r\n",
    //                  nextMoonrise, mr.c_str(), nextMoonset,  ms.c_str());
    Serial.print("OWM OneCall Complete at ");
    printMyTime();
  }
}
