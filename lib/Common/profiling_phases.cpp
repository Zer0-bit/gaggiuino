/* 09:32 15/03/2023 - change triggering comment */
#include "profiling_phases.h"

//----------------------------------------------------------------------//
//------------------------------ Phase ---------------------------------//
//----------------------------------------------------------------------//
float Phase::getTarget(uint32_t timeInPhase, const ShotSnapshot& stateAtStart) const {
  uint32_t transitionTime = target.time;
  float startValue = target.start > 0.f
    ? target.start
    : type == PhaseType::FLOW ? stateAtStart.pumpFlow : stateAtStart.pressure;

  return mapRange(timeInPhase, 0.f, transitionTime, startValue, target.end, 1, target.curve);
}

float Phase::getRestriction() const {
  return restriction;
}

bool Phase::isStopConditionReached(SensorState& currentState, uint32_t timeInShot, ShotSnapshot stateAtPhaseStart) const {
  return stopConditions.isReached(currentState, timeInShot, stateAtPhaseStart);
}

//----------------------------------------------------------------------//
//-------------------------- StopConditions ----------------------------//
//----------------------------------------------------------------------//
/**
  * The method below predicts if we should already consider the condition achieved when we have a slow reaction time
  */
inline bool predictTargerAchieved(const float targetValue, const float currentValue, const float changeSpeed, const float reactionTime = 0.f) {
  if (changeSpeed == 0.f) { // protecting against zero speeds
    return currentValue == targetValue;
  }

  float remainingDose = targetValue - currentValue;
  float secondsRemaining = remainingDose / changeSpeed; // g / (g/sec) -> sec ;

  return secondsRemaining < reactionTime ? true : false;
}

bool PhaseStopConditions::isReached(SensorState& state, uint32_t timeInShot, ShotSnapshot stateAtPhaseStart) const {
  auto stopOn = this;
  uint32_t timeInPhase = timeInShot - stateAtPhaseStart.timeInShot;
  float flow = state.weight > 0.4f ? state.smoothedWeightFlow : state.smoothedPumpFlow;
  float currentWaterPumpedInPhase = state.waterPumped - stateAtPhaseStart.waterPumped;

  return (stopOn->time > 0L && timeInPhase >= static_cast<uint32_t>(stopOn->time)) ||
    (stopOn->weight > 0.f && state.shotWeight > stopOn->weight) ||
    (stopOn->pressureAbove > 0.f && state.smoothedPressure > stopOn->pressureAbove) ||
    (stopOn->pressureBelow > 0.f && state.smoothedPressure < stopOn->pressureBelow) ||
    (stopOn->waterPumpedInPhase > 0.f && currentWaterPumpedInPhase >= stopOn->waterPumpedInPhase) ||
    (stopOn->flowAbove > 0.f && state.smoothedPumpFlow > stopOn->flowAbove) ||
    (stopOn->flowBelow > 0.f && state.smoothedPumpFlow < stopOn->flowBelow);
}

bool GlobalStopConditions::isReached(const SensorState& state, uint32_t timeInShot) const {
  if (timeInShot < 1000) { // No shot lasts less than 1 second
    return false;
  }

  auto stopOn = this;
  float flow = state.weight > 0.4f ? state.smoothedWeightFlow : state.smoothedPumpFlow;

  return (stopOn->weight > 0.f && predictTargerAchieved(stopOn->weight, state.shotWeight, flow, 0.5f)) ||
    (stopOn->waterPumped > 0.f && state.waterPumped > stopOn->waterPumped) ||
    (stopOn->time > 0L && timeInShot >= stopOn->time);
}
