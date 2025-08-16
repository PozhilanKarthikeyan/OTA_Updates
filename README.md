# ESP32 OTA Update Server (Flask)

This is a simple OTA (Over-the-Air) firmware update system using Flask and two ESP32s.

## Directory Structure of `src`

```
.
├── access_point/           # ESP32 code to create Wi-Fi Access Point
│   └── access_point.ino
├── receive/                # ESP32 code that connects to AP and receives OTA updates
│   └── receive.ino
├── firmware/               # Folder containing the compiled binary and the SHA file
│   └── firmware.bin
│   └── firmware.sha256
├── update.py               # Flask OTA update server (runs on laptop)
├── README.md               # This file
├── esp_version.txt         # File containing ESP Firmware Versions
├── server_version.txt      # File containing Server Firmware versions
└── templates               # HTML Templates
```

## Requirements

- **Python 3.10 or newer** is required to run the OTA server.
- Install dependencies using:

```bash
pip install -r requirements.txt
``` 

## Instructions

1. Flash `access_point/access_point.ino` to one ESP32 — this creates a Wi-Fi Access Point.
2. Flash `receive/receive.ino` to the motor driver ESP — this will:
   - Connect to the Wi-Fi AP created by the AP ESP
   - Contact the Flask OTA server for updates

3. On your laptop:
   - Connect to the AP Wi-Fi network hosted by the AP ESP.
   - Start the OTA server:
     ```bash
     python3 update.py
     ```

4. Whenever you need to update, compile the firmware for the **motor driver ESP** and place the binary as:
   ```
   firmware/firmware.bin
   ```

### For non GUI Users

5. - Upload the firmware to the OTA server:
     ```bash
      curl -X POST -F "file=@firmware/firmware.bin" -F "version={version}" -F "dip_value=01" -F "dip_value-03" http://192.168.0.50:5000/upload
     ```
   - dip_value is an **optional argument** which runs from 00 to 07.
  
### For GUI users

5. Go to 
   ```bash
   http://192.168.0.50:5000/
   ```
