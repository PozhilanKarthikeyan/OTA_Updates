import hashlib
import json
import os
import threading
from datetime import datetime
from typing import Dict, List

from flask import Flask, jsonify, render_template, request, send_from_directory

LAPTOP_IP: str = "192.168.0.50"

app: Flask = Flask(__name__)
UPLOAD_FOLDER: str = "firmware"
LATEST_BIN: str = "firmware.bin"

log_buffer: list[str] = []
dip_values: list[int] = []

UPDATE_ALL: bool = False
upload_lock: threading.Lock = threading.Lock()


try:
    with open("server_version.txt", "r") as file:
        version: str = file.read().strip().split("=")[1]
except FileNotFoundError:
    version: str = "0.0.0"

try:
    with open("esp_version.txt", "r") as file:
        esp_version: Dict[str, str] = json.load(file)
except FileNotFoundError:
    esp_version: Dict[str, str] = {}

os.makedirs(UPLOAD_FOLDER, exist_ok=True)


def log_event(msg: str) -> None:
    timestamp: str = datetime.now().strftime("[%Y-%m-%d %H:%M:%S]")
    entry: str = f"{timestamp} {msg}"
    log_buffer.append(entry)
    if len(log_buffer) > 100:
        log_buffer.pop(0)
    print(entry)


def update_esp_version_files(dip_values: List[str], version: str) -> None:
    global esp_version

    try:
        with open("esp_version.txt", "r") as file:
            existing_esp_versions: Dict[str, str] = json.load(file)
    except FileNotFoundError:
        existing_esp_versions = {}

    for dip_value in dip_values:
        existing_esp_versions[dip_value] = version

    esp_version = existing_esp_versions

    try:
        with open("esp_version.txt", "w") as file:
            json.dump(existing_esp_versions, file, indent=4)
    except FileNotFoundError:
        log_event("[ERROR] cannot update esp_version.txt: FileNotFound")


def generate_sha256(file_path: str) -> str:
    sha256_hash = hashlib.sha256()
    with open(file_path, "rb") as f:
        for byte_block in iter(lambda: f.read(4096), b""):
            sha256_hash.update(byte_block)
    return sha256_hash.hexdigest()


log_event(f"[SERVER STARTED] server version: {version}")


@app.route("/")
def index():
    return render_template("index.html", version=version, esp_version=esp_version)


@app.route("/upload", methods=["POST", "GET"])
def upload_firmware():
    global version, dip_values, UPDATE_ALL

    if request.method == "GET":
        return render_template("upload.html", version=version, esp_version=esp_version)

    acquired: bool = upload_lock.acquire(blocking=False)

    if not acquired:
        message: str = "Another upload is in progress. Please wait."
        if request.accept_mimetypes.accept_html:
            return render_template("error.html", message=message)
        return message + "\n", 503

    file = request.files.get("file")
    if not file:
        message: str = "No file found."
        upload_lock.release()
        if request.accept_mimetypes.accept_html:
            return render_template("error.html", message=message)
        return message + "\n", 400

    if not file.filename.lower().endswith(".bin"):
        message: str = "Only .bin file is allowed."
        upload_lock.release()
        if request.accept_mimetypes.accept_html:
            return render_template("error.html", message=message)
        return message + "\n", 400

    new_version: str = request.form.get("version")
    if new_version:
        version = new_version
        with open("version.txt", "w") as f:
            f.write(f"version={version}")

    given_dip_values: List[str] = request.form.getlist("dip_value")
    if not given_dip_values:
        UPDATE_ALL = True
    else:
        dip_values = given_dip_values

    save_path: str = os.path.join(UPLOAD_FOLDER, LATEST_BIN)
    file.save(save_path)

    sha256: str = generate_sha256(save_path)
    with open(os.path.join(UPLOAD_FOLDER, "firmware.sha256"), "w") as f:
        f.write(sha256)

    msg: str
    if UPDATE_ALL:
        msg = (
            f"[UPLOAD] New Firmware uploaded, version: {version}, "
            f"DIP-Values: ALL, SHA256: {sha256}"
        )
    else:
        msg = (
            f"[UPLOAD] New Firmware uploaded, version: {version}, "
            f"DIP-Values: {', '.join(dip_values)}, SHA256: {sha256}"
        )
    log_event(msg)

    log_msg: str = (
        f"Firmware uploaded successfully to DIP-Values: "
        f"{', '.join(dip_values)}. Version set to {version}"
    )

    upload_lock.release()
    if request.accept_mimetypes.accept_html:
        return render_template("success.html", message=log_msg)
    return log_msg + "\n", 200


@app.route("/firmware.bin")
def firmware():
    return send_from_directory(UPLOAD_FOLDER, LATEST_BIN)


@app.route("/update", methods=["POST"])
def update():
    global version
    device_version: str = request.headers.get("Version-ID")
    dip_value: str = request.headers.get("DIP-Value")

    if not UPDATE_ALL and dip_value not in dip_values:
        return jsonify({"update": False})
    if not device_version:
        return jsonify({"update": False})
    if device_version.strip() == version.strip():
        return jsonify({"update": False})
    else:
        log_event(
            f"[UPDATE STARTED] DIP Value: {dip_value}, device_version: {device_version}"
            f", server_version: {version}"
        )
        return jsonify(
            {"update": True, "url": f"http://{LAPTOP_IP}:5000/firmware.bin"}
        )


@app.route("/report_success", methods=["POST"])
def report_success():
    global version
    dip_value: str = request.headers.get("DIP-Value")

    sha_path: str = os.path.join(UPLOAD_FOLDER, "firmware.sha256")
    sha256: str = "N/A"
    if os.path.exists(sha_path):
        with open(sha_path, "r") as f:
            sha256 = f.read().strip()

    log_event(
        f"[UPDATE COMPLETE] DIP Value: {dip_value}, device_version: {version}"
        f", SHA256: {sha256}"
    )
    update_esp_version_files([dip_value] if dip_value else [], version)
    return jsonify({"success_code": 200, "current_version": version})


@app.route("/get_version", methods=["GET"])
def get_version():
    return f"Current Server Version: {version}\n"


@app.route("/logs")
def get_logs():
    return render_template("logs.html", logs=log_buffer)


if __name__ == "__main__":
    app.run("0.0.0.0", port=5000, threaded=True)
