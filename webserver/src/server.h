#ifndef SERVER_H
#define SERVER_H
#include "WiFi.h"
#include <LittleFS.h>
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include "secrets.h"

void loadFSJavaScript();
void loadFSCSS();
void loadFSHTML();
void loadFSStatic();
long getSeconds();
long getMinutes();

const char* ssid = WIFI_SSID;
const char* password =  WIFI_PASSWORD;

#define FORMAT_LITTLEFS_IF_FAILED   true
//flag to use from web update to reboot the ESP
bool shouldReboot = false;

#endif
