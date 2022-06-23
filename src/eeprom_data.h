#ifndef EEPROM_DATA_H
#define EEPROM_DATA_H

#include <Arduino.h>
#include "log.h"

#define EEPROM_DATA_VERSION 1

struct eepromValues_t {
  uint16_t setpoint;
  uint16_t offsetTemp;
  uint16_t hpwr;
  uint16_t mainDivider;
  uint16_t brewDivider;
  uint16_t pressureProfilingStart;
  uint16_t pressureProfilingFinish;
  uint16_t pressureProfilingHold;
  uint16_t pressureProfilingLength;
  bool     pressureProfilingState;
  bool     preinfusionState;
  uint16_t preinfusionSec;
  uint16_t preinfusionBar;
  uint16_t preinfusionSoak;
  uint16_t powerLineFrequency;
  bool     warmupState;
  bool     homeOnShotFinish;
  bool     graphBrew;
  bool     brewDeltaState;
  float    scalesF1;
  float    scalesF2;
};

struct eepromMetadata_t {
  uint16_t version;
  unsigned long timestamp;
  struct eepromValues_t values;
  uint32_t versionTimestampXOR;
};

void eepromInit(void);
bool eepromWrite(eepromValues_t);
struct eepromValues_t eepromGetCurrentValues(void);

#endif
