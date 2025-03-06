# Weather station bsed on esp32

Display room clima on small screen and send out metrics to openhab via mqtt.

## Used components and wirering

Wroom32
https://de.aliexpress.com/item/1005005776600877.html

LCD st7735
https://de.aliexpress.com/item/1005005396134138.html
https://www.makerguides.com/interface-tft-st7735-display-with-esp32/
https://randomnerdtutorials.com/esp32-pinout-reference-gpios/#SPI
- gnd  braun    gnd
- vcc  red      3v
- scl  orang    D18  serial clock 
- sda  gelb     D23  serial data aka mosi
- res  grün     D16  lcm reset  permanent 3v
-  dc  blau     D17  data command aka miso aka dc
-  cs  violet   D5	 chip select
-  bl  weis     3v or D25  back ligth  permanent 3v

SCD41 Modul
https://de.aliexpress.com/item/1005005061351244.html   The blue 
- gnd  black  gnd
- vcc  weiss  D33 or other 3v source
- scl  grau   D22
- sda  violet D21

## How to compile.

Copy `screts.h.tpl` to `secrets.h` and enter your secrets.
Now just compile and roll out to your arduino. 
I used for the board settings:  ESP32-WROOM-DA MODULE   from:
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json

The serial monitor frequence is: 115200

