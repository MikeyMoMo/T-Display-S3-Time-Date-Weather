/*===========================================================================
  OneCall_Decode.ino  -  FETCH + DECODE OpenWeatherMap "One Call 3.0"
  ===========================================================================
  WHAT IT DOES
    Asks OWM for the selected city's CURRENT weather and the DAILY forecast
    (8 days), and copies the values into the global current_* and
    daily_item_*[] variables (declared in Defines_and_Docs.h).

  REQUEST
    https://api.openweathermap.org/data/3.0/onecall
        ?lat=..&lon=..&units=metric|imperial&exclude=minutely&appid=KEY

  REPLY (simplified)
    { "lat":..,"lon":..,"timezone":"Asia/Manila","timezone_offset":28800,
      "current": { "dt","sunrise","sunset","temp","humidity","wind_speed",
                   "wind_deg","clouds", "weather":[{"id","main",...}] ...},
      "hourly":  [ 48 x { "dt","temp","pop",... } ],
      "daily":   [  8 x { "dt","sunrise","sunset","moonrise","moonset",
                          "moon_phase","temp":{"min","max",...},
                          "weather":[{"id",...}],"pop",... } ] }

  WHO USES THE RESULT
    showCurrentCondx()  current_temp/humidity/wind/clouds/weather_0_main
    ShowForecast()      daily_item_dt/temp_min/temp_max/weather_0_id/pop
    ShowSunMoon()       daily_item_dt/sunrise/sunset/moonrise/moonset/phase
    wake splash (loop)  current_temp, current_humidity

  IF THE FETCH FAILS
    The function just returns; the previous data stays on screen.  (That is
    a sensible choice - but nothing on screen tells you the data is old.)

  >>> REVIEW - suggested rewrite of the fetch part (much less RAM, no
  >>> 20 KB String, only the fields you use are kept):
  >>>
  >>>   JsonDocument filter;
  >>>   filter["current"]["temp"] = true;  filter["current"]["humidity"] = true;
  >>>   filter["current"]["wind_speed"] = true; filter["current"]["wind_deg"] = true;
  >>>   filter["current"]["clouds"] = true;
  >>>   filter["current"]["weather"][0]["main"] = true;
  >>>   filter["daily"][0]["dt"] = true;   // [0] = "same for every item"
  >>>   filter["daily"][0]["sunrise"] = true;
  >>>   filter["daily"][0]["sunset"] = true;
  >>>   filter["daily"][0]["moonrise"] = true;
  >>>   filter["daily"][0]["moonset"] = true;
  >>>   filter["daily"][0]["moon_phase"] = true;
  >>>   filter["daily"][0]["pop"] = true;
  >>>   filter["daily"][0]["temp"]["min"] = true;
  >>>   filter["daily"][0]["temp"]["max"] = true;
  >>>   filter["daily"][0]["weather"][0]["id"] = true;
  >>>
  >>>   HTTPClient http;
  >>>   http.begin(oneCallEndpoint);
  >>>   http.useHTTP10(true);        // lets ArduinoJson read the stream directly
  >>>   int code = http.GET();
  >>>   if (code == 200) {
  >>>     JsonDocument doc;
  >>>     auto err = deserializeJson(doc, http.getStream(),
  >>>                                DeserializationOption::Filter(filter));
  >>>     ...
  >>>   }
  >>>   http.end();                  // ALWAYS, success or failure
  >>>
  >>> Add "&exclude=minutely,hourly,alerts" too - you don't use hourly, and
  >>> it is the biggest part of the reply.
  ===========================================================================*/
/***************************************************************************/
void fetch_and_decode_OneCall()
/***************************************************************************/
{
  int looper = 0;
  int thisHour = -1;
  int httpCode = -1;

  // Only try if WiFi is up.  If not, silently skip (old data stays).
  if ((WiFi.status() == WL_CONNECTED)) //Check the current connection status
  {
    //--- Build the request URL for the selected city -----------------------
    oneCallEndpoint = "https://api.openweathermap.org/data/3.0/"
                      "onecall?lat=" + multiCity[whichCity].lat +
                      "&lon=" + multiCity[whichCity].lon +
                      "&units=" + multiCity[whichCity].units +
                      "&exclude=minutely&appid=" + api_key;

    //--- Send the request; up to 4 tries, 10 s apart ------------------------
    // >>> REVIEW: "looper++ > 2" gives 4 attempts, so a dead server freezes
    // >>>   the clock for ~30-70 s (4 x up-to-10 s timeout + 3 x 10 s delay),
    // >>>   and fetch_and_decode_Forecast() can do the same right before this.
    // >>>   For a regular 31-minute refresh, 1 try is enough - just try again
    // >>>   at the next interval (or in 2 minutes).
    // >>> REVIEW: http.end() is never called - not after success, not before a
    // >>>   retry, not before "return".  The HTTPClient destructor tidies up
    // >>>   most of it, but begin() on top of an un-ended begin() is asking for
    // >>>   a slow socket/memory leak.  Call http.end() before each retry and
    // >>>   once after getString().
    // >>> NOTE: begin(url) with https and no certificate means the TLS link is
    // >>>   NOT verified (encrypted, but anyone in the middle could pose as OWM
    // >>>   and read your API key).  Low risk on a home network; fixable with
    // >>>   WiFiClientSecure + setCACert().
    HTTPClient http;
    http.setTimeout(10000);
    while (httpCode != 200) {
      Serial.print("\r\nOWM OneCall Request at ");
      getMyTime(); Serial.print(fullTimeDate);
      Serial.printf(" for %s.\r\n", multiCity[whichCity].CityName.c_str());
      Serial.println(oneCallEndpoint);
      // >>> NOTE: this prints your API key to the Serial Monitor.  Be careful
      // >>>   when pasting logs/screenshots into forums or GitHub issues.
      // >>> NOTE: this rebuilds exactly the same URL as oneCallEndpoint above.
      // >>>   Just use:  http.begin(oneCallEndpoint);
      http.begin("https://api.openweathermap.org/data/3.0/"
                 "onecall?lat=" + multiCity[whichCity].lat +
                 "&lon=" + multiCity[whichCity].lon +
                 "&units=" + multiCity[whichCity].units +
                 "&exclude=minutely&appid=" + api_key);  //Specify the URL et.al.
      httpCode = http.GET();        //Make the request
      if (httpCode != 200) {
        Serial.printf("Error %i on OneCall HTTP request. "
                      "Retrying in 10 seconds.\r\n", httpCode);
        if (looper++ > 2) return;
        delay(10000);
      }
    }
    //--- Got HTTP 200: read the whole reply and parse it --------------------
    JsonDocument doc;
    payload = http.getString();
    Serial.println("OWM OneCall Return Packet");
    Serial.println(payload);
    // >>> REVIEW: dumps 15-20 KB of JSON to Serial on every fetch.  Put it
    // >>>   inside #if defined DO_ONECALL_PRINTS like the other debug output.

    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print("OneCall deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
      //      DeserializationError error = deserializeJson(doc, http.getStream());
      //      if (error) {
      //        Serial.print(F("deserializeJson() failed: ")); Serial.println(error.f_str());
      //        http.end();
      //        return;
    }

#if defined DO_ONECALL_PRINTS
    Serial.println("From OneCall");
#endif

    // Now, save the data for later displaying.
    // ArduinoJson tip: doc["x"] returns 0 / NULL when a field is missing,
    // so e.g. "wind_gust" (often absent) quietly becomes 0.

    //--- Top level -----------------------------------------------------------

    lat = doc["lat"]; // 18.5376
    lon = doc["lon"]; // 120.7671
    timezone = doc["timezone"]; // "Asia/Manila"
    // >>> BUG (latent): "timezone" now points INTO doc; doc is freed when this
    // >>>   function ends.  Don't use it outside this function.
    timezone_offset = doc["timezone_offset"]; // 28800
#if defined DO_ONECALL_PRINTS
    Serial.printf("Lat %f\r\n", lat);
    Serial.printf("Lon %f\r\n", lon);
    Serial.println(timezone);
    Serial.println(timezone_offset);
#endif

    //--- "current" block: conditions right now --------------------------------
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

    // .as<String>() COPIES the text, so these survive after doc is freed.
    // (This is the right way - the hourly/daily const char* below are not.)
    current_weather_0_id = current_weather_0["id"];
    //current_weather_0_main = current_weather_0["main"];
    current_weather_0_main = current_weather_0["main"].as<String>();
    //    current_weather_0_description = current_weather_0["description"];
    current_weather_0_description = current_weather_0["description"].as<String>();
    //    current_weather_0_icon = current_weather_0["icon"];
    //    current_weather_0_icon = current_weather_0["icon"].as<String>();

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
    // >>> BUG (debug only): the next printf and the "icon" one pass a String to
    // >>>   %s - needs .c_str().  Only matters when DO_ONECALL_PRINTS is on.
    Serial.printf("current_weather_0_main\t%s\r\n", current_weather_0_main);
    Serial.print("current_weather_0_description\t"); Serial.println(current_weather_0_description);
    //    Serial.printf("current_weather_0_icon\t%s\r\n", current_weather_0_icon);
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
    //--- "hourly" block: 48 hourly forecasts ---------------------------------
    // >>> REVIEW: NONE of these hourly_* values are used by any screen.  This
    // >>>   whole loop (and the arrays) can go - or exclude "hourly" in the URL.
    // >>> BUG (latent): no bounds check.  If OWM ever sent more than 48 items
    // >>>   this would write past the end of the arrays.  Add:
    // >>>   if (thisHour >= 47) break;   (at the top of the loop)
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
      // >>> BUG (latent): these three const char* point into doc (freed on return).
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
    //--- "daily" block: 8 daily forecasts (0 = today) --------------------------
    // Used by ShowForecast() and ShowSunMoon() (they show the first 7).
    // >>> BUG (latent): no bounds check either - arrays hold 10.  Add:
    // >>>   if (thisHour >= 9) break;
    // >>> NOTE: the counter is called thisHour but counts DAYS here.
    for (JsonObject daily_item : doc["daily"].as<JsonArray>()) {

      thisHour++;

      daily_item_dt[thisHour]         = daily_item["dt"];
      daily_item_sunrise[thisHour]    = daily_item["sunrise"];
      daily_item_sunset[thisHour]     = daily_item["sunset"];
      daily_item_moonrise[thisHour]   = daily_item["moonrise"];
      daily_item_moonset[thisHour]    = daily_item["moonset"];

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
      // >>> BUG (latent): main/description/icon point into doc (freed on return).
      // >>>   Only daily_item_weather_0_id is actually used - keep that one.
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
    //--- Done ----------------------------------------------------------------
    Serial.print("OWM OneCall Complete at ");
    printMyTime();
    Serial.println("---------------------");
  }
}
