#include <ArduinoOTA.h>
#include <WiFi.h>

const char* ssid = "Pavan Hotspot";
const char* password = "q42ibhfe";

void start_callback_fn() {
  Serial.println("Uploading started");
}

void end_callback_fn() {
  Serial.println("Successful");
}

void progress_callback_fn(unsigned int progress, unsigned int total) {
  Serial.println(progress * 100 / total);
}

void error_callback_fn(ota_error_t error) {
  Serial.printf("Error : %u\n", error);
  switch (error) {
    case OTA_AUTH_ERROR:
      Serial.println("Auth failed");
      break;
    case OTA_BEGIN_ERROR:
      Serial.println("start failed");
      break;
    case OTA_CONNECT_ERROR:
      Serial.println("connection failed");
      break;
    case OTA_RECEIVE_ERROR:
      Serial.println("receive failed");
      break;
    case OTA_END_ERROR:
      Serial.println("end failed");
      break;
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.waitForConnectResult(10000) != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println();
  Serial.println("Connected");
  Serial.println(WiFi.localIP());

  ArduinoOTA.onStart(&start_callback_fn);
  ArduinoOTA.onEnd(&end_callback_fn);
  ArduinoOTA.onProgress(&progress_callback_fn);
  ArduinoOTA.onError(&error_callback_fn);
  ArduinoOTA.setPassword("mysecurepassword");

  ArduinoOTA.begin();
}

// Fibonacci state
int n1 = 0;
int n2 = 1;
unsigned long lastPrint = 0;
const unsigned long interval = 1000; 

void loop() {
  ArduinoOTA.handle();

  if (millis() - lastPrint >= interval) {
    Serial.println(n1);
    int next = n1 + n2;
    n1 = n2;
    n2 = next;
    lastPrint = millis();
  }
}
