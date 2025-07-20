/*

THIS IS AN EXAMPLE CODE 

to update the code,
ip addr show and replace the ip here and in update.py 
generate a bin file and put it on firmware and name it firmware.bin
python3 update.py
curl -X POST -F "file=@firmware/firmware.bin" http://{your_ip}:5000/upload

*/

#include <WiFi.h>
#include <esp_wpa2.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>

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
const char* LAPTOP_IP = "10.42.67.196";

// Device Name
const char* deviceName = "esp32_001";

// webserver URL triggers
const String updateTriggerURL = "http://" + String(LAPTOP_IP) + ":5000/update";
const String firmwareURL      = "http://" + String(LAPTOP_IP) + ":5000/firmware.bin";  

// update check variables
unsigned long lastUpdateCheck = millis();

// THE BELOW VARIABLES ARE NOT REQUIRED FOR A BASIC OTA/USB UPLOAD
// for looping in loop()
unsigned long lastPrintTime = millis();
int n = 0;

void checkForUpdate();
void performUpdate();

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

    Serial.println("Enter Password: ");
    while (!Serial.available()) delay(10);
    password = Serial.readStringUntil('\n');
    password.trim(); // remove whitespace

    Serial.print("Connecting to WiFi");
    // Configure WPA2 Enterprise
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)username.c_str(), username.length());
    esp_wifi_sta_wpa2_ent_set_username((uint8_t*)username.c_str(), username.length());
    esp_wifi_sta_wpa2_ent_set_password((uint8_t*)password.c_str(), password.length()); 
    esp_wifi_sta_wpa2_ent_enable();

    // connect to WiFi
    WiFi.begin(ssid.c_str()); // use c strings
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
    Serial.print(" and with IP address ");
    Serial.print(WiFi.localIP());
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

void checkForUpdate(){

  HTTPClient http;
  http.begin(updateTriggerURL);
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

    if (update) {
      Serial.println("HTTP Update Triggered, performing OTA.");
      http.end();
      performUpdate();
    }
  }
  else {
    Serial.print("HTTP Error: ");
    Serial.println(httpCode);
  }

  http.end();
}

void performUpdate() {
  // HTTP OTA initialisation
  Serial.println("OTA is starting...");

  WiFiClient client;

  HTTPClient https;

  Serial.println("Connecting to: " + String(firmwareURL));
  if (https.begin(client, firmwareURL)) {
    int httpCode = https.GET();

    if (httpCode == HTTP_CODE_OK) {
      int contentLength = https.getSize();
      bool canBegin = Update.begin(contentLength);

      if (canBegin) {
        WiFiClient* updateClient = https.getStreamPtr();
        size_t written = Update.writeStream(*updateClient);

        if (written == contentLength) {
          Serial.println("Written : " + String(written) + " successfully");
        } else {
          Serial.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?");
        }

        if (Update.end()) {
          Serial.println("OTA done!");
          if (Update.isFinished()) {
            Serial.println("Update successfully completed. Rebooting...");
            
            ESP.restart();
          } else {
            Serial.println("Update not finished? Something went wrong!");
          }
        } else {
          Serial.println("Error Occurred. Error #: " + String(Update.getError()));
        }

      } else {
        Serial.println("Not enough space to begin OTA");
      }

    } else {
      Serial.println("GET request failed. HTTP Code: " + https.errorToString(httpCode));
    }

    https.end();
  } else {
    Serial.println("HTTPS connection failed");
  }
}