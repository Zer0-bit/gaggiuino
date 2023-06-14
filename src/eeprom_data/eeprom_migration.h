#ifndef EEPROM_MIGRATION_H
#define EEPROM_MIGRATION_H

#include <Arduino.h>
#include <FlashStorage_STM32.hpp>
#include "eeprom_data.h"
#include <memory>

// This provider is so that we can use a fake EEPROM to run proper migration tests
// Notice it loads an EEPROMMock when RUNNING_TESTS is defined
class EEPROMProvider {
public:
#ifdef RUNNING_TESTS
  static EEPROMMockClass get() {
    return EEPROMMock;
  }
#else
  static EEPROMClass get() {
    return EEPROM;
  }
#endif
};
// << End EEPROMProvider

// This keeps an array of functions that know how to fetch the eeprom values given metadata as an input
using eepromValueLoaderFunc_t = std::unique_ptr<BaseValues_t>(*)(void);
// This keeps an array of functions that know how to migrate to the given version from the previous version
using eepromValueMigrationFunc_t = bool(*)(BaseValues_t& source, BaseValues_t& target);
// This keeps an array of functions that know how to instantiate a default class of a given version
using eepromValueInstantiationFunc_t = std::unique_ptr<BaseValues_t>(*)();

// Defining these arrays as `extern` to avoid multiple definition errors.
extern eepromValueLoaderFunc_t    eepromValueLoaders[EEPROM_DATA_VERSION];
extern eepromValueMigrationFunc_t eepromValueMigrators[EEPROM_DATA_VERSION];
extern eepromValueInstantiationFunc_t eepromValueInstantiators[EEPROM_DATA_VERSION];

bool migrate(uint8_t startingVersion, uint8_t targetVersion, BaseValues_t* targetValues);

template <typename T>
std::unique_ptr<BaseValues_t> instantiateEepromVersion() {
  return std::make_unique<T>();
}
template <typename T>
T getTargetDefaultsIfAvailable(BaseValues_t* targetDefaults) {
  return targetDefaults == nullptr ? T{} : static_cast<T&>(*targetDefaults);
}


#define EEPROM_VALUES_LOADER(__eepromDataVersion, __eepromMetadata_tName, __fetchValues_fName)       \
  ()                                                                                                 \
  {                                                                                                  \
    __eepromMetadata_tName eepromMetadata;                                                           \
    /* we read the metadata from EEPROM */                                                           \
    EEPROMProvider::get().get(0, eepromMetadata);                                                    \
    /* we validate all we can based on the version */                                                \
    uint32_t XOR = eepromMetadata.timestamp ^ eepromMetadata.version;                                \
    if (eepromMetadata.version != __eepromDataVersion || eepromMetadata.versionTimestampXOR != XOR) {\
      return nullptr;                                                                                \
    }                                                                                                \
    /* Invoke the provided function on the metadata. The function should return the eepromValues */  \
    return __fetchValues_fName(eepromMetadata);                                                      \
  }


#define REGISTER_LEGACY_EEPROM_DATA(version, eepromMetadata_tName, eepromValues_tName, fetchValues_fName) \
  std::unique_ptr<BaseValues_t> _loadEepromValues_v##version                           \
      EEPROM_VALUES_LOADER(version, eepromMetadata_tName, fetchValues_fName);          \
  /* register loader ptr in array */                                                   \
  void __attribute__((constructor)) _registerEepromValueLoader_v##version()            \
  {                                                                                    \
    eepromValueLoaders[version] = &_loadEepromValues_v##version;                       \
    eepromValueInstantiators[version] = &instantiateEepromVersion<eepromValues_tName>; \
  }

#define REGISTER_EEPROM_DATA_MIGRATOR(version, upgradeSchema_fName)                    \
  void __attribute__((constructor)) _registerEepromValueMigrator_v##version()          \
  {                                                                                    \
    eepromValueMigrators[version-1] = &upgradeSchema_fName;                            \
  }

#endif
