/* 09:32 15/03/2023 - change triggering comment */
#include "eeprom_data.h"

#include "eeprom_migration.h"
#include "proto/proto_serializer.h"
#include "proto/settings_converters.h"
#include "eeprom_data_latest.h"

#include "../log.h"

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
  return {};
}

/* Spot-check various settings values */
bool validateSettings(const GaggiaSettings newGaggiaSettings) {
  const char* errMsg = "Data out of range";

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
