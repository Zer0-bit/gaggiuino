#include "eeprom_migration.h"

// Initializing the eeprom value and migrator arrays.
eepromValueLoaderFunc_t        eepromValueLoaders[EEPROM_DATA_VERSION];
eepromValueMigrationFunc_t     eepromValueMigrators[EEPROM_DATA_VERSION];
eepromValueInstantiationFunc_t eepromValueInstantiators[EEPROM_DATA_VERSION];
std::vector<uint8_t>           eepromSupportedVersions;

using namespace std;

// It returns {true, values} on success - where values are the fully migrated values to targetVersion
// or {false, emptyValues} on error
bool migrate(uint8_t startingVersion, uint8_t targetVersion, BaseValues_t* targetValues) {

  // Load the starting version values
  eepromValueLoaderFunc_t loader = eepromValueLoaders[startingVersion];
  if (loader == nullptr) {
    return false;
  }

  auto currentVersionValues = loader();
  if (currentVersionValues.get() == nullptr) {
    return false;
  }

  // Iterate over the versions and migrate them accordingly
  for (uint8_t version = startingVersion; version < targetVersion - 1; version++) {
    // Fetch the migrator function for the next version and validate its existence
    eepromValueMigrationFunc_t migrateToNextVersion = eepromValueMigrators[version];
    if (migrateToNextVersion == nullptr) {
      return false;
    }

    // Instantiate an object of targetVersion type
    if (eepromValueInstantiators[version] == nullptr) {
      return false;
    }

    // Migrate to the next version. On error return
    unique_ptr<BaseValues_t> nextVersionValues = eepromValueInstantiators[version + 1]();
    bool result = migrateToNextVersion(*currentVersionValues, *nextVersionValues);
    if (!result) {
      return false;
    }

    currentVersionValues = std::move(nextVersionValues);
  }

  // Perform final migration step passing the final version defaults
  eepromValueMigrationFunc_t migrateToLastVersion = eepromValueMigrators[targetVersion - 1];
  if (migrateToLastVersion == nullptr) {
    return false;
  }

  migrateToLastVersion(*currentVersionValues, *targetValues);
  return true;
}
