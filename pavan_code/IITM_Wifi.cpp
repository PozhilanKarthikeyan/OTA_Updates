#include <WiFi.h>

const char* ssid = "iitmwifi";
const char* identity = "ee24b048";
const char* username = "ee24b048";
const char* password = "pxA5SkzSu84";

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.disconnect(true);     // Disconnect if already connected
  WiFi.mode(WIFI_STA);       // Set WiFi to Station mode

  // WPA2 Enterprise connection
  wl_status_t status = WiFi.begin(
    ssid,
    WPA2_AUTH_PEAP,
    identity,
    username,
    password
  );

  Serial.println("Connecting to WiFi...");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    delay(10000);
  } else {
    Serial.print(".");
    delay(1000);
  }
}
