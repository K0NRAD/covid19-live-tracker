# COVID19 Live Tracker #

![M5Stick-C COVID19 Tracker](https://github.com/K0NRAD/covid19-live-tracker/blob/main/assets/m5stickc_covid19_live_tracker.jpg?raw=true )

With the Corana Tracker you are always up to date with the COVID-19 infection, recovery and death tolls.

The current data is provided online by [worldOmeters](https://www.worldometers.info).

## Hardware ##  
M5StickC is a ESP32 development board with 0.96 inch TFT color screen (80 * 160 resolution), Red LED, 
button, Microphone, IR transmitter, 6-axis IMU (SH200Q) and 80 mAH battery. 

## Firmware ##
The firmware is created with the Arduino framework. [Visual Studio Code](https://code.visualstudio.com)
with the [PlatformIO](https://platformio.org/platformio-ide) plugin is used as IDE.



*Thanks for project information provided by @Niyas Thalappil*

>Add a file with WIFI SSID and password to the source directory, name this file `credentials.h`.
>The file `credentials.h` has the following content.
>
> ```c++
>#ifndef COVIT19_LIVE_TRACKER_CREDNTIALS_H
>#define COVIT19_LIVE_TRACKER_CREDNTIALS_H
>
>#define WIFI_SSID "SSID"       // Enter your SSID here
>#define WIFI_PASS "PASSWORD"   // Enter your WiFi password here
>
>#endif //COVIT19_LIVE_TRACKER_CREDNTIALS_H
```
