#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClient.h>
#include <Preferences.h>

#define LED_PIN 2
#define FIRMWARE_VERSION "1.0.9"

const char* ssid     = "iitmwifi";
const char* identity = "ee24b048";  
const char* username = "ee24b048";
const char* password = "pxA5SkzSu84";

#define FIRMWARE_URL "http://10.44.6.103:80/Working_Blink.ino.bin"
#define VERSION_URL  "http://10.44.6.103:80/version.txt"

Preferences prefs;

void connectWiFiIfNeeded() {
  if (WiFi.status() == WL_CONNECTED) return;

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);

  Serial.print("Reconnecting to WiFi");
  WiFi.begin(ssid, WPA2_AUTH_PEAP, identity, username, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi");
  Serial.println(WiFi.localIP());
}

String getStoredVersion() {
  prefs.begin("ota", true);
  String version = prefs.getString("version", "");
  prefs.end();
  return version;
}

void saveVersionToPrefs(const String& version) {
  prefs.begin("ota", false);
  prefs.putString("version", version);
  prefs.end();
}

String getServerVersion(const char* url) {
  HTTPClient http;
  WiFiClient client;

  http.begin(client, url);
  int httpCode = http.GET();
  Serial.printf("HTTP GET version.txt => code: %d\n", httpCode);

  String version = "";
  if (httpCode == 200) {
    version = http.getString();
    version.trim();
    Serial.printf("Received server version: %s\n", version.c_str());
  }

  http.end();
  return version;
}

void checkAndUpdateOTA() {
  connectWiFiIfNeeded();

  String storedVersion = getStoredVersion();
  String serverVersion = getServerVersion(VERSION_URL);

  Serial.printf("Stored version: %s\n", storedVersion.c_str());
  Serial.printf("Current version: %s\n", FIRMWARE_VERSION);
  Serial.printf("Server version: %s\n", serverVersion.c_str());

  if (serverVersion == "" || serverVersion == FIRMWARE_VERSION) {
    Serial.println("No update available.");
    return;
  }

  Serial.println("Performing OTA update...");
  WiFiClient client;
  t_httpUpdate_return ret = httpUpdate.update(client, FIRMWARE_URL);

  switch (ret) {
    case HTTP_UPDATE_OK:
      Serial.println("OTA success. Saving version and rebooting...");
      saveVersionToPrefs(serverVersion);
      delay(1000);
      ESP.restart();
      break;
    case HTTP_UPDATE_FAILED:
      Serial.printf("OTA failed (%d): %s\n",
        httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("No updates found.");
      break;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  Serial.printf("Running firmware version: %s\n", FIRMWARE_VERSION);

  checkAndUpdateOTA();  // optional: only call this periodically in loop
}

int n = 0;

void loop() {
  Serial.println(n);
  digitalWrite(LED_PIN, n % 2);
  n+=3;
  delay(2000);
}




