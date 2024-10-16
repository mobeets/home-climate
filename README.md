# home-climate

## hardware

- WEMOS D1 Mini (ESP8266) [$2]
- DHT22 [$2]
- PMS5003 [$13]
- dupont female-female cables [$2]
- ventilated plastic enclosure, 108x56x40mm [$2]
- USB-C power supply

All costs are from AliExpress.
If your ESP8266 doesn't come with headers already attached, you'll need a soldering iron and solder.
If your PMS5003 has a GH1.25 cable, you'll need wire strippers and electrical tape to splice it with the dupont cables in order to plug it into the ESP8266. Honestly though the wires are so thin and fiddly that it's worth spending $3 to buy an adapter to convert the JST 1.25mm 8-pin cable to 2.54mm for dupont.

## dependencies

- [HTTPSRedirect](https://github.com/electronicsguy/HTTPSRedirect)
- [ESP8266WiFi](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi)

## resources

- https://variax.wordpress.com/2021/08/22/air-quality-sensor-with-wemos-d1-mini-esp8266-and-pms5003/
- https://github.com/StorageB/Google-Sheets-Logging
