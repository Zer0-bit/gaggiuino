#ifndef STATE_H
#define STATE_H

#include "gaggia_settings.h"
#include "profiling_phases.h"
#include "../scales/ble_scales.h"
#include "system_state.h"
#include "../persistence/saved_profiles.h"

namespace state {
  void init();

// ---------------------------------------------------------------------------------
// ---------------------------------- SETTINGS -------------------------------------
// ---------------------------------------------------------------------------------
  const GaggiaSettings& getSettings();
  const SensorState& getSensor();
  void updateAllSettings(const GaggiaSettings& settings);
  void updateBrewSettings(const BrewSettings& settings);
  void updateBoilerSettings(const BoilerSettings& settings);
  void updateLedSettings(const LedSettings& settings);
  void updateSystemSettings(const SystemSettings& settings);
  void updateScalesSettings(const ScalesSettings& settings);

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
  SystemState getSystemState();
  void updateSystemState(const SystemState& command);
  void updateTarePending(bool tarePending);
  void updateOperationMode(OperationMode operationMode);


// ---------------------------------------------------------------------------------
// ----------------------------------- SCALES --------------------------------------
// ---------------------------------------------------------------------------------
  blescales::Scales getConnectedScales();
  void   updateConnectedScales(const blescales::Scales& scales);

// ---------------------------------------------------------------------------------
// --------------------------------- CALLBACKS -------------------------------------
// ---------------------------------------------------------------------------------
  void onActiveProfileUpdated(const Profile& profile);
  void onAllSettingsUpdated(const GaggiaSettings& settings);
  void onBrewSettingsUpdated(const BrewSettings& settings);
  void onBoilerSettingsUpdated(const BoilerSettings& settings);
  void onLedSettingsUpdated(const LedSettings& settings);
  void onSystemSettingsUpdated(const SystemSettings& settings);
  void onScalesSettingsUpdated(const ScalesSettings& settings);

  void onSystemStateUpdated(const SystemState& systemState);
  void onUpdateSystemStateCommandSubmitted(const UpdateSystemStateComand& command);
  void onConnectedBleScalesUpdated(const blescales::Scales& scales);
}

#endif
