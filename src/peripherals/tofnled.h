#ifndef TOFNLED_H
#define TOFNLED_H

#include <stdint.h> // for uint8_t
#include <ncp5623.h>

using namespace std;

class TOFnLED {
  public:
    TOFnLED();
    void begin();
    void setColor(uint8_t red, uint8_t green, uint8_t blue);

  private:
    NCP5623 tankLED;
};

TOFnLED::TOFnLED(){};

void TOFnLED::begin() {
  #ifdef LED_NCP5623
  tankLED.begin();
  #endif
}

void TOFnLED::setColor(uint8_t red, uint8_t green, uint8_t blue) {
  #ifdef LED_NCP5623
  tankLED.setColor(red, green, blue);
  #endif
}
#endif
