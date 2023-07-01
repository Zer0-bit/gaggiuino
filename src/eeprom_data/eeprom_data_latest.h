#ifndef EEPROM_DATA_LATEST_H
#define EEPROM_DATA_LATEST_H

#include "gaggia_settings.h"
#include "eeprom_base.h"

// ---------------------------------------------------------------------------------------------
// ------------------------------------------ DATA ---------------------------------------------
// ---------------------------------------------------------------------------------------------
struct eepromMetadata_t {
  uint16_t version;
  unsigned long timestamp;
  size_t dataLength;
  uint32_t versionTimestampXOR;
};

struct eepromValues_t : public BaseValues_t {
  GaggiaSettings values;
};

#endif
