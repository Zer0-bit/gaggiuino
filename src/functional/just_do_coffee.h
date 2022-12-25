#ifndef JUST_DO_COFFEE_H
#define JUST_DO_COFFEE_H

#include "../utils.h"
#include "../peripherals/peripherals.h"
#include "../peripherals/pump.h"
#include "../eeprom_data/eeprom_data.h"
#include "../sensors_state.h"
#include <Arduino.h>


#define STEAM_TEMPERATURE         162.f
#define STEAM_WAND_HOT_WATER_TEMP 105.f
#define STEAM_TIMEOUT           900000
#define DELTA_RANGE             0.25f // % to apply as delta

// Steam global timer
extern unsigned long steamTime;

void justDoCoffee(eepromValues_t &runningCfg, SensorState &currentState, bool brewActive, bool preinfusionFinished);
void steamCtrl(eepromValues_t &runningCfg, SensorState &currentState, bool brewActive);

#endif

