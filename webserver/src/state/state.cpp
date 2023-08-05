#include "state.h"
#include "../persistence/persistence.h"
#include "../log/log.h"

namespace state {
  GaggiaSettings currentSettings;
  Profile activeProfile;
  ProfileId activeProfileId;
  SystemState systemState;
  blescales::Scales connectedScales;

  void init() {
    currentSettings = persistence::getSettings();
    activeProfileId = persistence::getActiveProfileId();
    activeProfile = persistence::getProfile(activeProfileId).second;

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
  void updateScalesSettings(const ScalesSettings& settings) {
    currentSettings.scales = settings;
    onScalesSettingsUpdated(currentSettings.scales);
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

  // Persists the `activeProfileId` to NVS
  bool persistActiveProfileId() {
    return persistence::saveActiveProfileId(activeProfileId);
  }

  // ---------------------------------------------------------------------------------
  // -------------------------------- SYSTEM_STATE -----------------------------------
  // ---------------------------------------------------------------------------------
  SystemState getSystemState() {
    return systemState;
  }

  void updateSystemState(const SystemState& newState) {
    systemState = newState;
    onSystemStateUpdated(systemState);
  }

  void sumitUpdateSystemStateCommand(const UpdateSystemStateComand& command) {
    systemState.tarePending = command.tarePending;
    systemState.operationMode = command.operationMode;
    onUpdateSystemStateCommandSubmitted(command);
  }

  void updateTarePending(bool tarePending) {
    sumitUpdateSystemStateCommand({ .operationMode = systemState.operationMode, .tarePending = tarePending });
  }

  void updateOperationMode(OperationMode operationMode) {
    sumitUpdateSystemStateCommand({ .operationMode = operationMode, .tarePending = systemState.tarePending });
  }

  // ---------------------------------------------------------------------------------
  // ----------------------------------- SCALES --------------------------------------
  // ---------------------------------------------------------------------------------
  blescales::Scales getConnectedScales() {
    return connectedScales;
  }

  void updateConnectedScales(const blescales::Scales& scales) {
    if (scales.address == connectedScales.address && scales.name == connectedScales.name) return;

    connectedScales = scales;
    onConnectedBleScalesUpdated(connectedScales);
  }
}
