# SFMD - Smart Fault Monitoring Device
# A Mongoose OS app that sends sensor data to AWS-IOT

## Overview

This is a project to receives sensor data over UART & sends it to AWS-IOT. It publishes data to a thing shadow in AWS-IOT. It also has a handler to turn on or off the on-board ESP8266 LED.


## How to install this app

 1. Clone this repository
 2. Change JSON field names in init.js as required
 3. Change conf1.json file to match your environment configuration
 4. cd into repo and run "sudo mos build --arch \<device-name\>" (e.g. sudo mos build --arch esp8266)
 5. Connect your device (To chk USB devices available:  ls /dev/tty*  )
 5. To program run: "sudo mos flash" or sudo mos flash –-port /dev/ttyUSB0 or mos flash –-port /COM6
 6. Download certificates from aws dashboard or iot-platform and push them to the device filesystem and change the conf file
 7. Or Configure aws-cli and do certificate settings from mos gui

