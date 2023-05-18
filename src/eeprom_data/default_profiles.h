/* 09:32 15/03/2023 - change triggering comment */
#ifndef DEFAULT_PROFILES_H
#define DEFAULT_PROFILES_H

#include <Arduino.h>
#include "eeprom_data.h"

namespace {
  typedef struct {
    char     name[24];
    bool     preinfusionState;/**< This is the first field. */
    bool     preinfusionFlowState;/**< This is the first field. */
    uint16_t preinfusionSec;/**< This is the first field. */
    float    preinfusionBar;/**< This is the first field. */
    float    preinfusionFlowVol;
    uint16_t preinfusionFlowTime;
    float    preinfusionFlowPressureTarget;
    float    preinfusionPressureFlowTarget;
    float    preinfusionFilled;
    bool     preinfusionPressureAbove;
    float    preinfusionWeightAbove;
    bool     soakState;
    uint16_t soakTimePressure;
    uint16_t soakTimeFlow;
    float    soakKeepPressure;
    float    soakKeepFlow;
    float    soakBelowPressure;
    float    soakAbovePressure;
    float    soakAboveWeight;
    uint16_t preinfusionRamp;
    uint16_t preinfusionRampSlope;
    bool     profilingState;
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
  } profileDefaults_t;

  // DONE:: Fully customised default profiles in line wiuth the names
  const profileDefaults_t defaultsProfile[MAX_PROFILES] = {
    {"IUIUIU Classic",/*pi*/true, true, 10, 2.f, 3.f, 20, 4.f, 3.f, 600, true, 4.f,/*sk*/true, 7, 30, 0.f, 0.f, 2.f, 0.f, 4.f, 5, 2,/*tp*/true, false, 7.5f, 6.f, 0, 3.f, 4, 0, 3.f, 0.f, 0.f, 0, 0.f, 0, 0, 0.f,/*pf*/true, 0.f, 0.f, 0, 0, 0.f, 2.5f, 1.f, 15, 0, 6.f,/*other*/ 93, true, 18.f, 0.f, 2},  // profile 0
    {"Londinium",/*pi*/true, true, 10, 2.f, 3.f, 20, 4.f, 9.f, 650, true, 0.f,/*sk*/true, 15, 10, 0.f, 0.f, 0.7f, 0.f, 0.f, 5, 2,/*tp*/true, false, 9.f, 9.f, 1, 3.f, 0, 0, 3.f, 0.f, 0.f, 0, 0.f, 0, 0, 0.f,/*pf*/false, 9.f, 3.f, 20, 0, 3.f, 0.f, 0.f, 0, 0, 0.f,/*other*/ 92, true, 20.f, 0.f, 2},  // profile 1
    {"Adaptive",/*pi*/true, true, 0, 0.f, 0.f, 10, 3.f, 6.f, 600, true, 0.f,/*sk*/true, 0, 5, 3.f, 0.f, 0.f, 0.f, 0.f, 1, 3,/*tp*/true, false, 3.f, 9.f, 1, 3.f, 15, 0, 3.f, 2.f, 2.f, 1, 7.f, 15, 0, 7.f,/*pf*/true, 0.f, 0.f, 15, 0, 3.f, 2.5f, 1.f, 15, 0, 6.f,/*other*/ 93, true, 18.f, 0.f, 2},  // profile 2
    {"Filter 2.1",/*pi*/true, true, 10, 2.f, 3.f, 15, 1.f, 4.f, 600, true, 0.f,/*sk*/true, 7, 90, 0.f, 0.2f, 0.f, 0.f, 45.f, 0, 0,/*tp*/true, true, 9.f, 6.f, 1, 3.f, 15, 0, 3.f, 0.2f, 3.f, 0, 0.f, 8, 0, 0.f,/*pf*/true, 0.f, 0.f, 15, 0, 3.f, 2.5f, 1.f, 15, 0, 6.f,/*other*/ 93, true, 18.f, 0.f, 2},  // profile 3
    {"Blooming espresso",/*pi*/true, true, 10, 2.f, 3.f, 20, 7.f, 4.f, 650, true, 0.f,/*sk*/true, 7, 30, 0.f, 0.f, 0.6f, 0.f, 5.f, 5, 2,/*tp*/true, true, 9.f, 6.f, 1, 3.f, 15, 0, 2.f, 2.f, 2.f, 1, 9.f, 15, 4, 9.f,/*pf*/true, 0.f, 0.f, 15, 0, 3.f, 2.5f, 1.f, 15, 0, 6.f,/*other*/ 93, true, 18.f, 0.f, 2}  // profile 4
  };
}

#endif
