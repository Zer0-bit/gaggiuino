#ifndef STATIC_H
#define STATIC_H

#include <LittleFS.h>
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"

#define FORMAT_LITTLEFS_IF_FAILED   true

void fsMount(void);
void serverAddSseHandler(void);
void sseHandleEvents(void);
void serverBeginServe(void);
void sseSend(String evnt, char* variable);

void loadFSFiles();

// void loadFSHTML(void);
// void loadFSCSS(void);
// void loadFSJavaScript(void);
// void loadFSStatic(void);

#endif