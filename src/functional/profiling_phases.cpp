#include "profiling_phases.h"

//----------------------------------------------------------------------//
//------------------------------ Phase ---------------------------------//
//----------------------------------------------------------------------//
float Phase::getTarget(unsigned long timeInPhase) {
  long transitionTime = fmax(0L, target.time > 0 ? target.time : stopConditions.time);
  return mapRange(timeInPhase, 0, transitionTime, target.start, target.end, 1, target.curve);
}

float Phase::getRestriction() {
  return restriction;
}

bool Phase::isStopConditionReached(SensorState& currentState, unsigned long timeInShot, ShotSnapshot stateAtPhaseStart) {
  return stopConditions.isReached(currentState, timeInShot, stateAtPhaseStart);
}

//----------------------------------------------------------------------//
//-------------------------- StopConditions ----------------------------//
//----------------------------------------------------------------------//
bool PhaseStopConditions::isReached(SensorState& state, long timeInShot, ShotSnapshot stateAtPhaseStart) {
  float flow = state.weight > 0.4f ? state.weightFlow : state.smoothedPumpFlow;
  float stopDelta = flow / 2.f;

  return (time >= 0 && timeInShot - stateAtPhaseStart.timeInShot >= time) ||
    (weight > 0 && state.shotWeight > weight - stopDelta) ||
    (pressureAbove > 0 && state.pressure > pressureAbove) ||
    (pressureBelow > 0 && state.pressure < pressureBelow) ||
    (waterPumpedInPhase > 0 && state.waterPumped - stateAtPhaseStart.waterPumped > waterPumpedInPhase - stopDelta) ||
    (flowAbove > 0 && state.smoothedPumpFlow > flowAbove) ||
    (flowBelow > 0 && state.smoothedPumpFlow < flowBelow);
}

bool GlobalStopConditions::isReached(SensorState& state, long timeInShot) {
  float flow = state.weight > 0.4f ? state.weightFlow : state.smoothedPumpFlow;
  float stopDelta = flow / 2.f;

  return (weight > 0 && state.shotWeight > weight - stopDelta) ||
    (waterPumped > 0 && state.waterPumped > waterPumped) ||
    (time >= 0 && timeInShot >= time);
}

//----------------------------------------------------------------------//
//--------------------------- CurrentPhase -----------------------------//
//----------------------------------------------------------------------//
CurrentPhase::CurrentPhase(int index, Phase& phase, unsigned long timeInPhase) : index(index), phase{ phase }, timeInPhase(timeInPhase) {}
CurrentPhase::CurrentPhase(const CurrentPhase& currentPhase) : index(currentPhase.index), phase{ currentPhase.phase }, timeInPhase(currentPhase.timeInPhase) {}

Phase CurrentPhase::getPhase() { return phase; }

PHASE_TYPE CurrentPhase::getType() { return phase.type; }

int CurrentPhase::getIndex() { return index; }

long CurrentPhase::getTimeInPhase() { return timeInPhase; }

float CurrentPhase::getTarget() { return phase.getTarget(timeInPhase); }

float CurrentPhase::getRestriction() { return phase.getRestriction(); }

void CurrentPhase::update(int index, Phase& phase, unsigned long timeInPhase) {
  CurrentPhase::index = index;
  CurrentPhase::phase = phase;
  CurrentPhase::timeInPhase = timeInPhase;
}

//----------------------------------------------------------------------//
//-------------------------- PhaseProfiler -----------------------------//
//----------------------------------------------------------------------//

PhaseProfiler::PhaseProfiler(Phases& phases) : phases(phases) {}

void PhaseProfiler::updatePhase(long timeInShot, SensorState& state) {
  short phaseIdx = currentPhaseIdx;
  unsigned long timeInPhase = timeInShot - phaseChangedSnapshot.timeInShot;

  if (phaseIdx >= phases.count || globalStopConditions.isReached(state, timeInShot)) {
    currentPhaseIdx = phases.count;
    phaseIdx = phases.count - 1;
    currentPhase.update(phaseIdx, phases.phases[phaseIdx], timeInPhase);
    return;
  }

  if (!phases.phases[phaseIdx].isStopConditionReached(state, timeInShot, phaseChangedSnapshot)) {
    currentPhase.update(phaseIdx, phases.phases[phaseIdx], timeInPhase);
    return;
  }

  currentPhaseIdx += 1;
  long maxTimeAdvancement = (phases.phases[phaseIdx].stopConditions.time > 0) ? phases.phases[phaseIdx].stopConditions.time : timeInPhase;
  phaseChangedSnapshot = ShotSnapshot{phaseChangedSnapshot.timeInShot + maxTimeAdvancement, state.pressure, state.pumpFlow, state.temperature, state.shotWeight, state.waterPumped};
  updatePhase(timeInShot, state);
}

// Gets the profiling phase we should be in based on the timeInShot and the Sensors state
CurrentPhase& PhaseProfiler::getCurrentPhase() {
  return currentPhase;
}

bool PhaseProfiler::isFinished() {
  return currentPhaseIdx >= phases.count;
}

void PhaseProfiler::reset() {
  currentPhaseIdx = 0;
  phaseChangedSnapshot = ShotSnapshot{0, 0, 0, 0, 0, 0};
}

void PhaseProfiler::updateGlobalStopConditions(float weight, long time, float waterVolume) {
  globalStopConditions.weight = weight;
  globalStopConditions.time = time;
  globalStopConditions.waterPumped = waterVolume;
}
