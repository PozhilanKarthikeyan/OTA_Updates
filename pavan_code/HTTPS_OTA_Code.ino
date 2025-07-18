#include <WiFi.h>
#include <HTTPUpdate.h>
#include <WiFiClient.h>

// === WiFi Enterprise Credentials ===
const char* ssid     = "iitmwifi";
const char* identity = "ee24b048";  // outer identity
const char* username = "ee24b048";  // inner identity (for PEAP/MSCHAPv2)
const char* password = "pxA5SkzSu84";

// === OTA Binary URL ===
#define OTA_URL "http://10.42.64.83/Blink.ino.bin"  // Change IP if needed

// === Connect to WPA2-Enterprise WiFi ===
void wifi_connect_enterprise() {
  WiFi.disconnect(true);     // clear previous config
  WiFi.mode(WIFI_STA);       // station mode

  Serial.print("Connecting to WPA2-Enterprise WiFi");

  wl_status_t status = WiFi.begin(
    ssid,
    WPA2_AUTH_PEAP,
    identity,
    username,
    password
  );

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// === Start OTA using HTTP Update ===
void start_ota() {
  Serial.println("Starting OTA update...");

  WiFiClient client;
  t_httpUpdate_return ret = httpUpdate.update(client, OTA_URL);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("OTA failed. Error (%d): %s\n",
        httpUpdate.getLastError(),
        httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("No OTA update available.");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("OTA successful! Rebooting...");
      break;
  }
}

// === Arduino Entry Point ===
void setup() {
  Serial.begin(115200);
  delay(1000);

  wifi_connect_enterprise();  // connect to WPA2-Enterprise
  delay(3000);                // wait before OTA start

  start_ota();                // perform OTA update
}

void loop() {
  // Optional: Add watchdog / LED blink etc.
}
