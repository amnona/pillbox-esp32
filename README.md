# pillbox-esp32
pillbox for esp32

using sparkfun thing plus esp32 wroom WRL-20168
To connect:
* to GND connect ~300 ohm resistor, then connect to lid switch and to red and green led out
* to GPIO15 connect lid switch other side (lid switch is short when open, disconnect when closed)
* to GPIO 14 connect RED led input
* to GPIO 12 connect GREEN led input

* in the sketch directory, create a secrets.h file by copying secrets-example.h and editing all usernames/passwords

* To work with Arduino IDE, need to work slow connection rate 115200 and select Sparkfun ESP32 Thing Plus C

* For setting up the server (which keeps a log of open/close events):

For raspberry pi running BookWorm, install conda using:
```
wget https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-Linux-aarch64.sh
```

Create the conda env for the server:
```
conda create --name pillbox-server python=3
pip install flask
```

Optional: add the server to the computer autorun. On raspberry pi:

Create a /lib/systemd/system/pillbox.service file containing:

 ```
 [Unit]
 Description=ESP32 Pillbox log server running on port 5000
 After=multi-user.target

 [Service]
 Type=idle
 ExecStart=/usr/bin/bash /home/batata/git/pillbox-esp32/server/run-server.sh

 [Install]
 WantedBy=multi-user.target
```

and then execute:
```
sudo chmod 644 /lib/systemd/system/pillbox.service
sudo systemctl daemon-reload
sudo systemctl enable pillbox.service
```

and finally

```
sudo reboot
```

* The server can be accessed using:

```
curl servo.local:5000/list_events
```

or by accessing on web browser:
http://servo.local:5000/
