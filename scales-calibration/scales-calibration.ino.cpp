# 1 "C:\\Users\\Arkwolf\\AppData\\Local\\Temp\\tmptgg8a5ji"
#include <Arduino.h>
# 1 "A:/Coding/gaggiuino/scales-calibration/scales-calibration.ino"
#include <EasyNextionLibrary.h>
#include <HX711_2.h>

#include "ADS1X15.h"

#define relayPin PA15
#define LOADCELL_1_DOUT_PIN PB8
#define LOADCELL_2_DOUT_PIN PB9
#define LOADCELL_1_SCK_PIN PB0
#define LOADCELL_2_SCK_PIN PB1
#define UART_LCD Serial2

HX711_2 loadcell;

#if defined SINGLE_HX711_BOARD
unsigned char scale_clk = OUTPUT;
#else
unsigned char scale_clk = OUTPUT_OPEN_DRAIN;
#endif

float calibration_factor_lc1 = 4000;
float calibration_factor_lc2 = 4000;

EasyNex myNex(UART_LCD);
void setup();
void loop();
void trigger0();
void trigger1();
void trigger2();
void trigger3();
void trigger4();
void trigger5();
#line 26 "A:/Coding/gaggiuino/scales-calibration/scales-calibration.ino"
void setup() {
    myNex.begin(115200);
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, LOW);

    while (myNex.readNumber("initCheck") != 100) {
        delay(600);
    }
    loadcell.begin(LOADCELL_1_DOUT_PIN, LOADCELL_2_DOUT_PIN, LOADCELL_1_SCK_PIN, LOADCELL_2_SCK_PIN, 128, scale_clk);
    loadcell.set_scale();
    loadcell.tare();
}

void loop() {
    static unsigned long timer = millis();
    float values[2];
    static float previousFactor1, previousFactor2;

    myNex.NextionListen();

    if (calibration_factor_lc1 != previousFactor1 || calibration_factor_lc2 != previousFactor2) {
        loadcell.set_scale(calibration_factor_lc1, calibration_factor_lc2);
        previousFactor1 = calibration_factor_lc1;
        previousFactor2 = calibration_factor_lc2;
    }

    if (myNex.currentPageId == 0) {
        if (millis() > timer) {
            loadcell.get_units(values);
            myNex.writeStr("t0.txt", String(values[0], 2));
            myNex.writeStr("t1.txt", String(values[1], 2));

            myNex.writeStr("t2.txt", String(calibration_factor_lc1, 2));
            myNex.writeStr("t3.txt", String(calibration_factor_lc2, 2));

            timer = millis() + 100.0;
        }
    }
}

void trigger0() {
    calibration_factor_lc1 -= myNex.readNumber("n1.val");
}

void trigger1() {
    calibration_factor_lc1 += myNex.readNumber("n1.val");
}

void trigger2() {
    calibration_factor_lc2 -= myNex.readNumber("n2.val");
}

void trigger3() {
    calibration_factor_lc2 += myNex.readNumber("n2.val");
}
void trigger4() {
}

void trigger5() {
    calibration_factor_lc1 = 4000;
    calibration_factor_lc2 = 4000;
}