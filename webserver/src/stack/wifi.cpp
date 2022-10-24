#include "wifi.h"

void wifiInit() {
    WiFi.begin(wifi_ssid, wifi_password);
}

void wifiConnect() {
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }
    Serial.print("Connection successful, server address: ");
    Serial.println(WiFi.localIP());
}