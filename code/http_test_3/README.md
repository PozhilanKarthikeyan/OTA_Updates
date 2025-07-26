# ESP32 OTA Update Server (Flask)

This is a simple OTA (Over-the-Air) firmware update server for ESP32 using Flask.

## Instructions

1. Run `ip addr show` and replace the IP in `main.py` and `update.py`
2. Compile your ESP32 code and generate a `.bin` file
3. Place the binary as `firmware/firmware.bin`
4. Start the server:
   python3 main.py
5. Upload firmware:
   curl -X POST -F "file=@firmware/firmware.bin" http://{your_ip}:5000/upload

readme by OPENAI
