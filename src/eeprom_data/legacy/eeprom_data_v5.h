/* 09:32 15/03/2023 - change triggering comment */
#ifndef EEPROM_DATA_V5_H
#define EEPROM_DATA_V5_H

#include "../eeprom_data.h"
#include "../eeprom_metadata.h"

/**
* Version 5:
* - added steamSetPoint
* - added basketPrefill
*/
struct eepromValues_t_v5 {
  uint16_t setpoint;
  uint16_t steamSetPoint;
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
  bool     switchPhaseOnThreshold;
  bool     basketPrefill;
  int      scalesF1;
  int      scalesF2;
  float    pumpFlowAtZero;
  bool     stopOnWeightState;
  float    shotDose;
  float    shotStopOnCustomWeight;
  uint16_t shotPreset;
};

static bool upgradeSchema_v5(eepromValues_t &targetValues, eepromValues_t_v5 &loadedValues) {
  targetValues.setpoint = loadedValues.setpoint;
  targetValues.steamSetPoint = loadedValues.steamSetPoint;
  targetValues.offsetTemp = loadedValues.offsetTemp;
  targetValues.hpwr = loadedValues.hpwr;
  targetValues.mainDivider = loadedValues.mainDivider;
  targetValues.brewDivider = loadedValues.brewDivider;
  targetValues.pressureProfilingStart = (float) loadedValues.pressureProfilingStart;
  targetValues.pressureProfilingFinish = (float) loadedValues.pressureProfilingFinish;
  targetValues.pressureProfilingHold = loadedValues.pressureProfilingHold;
  targetValues.pressureProfilingLength = loadedValues.pressureProfilingLength;
  targetValues.pressureProfilingState = loadedValues.pressureProfilingState;
  targetValues.preinfusionState = loadedValues.preinfusionState;
  targetValues.preinfusionSec = loadedValues.preinfusionSec;
  targetValues.preinfusionBar = (float) loadedValues.preinfusionBar;
  targetValues.preinfusionSoak = loadedValues.preinfusionSoak;
  targetValues.preinfusionRamp = loadedValues.preinfusionRamp;
  targetValues.preinfusionFlowState = loadedValues.preinfusionFlowState;
  targetValues.preinfusionFlowVol = loadedValues.preinfusionFlowVol;
  targetValues.preinfusionFlowTime = loadedValues.preinfusionFlowTime;
  targetValues.preinfusionFlowSoakTime = loadedValues.preinfusionFlowSoakTime;
  targetValues.preinfusionFlowPressureTarget = (float) loadedValues.preinfusionFlowPressureTarget;
  targetValues.flowProfileState = loadedValues.flowProfileState;
  targetValues.flowProfileStart = loadedValues.flowProfileStart;
  targetValues.flowProfileEnd = loadedValues.flowProfileEnd;
  targetValues.flowProfilePressureTarget = (float) loadedValues.flowProfilePressureTarget;
  targetValues.flowProfileCurveSpeed = loadedValues.flowProfileCurveSpeed;
  targetValues.powerLineFrequency = loadedValues.powerLineFrequency;
  targetValues.lcdSleep = loadedValues.lcdSleep;
  targetValues.warmupState = loadedValues.warmupState;
  targetValues.homeOnShotFinish = loadedValues.homeOnShotFinish;
  targetValues.graphBrew = loadedValues.graphBrew;
  targetValues.brewDeltaState = loadedValues.brewDeltaState;
  targetValues.switchPhaseOnThreshold = loadedValues.switchPhaseOnThreshold;
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

REGISTER_LEGACY_EEPROM_DATA(5, eepromValues_t_v5, upgradeSchema_v5)

#endif
