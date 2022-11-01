#include <Arduino.h>
#include <LittleFS.h>
#include "server/server_setup.h"
#include "wifi/wifi_setup.h"
#include "server/websocket.h"

// File system

void initFS();

void setup() {
  Serial.begin(115200);
  initFS();
  wifiSetup();
  setupServer();
}

static long nextTimer;
static long startTime = millis();
void loop() {
  wsCleanup();

  if (millis() > nextTimer) {
    float rTemp = random(920, 1000) / 10.f;
    float rFlow = random(30, 40) / 10.f;
    float rPress = random(50, 120) / 10.f;
    float rWeight = random(0, 300) / 10.f;
    wsSendSensorStatesToClients(millis() - startTime, rTemp, rPress, rFlow, rWeight);
    nextTimer = millis() + 200;
  }
}

// Initialize SPIFFS
void initFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}
