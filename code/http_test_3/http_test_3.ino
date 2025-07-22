/*

THIS IS AN EXAMPLE CODE 

to update the code,
ip addr show and replace the ip here and in update.py 
generate a bin file and put it on firmware and name it firmware.bin
python3 update.py
curl -X POST -F "file=@firmware/firmware.bin" http://{your_ip}:5000/upload

*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <HTTPUpdate.h>
#include "esp_wifi.h"
#include "esp_system.h"

// bool variables to check conditions
bool isEnterprise = false;
bool isLooped = false;
bool isOTA = false;
bool isSeriesTitlePrinted = false;

// enterprise wifi details
const String ENTERPRISE_WIFI[] = {
  "iitmwifi",
};
const int NO_ENTERPRISE_WIFI = sizeof(ENTERPRISE_WIFI)/sizeof(ENTERPRISE_WIFI[0]);

// WiFi details
String ssid;
String password;
String username;

// Laptop IP
const char* LAPTOP_IP = "192.168.0.102";

// Device Name
const char* deviceName = "esp32_001";

// webserver URL triggers
const String updateTriggerURL = "http://" + String(LAPTOP_IP) + ":5000/update";

// update check variables
unsigned long lastUpdateCheck = millis();

// THE BELOW VARIABLES ARE NOT REQUIRED FOR A BASIC OTA/USB UPLOAD
// for looping in loop()
unsigned long lastPrintTime = millis();
int n = 0;

void checkForUpdate();
void performUpdate();
String getMAC();

void setup() {
  
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  WiFi.mode(WIFI_STA);
  Serial.println("Initiating WiFi Connection");

  // scan wifi networks
  WiFi.disconnect();
  delay(100);

  int noOfNetworks = WiFi.scanNetworks();

  if (noOfNetworks == 0) {
    Serial.println("No networks found");
  }
  else {
    Serial.print("Networks found with SSID: ");
    for (int i = 0; i < noOfNetworks; i++) {
      Serial.print(WiFi.SSID(i).c_str());
      Serial.print(", ");
    }

    Serial.println();
  }

  // getting user input 
  Serial.println("Enter SSID: ");
  while (!Serial.available()) delay(10);
  ssid = Serial.readStringUntil('\n');
  ssid.trim(); // remove whitespace

  // check for enterprise networks
  isEnterprise = false;
  for(int i = 0; i < NO_ENTERPRISE_WIFI; i++) {
    if (ssid == ENTERPRISE_WIFI[i]) {
      isEnterprise = true;
      break;
    }
  }

  // proceed with 2 actions based on isEnterprise or not
  if (!isEnterprise) {
    Serial.println("Enter Password: ");
    while (!Serial.available()) delay(10);
    password = Serial.readStringUntil('\n');
    password.trim(); // remove whitespace

    // connect to WiFi
    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid.c_str(), password.c_str()); // convert to c string before sending connection
  }
  else {
    Serial.println("Enter Username: ");
    while (!Serial.available()) delay(10);
    username = Serial.readStringUntil('\n');
    username.trim(); // remove whitespace

    const String identity = username;

    Serial.println("Enter Password: ");
    while (!Serial.available()) delay(10);
    password = Serial.readStringUntil('\n');
    password.trim(); // remove whitespace
    
    // connect to wifi
    Serial.print("Connecting to WiFi");
    wl_status_t status = WiFi.begin(
      ssid.c_str(),
      WPA2_AUTH_PEAP,
      identity.c_str(),
      username.c_str(),
      password.c_str()
    );
  }

  // restrict the no of connection attempts
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40){
    delay(500);
    Serial.print('.');
    attempts++;
  }

  Serial.println();

  // print success message
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected to WIFI with SSID ");
    Serial.print(ssid);
    Serial.print(" ,with IP address ");
    Serial.print(WiFi.localIP());
    Serial.print(" and with MAC address ");
    Serial.print(getMAC());
    Serial.println(); 

  }
  else {
    Serial.print("WiFi connection failed. Rebooting.");
    ESP.restart();
  }
}

void loop() {

 if (WiFi.status() == WL_CONNECTED) {

  unsigned long currentTime = millis();

  // check for update

  if (currentTime - lastUpdateCheck > 10000) {
    checkForUpdate();
    lastUpdateCheck = currentTime;
  }

  // main code: generate square series 
  // SKIP IF YOU DON'T WANT THIS FUNCTIONALITY


  if (currentTime - lastPrintTime > 1000) {
    
    Serial.println(n*n);
    n++;

    lastPrintTime = currentTime;
    
    } 
  }
}

// in enterprise MAC may get randomised so we get the true mac
String getMAC() {
  uint8_t mac[6];
  esp_wifi_get_mac(WIFI_IF_STA, mac);  // Get base MAC
  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(macStr);
}

void checkForUpdate(){

  HTTPClient http;
  http.begin(updateTriggerURL);

  String deviceID = getMAC();  // Use MAC address as unique ID
  http.addHeader("X-Device-ID", deviceID);
  
  int httpCode = http.POST("");

  if (httpCode == 200) {
    String payload = http.getString();
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print("JSON parsing failed: ");
      Serial.println(error.c_str());
      return;
    }

    bool update = doc["update"];
    const char* url = doc["url"];

    if (update) {
      Serial.println("HTTP Update Triggered, performing OTA.");
      http.end();
      performUpdate(url);
    }
  }
  else {
    Serial.print("HTTP Error: ");
    Serial.println(httpCode);
  }

  http.end();
}

void performUpdate(const char* url) {
  Serial.println("Starting OTA update...");

  WiFiClient client;
  t_httpUpdate_return ret = httpUpdate.update(client, url);

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