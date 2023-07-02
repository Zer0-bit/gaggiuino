#ifndef LED_H
#define LED_H

#include <Arduino.h>
#include <NCP5623.h>
#include <PCA9632.h>

#ifdef LED_NCP5623
NCP5623 tankLED;
#endif
#ifdef LED_PCA9632
PCA9632 tankLED;
#endif

class LED {
  public:
    LED() noexcept;
    void begin(void);
    void setColor(uint8_t red, uint8_t green, uint8_t blue);
    void setRed(uint8_t red);
    void setBlue(uint8_t blue);
    void setGreen(uint8_t green);
    void setDisco(uint32_t timer);
    enum discoMode : uint16_t {
      STROBE = 100u,
      CLASSIC = 15u,
      DESCALE = 1000u
    };
  private:
    uint32_t timer;
};

LED::LED() : timer(millis()) {};

void LED::begin() {
  #if defined LED_NCP5623 || defined LED_PCA9632
  tankLED.begin();
  #endif
}

void LED::setColor(uint8_t red, uint8_t green, uint8_t blue) {
  #if defined LED_NCP5623 || defined LED_PCA9632
  tankLED.setColor(red, green, blue);
  #endif
}

void LED::setRed(uint8_t red) {
  #if defined LED_NCP5623 || defined LED_PCA9632
  tankLED.setRed(red);
  #endif
}

void LED::setBlue(uint8_t blue) {
  #if defined LED_NCP5623 || defined LED_PCA9632
  tankLED.setBlue(blue);
  #endif
}

void LED::setGreen(uint8_t green) {
  #if defined LED_NCP5623 || defined LED_PCA9632
  tankLED.setGreen(green);
  #endif
}

void LED::setDisco(uint32_t increment) {
  switch(increment) {
    case LED::CLASSIC: // brew time disco
    case LED::DESCALE: // slow fade during descale
      if(millis() > LED::timer) {
        static uint8_t cstate = 1, val = 0;
        static uint8_t r,g,b;
        // val<<3 adjusts from 5 bit quantity to 8 bit for the library
        if(val % 2 == 0) {
          b = (cstate & 4) ? val<<3 : r; // Blue channel enabled on cstate = 4,5,6,7
          g = (cstate & 2) ? val<<3 : g; // Green channel enabled on cstate = 2,3,6,7
          r = (cstate & 1) ? val<<3 : b; // Red channel enabled on cstate = 1,3,5,7
        } else {
          r = (cstate & 4) ? val<<3 : b; // Red channel enabled on cstate = 4,5,6,7
          g = (cstate & 2) ? val<<3 : g; // Green channel enabled on cstate = 2,3,6,7
          b = (cstate & 1) ? val<<3 : r; // Blue channel enabled on cstate = 1,3,5,7
        }
        setColor(r,g,b);
        val++;
        if (val>31) { // if val has reached max,
          val = 0;  // reset val
          cstate++; // next state
          if (cstate > 7) { // if state has reached max
            cstate = 0; // reset state
          }
        }
        LED::timer = millis() + increment;
      }
    case LED::STROBE: // flush strobe
      if(millis() > LED::timer) {
        static uint8_t val = 0;
        // val<<3 adjusts from 5 bit quantity to 8 bit for the library
        if(val % 2 == 0) {
          setColor(255, 255, 255);
        } else {
          setColor(0, 0, 0);
        }
        val++;
        if (val>31) val = 0;  // reset val
        LED::timer = millis() + increment;
      }
  }
}

#endif
