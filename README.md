# Paper

Firmware for DIY e-Paper Clock

---

Board: Arduino Nano ESP32

Display: [Waveshare 4.2inch e-Paper Module](https://www.waveshare.com/wiki/4.2inch_e-Paper_Module_Manual)

Toolchain: PlatformIO with Arduino framework

---

Features:
- Automatic NTP synchronization by geolocation
- On-the-fly TTF rendering
- Configurable using serial terminal
- Tells the time

---

Usage:
- To set up, connect to your PC and begin a serial terminal session using a program like PuTTY. Baud rate is 115200.
- If the geolocation API reports an incorrect time zone, set the time zone manually and disable geolocation in the terminal.
- Look at it.
