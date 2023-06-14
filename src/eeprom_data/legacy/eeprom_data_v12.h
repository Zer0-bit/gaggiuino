#ifndef EEPROM_DATA_V12_H
#define EEPROM_DATA_V12_H

#include "../eeprom_data.h"
#include "../eeprom_migration.h"
#include "../../lcd/nextion_profile_mapping.h"

#define V12_PROFILE_NAME_LENGTH 25
/**
* Version 12: Legacy since TBD
* - Multiple changes to the brew profile settings to support more customization of phases
* - Re-organize fields
*/
struct eepromValues_t_v12 : public BaseValues_t {
  uint16_t steamSetPoint;
  uint16_t offsetTemp;
  uint16_t hpwr;
  uint16_t mainDivider;
  uint16_t brewDivider;
  uint8_t activeProfile;
  struct profile_t {
    char     name[V12_PROFILE_NAME_LENGTH];
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
  } profiles[5];
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
  bool     ledState;
  uint8_t  ledR;
  uint8_t  ledG;
  uint8_t  ledB;
};
struct eepromMetadata_t_v12 : public BaseMetadata_t {
  struct eepromValues_t_v12 values;
  uint32_t versionTimestampXOR;
};

static std::unique_ptr<BaseValues_t> fetchEepromValues_v12(BaseMetadata_t& metadata) {
  eepromMetadata_t_v12& typedMetadata = static_cast<eepromMetadata_t_v12&>(metadata);
  return std::make_unique<eepromValues_t_v12>(typedMetadata.values);
}

REGISTER_LEGACY_EEPROM_DATA(12, eepromMetadata_t_v12, eepromValues_t_v12, fetchEepromValues_v12);

#endif
