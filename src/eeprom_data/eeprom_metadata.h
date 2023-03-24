/* 09:32 15/03/2023 - change triggering comment */
#ifndef EEPROM_METADATA_H
#define EEPROM_METADATA_H

#include <Arduino.h>

#define EEPROM_DATA_VERSION 6

#define EEPROM_METADATA_T(__eepromValues_tName) \
  {                                             \
    uint16_t version;                           \
    unsigned long timestamp;                    \
    struct __eepromValues_tName values;         \
    uint32_t versionTimestampXOR;               \
  }

struct eepromMetadata_t EEPROM_METADATA_T(eepromValues_t);

#define EEPROM_METADATA_LOADER(__eepromDataVersion, __eepromMetadata_tName, __upgradeSchema_fName)  \
  (eepromValues_t & targetValues)                                                                   \
  {                                                                                                 \
    __eepromMetadata_tName eepromMetadata;                                                          \
    EEPROM.get(0, eepromMetadata);                                                                  \
    uint32_t XOR = eepromMetadata.timestamp ^ eepromMetadata.version;                               \
    if (eepromMetadata.version != __eepromDataVersion || eepromMetadata.versionTimestampXOR != XOR) \
    {                                                                                               \
      return false;                                                                                 \
    }                                                                                               \
    return __upgradeSchema_fName(targetValues, eepromMetadata.values);                              \
  }

bool (*legacyEepromDataLoaders[EEPROM_DATA_VERSION])(eepromValues_t &);

#define REGISTER_LEGACY_EEPROM_DATA(version, eepromData_tName, upgradeSchema_fName)       \
  /* declare & define metadata */                                                         \
  struct _eepromMetadata_t_v##version EEPROM_METADATA_T(eepromData_tName);                \
  /* define loader function */                                                            \
  static bool _loadEepromMetadata_v##version                                              \
      EEPROM_METADATA_LOADER(version, _eepromMetadata_t_v##version, upgradeSchema_fName); \
  /* register loader ptr in array */                                                      \
  void __attribute__((constructor)) _registerEepromMetadataLoader_v##version()            \
  {                                                                                       \
    legacyEepromDataLoaders[version] = &_loadEepromMetadata_v##version;                   \
  }

#endif
