/* 09:32 15/03/2023 - change triggering comment */
#ifndef EEPROM_DATA_H
#define EEPROM_DATA_H

#include <Arduino.h>
#include "../log.h"

/**
* NOTE: changing this variable requires:
*        - bumping the version number! the serialized data will have
*          a different layout to accommodate more array elements
*        - hardcoding the current value in the archived version and
*          its upgrade function, to ensure loading that version
*          deserializes correctly
*/
#define MAX_PROFILES 5

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
  uint16_t setpoint;
  uint16_t steamSetPoint;
  uint16_t offsetTemp;
  uint16_t hpwr;
  uint16_t mainDivider;
  uint16_t brewDivider;
  uint8_t activeProfile;
  struct profile_t {
    // name length is intentionally not macro/constant to avoid
    // separating them from the version. changing the length needs
    // a version bump!
    char     name[50];
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
  bool     stopOnWeightState;
  float    shotDose;
  float    shotStopOnCustomWeight;
  uint16_t shotPreset;
};

void eepromInit(void);
bool eepromWrite(eepromValues_t);
struct eepromValues_t eepromGetCurrentValues(void);

#define PROFILE(eepromValues, i) eepromValues.profiles[i]
#define ACTIVE_PROFILE(eepromValues) PROFILE(eepromValues, eepromValues.activeProfile)

#endif
