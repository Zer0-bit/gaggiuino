b#define SINGLE_HX711_CLOCK
#if defined(SINGLE_HX711_CLOCK)
    #include <HX711_2.h>
#else
    #include <HX711.h>
#endif

#include <EasyNextionLibrary.h>
#if defined(ARDUINO_ARCH_AVR)
  #include <EEPROM.h>
#elif defined(ARDUINO_ARCH_STM32)
  #include "ADS1X15.h"
#endif


#if defined(ARDUINO_ARCH_AVR)
    #define relayPin 8  // PB0
    #define LOADCELL_1_DOUT_PIN 12 //mcu > HX711 no 1 dout pin
    #define LOADCELL_2_DOUT_PIN 13 //mcu > HX711 no 2 dout pin
    #define LOADCELL_1_SCK_PIN 10 //mcu > HX711 no 1 sck pin
    #define LOADCELL_2_SCK_PIN 11 //mcu > HX711 no 2 sck pin
#elif defined(ARDUINO_ARCH_STM32)
    #define relayPin PB9  // PB0
    #define LOADCELL_1_DOUT_PIN  PA1
    #define LOADCELL_2_DOUT_PIN  PA2
    #define LOADCELL_1_SCK_PIN  PB0
    #define LOADCELL_2_SCK_PIN  PB1
#endif

#if defined(SINGLE_HX711_CLOCK)
HX711_2 loadcell;
#else
HX711 loadcell_1;
HX711 loadcell_2;
#endif

#if defined(ARDUINO_ARCH_AVR)
const unsigned int  EEP_SCALES_F1 = 215;
const unsigned int  EEP_SCALES_F2 = 220;
#endif
float calibration_factor_lc1 = 4000; //-7050 worked for my 440lb max scale setup
float calibration_factor_lc2 = 4000; //-7050 worked for my 440lb max scale setup

//Nextion object init
EasyNex myNex(Serial);

void setup() {
  Serial.begin(115200);
  digitalWrite(relayPin, LOW);

  while (myNex.readNumber("initCheck") != 100 )
  {
    delay(600);
  }

#if defined(SINGLE_HX711_CLOCK)
    loadcell.begin(LOADCELL_1_DOUT_PIN,LOADCELL_2_DOUT_PIN, LOADCELL_1_SCK_PIN);
    loadcell.set_scale();
    loadcell.tare();
#else
    //Initialize library
    loadcell_1.begin(LOADCELL_1_DOUT_PIN, LOADCELL_1_SCK_PIN);
    loadcell_2.begin(LOADCELL_2_DOUT_PIN, LOADCELL_2_SCK_PIN);
    loadcell_1.set_scale();
    loadcell_2.set_scale();
    loadcell_1.tare();  //Reset the scale to 0
    loadcell_2.tare();  //Reset the scale to 0
#endif
}

void loop() {
  unsigned long timer = millis();
  float values[2];
  static float previousFactor1, previousFactor2;

  myNex.NextionListen();

  if (calibration_factor_lc1 != previousFactor1 || calibration_factor_lc2 != previousFactor2) {
    #if defined(SINGLE_HX711_CLOCK)
    loadcell.set_scale(calibration_factor_lc1, calibration_factor_lc2);
    #else
    loadcell_1.set_scale(calibration_factor_lc1); //Adjust to this calibration factor
    loadcell_2.set_scale(calibration_factor_lc2); //Adjust to this calibration factor
    #endif
    previousFactor1 = calibration_factor_lc1;
    previousFactor2 = calibration_factor_lc2;
  }

  if (myNex.currentPageId == 0) {
    if (millis() > timer) {
      #if defined(SINGLE_HX711_CLOCK)
      loadcell.get_units(values);
      myNex.writeStr("t0.txt",String(values[0],2));
      myNex.writeStr("t1.txt",String(values[1],2));
      #else
      myNex.writeStr("t0.txt",String(loadcell_1.get_units(),2));
      myNex.writeStr("t1.txt",String(loadcell_2.get_units(),2));
      #endif

      myNex.writeStr("t2.txt",String(calibration_factor_lc1,2));
      myNex.writeStr("t3.txt",String(calibration_factor_lc2,2));

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
  #if defined(ARDUINO_ARCH_AVR)
  EEPROM.put(EEP_SCALES_F1, calibration_factor_lc1);
  EEPROM.put(EEP_SCALES_F2, calibration_factor_lc2);
  #endif
}

void trigger5() {
  calibration_factor_lc1 = 4000;
  calibration_factor_lc2 = 4000;
}

// void autoCalibration(int targetWeight1, int targetWeight2) {
//   static float val1,val2;
//   static bool calDirection1, calDirection2, runOnce;

//   val1 = loadcell_1.get_units();
//   val2 = loadcell_2.get_units();

//   if (!runOnce) {
//     loadcell_1.set_scale(calibration_factor_lc1 + 5000);
//     float tempVal1 = loadcell_1.get_units();
//     if(tempVal1 < val1) calDirection1 = true;

//     loadcell_2.set_scale(calibration_factor_lc2 + 5000);
//     float tempVal2 = loadcell_2.get_units();
//     if(tempVal2 < val2) calDirection2 = true;
//   }

//   if (calDirection1 && val1 != targetWeight1) calibration_factor_lc1++;
//   else if (!calDirection1 && val1 != targetWeight1) calibration_factor_lc1--;

//   if (calDirection2 && val2 != targetWeight2) calibration_factor_lc2++;
//   else if (!calDirection2 && val2 != targetWeight2) calibration_factor_lc2--;
// }
