#ifndef SERVER_SETUP_H
#define SERVER_SETUP_H

#include <LittleFS.h>

#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"

void setupServer();

extern AsyncWebServer server;

#endif
