#ifndef STATE_H
#define STATE_H

#include "gaggia_settings.h"
#include "profiling_phases.h"
#include "system_state.h"
#include "../persistence/saved_profiles.h"

namespace state {
  void init();

// ---------------------------------------------------------------------------------
// ---------------------------------- SETTINGS -------------------------------------
// ---------------------------------------------------------------------------------
  const GaggiaSettings& getSettings();
  void updateAllSettings(const GaggiaSettings& settings);
  void updateBrewSettings(const BrewSettings& settings);
  void updateBoilerSettings(const BoilerSettings& settings);
  void updateLedSettings(const LedSettings& settings);
  void updateSystemSettings(const SystemSettings& settings);

  bool persistSettings();

// ---------------------------------------------------------------------------------
// ---------------------------------- PROFILES -------------------------------------
// ---------------------------------------------------------------------------------
  const Profile& getActiveProfile();
  void updateActiveProfile(const Profile& profile);
  ProfileId getActiveProfileId();
  bool updateActiveProfileId(ProfileId id);

  bool persistActiveProfileId();
  bool persistActiveProfile();

// ---------------------------------------------------------------------------------
// -------------------------------- SYSTEM_STATE -----------------------------------
// ---------------------------------------------------------------------------------
  OperationMode getOperationMode();
  void updateOperationMode(OperationMode operationMode);

// ---------------------------------------------------------------------------------
// --------------------------------- CALLBACKS -------------------------------------
// ---------------------------------------------------------------------------------
  void onActiveProfileUpdated(const Profile& profile);
  void onAllSettingsUpdated(const GaggiaSettings& settings);
  void onBrewSettingsUpdated(const BrewSettings& settings);
  void onBoilerSettingsUpdated(const BoilerSettings& settings);
  void onLedSettingsUpdated(const LedSettings& settings);
  void onSystemSettingsUpdated(const SystemSettings& settings);
  void onOperationModeUpdated(const OperationMode operationMode);
}

#endif
