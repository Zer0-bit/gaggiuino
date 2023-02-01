#include "profiling_phases.h"

//----------------------------------------------------------------------//
//------------------------- ShotSnapshot -------------------------------//
//----------------------------------------------------------------------//

ShotSnapshot buildShotSnapshot(uint32_t timeInShot, SensorState& state, CurrentPhase& phase) {
  float targetFlow = (phase.getType() == PHASE_TYPE::PHASE_TYPE_FLOW) ? phase.getTarget() : phase.getRestriction();
  float targetPressure = (phase.getType() == PHASE_TYPE::PHASE_TYPE_PRESSURE) ? phase.getTarget() : phase.getRestriction();

  return ShotSnapshot{
    .timeInShot=timeInShot,
    .pressure=state.smoothedPressure,
    .pumpFlow=state.smoothedPumpFlow,
    .weightFlow=state.weightFlow,
    .temperature=state.temperature,
    .shotWeight=state.shotWeight,
    .waterPumped=state.waterPumped,
    .targetTemperature=-1,
    .targetPumpFlow=targetFlow,
    .targetPressure=targetPressure
  };
};

//----------------------------------------------------------------------//
//------------------------------ Phase ---------------------------------//
//----------------------------------------------------------------------//
float Phase::getTarget(uint32_t timeInPhase) const {
  long transitionTime = fmax(0L, target.time > 0L ? target.time : stopConditions.time);
  return mapRange(timeInPhase, 0.f, transitionTime, target.start, target.end, 1, target.curve);
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
bool PhaseStopConditions::isReached(SensorState& state, long timeInShot, ShotSnapshot stateAtPhaseStart) const {
  float flow = state.weight > 0.4f ? state.weightFlow : state.smoothedPumpFlow;
  float stopDelta = flow * (state.shotWeight / 100.f);

  return (time >= 0L && timeInShot - stateAtPhaseStart.timeInShot >= (uint32_t) time) ||
    (weight > 0.f && state.shotWeight > weight - stopDelta) ||
    (pressureAbove > 0.f && state.smoothedPressure > pressureAbove) ||
    (pressureBelow > 0.f && state.smoothedPressure < pressureBelow) ||
    (waterPumpedInPhase > 0.f && state.waterPumped - stateAtPhaseStart.waterPumped > waterPumpedInPhase - stopDelta) ||
    (flowAbove > 0.f && state.smoothedPumpFlow > flowAbove) ||
    (flowBelow > 0.f && state.smoothedPumpFlow < flowBelow);
}

bool GlobalStopConditions::isReached(SensorState& state, long timeInShot) {
  float flow = state.weight > 0.4f ? state.weightFlow : state.smoothedPumpFlow;
  float stopDelta = flow * (state.shotWeight / 100.f);

  return (weight > 0.f && state.shotWeight > weight - stopDelta) ||
    (waterPumped > 0.f && state.waterPumped > waterPumped) ||
    (time >= 0L && timeInShot >= time);
}

//----------------------------------------------------------------------//
//--------------------------- CurrentPhase -----------------------------//
//----------------------------------------------------------------------//
CurrentPhase::CurrentPhase(int index, const Phase& phase, uint32_t timeInPhase) : index(index), phase{ &phase }, timeInPhase(timeInPhase) {}
CurrentPhase::CurrentPhase(const CurrentPhase& currentPhase) : index(currentPhase.index), phase{ currentPhase.phase }, timeInPhase(currentPhase.timeInPhase) {}

Phase CurrentPhase::getPhase() { return *phase; }

PHASE_TYPE CurrentPhase::getType() { return phase->type; }

int CurrentPhase::getIndex() { return index; }

long CurrentPhase::getTimeInPhase() { return timeInPhase; }

float CurrentPhase::getTarget() { return phase->getTarget(timeInPhase); }

float CurrentPhase::getRestriction() { return phase->getRestriction(); }

void CurrentPhase::update(int index, const Phase& phase, uint32_t timeInPhase) {
  CurrentPhase::index = index;
  CurrentPhase::phase = &phase;
  CurrentPhase::timeInPhase = timeInPhase;
}

//----------------------------------------------------------------------//
//-------------------------- PhaseProfiler -----------------------------//
//----------------------------------------------------------------------//

PhaseProfiler::PhaseProfiler(Profile& profile) : profile(profile) {}

void PhaseProfiler::updatePhase(uint32_t timeInShot, SensorState& state) {
  size_t phaseIdx = currentPhaseIdx;
  uint32_t timeInPhase = timeInShot - phaseChangedSnapshot.timeInShot;

  if (phaseIdx >= profile.phaseCount() || profile.globalStopConditions.isReached(state, timeInShot)) {
    currentPhaseIdx = profile.phaseCount();
    phaseIdx = profile.phaseCount() - 1;
    currentPhase.update(phaseIdx, profile.phases[phaseIdx], timeInPhase);
    return;
  }

  if (!profile.phases[phaseIdx].isStopConditionReached(state, timeInShot, phaseChangedSnapshot)) {
    currentPhase.update(phaseIdx, profile.phases[phaseIdx], timeInPhase);
    return;
  }

  currentPhase.update(phaseIdx, profile.phases[phaseIdx], timeInPhase);
  phaseChangedSnapshot = buildShotSnapshot(timeInShot, state, currentPhase);
  currentPhaseIdx += 1;
  updatePhase(timeInShot, state);
}

// Gets the profiling phase we should be in based on the timeInShot and the Sensors state
CurrentPhase& PhaseProfiler::getCurrentPhase() {
  return currentPhase;
}

bool PhaseProfiler::isFinished() {
  return currentPhaseIdx >= profile.phaseCount();
}

void PhaseProfiler::reset() {
  currentPhaseIdx = 0;
  phaseChangedSnapshot = ShotSnapshot{};
  currentPhase.update(0, profile.phases[0], 0);
}
