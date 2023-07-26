#include "api_static_files.h"
#include <LittleFS.h>
#include "../../log/log.h"

void setupStaticFiles(AsyncWebServer& server) {

  server.serveStatic("/", LittleFS, "/")
    .setDefaultFile("index.html");
}
