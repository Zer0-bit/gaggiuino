#ifndef GAGGIA_SETTINGS_H
#define GAGGIA_SETTINGS_H

#include <Arduino.h>
#include <vector>
#include <string>

struct BoilerSettings {
  uint16_t steamSetPoint; /* Desired steam temperature */
  uint16_t offsetTemp;    /* Temperature offset from what the thermocouple measures to the water temp */
  uint16_t hpwr;
  uint16_t mainDivider;
  uint16_t brewDivider;
};
struct SystemSettings {
  float    pumpFlowAtZero;     /* A PZ constant used for pump calibration */
  uint16_t lcdSleep;           /* Time (in minutes) after which the screen should go to sleep */
  bool     warmupState;        /* Should gaggia wait to warmup */
};

struct ScalesSettings {
  bool forcePredictive;
  bool hwScalesEnabled;
  int  hwScalesF1;
  int  hwScalesF2;
  bool btScalesEnabled;
  bool btScalesAutoConnect;

  friend bool operator==(const ScalesSettings& lhs, const ScalesSettings& rhs) {
    return lhs.forcePredictive == rhs.forcePredictive &&
      lhs.hwScalesEnabled == rhs.hwScalesEnabled &&
      lhs.hwScalesF1 == rhs.hwScalesF1 &&
      lhs.hwScalesF2 == rhs.hwScalesF2 &&
      lhs.btScalesEnabled == rhs.btScalesEnabled &&
      lhs.btScalesAutoConnect == rhs.btScalesAutoConnect;
  }
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
  ScalesSettings scales;
};

#endif
