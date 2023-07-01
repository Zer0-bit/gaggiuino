/* 09:32 15/03/2023 - change triggering comment */
#ifndef EEPROM_DATA_H
#define EEPROM_DATA_H

#include <Arduino.h>
#include "gaggia_settings.h"
#include "eeprom_data_latest.h"

void eepromInit(void);
bool eepromWrite(const GaggiaSettings);
GaggiaSettings eepromGetDefaultSettings(void);
GaggiaSettings eepromGetCurrentSettings(void);

#endif
