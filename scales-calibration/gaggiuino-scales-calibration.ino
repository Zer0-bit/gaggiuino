#include <EEPROM.h>
#include <EasyNextionLibrary.h>
#include <HX711.h>

#define relayPin 8
#define LOADCELL_1_OFFSET 215
#define LOADCELL_1_OFFSET 220

HX711 loadcell_1;
HX711 loadcell_2;

// 1. HX711 circuit wiring
const int LOADCELL_DOUT_PIN_12 = 12;
const int LOADCELL_SCK_PIN_10 = 10;
const int LOADCELL_DOUT_PIN_13 = 13;
const int LOADCELL_SCK_PIN_11 = 11;

long reading_1, reading_2;
bool calibration;

//Nextion object init
EasyNex myNex(Serial);

void setup() {
  Serial.begin(115200);
  digitalWrite(relayPin, LOW);

  while (myNex.readNumber("initCheck") != 100 )
  {
    delay(600);
  }

  //Initialize library
  loadcell_1.begin(LOADCELL_DOUT_PIN_12, LOADCELL_SCK_PIN_10);
  loadcell_1.begin(LOADCELL_DOUT_PIN_13, LOADCELL_SCK_PIN_11);
}

void loop() {
  myNex.NextionListen();
  if (calibration == 1) {
    CALIBRATE:
    if (loadcell_1.wait_ready_timeout(100) && loadcell_2.wait_ready_timeout(100)) {
      loadcell_1.set_scale();
      loadcell_2.set_scale();
      delay(500);
      loadcell_1.tare();
      loadcell_2.tare();
      calibration = 0;
      myNex.writeStr("page page1");
    }else goto CALIBRATE;
  }
}

void trigger0() {
  int divider = myNex.readNumber("n0.val");
  float result1,result2;
  AGAIN:
  if (loadcell_1.wait_ready_timeout(100) && loadcell_2.wait_ready_timeout(100)) {
    reading_1 = loadcell_1.get_units(10);
    reading_2 = loadcell_2.get_units(10);
    result1 = reading_1/divider;
    result2 = reading_2/divider;
  }else goto AGAIN;
  myNex.writeStr("t0.txt",String(result1,3));
  myNex.writeStr("t1.txt",String(result2,3));
}

void trigger1() {
  calibration = 0;
}

