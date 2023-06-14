/* 09:32 15/03/2023 - change triggering comment */
#include "eeprom_data.h"
#include <FlashStorage_STM32.hpp>
#include "proto/settings_converters.h"
#include "proto/proto_serializer.h"
#include "default_settings.h"
#include "eeprom_migration.h"
#include "../log.h"

#include "legacy/eeprom_data_v12.h"

#define MAX_PROFILES 5

namespace eeprom {
  struct eepromMetadata_t metadata;
  struct eepromValues_t   values;
}

bool validateSettings(const GaggiaSettings newGaggiaSettings);
bool eepromWrite(const GaggiaSettings newGaggiaSettings) {
  if (!validateSettings(newGaggiaSettings)) return false;

  /* Serializing and saving the settings and versioning metadata */
  eeprom::values.values = newGaggiaSettings;
  std::vector<uint8_t> dataInBytes = ProtoSerializer::serialize<GaggiaSettingsConverter>(eeprom::values.values);

  eeprom::metadata.version = EEPROM_DATA_VERSION;
  eeprom::metadata.timestamp = millis();
  eeprom::metadata.dataLength = dataInBytes.size();
  eeprom::metadata.versionTimestampXOR = eeprom::metadata.timestamp ^ eeprom::metadata.version;

  // Disable auto commit so we can flush the buffer manually.
  bool commitAsapBackup = EEPROMProvider::get().getCommitASAP();
  EEPROMProvider::get().setCommitASAP(false);

  // Perform the data transfer for the metadata and the settings
  size_t metaDataSize = sizeof(eeprom::metadata);
  EEPROMProvider::get().put(0, eeprom::metadata);

  for (size_t i = 0; i < dataInBytes.size(); ++i) {
    EEPROMProvider::get().write(metaDataSize + i, dataInBytes[i]);
  }

  EEPROMProvider::get().commit();

  // Restore original value of commit ASAP
  EEPROMProvider::get().setCommitASAP(commitAsapBackup);

  return true;
}

bool loadCurrentEepromData() {
  EEPROMProvider::get().get(0, eeprom::metadata);
  size_t metaDataSize = sizeof(eeprom::metadata);

  std::vector<uint8_t> serializedSettings;
  serializedSettings.resize(eeprom::metadata.dataLength);

  for (size_t i = 0; i < eeprom::metadata.dataLength; i++) {
    serializedSettings[i] = EEPROMProvider::get().read(metaDataSize + i);
  }

  eeprom::values.values = {}; // This is important to make sure we pass an empty state for deserialization
  return ProtoSerializer::deserialize<GaggiaSettingsConverter>(serializedSettings, eeprom::values.values);
}

void eepromInit(void) {
  eeprom::values.values = eepromGetDefaultSettings();

  // read version
  uint16_t version;
  EEPROMProvider::get().get(0, version);

  // load appropriate version (including current)
  bool readSuccess = false;

  if (version == EEPROM_DATA_VERSION) {
    readSuccess = loadCurrentEepromData();
  }
  else if (version < EEPROM_DATA_VERSION) {
    readSuccess = migrate(version, EEPROM_DATA_VERSION, &eeprom::values);
  }

  if (!readSuccess) {
    LOG_ERROR("SECU_CHECK FAILED! Applying defaults! eeprom::metadata.version=%d", version);
  }

  if (!readSuccess || version != EEPROM_DATA_VERSION) {
    eepromWrite(eeprom::values.values);
  }
}

GaggiaSettings eepromGetCurrentSettings(void) {
  return eeprom::values.values;
}

GaggiaSettings eepromGetDefaultSettings(void) {
  return getDefaultGaggiaSettings();
}

/* Spot-check various settings values */
bool validateSettings(const GaggiaSettings newGaggiaSettings) {
  const char* errMsg = "Data out of range";
  if (newGaggiaSettings.profiles.savedProfiles.size() > MAX_PROFILES) {
    LOG_ERROR(errMsg);
    return false;
  }

  for (auto profile : newGaggiaSettings.profiles.savedProfiles) {
    if (profile.name.length() == 0
      || profile.phaseCount() == 0
      || profile.phaseCount() > 8
      || profile.waterTemperature < 1)
    {
      LOG_ERROR(errMsg);
      return false;
    }
  }

  if (newGaggiaSettings.boiler.steamSetPoint < 1
    || newGaggiaSettings.boiler.steamSetPoint > 165
    || newGaggiaSettings.boiler.mainDivider < 1
    || newGaggiaSettings.boiler.brewDivider < 1
    || newGaggiaSettings.system.pumpFlowAtZero < 0.210f
    || newGaggiaSettings.system.pumpFlowAtZero > 0.310f
    || newGaggiaSettings.system.scalesF1 < -20000
    || newGaggiaSettings.system.scalesF2 > 20000)
  {
    LOG_ERROR(errMsg);
    return false;
  }

  return true;
}

// We always register the latest migration.
// This way we don't need to keep changing the last version in 2 places
REGISTER_EEPROM_DATA_MIGRATOR(EEPROM_DATA_VERSION, upgradeSchema_latest);

static void mapToNextionProfile(eepromValues_t_v12::profile_t& input, nextion_profile_t& output);
bool upgradeSchema_latest(BaseValues_t& input, BaseValues_t& output) {
  eepromValues_t_v12& source = static_cast<eepromValues_t_v12&>(input);
  eepromValues_t& target = static_cast<eepromValues_t&>(output);

  target.values.boiler.steamSetPoint = source.steamSetPoint;
  target.values.boiler.offsetTemp = source.offsetTemp;
  target.values.boiler.hpwr = source.hpwr;
  target.values.boiler.mainDivider = source.mainDivider;
  target.values.boiler.brewDivider = source.brewDivider;
  target.values.system.powerLineFrequency = source.powerLineFrequency;
  target.values.system.lcdSleep = source.lcdSleep;
  target.values.system.warmupState = source.warmupState;
  target.values.system.lcdSleep = source.lcdSleep;
  target.values.brew.homeOnShotFinish = source.homeOnShotFinish;
  target.values.brew.brewDeltaState = source.brewDeltaState;
  target.values.brew.basketPrefill = source.basketPrefill;
  target.values.system.scalesF1 = source.scalesF1;
  target.values.system.scalesF2 = source.scalesF2;
  target.values.system.pumpFlowAtZero = source.pumpFlowAtZero;
  target.values.profiles.activeProfileIndex = source.activeProfile;
  target.values.led.state  = source.ledState;
  target.values.led.color.R = source.ledR;
  target.values.led.color.G = source.ledG;
  target.values.led.color.B = source.ledB;

  if (target.values.profiles.savedProfiles.size() < 5) {
    target.values.profiles.savedProfiles.resize(5);
  }

  for (int i = 0; i < 5; i++) {
    Profile& targetProfile = target.values.profiles.savedProfiles[i];
    nextion_profile_t nextionProfile;
    mapToNextionProfile(source.profiles[i], nextionProfile);
    mapNextionProfileToProfile(nextionProfile, targetProfile);
  }
  return true;
}

static void mapToNextionProfile(eepromValues_t_v12::profile_t& input, nextion_profile_t& output) {
  strcpy(output.name, input.name);
  output.preinfusionState = input.preinfusionState;
  output.preinfusionFlowState = input.preinfusionFlowState;
  output.preinfusionSec = input.preinfusionSec;
  output.preinfusionBar = input.preinfusionBar;
  output.preinfusionFlowVol = input.preinfusionFlowVol;
  output.preinfusionFlowTime = input.preinfusionFlowTime;
  output.preinfusionFlowPressureTarget = input.preinfusionFlowPressureTarget;
  output.preinfusionPressureFlowTarget = input.preinfusionPressureFlowTarget;
  output.preinfusionFilled = input.preinfusionFilled;
  output.preinfusionPressureAbove = input.preinfusionPressureAbove;
  output.preinfusionWeightAbove = input.preinfusionWeightAbove;
  output.soakState = input.soakState;
  output.soakTimePressure = input.soakTimePressure;
  output.soakTimeFlow = input.soakTimeFlow;
  output.soakKeepPressure = input.soakKeepPressure;
  output.soakKeepFlow = input.soakKeepFlow;
  output.soakBelowPressure = input.soakBelowPressure;
  output.soakAbovePressure = input.soakAbovePressure;
  output.soakAboveWeight = input.soakAboveWeight;
  output.preinfusionRamp = input.preinfusionRamp;
  output.preinfusionRampSlope = input.preinfusionRampSlope;
  output.tpState = input.tpState;
  output.tpType = input.tpType;
  output.tpProfilingStart = input.tpProfilingStart;
  output.tpProfilingFinish = input.tpProfilingFinish;
  output.tpProfilingHold = input.tpProfilingHold;
  output.tpProfilingHoldLimit = input.tpProfilingHoldLimit;
  output.tpProfilingSlope = input.tpProfilingSlope;
  output.tpProfilingSlopeShape = input.tpProfilingSlopeShape;
  output.tpProfilingFlowRestriction = input.tpProfilingFlowRestriction;
  output.tfProfileStart = input.tfProfileStart;
  output.tfProfileEnd = input.tfProfileEnd;
  output.tfProfileHold = input.tfProfileHold;
  output.tfProfileHoldLimit = input.tfProfileHoldLimit;
  output.tfProfileSlope = input.tfProfileSlope;
  output.tfProfileSlopeShape = input.tfProfileSlopeShape;
  output.tfProfilingPressureRestriction = input.tfProfilingPressureRestriction;
  output.profilingState = input.profilingState;
  output.mfProfileState = input.mfProfileState;
  output.mpProfilingStart = input.mpProfilingStart;
  output.mpProfilingFinish = input.mpProfilingFinish;
  output.mpProfilingSlope = input.mpProfilingSlope;
  output.mpProfilingSlopeShape = input.mpProfilingSlopeShape;
  output.mpProfilingFlowRestriction = input.mpProfilingFlowRestriction;
  output.mfProfileStart = input.mfProfileStart;
  output.mfProfileEnd = input.mfProfileEnd;
  output.mfProfileSlope = input.mfProfileSlope;
  output.mfProfileSlopeShape = input.mfProfileSlopeShape;
  output.mfProfilingPressureRestriction = input.mfProfilingPressureRestriction;
  output.setpoint = input.setpoint;
  output.stopOnWeightState = input.stopOnWeightState;
  output.shotDose = input.shotDose;
  output.shotStopOnCustomWeight = input.shotStopOnCustomWeight;
  output.shotPreset = input.shotPreset;
}
