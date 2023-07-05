#include "shot_profiler.h"

//----------------------------------------------------------------------//
//-------------------------- PhaseProfiler -----------------------------//
//----------------------------------------------------------------------//

PhaseProfiler::PhaseProfiler() {}
PhaseProfiler::PhaseProfiler(const Profile& profile) : profile(&profile) {}

void PhaseProfiler::updatePhase(uint32_t timeInShot, SensorState& state) {
  size_t phaseIdx = currentPhaseIdx;
  uint32_t timeInPhase = timeInShot - phaseChangedSnapshot.timeInShot;

  if (phaseIdx >= profile->phaseCount() || profile->globalStopConditions.isReached(state, timeInShot)) {
    currentPhaseIdx = profile->phaseCount();
    currentPhase.update(currentPhaseIdx - 1, profile->phases[phaseIdx], timeInPhase);
    return;
  }

  if (!profile->phases[phaseIdx].skip && !profile->phases[phaseIdx].isStopConditionReached(state, timeInShot, phaseChangedSnapshot)) {
    currentPhase.update(phaseIdx, profile->phases[phaseIdx], timeInPhase);
    return;
  }

  currentPhase.update(phaseIdx, profile->phases[phaseIdx], timeInPhase);
  phaseChangedSnapshot = buildShotSnapshot(timeInShot, state, *this);
  currentPhaseIdx += 1;
  updatePhase(timeInShot, state);
}

// Gets the profiling phase we should be in based on the timeInShot and the Sensors state
const CurrentPhase& PhaseProfiler::getCurrentPhase() const {
  return currentPhase;
}

bool PhaseProfiler::isFinished() {
  return currentPhaseIdx >= profile->phaseCount();
}

void PhaseProfiler::reset() {
  currentPhaseIdx = 0;
  phaseChangedSnapshot = ShotSnapshot{};
  currentPhase.update(0, profile->phases[0], 0);
}

const Profile& PhaseProfiler::getActiveProfile() const {
  return *(profile);
}

//----------------------------------------------------------------------//
//--------------------------- CurrentPhase -----------------------------//
//----------------------------------------------------------------------//
CurrentPhase::CurrentPhase(int index, const Phase& phase, uint32_t timeInPhase, const ShotSnapshot& shotSnapshotAtStart) : index(index), phase{ &phase }, timeInPhase(timeInPhase), shotSnapshotAtStart{ &shotSnapshotAtStart } {}
CurrentPhase::CurrentPhase(const CurrentPhase& currentPhase) : index(currentPhase.index), phase{ currentPhase.phase }, timeInPhase(currentPhase.timeInPhase), shotSnapshotAtStart{ currentPhase.shotSnapshotAtStart } {}

const Phase* CurrentPhase::getPhase() const { return phase; }

PhaseType CurrentPhase::getType() const { return phase->type; }

int CurrentPhase::getIndex() const { return index; }

uint32_t CurrentPhase::getTimeInPhase() const { return timeInPhase; }

float CurrentPhase::getTarget() const { return phase->getTarget(timeInPhase, *shotSnapshotAtStart); }

float CurrentPhase::getRestriction() const { return phase->getRestriction(); }

void CurrentPhase::update(int index, const Phase& phase, uint32_t timeInPhase) {
  CurrentPhase::index = index;
  CurrentPhase::phase = &phase;
  CurrentPhase::timeInPhase = timeInPhase;
}

//----------------------------------------------------------------------//
//------------------------- ShotSnapshot -------------------------------//
//----------------------------------------------------------------------//

ShotSnapshot buildShotSnapshot(uint32_t timeInShot, const SensorState& state, const PhaseProfiler& profiler) {
  const CurrentPhase& phase = profiler.getCurrentPhase();
  const Profile& activeProfile = profiler.getActiveProfile();
  float targetFlow = (phase.getType() == PhaseType::FLOW) ? phase.getTarget() : phase.getRestriction();
  float targetPressure = (phase.getType() == PhaseType::PRESSURE) ? phase.getTarget() : phase.getRestriction();

  return ShotSnapshot{
    .timeInShot = timeInShot,
    .pressure = state.smoothedPressure,
    .pumpFlow = state.smoothedPumpFlow,
    .weightFlow = state.smoothedWeightFlow,
    .temperature = state.waterTemperature,
    .shotWeight = state.shotWeight,
    .waterPumped = state.waterPumped,
    .targetTemperature = activeProfile.waterTemperature,
    .targetPumpFlow = targetFlow,
    .targetPressure = targetPressure
  };
};
