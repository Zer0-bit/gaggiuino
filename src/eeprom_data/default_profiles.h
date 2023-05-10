/* 09:32 15/03/2023 - change triggering comment */
#ifndef DEFAULT_PROFILES_H
#define DEFAULT_PROFILES_H

#include <Arduino.h>
#include "eeprom_data.h"

namespace {
  typedef struct {
    char     name[50];
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
  } profileDefaults_t;

  // TO-DO:: Actually customise the profiles vals to follow the names
  const profileDefaults_t defaultsProfile[MAX_PROFILES] = {
    {"IUIUIU Classic", true, true, 10, 2.f, 3.f, 20, 4.f, 3.f, 600, true, 4.f, true, 7, 30, 0.f, 0.f, 2.f, 0.f, 4.f, 5, 2, true, true, 9.f, 6.f, 1, 3.f, 15, 0, 3.f, 2.5f, 1.f, 1, 7.5f, 15, 0, 6.f},  // profile 0
    {"Londinium", true, true, 10, 2.f, 3.f, 20, 4.f, 9.f, 650, true, 0.f, true, 7, 10, 0.f, 0.f, 0.7f, 0.f, 0.f, 5, 2, true, false, 9.f, 3.f, 1, 3.f, 20, 0, 3.f, 2.5f, 1.f, 1, 7.5f, 15, 0, 6.f},  // profile 1
    {"D-Flow", true, true, 10, 2.f, 3.f, 20, 3.f, 7.f, 600, true, 0.f, true, 7, 6, 3.f, 0.8f, 0.f, 0.f, 0.f, 5, 2, true, true, 9.f, 6.f, 1, 3.f, 15, 0, 3.f, 2.f, 2.f, 1, 7.f, 15, 0, 7.f},  // profile 2
    {"Filter 2.1", true, true, 10, 2.f, 3.f, 5, 0.f, 4.f, 600, true, 0.f, true, 7, 90, 0.f, 0.2f, 0.f, 0.f, 45.f, 0, 0, true, true, 9.f, 6.f, 1, 3.f, 15, 0, 3.f, 0.2f, 3.f, 0, 0.f, 8, 0, 0.f},  // profile 3
    {"Blooming espresso", true, true, 10, 2.f, 3.f, 20, 7.f, 4.f, 650, true, 0.f, true, 7, 30, 0.f, 0.f, 0.6f, 0.f, 5.f, 5, 2, true, true, 9.f, 6.f, 1, 3.f, 15, 0, 2.f, 2.f, 2.f, 1, 9.f, 15, 4, 9.f}  // profile 4
  };
}

#endif
