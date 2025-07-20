from flask import Flask, request, jsonify, send_from_directory
import os

LAPTOP_IP = "10.42.67.196"

app = Flask(__name__)
UPLOAD_FOLDER = "firmware"
LATEST_BIN = "firmware.bin"
TRIGGER_FLAG = False

os.makedirs(UPLOAD_FOLDER, exist_ok=True)

@app.route("/")
def index():
    return "ESP32 OTA Server is Running\n"

@app.route("/upload", methods=['POST'])
def upload_firmware():
    global TRIGGER_FLAG

    file = request.files.get("file")
    if not file:
        return "No file found\n", 400
    
    save_path = os.path.join(UPLOAD_FOLDER, LATEST_BIN)
    file.save(save_path)
    TRIGGER_FLAG = True

    return "Firmware uploaded successfully\n", 200

@app.route("/firmware.bin")
def firmware():
    return send_from_directory(UPLOAD_FOLDER, LATEST_BIN)

@app.route("/update", methods=['POST'])
def update():
    global TRIGGER_FLAG
    if not TRIGGER_FLAG:
        return jsonify({"update": False})
    else:
        TRIGGER_FLAG = False
        return jsonify({
            "update": True,
            "url": "http://" + LAPTOP_IP + ":5000/firmware.bin"
        })

if __name__ == "__main__":
    app.run("0.0.0.0", port=5000)