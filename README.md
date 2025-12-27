# ESP32-S3 WebUI
----

## Goal
This project's goal is to create a WebUI and remote access for the ESP32-S3, this can be modified to perform different tasks but in this specific case will be used to control the on-board NeoPixel

#### Other wifi-related functions won't work while simultaneously using the WebUI

----
## Features

- Creates an access point
- Creates lightweight web server (port 80)
- Controls NeoPixel
- Buttons trigger HTTP endpoints that lead to actions being executed
- Logs which actions were taken
- Shows what color is being displayed by NeoPixel
- Restart function in case necessary

----
### TODO
Install LittleFS and have WebUI on separate files