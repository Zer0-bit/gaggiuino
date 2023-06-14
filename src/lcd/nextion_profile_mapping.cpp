#include "nextion_profile_mapping.h"
// #include <iostream>

// -----------------------------------------------------------------------------
// -----------------------------Nextion -> Profile------------------------------
// -----------------------------------------------------------------------------Ž
void mapNextionProfileToProfile(const nextion_profile_t& nextionProfile, Profile& profile) {
  if (profile.phaseCount() != PHASE_MAIN_SLOPE_INDEX + 1) {
    // This shouldn't happen when using nextion. Try to recover.
    profile.phases.clear();
    profile.phases.resize(PHASE_MAIN_SLOPE_INDEX + 1);
  }
  mapGlobalVarsFromNextion(nextionProfile, profile);
  mapPreinfusionPhaseFromNextion(nextionProfile, profile);
  mapSoakPhaseFromNextion(nextionProfile, profile);
  mapRampPhaseFromNextion(nextionProfile, profile);
  mapTransitionPhaseFromNextion(nextionProfile, profile);
  mapMainSlopeFromNextion(nextionProfile, profile);
}

// -----------------------------------------------------------------------------
// ---------------------------- Profile -> Nextion -----------------------------
// -----------------------------------------------------------------------------
void mapProfileToNextionProfile(const Profile& profile, nextion_profile_t& nextionProfile) {
  if (profile.phaseCount() < PHASE_MAIN_SLOPE_INDEX + 1) {
    return;
  }
  mapGlobalVarsToNextion(profile, nextionProfile);
  mapPreinfusionPhaseToNextion(profile, nextionProfile);
  mapSoakPhaseToNextion(profile, nextionProfile);
  mapRampPhaseToNextion(profile, nextionProfile);
  mapTransitionPhaseToNextion(profile, nextionProfile);
  mapMainSlopePhaseToNextion(profile, nextionProfile);
}

// -------------------------------------------------------------------------------
// ------------------------------- PREINFUSION -----------------------------------
// -------------------------------------------------------------------------------
void mapPreinfusionPhaseFromNextion(const nextion_profile_t& nextionProfile, Profile& profile) {
  Phase& piPhase = profile.phases[PHASE_PI_INDEX];
  uint32_t time = nextionProfile.preinfusionFlowState ? nextionProfile.preinfusionFlowTime : nextionProfile.preinfusionSec;
  float pressureAbove = -1.f;

  if (nextionProfile.preinfusionPressureAbove) {
    pressureAbove = nextionProfile.preinfusionFlowState
      ? nextionProfile.preinfusionFlowPressureTarget : nextionProfile.preinfusionBar;
  }

  piPhase.type = nextionProfile.preinfusionFlowState ? PhaseType::FLOW : PhaseType::PRESSURE;
  piPhase.skip = !nextionProfile.preinfusionState || time == 0;
  piPhase.target = piPhase.type == PhaseType::FLOW ? Transition(nextionProfile.preinfusionFlowVol) : Transition(nextionProfile.preinfusionBar);
  piPhase.restriction = piPhase.type == PhaseType::FLOW ? nextionProfile.preinfusionFlowPressureTarget : nextionProfile.preinfusionPressureFlowTarget;
  piPhase.stopConditions = {
    .time = time * 1000,
    .pressureAbove = pressureAbove,
    .weight = nextionProfile.preinfusionWeightAbove,
    .waterPumpedInPhase = nextionProfile.preinfusionFilled / 10.f,
  };
}

void mapPreinfusionPhaseToNextion(const Profile& profile, nextion_profile_t& nextionProfile) {
  const Phase& piPhase = profile.phases[PHASE_PI_INDEX];
  nextionProfile.preinfusionState = !piPhase.skip;
  nextionProfile.preinfusionFlowState = piPhase.type == PhaseType::FLOW;
  nextionProfile.preinfusionBar = piPhase.target.end;
  nextionProfile.preinfusionSec = piPhase.stopConditions.time / 1000;
  nextionProfile.preinfusionPressureFlowTarget = piPhase.restriction;
  nextionProfile.preinfusionFlowVol = piPhase.target.end;
  nextionProfile.preinfusionFlowTime = piPhase.stopConditions.time / 1000;
  nextionProfile.preinfusionFlowPressureTarget = piPhase.restriction;
  nextionProfile.preinfusionFilled = piPhase.stopConditions.waterPumpedInPhase * 10;
  nextionProfile.preinfusionPressureAbove = piPhase.stopConditions.pressureAbove > 0;
  nextionProfile.preinfusionWeightAbove = piPhase.stopConditions.weight;
}

// -------------------------------------------------------------------------------
// ----------------------------------- SOAK --------------------------------------
// -------------------------------------------------------------------------------
void mapSoakPhaseFromNextion(const nextion_profile_t& nextionProfile, Profile& profile) {
  Phase& soakPhase = profile.phases[PHASE_SOAK_INDEX];
  uint32_t time = nextionProfile.preinfusionFlowState ? nextionProfile.soakTimeFlow : nextionProfile.soakTimePressure;

  float maintainFlow = nextionProfile.soakKeepFlow;
  float maintainPressure = nextionProfile.soakKeepPressure;
  float isPressureBelow = nextionProfile.soakBelowPressure;
  float isPressureAbove = nextionProfile.soakAbovePressure;
  float isWeightAbove = nextionProfile.soakAboveWeight;

  soakPhase.type = maintainPressure > 0.f ? PhaseType::PRESSURE : PhaseType::FLOW;
  soakPhase.skip = !nextionProfile.soakState || time == 0;
  soakPhase.target = Transition(soakPhase.type == PhaseType::PRESSURE ? maintainPressure : maintainFlow);
  soakPhase.restriction = soakPhase.type == PhaseType::PRESSURE ? maintainFlow : 0;
  soakPhase.stopConditions = PhaseStopConditions{
    .time = time * 1000,
    .pressureAbove = isPressureAbove,
    .pressureBelow = isPressureBelow,
    .weight = isWeightAbove,
  };
}

void mapSoakPhaseToNextion(const Profile& profile, nextion_profile_t& nextionProfile) {
  const Phase& soakPhase = profile.phases[PHASE_SOAK_INDEX];
  nextionProfile.soakState = !soakPhase.skip;
  nextionProfile.soakTimePressure = soakPhase.stopConditions.time / 1000;
  nextionProfile.soakTimeFlow = soakPhase.stopConditions.time / 1000;
  nextionProfile.soakKeepPressure = soakPhase.type == PhaseType::PRESSURE ? soakPhase.target.end : 0;
  nextionProfile.soakKeepFlow = soakPhase.type == PhaseType::FLOW ? soakPhase.target.end : 0;
  nextionProfile.soakBelowPressure = soakPhase.stopConditions.pressureBelow;
  nextionProfile.soakAbovePressure = soakPhase.stopConditions.pressureAbove;
  nextionProfile.soakAboveWeight = soakPhase.stopConditions.weight;
}

// -------------------------------------------------------------------------------
// ----------------------------------- RAMP --------------------------------------
// -------------------------------------------------------------------------------
void mapRampPhaseFromNextion(const nextion_profile_t& nextionProfile, Profile& profile) {
  Phase& rampPhase = profile.phases[PHASE_RAMP_INDEX];
  uint32_t rampTime = nextionProfile.preinfusionRamp * 1000;
  TransitionCurve rampCurve = (TransitionCurve)nextionProfile.preinfusionRampSlope;

  // Here we try to find the next possible phase to ramp to so that we can set the rampTarget and rampType correctly
  // If profiling is disable we default to ramping to a classic 9bar shot.
  PhaseType rampType = PhaseType::PRESSURE;
  float rampTarget = 9.f;
  if (nextionProfile.profilingState) {
    if (nextionProfile.tpState) {
      if (nextionProfile.tpType) { // Advanced Profile FLOW type
        rampType = PhaseType::FLOW;
        rampTarget = nextionProfile.tfProfileStart > 0 ? nextionProfile.tfProfileStart : nextionProfile.tfProfileEnd;
      }
      else { // Advanced Profile PRESSURE type
        rampType = PhaseType::PRESSURE;
        rampTarget = nextionProfile.tpProfilingStart > 0 ? nextionProfile.tpProfilingStart : nextionProfile.tpProfilingFinish;
      }
    }
    else if (nextionProfile.mfProfileState) { // Main Profile FLOW type
      rampType = PhaseType::FLOW;
      rampTarget = nextionProfile.mfProfileStart > 0 ? nextionProfile.mfProfileStart : nextionProfile.mfProfileEnd;
    }
    else { // Main Profile PRESSURE type
      rampType = PhaseType::PRESSURE;
      rampTarget = nextionProfile.mpProfilingStart > 0 ? nextionProfile.mpProfilingStart : nextionProfile.mpProfilingFinish;
    }
  }

  rampPhase.type = rampType;
  rampPhase.skip = rampTime == 0;
  rampPhase.target = Transition(rampTarget, rampCurve, rampTime);
  rampPhase.stopConditions = { .time = rampTime };
}

void mapRampPhaseToNextion(const Profile& profile, nextion_profile_t& nextionProfile) {
  const Phase& rampPhase = profile.phases[PHASE_RAMP_INDEX];
  nextionProfile.preinfusionRamp = rampPhase.skip ? 0 : rampPhase.stopConditions.time / 1000;
  nextionProfile.preinfusionRampSlope = (uint16_t)rampPhase.target.curve;
}

// -------------------------------------------------------------------------------
// ----------------------------- TRANSITION PHASE --------------------------------
// -------------------------------------------------------------------------------
void mapTransitionPhaseFromNextion(const nextion_profile_t& nextionProfile, Profile& profile) {
  Phase& holdPhase = profile.phases[PHASE_TRANSITION_HOLD_INDEX];
  Phase& slopePhase = profile.phases[PHASE_TRANSITION_SLOPE_INDEX];
  PhaseType phaseType = nextionProfile.tpType ? PhaseType::FLOW : PhaseType::PRESSURE;

  uint32_t holdTime = (nextionProfile.tpType ? nextionProfile.tfProfileHold : nextionProfile.tpProfilingHold) * 1000;
  float start = phaseType == PhaseType::FLOW ? nextionProfile.tfProfileStart : nextionProfile.tpProfilingStart;
  float holdRestriction = phaseType == PhaseType::FLOW ? nextionProfile.tfProfileHoldLimit : nextionProfile.tpProfilingHoldLimit;

  holdPhase.type = phaseType;
  holdPhase.skip = !nextionProfile.tpState || holdTime == 0;
  holdPhase.target = Transition(start);
  holdPhase.restriction = holdRestriction;
  holdPhase.stopConditions = { .time = holdTime };

  float slopeRestriction = phaseType == PhaseType::FLOW ? nextionProfile.tfProfilingPressureRestriction : nextionProfile.tpProfilingFlowRestriction;
  float end = phaseType == PhaseType::FLOW ? nextionProfile.tfProfileEnd : nextionProfile.tpProfilingFinish;
  TransitionCurve slopeCurve = (TransitionCurve)(phaseType == PhaseType::FLOW ? nextionProfile.tfProfileSlopeShape : nextionProfile.tpProfilingSlopeShape);
  uint32_t slopeTime = phaseType == PhaseType::FLOW ? nextionProfile.tfProfileSlope * 1000 : nextionProfile.tpProfilingSlope * 1000;

  slopePhase.type = phaseType;
  slopePhase.skip = !nextionProfile.tpState || slopeTime == 0;
  slopePhase.target = Transition(start, end, slopeCurve, slopeTime);
  slopePhase.restriction = slopeRestriction;
  slopePhase.stopConditions = { .time = slopeTime };
}

void mapTransitionPhaseToNextion(const Profile& profile, nextion_profile_t& nextionProfile) {
  const Phase& holdPhase = profile.phases[PHASE_TRANSITION_HOLD_INDEX];
  const Phase& slopePhase = profile.phases[PHASE_TRANSITION_SLOPE_INDEX];

  float start = slopePhase.skip ? holdPhase.target.end : slopePhase.target.start;
  float end   = slopePhase.skip ? 0 : slopePhase.target.end;

  nextionProfile.tpState = !slopePhase.skip || !holdPhase.skip;
  nextionProfile.tpType = !slopePhase.skip ? slopePhase.type == PhaseType::FLOW : holdPhase.type == PhaseType::FLOW;

  nextionProfile.tpProfilingHold = holdPhase.stopConditions.time / 1000;
  nextionProfile.tpProfilingHoldLimit = holdPhase.restriction;
  nextionProfile.tfProfileHold = holdPhase.stopConditions.time / 1000;
  nextionProfile.tfProfileHoldLimit = holdPhase.restriction;

  nextionProfile.tpProfilingStart = start;
  nextionProfile.tpProfilingFinish = end;
  nextionProfile.tfProfileStart = start;
  nextionProfile.tfProfileEnd = end;

  nextionProfile.tpProfilingSlope = slopePhase.stopConditions.time / 1000;
  nextionProfile.tpProfilingSlopeShape = (uint16_t)slopePhase.target.curve;
  nextionProfile.tpProfilingFlowRestriction = slopePhase.restriction;
  nextionProfile.tfProfileSlope = slopePhase.stopConditions.time / 1000;
  nextionProfile.tfProfileSlopeShape = (uint16_t)slopePhase.target.curve;
  nextionProfile.tfProfilingPressureRestriction = slopePhase.restriction;
}

// -------------------------------------------------------------------------------
// -------------------------------- MAIN SLOPE -----------------------------------
// -------------------------------------------------------------------------------
void mapMainSlopeFromNextion(const nextion_profile_t& nextionProfile, Profile& profile) {
  Phase& slopePhase = profile.phases[PHASE_MAIN_SLOPE_INDEX];

  if (nextionProfile.profilingState) {
    PhaseType phaseType = nextionProfile.mfProfileState ? PhaseType::FLOW : PhaseType::PRESSURE;
    uint32_t time = phaseType == PhaseType::FLOW ? nextionProfile.mfProfileSlope * 1000 : nextionProfile.mpProfilingSlope * 1000;
    float start = phaseType == PhaseType::FLOW ? nextionProfile.mfProfileStart : nextionProfile.mpProfilingStart;
    float end = phaseType == PhaseType::FLOW ? nextionProfile.mfProfileEnd : nextionProfile.mpProfilingFinish;
    TransitionCurve curve = (TransitionCurve)(phaseType == PhaseType::FLOW ? nextionProfile.mfProfileSlopeShape : nextionProfile.mpProfilingSlopeShape);
    float restriction = phaseType == PhaseType::FLOW ? nextionProfile.mfProfilingPressureRestriction : nextionProfile.mpProfilingFlowRestriction;

    slopePhase.type = phaseType;
    slopePhase.skip = false;
    slopePhase.target = Transition(start, end, curve, time);
    slopePhase.restriction = restriction;
    slopePhase.stopConditions = {};
  }
  else { // Fallback to flat 9bar
    slopePhase.type = PhaseType::PRESSURE;
    slopePhase.skip = false;
    slopePhase.target = Transition(9);
    slopePhase.restriction = 0;
    slopePhase.stopConditions = {};
  }
}

void mapMainSlopePhaseToNextion(const Profile& profile, nextion_profile_t& nextionProfile) {
  const Phase& slopePhase = profile.phases[PHASE_MAIN_SLOPE_INDEX];
  nextionProfile.profilingState = !slopePhase.skip;
  nextionProfile.mfProfileState = slopePhase.type == PhaseType::FLOW;
  nextionProfile.mpProfilingStart = slopePhase.target.start;
  nextionProfile.mpProfilingFinish = slopePhase.target.end;
  nextionProfile.mpProfilingSlope = slopePhase.target.time / 1000;
  nextionProfile.mpProfilingSlopeShape = (uint16_t)slopePhase.target.curve;
  nextionProfile.mpProfilingFlowRestriction = slopePhase.restriction;
  nextionProfile.mfProfileStart = slopePhase.target.start;
  nextionProfile.mfProfileEnd = slopePhase.target.end;
  nextionProfile.mfProfileSlope = slopePhase.target.time / 1000;
  nextionProfile.mfProfileSlopeShape = (uint16_t)slopePhase.target.curve;
  nextionProfile.mfProfilingPressureRestriction = slopePhase.restriction;
}

// -------------------------------------------------------------------------------
// ------------------------------- Global Vars -----------------------------------
// -------------------------------------------------------------------------------
void mapGlobalVarsFromNextion(const nextion_profile_t& nextionProfile, Profile& profile) {
  // Profile name
  profile.name = std::string(nextionProfile.name);

  // Brew Recipe
  profile.recipe.coffeeIn = nextionProfile.shotDose;
  profile.recipe.coffeeOut = nextionProfile.shotStopOnCustomWeight;
  profile.recipe.ratio = (float) nextionProfile.shotPreset;

  // Global stop conditions
  profile.globalStopConditions = {
    .weight = nextionProfile.stopOnWeightState ? profile.recipe.getCoffeeOut() : 0,
  };

  // Water temperature
  profile.waterTemperature = nextionProfile.setpoint;
}


void mapGlobalVarsToNextion(const Profile& profile, nextion_profile_t& nextionProfile) {
  // Profile name
  size_t length = std::min(profile.name.size(), sizeof(nextionProfile.name) - 1);
  strncpy(nextionProfile.name, profile.name.c_str(), length);
  nextionProfile.name[length] = '\0';

  // Brew Settings
  nextionProfile.shotDose = profile.recipe.coffeeIn;
  nextionProfile.shotPreset = (uint16_t)profile.recipe.ratio;
  nextionProfile.shotStopOnCustomWeight = profile.recipe.coffeeOut;

  // Global stop conditions
  nextionProfile.stopOnWeightState = profile.globalStopConditions.weight > 0.f;

  // Water temperature
  nextionProfile.setpoint = (uint16_t)profile.waterTemperature;
}
