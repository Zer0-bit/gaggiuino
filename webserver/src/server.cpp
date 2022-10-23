#include <WiFi.h>
#include "server.h"



void setup(){
    Serial.begin(115200);

    if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LITTLEFS Mount Failed");
        return;
    }

    WiFi.begin(wifi_ssid, wifi_password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }

    Serial.print("Connection successful, server address: ");
    Serial.println(WiFi.localIP());

    serverAddSseHandler();

    //loading webserver frontend assets
    loadFSHTML();
    loadFSCSS();
    loadFSJavaScript();
    loadFSStatic();

    // firmwareUpdate();
    // Handle Web server Events
    sseHandleEvents();
    serverBeginServe();
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

        // Send Events to the Web server with the Sensor Readings
        sseSend("ping",NULL);
        sseSend(String(getSeconds()).c_str(),"seconds");
        sseSend(String(getMinutes()).c_str(),"minutes");
        sseSend(String(temp).c_str(),"temp");
        sseSend(String(press).c_str(),"press");
        sseSend(String(flow).c_str(),"flow");
        sseSend(String(weight).c_str(),"weight");

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
