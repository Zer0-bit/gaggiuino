/* 09:32 15/03/2023 - change triggering comment */
#ifndef EEPROM_DATA_H
#define EEPROM_DATA_H

#include <Arduino.h>
#include "gaggia_settings.h"

#define EEPROM_DATA_VERSION 13

// BaseMetadata class. Used to support loading potentially different versions of metadata
struct BaseMetadata_t {
  uint16_t version;
  unsigned long timestamp;
};


// BaseValue class extended by all value structs to support migrations from different versions.
struct BaseValues_t {};

struct eepromMetadata_t: public BaseMetadata_t {
  size_t dataLength;
  uint32_t versionTimestampXOR;
};

struct eepromValues_t: public BaseValues_t {
  GaggiaSettings values;
};

void eepromInit(void);
bool eepromWrite(const GaggiaSettings);
GaggiaSettings eepromGetDefaultSettings(void);
GaggiaSettings eepromGetCurrentSettings(void);

#define ACTIVE_PROFILE(settings) settings.profiles.savedProfiles[settings.profiles.activeProfileIndex]

bool upgradeSchema_latest(BaseValues_t& source, BaseValues_t& target);

#endif
