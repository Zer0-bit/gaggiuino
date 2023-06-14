#ifndef NEXTION_PROFILES_H
#define NEXTION_PROFILES_H

#include <Arduino.h>
#include "profiling_phases.h"

#define PROFILE_NAME_LENGTH 25

#define PHASE_PI_INDEX 0
#define PHASE_SOAK_INDEX 1
#define PHASE_RAMP_INDEX 2
#define PHASE_TRANSITION_HOLD_INDEX 3
#define PHASE_TRANSITION_SLOPE_INDEX 4
#define PHASE_MAIN_SLOPE_INDEX 5

// Nextion specific profile definition
struct nextion_profile_t {
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
};

// Nextion -> Profile
void mapGlobalVarsFromNextion(const nextion_profile_t& nextionProfile, Profile& profile);
void mapPreinfusionPhaseFromNextion(const nextion_profile_t& nextionProfile, Profile& profile);
void mapSoakPhaseFromNextion(const nextion_profile_t& nextionProfile, Profile& profile);
void mapRampPhaseFromNextion(const nextion_profile_t& nextionProfile, Profile& profile);
void mapTransitionPhaseFromNextion(const nextion_profile_t& nextionProfile, Profile& profile);
void mapMainSlopeFromNextion(const nextion_profile_t& nextionProfile, Profile& profile);

void mapNextionProfileToProfile(const nextion_profile_t& nextionProfile, Profile& profile);

// Profile -> Nextion
void mapGlobalVarsToNextion(const Profile& profile, nextion_profile_t& nextionProfile);
void mapPreinfusionPhaseToNextion(const Profile& profile, nextion_profile_t& nextionProfile);
void mapSoakPhaseToNextion(const Profile& profile, nextion_profile_t& nextionProfile);
void mapRampPhaseToNextion(const Profile& profile, nextion_profile_t& nextionProfile);
void mapTransitionPhaseToNextion(const Profile& profile, nextion_profile_t& nextionProfile);
void mapMainSlopePhaseToNextion(const Profile& profile, nextion_profile_t& nextionProfile);

void mapProfileToNextionProfile(const Profile& profile, nextion_profile_t& nextionProfile);

#endif
