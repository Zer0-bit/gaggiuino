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
void firmwareUpdate();
void espReboot();
long getSeconds();
long getMinutes();

//flag to use from web update to reboot the ESP
bool shouldReboot = false;

#endif