#ifndef EEPROM_DATA_V11_H
#define EEPROM_DATA_V11_H

#include "../eeprom_data.h"
#include "../eeprom_metadata.h"

/**
* Version 11:
* - Move brew setpoint and shot settings into profile.
* - Changed name length
*/

struct eepromValues_t_v11 {
  uint16_t steamSetPoint;
  uint16_t offsetTemp;
  uint16_t hpwr;
  uint16_t mainDivider;
  uint16_t brewDivider;
  uint8_t activeProfile;
  struct profile_t {
    char     name[PROFILE_NAME_LENGTH];
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
    bool     tpState;
    bool     tpType;
    float    tpProfilingStart;
    float    tpProfilingFinish;
    uint16_t tpProfilingHold;
    float    tpProfilingHoldLimit;
    uint16_t tpProfilingSlope;
    uint16_t tpProfilingSlopeShape;
    float    tpProfilingFlowRestriction;
    float    tfProfileStart;
    float    tfProfileEnd;
    uint16_t tfProfileHold;
    float    tfProfileHoldLimit;
    uint16_t tfProfileSlope;
    uint16_t tfProfileSlopeShape;
    float    tfProfilingPressureRestriction;
    bool     profilingState;
    bool     mfProfileState;
    float    mpProfilingStart;
    float    mpProfilingFinish;
    uint16_t mpProfilingSlope;
    uint16_t mpProfilingSlopeShape;
    float    mpProfilingFlowRestriction;
    float    mfProfileStart;
    float    mfProfileEnd;
    uint16_t mfProfileSlope;
    uint16_t mfProfileSlopeShape;
    float    mfProfilingPressureRestriction;
    /*-----------OTHER---------------------*/
    uint16_t setpoint;
    bool     stopOnWeightState;
    float    shotDose;
    float    shotStopOnCustomWeight;
    uint16_t shotPreset;
  } profiles[MAX_PROFILES];
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
};

static bool upgradeSchema_v11(eepromValues_t &targetValues, eepromValues_t_v11 &loadedValues) {
  targetValues.steamSetPoint = loadedValues.steamSetPoint;
  targetValues.offsetTemp = loadedValues.offsetTemp;
  targetValues.hpwr = loadedValues.hpwr;
  targetValues.mainDivider = loadedValues.mainDivider;
  targetValues.brewDivider = loadedValues.brewDivider;
  targetValues.activeProfile = loadedValues.activeProfile;
  for (int i=0; i<MAX_PROFILES; i++) {
    snprintf(targetValues.profiles[i].name, PROFILE_NAME_LENGTH, "%s", loadedValues.profiles[i].name);
    targetValues.profiles[i].preinfusionState = loadedValues.profiles[i].preinfusionState;
    targetValues.profiles[i].preinfusionFlowState = loadedValues.profiles[i].preinfusionFlowState;
    targetValues.profiles[i].preinfusionSec = loadedValues.profiles[i].preinfusionSec;
    targetValues.profiles[i].preinfusionBar = loadedValues.profiles[i].preinfusionBar;
    targetValues.profiles[i].preinfusionFlowVol = loadedValues.profiles[i].preinfusionFlowVol;
    targetValues.profiles[i].preinfusionFlowTime = loadedValues.profiles[i].preinfusionFlowTime;
    targetValues.profiles[i].preinfusionFlowPressureTarget = loadedValues.profiles[i].preinfusionFlowPressureTarget;
    targetValues.profiles[i].preinfusionPressureFlowTarget = loadedValues.profiles[i].preinfusionPressureFlowTarget;
    targetValues.profiles[i].preinfusionFilled = loadedValues.profiles[i].preinfusionFilled;
    targetValues.profiles[i].preinfusionPressureAbove = loadedValues.profiles[i].preinfusionPressureAbove;
    targetValues.profiles[i].preinfusionWeightAbove = loadedValues.profiles[i].preinfusionWeightAbove;
    targetValues.profiles[i].soakState = loadedValues.profiles[i].soakState;
    targetValues.profiles[i].soakTimePressure = loadedValues.profiles[i].soakTimePressure;
    targetValues.profiles[i].soakTimeFlow = loadedValues.profiles[i].soakTimeFlow;
    targetValues.profiles[i].soakKeepPressure = loadedValues.profiles[i].soakKeepPressure;
    targetValues.profiles[i].soakKeepFlow = loadedValues.profiles[i].soakKeepFlow;
    targetValues.profiles[i].soakBelowPressure = loadedValues.profiles[i].soakBelowPressure;
    targetValues.profiles[i].soakAbovePressure = loadedValues.profiles[i].soakAbovePressure;
    targetValues.profiles[i].soakAboveWeight = loadedValues.profiles[i].soakAboveWeight;
    targetValues.profiles[i].preinfusionRamp = loadedValues.profiles[i].preinfusionRamp;
    targetValues.profiles[i].preinfusionRampSlope = loadedValues.profiles[i].preinfusionRampSlope;
    targetValues.profiles[i].tpState = loadedValues.profiles[i].tpState;
    targetValues.profiles[i].tpType = loadedValues.profiles[i].tpType;
    targetValues.profiles[i].tpProfilingStart = loadedValues.profiles[i].tpProfilingStart;
    targetValues.profiles[i].tpProfilingFinish = loadedValues.profiles[i].tpProfilingFinish;
    targetValues.profiles[i].tpProfilingHold = loadedValues.profiles[i].tpProfilingHold;
    targetValues.profiles[i].tpProfilingHoldLimit = loadedValues.profiles[i].tpProfilingHoldLimit;
    targetValues.profiles[i].tpProfilingSlope = loadedValues.profiles[i].tpProfilingSlope;
    targetValues.profiles[i].tpProfilingSlopeShape = loadedValues.profiles[i].tpProfilingSlopeShape;
    targetValues.profiles[i].tpProfilingFlowRestriction = loadedValues.profiles[i].tpProfilingFlowRestriction;
    targetValues.profiles[i].tfProfileStart = loadedValues.profiles[i].tfProfileStart;
    targetValues.profiles[i].tfProfileEnd = loadedValues.profiles[i].tfProfileEnd;
    targetValues.profiles[i].tfProfileHold = loadedValues.profiles[i].tfProfileHold;
    targetValues.profiles[i].tfProfileHoldLimit = loadedValues.profiles[i].tfProfileHoldLimit;
    targetValues.profiles[i].tfProfileSlope = loadedValues.profiles[i].tfProfileSlope;
    targetValues.profiles[i].tfProfileSlopeShape = loadedValues.profiles[i].tfProfileSlopeShape;
    targetValues.profiles[i].tfProfilingPressureRestriction = loadedValues.profiles[i].tfProfilingPressureRestriction;
    targetValues.profiles[i].profilingState = loadedValues.profiles[i].profilingState;
    targetValues.profiles[i].mfProfileState = loadedValues.profiles[i].mfProfileState;
    targetValues.profiles[i].mpProfilingStart = loadedValues.profiles[i].mpProfilingStart;
    targetValues.profiles[i].mpProfilingFinish = loadedValues.profiles[i].mpProfilingFinish;
    targetValues.profiles[i].mpProfilingSlope = loadedValues.profiles[i].mpProfilingSlope;
    targetValues.profiles[i].mpProfilingSlopeShape = loadedValues.profiles[i].mpProfilingSlopeShape;
    targetValues.profiles[i].mpProfilingFlowRestriction = loadedValues.profiles[i].mpProfilingFlowRestriction;
    targetValues.profiles[i].mfProfileStart = loadedValues.profiles[i].mfProfileStart;
    targetValues.profiles[i].mfProfileEnd = loadedValues.profiles[i].mfProfileEnd;
    targetValues.profiles[i].mfProfileSlope = loadedValues.profiles[i].mfProfileSlope;
    targetValues.profiles[i].mfProfileSlopeShape = loadedValues.profiles[i].mfProfileSlopeShape;
    targetValues.profiles[i].mfProfilingPressureRestriction = loadedValues.profiles[i].mfProfilingPressureRestriction;
    targetValues.profiles[i].setpoint = loadedValues.profiles[i].setpoint;
    targetValues.profiles[i].stopOnWeightState = loadedValues.profiles[i].stopOnWeightState;
    targetValues.profiles[i].shotDose = loadedValues.profiles[i].shotDose;
    targetValues.profiles[i].shotStopOnCustomWeight = loadedValues.profiles[i].shotStopOnCustomWeight;
    targetValues.profiles[i].shotPreset = loadedValues.profiles[i].shotPreset;
  }
  targetValues.powerLineFrequency = loadedValues.powerLineFrequency;
  targetValues.lcdSleep = loadedValues.lcdSleep;
  targetValues.warmupState = loadedValues.warmupState;
  targetValues.homeOnShotFinish = loadedValues.homeOnShotFinish;
  targetValues.brewDeltaState = loadedValues.brewDeltaState;
  targetValues.basketPrefill = loadedValues.basketPrefill;
  targetValues.scalesF1 = loadedValues.scalesF1;
  targetValues.scalesF2 = loadedValues.scalesF2;
  targetValues.pumpFlowAtZero = loadedValues.pumpFlowAtZero;
  return true;
}

REGISTER_LEGACY_EEPROM_DATA(11, eepromValues_t_v11, upgradeSchema_v11)

#endif
