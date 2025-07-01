#include <WiFi.h>

bool configured = false;
bool isEnterprise = false;

const String ENTERPRISE_WIFI[] = {
  "iitmwifi",
};
const int NO_ENTERPRISE_WIFI = sizeof(ENTERPRISE_WIFI)/sizeof(ENTERPRISE_WIFI[0]);

String ssid;
String password;
String username;

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

    isEnterprise = false;
    for(int i = 0; i < NO_ENTERPRISE_WIFI; i++) {
      if (ssid == ENTERPRISE_WIFI[i]) {
        isEnterprise = true;
        break;
      }
    }

    if (isEnterprise) {
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
      WiFi.begin(ssid.c_str(), WPA2_AUTH_PEAP, username.c_str(), password.c_str()); // convert to c string before sending connection
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
      WiFi.disconnect();
    }
  }
}