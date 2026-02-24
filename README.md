# esp-network-demo

This is an demo designed to showcase the ESP32\'s ability to send messages
over a network for first-time ESP32 programmers interested in an interactive
programming experience. This was showcased by the Wilkes Math/CS club in a
collaboration with the Wilkes IEEE club. This demo also goes along with
talk including an into to networking and network programming in C, hence why
it was built using idf.py as opposed to Arduino IDE for lower level control
for demonstration purposes.

## Dependencies

### Linux

- ESP-IDF
- Python 3.12
- Git

### Windows

- ESP-IDF installer
- Latest version of idf.py installed via offline installer
- Git

## ESP32 Configuration

Configuration is intentionally not handled conveniently with command line
options or other configuration methods, and are instead handled by changing
the code directly. As part of the hands-on lab part of this demo, users will
need to update the code in esp/main/main.c if they expect it to work as intended.

Users primarily should focus on updating the AP_SSID, AP_PASSWORD, INET_ADDR,
and PORT_NUMBER macros as needed.

## Build

### Building the ESP32

#### Linux

1. Initialize idf.py environment:

```bash
source ~/esp/esp-idf/export.sh
```

2. Identify serial interface used by the ESP32 from `/dev/tty*`. E.g.:

```bash
ls /dev/ttyUSB0
```

3. Run the commands below from the repo root

NOTE: If your device is on serial port /dev/ttyUSB0, you can run the following commands
to change to ESP32 source directory, build, flash, and monitor your ESP32. Else,
replace the serial port with your port in the following commands then run.

```bash
cd esp/
idf.py build
idf.py -p /dev/ttyUSB0 flash
idf.py monitor
```

#### Windows

1. From the idf installation manager, enter your idf terminal

2. Identify serial interface used by the ESP32 from the device manager

3. Run the commands below from the repo root in the idf terminal

NOTE: If your device is on serial port COM6, you can run the following commands
to change to ESP32 source directory, build, flash, and monitor your ESP32. Else,
replace the serial port with your port in the following commands then run.

```ps
cd .\esp\
idf.py build
idf.py -p COM6 flash
idf.py monitor
```

If there are no errors, the code will run immediately and the server will receive
the message.

### Building the server

Run the following commands for setting up the server environment from the
project root, installing python dependencies, and running the server

#### Linux

```bash
python -m venv .venv
source .venv/bin/activate
pip install pygame
python server.py
```

#### Windows

Note: the ESP-IDF Installation Manager on Windows automatically starts a
python environment for you to use if you enter your idf terminal from the
installer

```ps
pip install pygame
python server.py
```

