/*

THIS IS AN EXAMPLE CODE ON FIBONACCI SERIES / SQUARE SERIES BASED ON OTA OR USB UPLOAD  

DOCUMENTATION ON HOW TO UPLOAD THE CODE VIA OTA AFTER UPLOADING IT USING USB (ENTERPRISE NETWORKS):

YOU CAN DIRECTLY USE UPLOAD BUTTON IF YOU ARE USING LOCAL NETWORK
1) After you made your changes, go to Sketch > Export Compiled Binary on Arduino IDE or 
build the project in PlatformIO to get a file like {file_name}.ino.bin
2) In case of Arduino IDE you have to go inside build folder to find the required file. In PlatformIO find the bin file inside pio/build
3) Locate espota.py using find ~ -name espota.py
4) Run python3 {path_to_espota.py} -i {your_wifi_IP} -p 3232 -auth={your_OTA_Password} --file {path_to_the_bin_file}
YOU CAN ALSO FIND THE VIDEO IN media

*/

#include <Preferences.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <esp_wpa2.h>

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

// OTA host details
const String OTA_HOSTNAME = "esp32test";
const String OTA_PASSWORD = "12345678";

// WiFi details
String ssid;
String password;
String username;

// for checking OTA or USB upload
Preferences preferences;

// THE BELOW VARIABLES ARE NOT REQUIRED FOR A BASIC OTA/USB UPLOAD
// for looping in loop()
unsigned long lastPrintTime = millis();

// Square Series variables
int square_n = 0;

// Fiboncacci Series variables
int fibo_a = 0;
int fibo_b = 1;

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

  // configure OTA and print success message
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected to WIFI with SSID ");
    Serial.print(ssid);
    Serial.print(" and with IP address ");
    Serial.print(WiFi.localIP());
    Serial.println();
    
    // get the isOTA variable state (upload is a folder where isOTA is a variable in preferences)
    preferences.begin("upload", true);
    isOTA = preferences.getBool("isOTA", false);
    preferences.end();

    // print upload via OTA or USB
    if(isOTA) {
      Serial.println("This code was uploaded via OTA.");
    }
    else {
      Serial.println("This code was uploaded via USB.");
    }

    // reset preferences
    preferences.begin("upload", false);
    preferences.putBool("isOTA", false); 
    preferences.end();

    // configure OTA
    ArduinoOTA.setHostname(OTA_HOSTNAME.c_str());
    ArduinoOTA.setPassword(OTA_PASSWORD.c_str());
    
    // OTA event handlers
    ArduinoOTA.onStart([]() {
      
      // set isOTA upload to true
      preferences.begin("upload", false);
      preferences.putBool("isOTA", true);
      preferences.end();

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

  // main code: generate fibonacci series for OTA upload and square series for USB upload
  // SKIP IF YOU DON'T WANT THIS FUNCTIONALITY

  unsigned long currentTime = millis();

  if (currentTime - lastPrintTime > 1000) {
    
    // update lastPrintTime
    lastPrintTime = currentTime;

    if (isOTA) {

      if (!isSeriesTitlePrinted) {
        Serial.println("Fibonacci Series:");
        isSeriesTitlePrinted = true;

        // print the initial 2 terms
        Serial.println(fibo_a);
        Serial.println(fibo_b);
      }

      // main Fibonacci series logic
      int fibo_c = fibo_a + fibo_b;
      Serial.println(fibo_c);
      fibo_a = fibo_b;
      fibo_b = fibo_c;

    }

    else {

      if (!isSeriesTitlePrinted) {
        Serial.println("Square Series:");
        isSeriesTitlePrinted = true;
      }

      // main Square series logic
      Serial.println(square_n * square_n);
      square_n++;
    
    }
    
  }

 }
  
}