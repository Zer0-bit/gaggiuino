#ifndef EEPROM_DATA_H
#define EEPROM_DATA_H

#include <Arduino.h>
#include "log.h"

#define EEPROM_DATA_VERSION 3

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
  uint16_t preinfusionRamp;
  bool     preinfusionFlowState;
  float    preinfusionFlowVol;
  uint16_t preinfusionFlowTime;
  uint16_t preinfusionFlowSoakTime;
  uint16_t preinfusionFlowPressureTarget;
  bool     flowProfileState;
  float    flowProfileStart;
  float    flowProfileEnd;
  uint16_t flowProfilePressureTarget;
  uint16_t flowProfileCurveSpeed;
  uint16_t powerLineFrequency;
  uint16_t lcdSleep;
  bool     warmupState;
  bool     homeOnShotFinish;
  bool     graphBrew;
  bool     brewDeltaState;
  float    scalesF1;
  float    scalesF2;
  float    pumpFlowAtZero;
  bool     stopOnWeightState;
  float    shotDose;
  float    shotStopOnCustomWeight;
  uint16_t shotPreset;
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
