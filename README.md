# T-Display-S3-Time-Date-Weather
This code obtains WX from OWM and shows it in several ways, current and forecasted.

This new version enables you to have multiple cities listed and change between them with the press of a button.  One button turns on the display and the other changes the city.  If you change the city, the display is turned on to show the results.  You need not press the other button.

You will need to obtain a free OneCall API key from the OpenWeatherMap site and put it in this code. It is good for up to 1,000 free calls/day.
There are other customiations to be made in the Defines_and_Docs tab, right at the top.  And, change your cities to your of interest.

There are now graphs for temperature, rain and snow.  The snow graph will not show unless there is snow in the 5-day forecast.  Rain and Temperature will always show.  The new graphs are filled in with color.  Red for temperature, blue for rain and white for snow.

The display now goes off after a time period.  It is currently 4 minutes.  You can easily change it.  If either button is pressed while the display is off, a time and date summary screen will be shown for a few seconds, then the screen rotation will resume for the timeout period.  If either button is pressed while the display is on, it will not show the time and date summary screen again, but will reset the timeout to the maximum.  It does not add time, just resets it to max timeout.  Also, the screen will wake up on the hour just as if a button was pressed.

This is still a work-in-progress but is working as coded.  Other functions, features or improvements may be made in the future until I am totally happy with it.  This is close to the end of coding for the design I have made.  If you have suggestions for additional functions or find errors, please let me know.  I accept suggestiong or bug reports without whining like some on Github do.  I am always open to improving the tool.

Enjoy,
Mike

![20231228_203142](https://github.com/MikeyMoMo/T-Display-S3-Time-Date-Weather/assets/15792417/9e774609-4260-4581-9aba-4e2b9df2b850)
![20231228_203208](https://github.com/MikeyMoMo/T-Display-S3-Time-Date-Weather/assets/15792417/5f68965d-9fed-4ced-b63c-9692e5e907ed)
![20231228_203230](https://github.com/MikeyMoMo/T-Display-S3-Time-Date-Weather/assets/15792417/02f1f5c6-7227-44b5-8542-e022b9423ff3)
![20240119_134816](https://github.com/MikeyMoMo/T-Display-S3-Time-Date-Weather/assets/15792417/e1fbd448-0715-4541-b1f2-31c7ab005a84)
![20240119_134923](https://github.com/MikeyMoMo/T-Display-S3-Time-Date-Weather/assets/15792417/96e77dc7-c631-47de-8ccc-1f575ee9ef93)
