#!/bin/bash

INTERFACE=$(ip route | grep '^default' | awk '{print $5}')

IP=$(ip addr show "$INTERFACE" | grep 'inet ' | awk '{print $2}' | cut -d/ -f1)

if [ -z "$IP" ]; then
    echo "IP Address can not be found."
    exit 1
fi

NEW_LINE="const char* LAPTOP_IP = \"$IP\";"

TARGET_FILE="http_test_2.ino"

DECLARATION_REGEX='^[[:space:]]*const[[:space:]]+char\*[[:space:]]+LAPTOP_IP[[:space:]]*=.*;'

if grep -qE "$DECLARATION_REGEX" "$TARGET_FILE"; then
    sed -i -E "s|$DECLARATION_REGEX|$NEW_LINE|" "$TARGET_FILE"
    echo "Updated LAPTOP_IP to \"$IP\" in $TARGET_FILE"
else
    echo "LAPTOP_IP declaration not found. No changes made."
fi

