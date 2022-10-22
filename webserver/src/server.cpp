#include "server.h"

const char* ssid = WIFI_SSID;
const char* password =  WIFI_PASSWORD;

#define FORMAT_LITTLEFS_IF_FAILED   true

AsyncWebServer server(80);
// Create an Event Source on /
AsyncEventSource events("/");


void setup(){
    Serial.begin(115200);

    if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LITTLEFS Mount Failed");
        return;
    }

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }

    Serial.print("Connection successful, server address: ");
    Serial.println(WiFi.localIP());

    server.addHandler(&events);

    loadFSHTML();
    loadFSCSS();
    loadFSJavaScript();
    loadFSStatic();

    // firmwareUpdate();
    // Handle Web Server Events
    events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
        Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }

    // send event with message "hello!", id current millis
    // and set reconnect delay to 200ms
    client->send("hello!", NULL, millis(), 200);
    });

    server.begin();
}

void loop(){
    static long timer;
    if ((millis() - timer) > 200) {
        int temp = random(90,93);
        int press = random(1,10);
        int flow = random(1,3);
        static int weight;
        if (weight<=45) weight += flow;
        else weight = 0;

        // Send Events to the Web Server with the Sensor Readings
        events.send("ping",NULL,millis());
        events.send(String(getSeconds()).c_str(),"seconds",millis());
        events.send(String(getMinutes()).c_str(),"minutes",millis());
        events.send(String(temp).c_str(),"temp",millis());
        events.send(String(press).c_str(),"press",millis());
        events.send(String(flow).c_str(),"flow",millis());
        events.send(String(weight).c_str(),"weight",millis());
        
        timer = millis();
    }
    // espReboot();
}

long getSeconds() {
    return millis() / 1000;
}

long getMinutes() {
    return millis() / 1000 / 60;
}

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
    // HTML
    // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    //     request->send(LittleFS, "/index.html", "text/html");
    // });
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

void firmwareUpdate() {
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request)
        {
            request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
        }
    );
 