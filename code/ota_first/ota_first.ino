/*

DOCUMENTATION ON HOW TO UPLOAD THE CODE VIA OTA AFTER UPLOADING IT USING USB (ENTERPRISE NETWORKS):

YOU CAN DIRECTLY USE UPLOAD BUTTON IF YOU ARE USING LOCAL NETWORK

1) After you made your changes, go to Sketch > Export Compiled Binary on Arduino IDE or 
build the project in PlatformIO to get a file like {file_name}.ino.bin

2) In case of Arduino IDE you have to go inside build folder to find the required file. In PlatformIO find the bin file inside pio/build

3) Locate espota.py using find ~ -name espota.py

4) Run python3 {path_to_espota.py} -i {your_wifi_IP} -p 3232 -auth={your_OTA_Password} --file {path_to_the_bin_file}

YOU CAN ALSO FIND THE VIDEO IN media

*/

#include <ArduinoOTA.h>
#include <WiFi.h>
#include <esp_wpa2.h>

bool isEnterprise = false;
bool isLooped = false;

const String ENTERPRISE_WIFI[] = {
  "iitmwifi",
};
const int NO_ENTERPRISE_WIFI = sizeof(ENTERPRISE_WIFI)/sizeof(ENTERPRISE_WIFI[0]);

const String OTA_HOSTNAME = "esp32test";
const String OTA_PASSWORD = "12345678";

String ssid;
String password;
String username;

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

  isEnterprise = false;
  for(int i = 0; i < NO_ENTERPRISE_WIFI; i++) {
    if (ssid == ENTERPRISE_WIFI[i]) {
      isEnterprise = true;
      break;
    }
  }

  if (!isEnterprise) {
    Serial.println("Enter Password: ");
    while (!Serial.available()) delay(10);
    password = Serial.readStringUntil('\n');
    password.trim(); // remove whitespace

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

    WiFi.begin(ssid.c_str()); // use c strings
  }

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40){
    delay(500);
    Serial.print('.');
    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected to WIFI with SSID ");
    Serial.print(ssid);
    Serial.print(" and with IP address ");
    Serial.print(WiFi.localIP());
    Serial.println(". This code is updated via USB upload.");

    // configure OTA
    ArduinoOTA.setHostname(OTA_HOSTNAME.c_str());
    ArduinoOTA.setPassword(OTA_PASSWORD.c_str());
    
    // OTA event handlers
    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else { // U_SPIFFS
        type = "filesystem";
      }
      Serial.println("Start updating " + type);
    });

    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });

    ArduinoOTA.begin();
    Serial.print("OTA configured with IP address ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("Connection failed. OTA Not available.");
  }

}

void loop() {

 if (WiFi.status() == WL_CONNECTED) {
  ArduinoOTA.handle();

  // main code

  if (!isLooped) {
    Serial.println("This code is uploaded via OTA");
    Serial.println("test35");
    isLooped = true;

  }
 }
  
}