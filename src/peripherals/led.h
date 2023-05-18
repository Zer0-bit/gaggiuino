#ifndef LED_H
#define LED_H

#include <stdint.h> // for uint8_t
#include <NCP5623.h>

class LED {
  public:
    LED();
    void begin();
    void setColor(uint8_t red, uint8_t green, uint8_t blue);

  private:
    NCP5623 tankLED;
};

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
#endif
