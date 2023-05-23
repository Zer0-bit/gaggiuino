/* 09:32 15/03/2023 - change triggering comment */
#ifndef EEPROM_DATA_H
#define EEPROM_DATA_H

#include <Arduino.h>
#include "../log.h"

/**
* NOTE: changing these constants requires:
*        - bumping the version number! the serialized data will have
*          a different layout to accommodate more array elements
*          or longer strings
*        - hardcoding the current value in the archived version and
*          its upgrade function, to ensure loading that version
*          deserializes correctly
*/
#define MAX_PROFILES 5
#define PROFILE_NAME_LENGTH 25

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
* Version 10:
* - Add multiple brew profiles
*/
struct eepromValues_t {
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

void eepromInit(void);
bool eepromWrite(eepromValues_t);
struct eepromValues_t eepromGetDefaultValues(void);
struct eepromValues_t eepromGetCurrentValues(void);

#define ACTIVE_PROFILE(eepromValues) eepromValues.profiles[eepromValues.activeProfile]

#endif
