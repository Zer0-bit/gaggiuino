#include "filesystem.h"
#include <LittleFS.h>
#include "../log/log.h"

// -----------------------------------------
// --------- Initialize SPIFFS -------------
// -----------------------------------------
void initFS() {
  if (!LittleFS.begin(true)) {
    LOG_INFO("An error has occurred while mounting LittleFS");
  }
  LOG_INFO("LittleFS mounted successfully");
}
