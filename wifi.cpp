#include "Arduino.h"
#include "ArduinoOTA.h"
#include "WiFi.h"
#include "wifi.h"
#include "storage.h"

String ssid     = "";
String password = "";
String otaPassword = "";
String hostname = "";

bool inOTA = false;

void SetupWiFi() {
  ssid = GetWifiSSID();
  password = GetWifiPassword();
  hostname = GetHostname();
  inOTA = false;
  Serial.println("[INFO] Setting up WiFi");
  Serial.print("[INFO] SSID: ");
  Serial.println(ssid);

  if (WiFi.getMode() != WIFI_STA) {
    WiFi.mode(WIFI_STA);
    delay(10);
  }

  if (WiFi.SSID() != ssid || WiFi.psk() != password) {
    Serial.println("[INFO] WiFi config changed.");
    // ... Try to connect to WiFi station.
    WiFi.begin(ssid.c_str(), password.c_str());
  } else {
    WiFi.begin();
  }

  WiFi.setHostname(hostname.c_str());

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(100);
    yield();
  }

  // Check connection
  if (WiFi.status() == WL_CONNECTED) {
    // ... print IP Address
    Serial.print("[INFO] IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[INFO] Can not connect to WiFi station. Configure using CLI");
  }

  // Start OTA server.
  ArduinoOTA.setHostname((const char *)hostname.c_str());

  ArduinoOTA.onStart([]() {
    inOTA = true;
    Serial.println("[INFO] OTA Update Start");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("[INFO] OTA Update End");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    String progressStr;
    progressStr = String((progress / (total / 100))) + String("%");
    Serial.print("[INFO] Progress: ");
    Serial.println(progressStr);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[ERROR] %u: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Start Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connection failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Error");
    else if (error == OTA_END_ERROR) Serial.println("End Fail");
  });

  ArduinoOTA.begin();
}