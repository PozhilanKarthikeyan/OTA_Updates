/*

THIS IS MQTT EXAMPLE CODE

replace the mqtt_server variable with your laptop ip
to update the code, generate a bin file and run pyton3 -m http.server 8000 on the directory where the bin file is located
do ip addr show to get your ip
and then do mosquitto_pub -h {laptop_ip} -t devices/esp32_001/ota/command -m update

*/

#include <WiFi.h>
#include <esp_wpa2.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include <PubSubClient.h>

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

// Laptop IP
const char* LAPTOP_IP = "192.168.228.68";

// Firmware URL
const char* firmwareURL = "http://" + LAPTOP_IP + ":8000/http_test.ino.bin"; 

// WiFi details
String ssid;
String password;
String username;

// WiFi and MQTT
WiFiClient espClient;
PubSubClient client(espClient);

const char* mqtt_server = LAPTOP_IP; // laptop IP
const char* mqtt_topic = "devices/esp32_001/ota/command";

// THE BELOW VARIABLES ARE NOT REQUIRED FOR A BASIC OTA/USB UPLOAD
// for looping in loop()
unsigned long lastPrintTime = millis();
int n = 0;

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

    // initialise mqtt server
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    connectMQTT();

  }
  else {
    Serial.println("WiFi connection failed. Rebooting.");
    ESP.restart();
  }
}

void loop() {

 if (WiFi.status() == WL_CONNECTED) {

  // loop mqtt connection to check for new topics
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  // main code: generate square series 
  // SKIP IF YOU DON'T WANT THIS FUNCTIONALITY

  unsigned long currentTime = millis();

  if (currentTime - lastPrintTime > 1000) {
    
    Serial.println(n*n);
    n++;

    lastPrintTime = currentTime;
    
    } 
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";

  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == "devices/esp32_001/ota/command") {
    if (message == "update") {
      Serial.println("✅ MQTT command received: update. Starting OTA...");
      performUpdate();
    }
  }
}

void connectMQTT(){
  while (!client.connected()) {

    // connect to MQTT server
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP32Client")) {
      Serial.println("Connected to MQTT");
      client.subscribe(mqtt_topic);
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5s");
      delay(5000);
    }
  }
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
