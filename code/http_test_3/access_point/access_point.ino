#include <WiFi.h>

const char* ap_ssid = "ESP-AP";
const char* ap_password = "12345678";

IPAddress local_ip(192, 168, 0, 10);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet_mask(255, 255, 255, 0);

void setup() {
  // connect to AP Netowrk
  Serial.begin(115200);
  WiFi.softAPConfig(local_ip, gateway, subnet_mask);
  WiFi.softAP(ap_ssid, ap_password, 1, 0, 9);
  Serial.print("Access point started with IP Address ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  int numClients = WiFi.softAPgetStationNum();
  Serial.print("Connected clients: ");
  Serial.println(numClients);
  
  delay(2000);
}