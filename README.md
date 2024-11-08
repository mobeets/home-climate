# home-climate

## overview

This is a low-cost ($25) air quality monitoring solution that monitors temperature, relative humidity, and particle pollution (0.3 µm, 0.5 µm, 1.0 µm, 2.5 µm, 5.0 µm, and 10.0 µm). The monitor connects to your wifi and uploads its measurements to Google sheets, which you can then visualize over time.

## hardware

- WEMOS D1 Mini (ESP8266) [$2]
- Adafruit SHT41 [$6]
- Plantower PMS5003 particle sensor [$13]
- dupont female-female cables [$2]
- ventilated plastic enclosure, 108x56x40mm [$2]
- USB-C power supply

Pricing is from AliExpress.

If your ESP8266 doesn't come with headers already attached, you'll also need a soldering iron and solder.

If your PMS5003 doesn't come with a breadboard adapter, you'll want to buy a JST 1.25mm 8-pin cable to 2.54mm dupont cable (~$5).

## dependencies

- [HTTPSRedirect](https://github.com/electronicsguy/HTTPSRedirect)
- [ESP8266WiFi](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi)
- [Adafruit SHT4x](https://github.com/adafruit/Adafruit_SHT4X)

## resources

- https://variax.wordpress.com/2021/08/22/air-quality-sensor-with-wemos-d1-mini-esp8266-and-pms5003/
- https://github.com/StorageB/Google-Sheets-Logging
