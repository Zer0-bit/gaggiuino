#include "state.h"
#include "../persistence/persistence.h"
#include "../log/log.h"

namespace state {
  GaggiaSettings currentSettings;
  Profile activeProfile;
  ProfileId activeProfileId;
  OperationMode operationMode;

  void init() {
    currentSettings = persistence::getSettings();
    activeProfileId = persistence::getActiveProfileId();
    activeProfile = persistence::getProfile(activeProfileId).second;
    operationMode = OperationMode::BREW_AUTO;

    LOG_INFO("Initialized state. s.hpwr=%d, apId=%d, ap.name=%s", currentSettings.boiler.hpwr, activeProfileId, activeProfile.name.c_str());
  }

  // ------------------------------------------------------------------------------
  // -------------------------- SETTINGS RELATED STATE ----------------------------
  // ------------------------------------------------------------------------------

  const GaggiaSettings& getSettings() {
    return currentSettings;
  }

  // Updates the instance of `currentSettings`. Does not persist to NVS.
  // Call `persistSettings` to persist the running instance of settings
  void updateAllSettings(const GaggiaSettings& settings) {
    currentSettings = settings;
    onAllSettingsUpdated(currentSettings);
  }
  void updateBrewSettings(const BrewSettings& settings) {
    currentSettings.brew = settings;
    onBrewSettingsUpdated(currentSettings.brew);
  }
  void updateBoilerSettings(const BoilerSettings& settings) {
    currentSettings.boiler = settings;
    onBoilerSettingsUpdated(currentSettings.boiler);
  }
  void updateLedSettings(const LedSettings& settings) {
    currentSettings.led = settings;
    onLedSettingsUpdated(currentSettings.led);
  }
  void updateSystemSettings(const SystemSettings& settings) {
    currentSettings.system = settings;
    onSystemSettingsUpdated(currentSettings.system);
  }

  // Persists the `currentSettings` to NVS
  bool persistSettings() {
    persistence::saveSettings(currentSettings);
    return true;
  }


  // ------------------------------------------------------------------------------
  // --------------------------- PROFILE RELATED STATE ----------------------------
  // ------------------------------------------------------------------------------

  const Profile& getActiveProfile() {
    return activeProfile;
  }

  // Updates the instance of `activeProfile`. Does not persist to NVS.
  // Call `persistActiveProfile` to persist the instance of activeProfile
  void updateActiveProfile(const Profile& profile) {
    activeProfile = profile;
    onActiveProfileUpdated(activeProfile);
  }

  ProfileId getActiveProfileId() {
    return activeProfileId;
  }

  bool updateActiveProfileId(ProfileId id) {
    auto result = persistence::getProfile(id);
    if (!result.first) {
      return false;
    }

    activeProfileId = id;
    activeProfile = result.second;
    onActiveProfileUpdated(activeProfile);
    return true;
  };

  // Persists the `activeProfile` to NVS
  bool persistActiveProfile() {
    return persistence::saveProfile(activeProfileId, activeProfile);
  };

  bool persistActiveProfileId() {
    return persistence::saveActiveProfileId(activeProfileId);
  }

  // ---------------------------------------------------------------------------------
  // -------------------------------- SYSTEM_STATE -----------------------------------
  // ---------------------------------------------------------------------------------
  OperationMode getOperationMode() {
    return operationMode;
  }

  void updateOperationMode(OperationMode operationMode) {
    state::operationMode = operationMode;
    onOperationModeUpdated(state::operationMode);
  }
}
