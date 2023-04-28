/* 09:32 15/03/2023 - change triggering comment */
#ifndef EEPROM_DATA_V9_H
#define EEPROM_DATA_V9_H

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
* Version 8:
* - Adding a lot of new stuff, there's no going back form this without a full reset.
*/
struct eepromValues_t_v9 {
  // profile version var
  uint8_t idx;
  // System settings
  uint16_t setpoint;
  uint16_t steamSetPoint;
  uint16_t offsetTemp;
  uint16_t hpwr;
  uint16_t mainDivider;
  uint16_t brewDivider;
  // Preinfusion vars section
  bool     preinfusionState;
  bool     preinfusionFlowState;
  uint16_t preinfusionSec;
  float    preinfusionBar;
  float    preinfusionFlowVol;
  uint16_t preinfusionFlowTime;
  float    preinfusionFlowPressureTarget;
  float    preinfusionPressureFlowTarget;
  float    preinfusionFilled;
  bool     preinfusionPressureAbove;
  float    preinfusionWeightAbove;
  // Soak vars section
  bool     soakState;
  uint16_t soakTimePressure;
  uint16_t soakTimeFlow;
  float    soakKeepPressure;
  float    soakKeepFlow;
  float    soakBelowPressure;
  float    soakAbovePressure;
  float    soakAboveWeight;
  // PI -> PF ramp settings
  uint16_t preinfusionRamp;
  uint16_t preinfusionRampSlope;
  // Profiling vars section
  bool     profilingState;
  bool     flowProfileState;
  float    pressureProfilingStart;
  float    pressureProfilingFinish;
  uint16_t pressureProfilingHold;
  float    pressureProfilingHoldLimit;
  uint16_t pressureProfilingSlope;
  uint16_t pressureProfilingSlopeShape;
  float    pressureProfilingFlowRestriction;
  float    flowProfileStart;
  float    flowProfileEnd;
  uint16_t flowProfileHold;
  float    flowProfileHoldLimit;
  uint16_t flowProfileSlope;
  uint16_t flowProfileSlopeShape;
  float    flowProfilingPressureRestriction;
  // Settings vars section
  uint16_t powerLineFrequency;
  uint16_t lcdSleep;
  bool     warmupState;
  bool     homeOnShotFinish;
  bool     brewDeltaState;
  bool     basketPrefill;
  int      scalesF1;
  int      scalesF2;
  float    pumpFlowAtZero;
  bool     stopOnWeightState;
  float    shotDose;
  float    shotStopOnCustomWeight;
  uint16_t shotPreset;
};

static bool upgradeSchema_v9(eepromValues_t &targetValues, eepromValues_t_v9 &loadedValues) {
  targetValues.idx = 0;
  targetValues.setpoint = loadedValues.setpoint;
  targetValues.steamSetPoint = loadedValues.steamSetPoint;
  targetValues.offsetTemp = loadedValues.offsetTemp;
  targetValues.hpwr = loadedValues.hpwr;
  targetValues.mainDivider = loadedValues.mainDivider;
  targetValues.brewDivider = loadedValues.brewDivider;
  targetValues.pressureProfilingStart = loadedValues.pressureProfilingStart;
  targetValues.pressureProfilingFinish = loadedValues.pressureProfilingFinish;
  targetValues.pressureProfilingHold = loadedValues.pressureProfilingHold;
  targetValues.pressureProfilingSlope = loadedValues.pressureProfilingSlope;
  targetValues.profilingState = loadedValues.profilingState;
  targetValues.preinfusionState = loadedValues.preinfusionState;
  targetValues.preinfusionSec = loadedValues.preinfusionSec;
  targetValues.preinfusionBar = loadedValues.preinfusionBar;
  targetValues.soakTimePressure = loadedValues.soakTimePressure;
  targetValues.preinfusionRamp = loadedValues.preinfusionRamp;
  targetValues.preinfusionFlowState = loadedValues.preinfusionFlowState;
  targetValues.preinfusionFlowVol = loadedValues.preinfusionFlowVol;
  targetValues.preinfusionFlowTime = loadedValues.preinfusionFlowTime;
  targetValues.soakTimeFlow = loadedValues.soakTimeFlow;
  targetValues.preinfusionFlowPressureTarget = loadedValues.preinfusionFlowPressureTarget;
  targetValues.flowProfileState = loadedValues.flowProfileState;
  targetValues.flowProfileStart = loadedValues.flowProfileStart;
  targetValues.flowProfileEnd = loadedValues.flowProfileEnd;
  targetValues.flowProfileSlope = loadedValues.flowProfileSlope;
  targetValues.powerLineFrequency = loadedValues.powerLineFrequency;
  targetValues.lcdSleep = loadedValues.lcdSleep;
  targetValues.warmupState = loadedValues.warmupState;
  targetValues.homeOnShotFinish = loadedValues.homeOnShotFinish;
  targetValues.brewDeltaState = loadedValues.brewDeltaState;
  targetValues.basketPrefill = loadedValues.basketPrefill;
  targetValues.scalesF1 = loadedValues.scalesF1;
  targetValues.scalesF2 = loadedValues.scalesF2;
  targetValues.pumpFlowAtZero = loadedValues.pumpFlowAtZero;
  targetValues.stopOnWeightState = loadedValues.stopOnWeightState;
  targetValues.shotDose = loadedValues.shotDose;
  targetValues.shotStopOnCustomWeight = loadedValues.shotStopOnCustomWeight;
  targetValues.shotPreset = loadedValues.shotPreset;
  return true;
}

REGISTER_LEGACY_EEPROM_DATA(9, eepromValues_t_v9, upgradeSchema_v9)

#endif
