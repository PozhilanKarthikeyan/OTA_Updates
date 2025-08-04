from flask import Flask, request, jsonify, send_from_directory, render_template
import os
from datetime import datetime
import json

LAPTOP_IP = "192.168.0.50"

app = Flask(__name__)
UPLOAD_FOLDER = "firmware"
LATEST_BIN = "firmware.bin"

log_buffer = []
dip_values = []

UPDATE_ALL = False

try:
    with open("server_version.txt", "r") as file:
        version = file.read().strip().split("=")[1]
except FileNotFoundError:
    version = "0.0.0"

try:
    with open("esp_version.txt", "r") as file:
        esp_version = json.load(file)
except FileNotFoundError:
    esp_version = {}

os.makedirs(UPLOAD_FOLDER, exist_ok=True)

def log_event(msg):
    timestamp = datetime.now().strftime("[%Y-%m-%d %H:%M:%S]")
    entry = f"{timestamp} {msg}"
    log_buffer.append(entry)
    if len(log_buffer) > 100:
        log_buffer.pop(0)
    print(entry)

def update_esp_version_files(dip_values, version):
    global esp_version

    try:
        with open("esp_version.txt", "r") as file:
            existing_esp_versions = json.load(file)
    except FileNotFoundError:
        existing_esp_versions = {}

    for dip_value in dip_values:
        existing_esp_versions[dip_value] = version

    esp_version = existing_esp_versions

    try:
        with open("esp_version.txt", "w") as file:
            json.dump(existing_esp_versions, file, indent=4)
    except FileNotFoundError:
        log_event(f"[ERROR] cannot update esp_version.txt: FileNotFound")

log_event(f"[SERVER STARED] server version: {version}")

@app.route("/")
def index():
    return render_template("index.html", version=version, esp_version=esp_version)

@app.route("/upload", methods=['POST', 'GET'])
def upload_firmware():
    global updated_devies, version, dip_values, UPDATE_ALL

    if request.method == "GET":
        return render_template("upload.html", version=version, esp_version=esp_version)

    file = request.files.get("file")
    if not file:
        message = "No file found."
        if request.accept_mimetypes.accept_html:
            return render_template("error.html", message=message)
        return message + "\n", 400
    
    if not file.filename.lower().endswith(".bin"):
        message = "Only .bin file is allowed."
        if request.accept_mimetypes.accept_html:
            return render_template("error.html", message=message)
        return message + "\n", 400
    
    new_version = request.form.get("version")
    if new_version:
        version = new_version
        with open("version.txt", "w") as f:
            f.write(f"version={version}")

    given_dip_values = request.form.getlist("dip_value")
    if not given_dip_values:
        UPDATE_ALL = True
    else: 
        dip_values = given_dip_values
        
    save_path = os.path.join(UPLOAD_FOLDER, LATEST_BIN)
    file.save(save_path)

    msg = None
    if UPDATE_ALL:
        msg = f"[UPLOAD] New Frimware uploaded, verison: {version}, DIP-Values: ALL"
    else: 
        msg = f"[UPLOAD] New Frimware uploaded, verison: {version}, DIP-Values: {', '.join(dip_values)}"

    log_event(msg)

    log_msg = f"Firmware uploaded successfully to DIP-Values: {', '.join(dip_values)}. Version set to {version}"
    if request.accept_mimetypes.accept_html:
        return render_template("success.html", message=log_msg)
    return message + "\n", 200

@app.route("/firmware.bin")
def firmware():
    return send_from_directory(UPLOAD_FOLDER, LATEST_BIN)

@app.route("/update", methods=['POST'])
def update():
    global version
    device_version = request.headers.get("Version-ID")
    dip_value = request.headers.get("DIP-Value")

    if not UPDATE_ALL and dip_value not in dip_values:
        return jsonify({"update": False})
    if not device_version:
        return jsonify({"update": False})
    if device_version.strip() == version.strip():
        return jsonify({"update": False})
    else:
        log_event(f"[UPDATE STARTED] DIP Value: {dip_value}, device_version: {device_version}, server_version: {version}")
        return jsonify({
            "update": True,
            "url": "http://" + LAPTOP_IP + ":5000/firmware.bin"
        })
        
@app.route("/report_success", methods = ["POST"])
def report_success():
    global version
    dip_value = request.headers.get("DIP-Value")

    log_event(f"[UPDATE COMPLETE] DIP Value: {dip_value}, device_version: {version}")
    update_esp_version_files(dip_value,version)
    return jsonify({
        "success_code": 200,
        "current_version": version
    })

@app.route("/get_version", methods=["GET"])
def get_version():
    return f"Current Server Version: {version}\n"

@app.route("/logs")
def get_logs():
    return render_template("logs.html", logs=log_buffer)

if __name__ == "__main__":
    app.run("0.0.0.0", port=5000, threaded=True)