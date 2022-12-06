#include "profiling_phases.h"

//----------------------------------------------------------------------//
//------------------------------ Phase ---------------------------------//
//----------------------------------------------------------------------//
float Phase::getTarget(unsigned long timeInPhase) {
  long transitionTime = max(0L, target.time > 0 ? target.time : stopConditions.phaseDuration);
  return mapRange(timeInPhase, 0, transitionTime, target.start, target.end, 1, target.curve);
}

float Phase::getRestriction() {
  return restriction;
}

bool Phase::isStopConditionReached(SensorState& currentState, unsigned long timeInPhase, unsigned long timeInShot) {
  return stopConditions.isReached(currentState, timeInPhase, timeInShot);
}

//----------------------------------------------------------------------//
//-------------------------- StopConditions ----------------------------//
//----------------------------------------------------------------------//
bool StopConditions::isReached(SensorState& state, long timeInPhase, long timeInShot) {
  float flow = state.weight > 0.4f ? state.weightFlow : state.smoothedPumpFlow;
  float stopWeight = weight + flow / 2.f;

  return (weight > 0 && state.shotWeight > stopWeight) ||
    (pressureAbove > 0 && state.pressure > pressureAbove) ||
    (pressureBelow > 0 && state.pressure < pressureBelow) ||
    (waterVolume > 0 && state.liquidPumped > waterVolume) ||
    (flowAbove > 0 && state.smoothedPumpFlow > flowAbove) ||
    (flowBelow > 0 && state.smoothedPumpFlow < flowBelow) ||
    (phaseDuration >= 0 && timeInPhase >= phaseDuration);
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
  unsigned long timeInPhase = timeInShot - phaseChangedTime;

  if (phaseIdx >= phases.count) {
    phaseIdx = phases.count - 1;
    currentPhase.update(phaseIdx, phases.phases[phaseIdx], timeInPhase);
    return;
  }

  if (!phases.phases[phaseIdx].isStopConditionReached(state, timeInPhase, timeInShot)) {
    currentPhase.update(phaseIdx, phases.phases[phaseIdx], timeInPhase);
    return;
  }

  Phase& phase = phases.phases[phaseIdx];
  long maxTimeAdvancement = (phase.stopConditions.phaseDuration > 0) ? phase.stopConditions.phaseDuration : timeInPhase;

  currentPhaseIdx += 1;
  phaseChangedTime += fmin(maxTimeAdvancement, timeInPhase);
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
  phaseChangedTime = 0;
}
