#ifndef JUST_DO_COFFEE_H
#define JUST_DO_COFFEE_H

#include "utils.h"
#include "../peripherals/peripherals.h"
#include "../peripherals/pump.h"
#include "../eeprom_data/eeprom_data.h"
#include "sensors_state.h"
#include <Arduino.h>


#define STEAM_TIMEOUT           900000
#define MAX_WATER_TEMP          105.f

void justDoCoffee(eepromValues_t &runningCfg, SensorState &currentState, bool brewActive, bool preinfusionFinished);
void pulseHeaters(uint32_t pulseLength, int factor_1, int factor_2, bool brewActive);
void steamCtrl(const eepromValues_t &runningCfg, SensorState &currentState, bool brewActive);
void hotWaterMode(SensorState &currentState);

#endif

