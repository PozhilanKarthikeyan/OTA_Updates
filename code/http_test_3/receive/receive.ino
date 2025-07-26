/*

THIS IS AN EXAMPLE CODE 

dir structure

firmware/firmware.bin
update.py
version.txt

to update the code,
ip addr show and replace the ip here and in update.py 
generate a bin file and put it on firmware and name it firmware.bin
python3 update.py
curl -X POST -F "file=@firmware/firmware.bin" -F "version="{your_version}"  http://192.168.0.50:5000/upload

*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <HTTPUpdate.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include <Preferences.h>

// bool variables to check conditions
bool isEnterprise = false;
bool isLooped = false;
bool isOTA = false;
bool isSeriesTitlePrinted = false;

// WiFi details
const char* ssid = "ESP-AP";
const char* password = "12345678";

// Laptop IP
const char* LAPTOP_IP = "192.168.0.50";

// firmware Version
String version = "";

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

// preferences
Preferences prefs;

void setup() {

  prefs.begin("ota", true);
  version = prefs.getString("device_version", "0.0.0");
  prefs.end();
  
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  WiFi.mode(WIFI_STA);
  Serial.print("Initiating WiFi Connection");

  WiFi.disconnect();
  delay(100);

  // connect to wifi
  WiFi.begin(ssid, password);

  // restrict the no of connection attempts
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20){
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
    Serial.print(" ,with MAC address ");
    Serial.print(getMAC());
    Serial.print(" and version ");
    Serial.print(version);
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
  http.addHeader("Version-ID", version);
  
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
    const char* reason = doc["error"];
    
    if (update) {
      Serial.println("HTTP Update Triggered, performing OTA.");
      http.end();
      performUpdate(url);
    }

  }

  http.end();
}

void performUpdate(const char* url) {
  Serial.println("Starting OTA update...");

  WiFiClient client;
  httpUpdate.rebootOnUpdate(false);
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

      HTTPClient client;
      client.begin("http://" + String(LAPTOP_IP) + ":5000/report_success");
      client.addHeader("X-Device-ID", getMAC());
      int httpCode = client.POST("");
      
      if(httpCode == 200) {
        String response = client.getString();
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, response);

        if (error) {
          Serial.print("JSON parsing failed: ");
          Serial.println(error.c_str());
        } 
        else {
          const char* received_version = doc["current_version"];
          const int success_code = doc["success_code"];
          Serial.print("Updated to: ");
          Serial.print(received_version);
          Serial.println();
          prefs.begin("ota", false);
          prefs.putString("device_version", received_version);
          prefs.end();
        }
      } 
      else {
        Serial.print("Failed to report success. HTTP code: ");
        Serial.println(httpCode);
      }

      client.end();
      ESP.restart();
      break;

  }
}