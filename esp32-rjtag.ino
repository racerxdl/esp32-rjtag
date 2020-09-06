// #define DEBUG

#define BAUD_RATE 921600

#include <ArduinoOTA.h>
#include "storage.h"
#include "programmer.h"
#include "wifi.h"
#include "webserver.h"

void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);
    Serial.println("[START]");
    InitStorage();
    SetupWiFi();
    ProgrammerInit();
    InitWebServer();

    Serial.println("[RDY]");
}


void loop() {
  ArduinoOTA.handle();
  ProgrammerLoop();
  if (isWifiProgramming()) {
    delay(1000); // Give enough time to programmer work
  }
}
