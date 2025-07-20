#!/bin/bash

# curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
# sudo mv bin/arduino-cli /usr/local/bin/
# Do this to install arduino cli

INTERFACE=$(ip route | grep '^default' | awk '{print $5}')

IP=$(ip addr show "$INTERFACE" | grep 'inet ' | awk '{print $2}' | cut -d/ -f1)

if [ -z "$IP" ]; then
    echo "IP Address can not be found."
    exit 1
fi

NEW_LINE="const char* LAPTOP_IP = \"$IP\";"

TARGET_FILE="$1"

if [ -z "$TARGET_FILE" ]; then
    echo "Usage: $0 <path_to_target_file.ino>"
    exit 1
fi


DECLARATION_REGEX='^[[:space:]]*const[[:space:]]+char\*[[:space:]]+LAPTOP_IP[[:space:]]*=.*;'

if grep -qE "$DECLARATION_REGEX" "$TARGET_FILE"; then
    sed -i -E "s|$DECLARATION_REGEX|$NEW_LINE|" "$TARGET_FILE"
    echo "Updated LAPTOP_IP to \"$IP\" in $TARGET_FILE"
else
    echo "LAPTOP_IP declaration not found. No changes made."
fi

echo "Building binary with arduino-cli..."
arduino-cli compile --fqbn esp32:esp32:esp32 "$TARGET_FILE" --output-dir ./build

BIN_FILE=$(find ./build -name "*.bin" | head -n 1)

if [ ! -f "$BIN_FILE" ]; then
    echo "Build failed. Binary not found."
    exit 1
fi

echo "Build succeeded: $BIN_FILE"

echo "Starting local HTTP server on port 8000..."
cd ./build
python3 -m http.server 8000 &
HTTP_PID=$!

sleep 2  

DEVICE_NAME="esp32_001"
TOPIC="devices/${DEVICE_NAME}/ota/command"

echo "Publishing MQTT OTA update command to $TOPIC"
mosquitto_pub
mosquitto_pub -h "$IP" -t "$TOPIC" -m "update"

echo "MQTT command sent. OTA update should start on the ESP32."

echo "Hosting the server for 60 seconds. Press Ctrl+C to stop early."
sleep 60
kill $HTTP_PID
