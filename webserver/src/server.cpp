#include "server.h"

void setup(){
    Serial.begin(115200);

    fsMount();

    wifiInit();
    wifiConnect();

    serverAddSseHandler();

    //loading webserver frontend assets
    loadFSFiles();


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
        sseSend(String(getSeconds()),"seconds");
        sseSend(String(getMinutes()),"minutes");
        sseSend(String(temp),"temp");
        sseSend(String(press),"press");
        sseSend(String(flow),"flow");
        sseSend(String(weight),"weight");
        timer = millis();
    }
    // espReboot();
}

long getSeconds() {
    return millis() /1000;
}

long getMinutes() {
    return millis() / 1000 / 60;
}
