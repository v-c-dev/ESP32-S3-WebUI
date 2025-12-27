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

<img width="701" height="398" alt="image" src="https://github.com/user-attachments/assets/7378a5c6-cec1-4ce7-8c61-41ee96da1328" />

----
### TODO
Install LittleFS and have WebUI on separate files
