#ifndef SERVER_SETUP_H
#define SERVER_SETUP_H

#include <LittleFS.h>
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"

void setupServer();

extern AsyncWebServer server;

#endif
