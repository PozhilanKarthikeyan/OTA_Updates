#include <Arduino.h>
#include <WiFi.h>

bool configured = false;

String ssid;
String password;

void setup() {
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.println("Initiating WiFi Connection");

}

void loop() {
  
  if (!configured) {

    // getting user input 
    Serial.println("Enter SSID: ");
    while (!Serial.available()) delay(10);
    ssid = Serial.readStringUntil('\n');
    ssid.trim(); // remove whitespace

    Serial.println("Enter Password: ");
    while (!Serial.available()) delay(10);
    password = Serial.readStringUntil('\n');
    password.trim(); // remove whitespace

    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid.c_str(), password.c_str()); // convert to c string before sending connection

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
    }
    else {
      Serial.println("Connection failed.");
    }

    configured = true;

  }

  // the following code is only for testing purposes
  else {
    Serial.println("Do you want to configure again (yes/no): ");
    while (!Serial.available()) delay(10);
    String userInput = Serial.readStringUntil('\n');

    if (userInput == "yes"){
      configured = false;
    }
  }

}