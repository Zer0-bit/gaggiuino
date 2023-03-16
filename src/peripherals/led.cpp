#include "led.h"

#ifdef LED_NCP5623
NCP5623 tankLED;
#endif

void ledInit(void) {
  #ifdef LED_NCP5623
  tankLED.begin();
  #endif
}

void ledColor(uint8_t red, uint8_t green, uint8_t blue) {
  #ifdef LED_NCP5623
  tankLED.setColor(red, green, blue);
  #endif
}
