#ifndef GAGGIA_SETTINGS_H
#define GAGGIA_SETTINGS_H

#include <Arduino.h>
#include <vector>

struct BoilerSettings {
  uint16_t steamSetPoint; /* Desired steam temperature */
  uint16_t offsetTemp;    /* Temperature offset from what the thermocouple measures to the water temp */
  uint16_t hpwr;
  uint16_t mainDivider;
  uint16_t brewDivider;
};
struct SystemSettings {
  uint16_t powerLineFrequency; /* Frequence of main power in Hz */
  float    pumpFlowAtZero;     /* A PZ constant used for pump calibration */
  int      scalesF1;           /* Scales calibration constant 1 */
  int      scalesF2;           /* Scales calibration constant 2 */
  uint16_t lcdSleep;           /* Time (in minutes) after which the screen should go to sleep */
  bool     warmupState;        /* Should gaggia wait to warmup */
};

struct BrewSettings {
  bool     basketPrefill;     /* Whether we should add a prefill phase at the beginning of the profile */
  bool     homeOnShotFinish;  /* Should we exit the graph a few seconds after the shot has finished */
  bool     brewDeltaState;    /* Should gaggia add more heating power during a shot to compensate for cold water entering */
};

struct LedSettings {
  bool state;        /* Should the led be ON or OFF */
  bool disco;        /* Should we activate disco mode during brew */
  struct Color {     /* The led color */
    uint8_t R;
    uint8_t G;
    uint8_t B;
  } color;
};

struct GaggiaSettings {
  BoilerSettings boiler;
  SystemSettings system;
  BrewSettings   brew;
  LedSettings    led;
};

#endif
