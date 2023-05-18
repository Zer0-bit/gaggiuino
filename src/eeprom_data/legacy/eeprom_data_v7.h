/* 09:32 15/03/2023 - change triggering comment */
#ifndef EEPROM_DATA_V7_H
#define EEPROM_DATA_V7_H

#include "../eeprom_data.h"
#include "../eeprom_metadata.h"

/**
* current data version definition below
*
* changing the schema requires bumping version number. this forces a schema update
* on boards with a lower version after flash. to make this graceful for users:
* - archive the current version in its corresponding header in legacy/
* - in that header, make sure to register the upgrade function for
*   the loader to pick it up
* - bump up the version in eeprom_metadata.h
* - make schema changes
*
* adding fields to data below shouldn't require changes to legacy upgrade
* functions - they just won't populate the new field, and it will use
* default value.
*
* removing fields might require deleting assignments in existing legacy
* functions that reference them. this will pop up as a compile time failure
*/

/**
* Version 7:
* - Add two more restrictions:
*   - switchPhaseOnPressureBelow
*   - switchPhaseOnFirstDrops
*/
struct eepromValues_t_v7 {
  uint16_t setpoint;
  uint16_t steamSetPoint;
  uint16_t offsetTemp;
  uint16_t hpwr;
  uint16_t mainDivider;
  uint16_t brewDivider;
  float    pressureProfilingStart;
  float    pressureProfilingFinish;
  uint16_t pressureProfilingHold;
  uint16_t pressureProfilingLength;
  bool     pressureProfilingState;
  bool     preinfusionState;
  uint16_t preinfusionSec;
  float    preinfusionBar;
  uint16_t preinfusionSoak;
  uint16_t preinfusionRamp;
  bool     preinfusionFlowState;
  float    preinfusionFlowVol;
  uint16_t preinfusionFlowTime;
  uint16_t preinfusionFlowSoakTime;
  float    preinfusionFlowPressureTarget;
  bool     flowProfileState;
  float    flowProfileStart;
  float    flowProfileEnd;
  float    flowProfilePressureTarget;
  uint16_t flowProfileCurveSpeed;
  uint16_t powerLineFrequency;
  uint16_t lcdSleep;
  bool     warmupState;
  bool     homeOnShotFinish;
  bool     graphBrew;
  bool     brewDeltaState;
  bool     switchPhaseOnThreshold;
  float    switchPhaseOnPressureBelow;
  bool     switchPhaseOnFirstDrops;
  bool     basketPrefill;
  int      scalesF1;
  int      scalesF2;
  float    pumpFlowAtZero;
  bool     stopOnWeightState;
  float    shotDose;
  float    shotStopOnCustomWeight;
  uint16_t shotPreset;
};

static bool upgradeSchema_v7(eepromValues_t &targetValues, eepromValues_t_v7 &loadedValues) {
  ACTIVE_PROFILE(targetValues).setpoint = loadedValues.setpoint;
  targetValues.steamSetPoint = loadedValues.steamSetPoint;
  targetValues.offsetTemp = loadedValues.offsetTemp;
  targetValues.hpwr = loadedValues.hpwr;
  targetValues.mainDivider = loadedValues.mainDivider;
  targetValues.brewDivider = loadedValues.brewDivider;
  ACTIVE_PROFILE(targetValues).mpProfilingStart = loadedValues.pressureProfilingStart;
  ACTIVE_PROFILE(targetValues).mpProfilingFinish = loadedValues.pressureProfilingFinish;
  ACTIVE_PROFILE(targetValues).preinfusionState = loadedValues.preinfusionState;
  ACTIVE_PROFILE(targetValues).preinfusionSec = loadedValues.preinfusionSec;
  ACTIVE_PROFILE(targetValues).preinfusionBar = loadedValues.preinfusionBar;
  ACTIVE_PROFILE(targetValues).preinfusionRamp = loadedValues.preinfusionRamp;
  ACTIVE_PROFILE(targetValues).preinfusionFlowState = loadedValues.preinfusionFlowState;
  ACTIVE_PROFILE(targetValues).preinfusionFlowVol = loadedValues.preinfusionFlowVol;
  ACTIVE_PROFILE(targetValues).preinfusionFlowTime = loadedValues.preinfusionFlowTime;
  ACTIVE_PROFILE(targetValues).preinfusionFlowPressureTarget = loadedValues.preinfusionFlowPressureTarget;
  ACTIVE_PROFILE(targetValues).mfProfileState = loadedValues.flowProfileState;
  ACTIVE_PROFILE(targetValues).mfProfileStart = loadedValues.flowProfileStart;
  ACTIVE_PROFILE(targetValues).mfProfileEnd = loadedValues.flowProfileEnd;
  targetValues.powerLineFrequency = loadedValues.powerLineFrequency;
  targetValues.lcdSleep = loadedValues.lcdSleep;
  targetValues.warmupState = loadedValues.warmupState;
  targetValues.homeOnShotFinish = loadedValues.homeOnShotFinish;
  targetValues.brewDeltaState = loadedValues.brewDeltaState;
  targetValues.basketPrefill = loadedValues.basketPrefill;
  targetValues.scalesF1 = loadedValues.scalesF1;
  targetValues.scalesF2 = loadedValues.scalesF2;
  targetValues.pumpFlowAtZero = loadedValues.pumpFlowAtZero;
  ACTIVE_PROFILE(targetValues).stopOnWeightState = loadedValues.stopOnWeightState;
  ACTIVE_PROFILE(targetValues).shotDose = loadedValues.shotDose;
  ACTIVE_PROFILE(targetValues).shotStopOnCustomWeight = loadedValues.shotStopOnCustomWeight;
  ACTIVE_PROFILE(targetValues).shotPreset = loadedValues.shotPreset;
  return true;
}

REGISTER_LEGACY_EEPROM_DATA(7, eepromValues_t_v7, upgradeSchema_v7)

#endif
