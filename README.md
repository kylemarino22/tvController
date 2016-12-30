# tvController
This is a Universal TV/IR device controller. It works by sending GET requests to an ESP8266 microcontroller, which handles the recording and playing of new IR codes.

#Project Wiring
This project uses and ESP8266, and Arduino Uno, and an SD Breakout Board. The SD card is used as a common memory location where the ESP8266 and the Arduino can write and read IR codes from. The arduino is used to record codes and to error correct codes that are being played by the ESP8266.

#Project Application
Along with the hardware, there is an application that goes with this project. It is in progress, but it mangages being the remotes being run on the hardware device. 

If anyone wants the wiring for this project, please let me know. 
