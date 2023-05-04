#ifndef EEPROM_DATA_V4_H
#define EEPROM_DATA_V4_H

#include "../eeprom_data.h"
#include "../eeprom_metadata.h"

/**
* Version 9:
* - Multiple changes to the brew profile settings to support more customization of phases
* - Re-organize fields
*/

struct eepromValues_t_v9 {
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
  targetValues.setpoint = loadedValues.setpoint;
  targetValues.steamSetPoint = loadedValues.steamSetPoint;
  targetValues.offsetTemp = loadedValues.offsetTemp;
  targetValues.hpwr = loadedValues.hpwr;
  targetValues.mainDivider = loadedValues.mainDivider;
  targetValues.brewDivider = loadedValues.brewDivider;
  targetValues.preinfusionState = loadedValues.preinfusionState;
  targetValues.preinfusionFlowState = loadedValues.preinfusionFlowState;
  targetValues.preinfusionSec = loadedValues.preinfusionSec;
  targetValues.preinfusionBar = loadedValues.preinfusionBar;
  targetValues.preinfusionFlowVol = loadedValues.preinfusionFlowVol;
  targetValues.preinfusionFlowTime = loadedValues.preinfusionFlowTime;
  targetValues.preinfusionFlowPressureTarget = loadedValues.preinfusionFlowPressureTarget;
  targetValues.preinfusionPressureFlowTarget = loadedValues.preinfusionPressureFlowTarget;
  targetValues.preinfusionFilled = loadedValues.preinfusionFilled;
  targetValues.preinfusionPressureAbove = loadedValues.preinfusionPressureAbove;
  targetValues.preinfusionWeightAbove = loadedValues.preinfusionWeightAbove;
  targetValues.soakState = loadedValues.soakState;
  targetValues.soakTimePressure = loadedValues.soakTimePressure;
  targetValues.soakTimeFlow = loadedValues.soakTimeFlow;
  targetValues.soakKeepPressure = loadedValues.soakKeepPressure;
  targetValues.soakKeepFlow = loadedValues.soakKeepFlow;
  targetValues.soakBelowPressure = loadedValues.soakBelowPressure;
  targetValues.soakAbovePressure = loadedValues.soakAbovePressure;
  targetValues.soakAboveWeight = loadedValues.soakAboveWeight;
  targetValues.preinfusionRamp = loadedValues.preinfusionRamp;
  targetValues.preinfusionRampSlope = loadedValues.preinfusionRampSlope;
  targetValues.profilingState = loadedValues.profilingState;
  targetValues.flowProfileState = loadedValues.flowProfileState;
  targetValues.pressureProfilingStart = loadedValues.pressureProfilingStart;
  targetValues.pressureProfilingFinish = loadedValues.pressureProfilingFinish;
  targetValues.pressureProfilingHold = loadedValues.pressureProfilingHold;
  targetValues.pressureProfilingHoldLimit = loadedValues.pressureProfilingHoldLimit;
  targetValues.pressureProfilingSlope = loadedValues.pressureProfilingSlope;
  targetValues.pressureProfilingSlopeShape = loadedValues.pressureProfilingSlopeShape;
  targetValues.pressureProfilingFlowRestriction = loadedValues.pressureProfilingFlowRestriction;
  targetValues.flowProfileStart = loadedValues.flowProfileStart;
  targetValues.flowProfileEnd = loadedValues.flowProfileEnd;
  targetValues.flowProfileHold = loadedValues.flowProfileHold;
  targetValues.flowProfileHoldLimit = loadedValues.flowProfileHoldLimit;
  targetValues.flowProfileSlope = loadedValues.flowProfileSlope;
  targetValues.flowProfileSlopeShape = loadedValues.flowProfileSlopeShape;
  targetValues.flowProfilingPressureRestriction = loadedValues.flowProfilingPressureRestriction;
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
