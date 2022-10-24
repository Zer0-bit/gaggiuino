#include "static.h"

AsyncWebServer server(80);
// Init SSE ahndler
AsyncEventSource events("/");

void serverAddSseHandler() {
    server.addHandler(&events);
}

void sseHandleEvents() {
    events.onConnect([](AsyncEventSourceClient *client){
        if(client->lastId()){
            Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
        }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 200ms
    client->send("hello!", NULL, millis(), 200);
    });
}

void serverBeginServe() {
    server.begin();
}

// Checking wether the filesystem can be mounted
void fsMount() {
    if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LITTLEFS Mount Failed");
        return;
    }
}

void loadFSFiles() {
    server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/index.html", "text/html");
    });
    server.serveStatic("/", LittleFS, "/");
}


void sseSend(String evnt, char* variable) {
    events.send(evnt.c_str(),variable,millis());
}
