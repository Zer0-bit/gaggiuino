#include <Arduino.h>
#include <HX711_2.h>
#include <EasyNextionLibrary.h>
#include <ADS1X15.h>

#define relayPin PA15  // PB0
#define LOADCELL_1_DOUT_PIN  PB8
#define LOADCELL_2_DOUT_PIN  PB9
#define LOADCELL_1_SCK_PIN  PB0
#define LOADCELL_2_SCK_PIN  PB1
#define UART_LCD Serial2
#define USART_DEBUG Serial  // USB-CDC (Takes PA8,PA9,PA10,PA11)

HX711_2 loadcell;

#if defined SINGLE_HX711_BOARD
  unsigned char scale_clk = OUTPUT;
#else
  unsigned char scale_clk = OUTPUT_OPEN_DRAIN;
#endif

float calibration_factor_lc1 = 4000; //-7050 worked for my 440lb max scale setup
float calibration_factor_lc2 = 4000; //-7050 worked for my 440lb max scale setup

//Nextion object init
EasyNex myNex(UART_LCD);

void setup() {
  USART_DEBUG.begin(115200);
  myNex.begin(115200);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  while (myNex.readNumber("initCheck") != 100) {
    delay(600);
  }

  loadcell.begin(LOADCELL_1_DOUT_PIN, LOADCELL_2_DOUT_PIN, LOADCELL_1_SCK_PIN, LOADCELL_2_SCK_PIN, 128, scale_clk);
  loadcell.set_scale();
  loadcell.power_up();
  loadcell.tare();

  USART_DEBUG.println("Initialized");
}

void loop() {
  static unsigned long timer = millis();
  float values[2];
  static float previousFactor1, previousFactor2;

  if (millis() > timer) {
    long rawValues[2];
    loadcell.read(rawValues);
    USART_DEBUG.printf("%8d %8d\n", rawValues[0], rawValues[1]);
  }

  myNex.NextionListen();

  if (calibration_factor_lc1 != previousFactor1 || calibration_factor_lc2 != previousFactor2) {
    loadcell.set_scale(calibration_factor_lc1, calibration_factor_lc2);
    previousFactor1 = calibration_factor_lc1;
    previousFactor2 = calibration_factor_lc2;
  }

  if (millis() > timer) {
    if (myNex.currentPageId == 0) {
      loadcell.get_units(values);
      myNex.writeStr("t0.txt",String(values[0],2));
      myNex.writeStr("t1.txt",String(values[1],2));

      myNex.writeStr("t2.txt",String(calibration_factor_lc1,2));
      myNex.writeStr("t3.txt",String(calibration_factor_lc2,2));
    }
    timer = millis() + 100ul;
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
