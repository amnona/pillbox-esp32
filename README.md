# pillbox-esp32
pillbox for esp32

using sparkfun thing plus esp32 wroom WRL-20168
To connect:
* to GND connect ~300 ohm resistor, then connect to lid switch and to red and green led out
* to GPIO15 connect lid switch other side (lid switch is short when open, disconnect when closed)
* to GPIO 14 connect RED led input
* to GPIO 12 connect GREEN led input

* in the sketch, replace the REPLACE strings (wifi name/password, mail password, recipient email)

* To work with Arduino IDE, need to work slow connection rate 115200 and select Sparkfun ESP32 Thing Plus C

