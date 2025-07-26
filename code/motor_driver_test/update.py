from flask import Flask, request, jsonify, send_from_directory
import os

LAPTOP_IP = "192.168.0.50"

app = Flask(__name__)
UPLOAD_FOLDER = "firmware"
LATEST_BIN = "firmware.bin"
updated_devies = set()

with open("version.txt", "r") as file:
    version = file.read().strip().split("=")[1]

os.makedirs(UPLOAD_FOLDER, exist_ok=True)

print(f"[SERVER STARED] server version: {version}")

@app.route("/")
def index():
    return f"ESP32 OTA Server is Running with version {version}\n"

@app.route("/upload", methods=['POST'])
def upload_firmware():
    global updated_devies, version

    file = request.files.get("file")
    if not file:
        return "No file found\n", 400
    
    new_version = request.form.get("version")
    if new_version:
        version = new_version
        with open("version.txt", "w") as f:
            f.write(f"version={version}")
        
    save_path = os.path.join(UPLOAD_FOLDER, LATEST_BIN)
    file.save(save_path)
    updated_devies.clear()

    print(f"[UPLOAD] New Frimware uploaded, verison: {version}")

    return f"Firmware uploaded successfully, version set to {version}\n", 200

@app.route("/firmware.bin")
def firmware():
    return send_from_directory(UPLOAD_FOLDER, LATEST_BIN)

@app.route("/update", methods=['POST'])
def update():
    global updated_devies, version
    device_id = request.headers.get("X-Device-ID")
    device_version = request.headers.get("Version-ID")

    if not device_id:
        return jsonify({"update": False})
    if not device_version:
        return jsonify({"update": False})
    if device_id in updated_devies:
        return jsonify({"update": False})
    if device_version.strip() == version.strip():
        return jsonify({"update": False})
    else:
        print(f"[UPDATE STARTED] device: {device_id}, device_version: {device_version}, server_version: {version}")
        return jsonify({
            "update": True,
            "url": "http://" + LAPTOP_IP + ":5000/firmware.bin"
        })
        
@app.route("/report_success", methods = ["POST"])
def report_success():
    global updated_devies, version
    device_id = request.headers.get("X-Device-ID")

    updated_devies.add(device_id)
    print(f"[UPDATE COMPLETE] device: {device_id}, device_version: {version}")
    return jsonify({
        "success_code": 200,
        "current_version": version
    })

@app.route("/get_version", methods=["GET"])
def get_version():
    return f"Current Server Version: {version}\n"

if __name__ == "__main__":
    app.run("0.0.0.0", port=5000, threaded=True)