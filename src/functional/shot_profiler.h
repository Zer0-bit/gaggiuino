#ifndef SHOT_PROFILER_H
#define SHOT_PROFILER_H

#include "profiling_phases.h"

class CurrentPhase {
private:
  int index;
  const Phase* phase;
  uint32_t timeInPhase;
  const ShotSnapshot* shotSnapshotAtStart;

public:
  CurrentPhase(int index, const Phase& phase, uint32_t timeInPhase, const ShotSnapshot& shotSnapshotAtStart);
  CurrentPhase(const CurrentPhase& currentPhase);

  const Phase* getPhase() const;
  PhaseType getType() const;
  int getIndex() const;
  uint32_t getTimeInPhase() const;
  float getTarget() const;
  float getRestriction() const;
  void update(int index, const Phase& phase, uint32_t timeInPhase);
};

class PhaseProfiler {
private:
  const Profile* profile = nullptr;
  size_t currentPhaseIdx = 0; // The index at which the profiler currently is.
  ShotSnapshot phaseChangedSnapshot = ShotSnapshot{ 0, 0, 0, 0, 0, 0 }; // State when the profiler move to this currentPhaseIdx
  CurrentPhase currentPhase = CurrentPhase(0, profile->phases[0], 0, phaseChangedSnapshot);

public:
  PhaseProfiler();
  PhaseProfiler(const Profile& profile);
  // Gets the profiling phase we should be in based on the timeInShot and the Sensors state
  void setProfile(const Profile& profile) { this->profile = &profile; };
  void updatePhase(uint32_t timeInShot, SensorState& state);
  const CurrentPhase& getCurrentPhase() const;
  bool isFinished();
  void reset();
  const Profile& getActiveProfile() const;
};

// Helper functions

ShotSnapshot buildShotSnapshot(uint32_t timeInShot, const SensorState& state, const PhaseProfiler& phase);

#endif
