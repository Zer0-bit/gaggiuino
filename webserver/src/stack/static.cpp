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

// Might return to this lambda later bu it's possibly making things unecessary abstract
// void loadFSFile(String filePath, String fileType) {
//     server.on([=](char& filePath){return filePath;}, HTTP_GET, [](AsyncWebServerRequest *request)
//         {
//             request->send(LittleFS, [=](char& filePath){return filePath;}, [=](char& fileType){return fileType;});
//         }
//     );
// }

void loadFSJavaScript() {
    //JAVASCRIPT-JS
    server.on("/assets/js/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/assets/js/bootstrap.min.js", "text/javascript");
    });
    server.on("/assets/js/chart.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/assets/js/chart.js", "text/javascript");
    });
    server.on("/assets/js/custom.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/assets/js/custom.js", "text/javascript");
    });
    server.on("/assets/js/easypiechart-data.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/assets/js/easypiechart-data.js", "text/javascript");
    });
    server.on("/assets/js/easypiechart.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/assets/js/easypiechart.js", "text/javascript");
    });
    server.on("/assets/js/jquery-1.10.2.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/assets/js/jquery-1.10.2.js", "text/javascript");
    });
    server.on("/assets/js/uiuiui-scripts.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/assets/js/uiuiui-scripts.js", "text/javascript");
    });
}

void loadFSCSS() {
    // CSS
    server.on("/assets/css/bootstrap-theme.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/assets/css/bootstrap-theme.min.css", "text/css");
    });
    server.on("/assets/css/bootstrap.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/assets/css/bootstrap.css", "text/css");
    });
    server.on("/assets/css/custom-styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/assets/css/custom-styles.css", "text/css");
    });
    server.on("/assets/css/font-awesome.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/assets/css/font-awesome.css", "text/css");
    });
    server.on("/assets/css/open_sans.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/assets/css/open_sans.css", "text/css");
    });
}

void loadFSHTML() {
    server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/index.html", "text/html");
    });
}

void loadFSStatic() {
    server.on("/assets/img/GAGGIUINO_ICON_transp.png", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(LittleFS, "/assets/img/GAGGIUINO_ICON_transp.png", "text/plain");
    });
    server.on("/assets/img/uiui-light.png", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(LittleFS, "/assets/img/uiui-light.png", "text/plain");
    });
    server.on("/assets/img/uiui-dark.png", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(LittleFS, "/assets/img/uiui-dark.png", "text/plain");
    });
}
void sseSend(String evnt, char* variable) {
    events.send(evnt.c_str(),variable,millis());
}
