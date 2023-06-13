#include "api_static_files.h"
#include <LittleFS.h>

void setupStaticFiles(AsyncWebServer& server) {
  server.serveStatic("/", LittleFS, "/");
}
