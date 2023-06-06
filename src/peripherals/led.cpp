#include "led.h"

LED::LED(){};

void LED::begin() {
  #ifdef LED_NCP5623
  tankLED.begin();
  #endif
}

void LED::setColor(uint8_t red, uint8_t green, uint8_t blue) {
  #ifdef LED_NCP5623
  tankLED.setColor(red, green, blue);
  #endif
}

void LED::setRed(uint8_t red) {
  #ifdef LED_NCP5623
  tankLED.setRed(red);
  #endif
}

void LED::setBlue(uint8_t blue) {
  #ifdef LED_NCP5623
  tankLED.setBlue(blue);
  #endif
}

void LED::setGreen(uint8_t green) {
  #ifdef LED_NCP5623
  tankLED.setGreen(green);
  #endif
}
