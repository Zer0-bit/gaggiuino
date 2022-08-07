#ifndef JUST_DO_COFFEE_H
#define JUST_DO_COFFEE_H

#include "../utils.h"
#include "../peripherals/peripherals.h"
#include "../peripherals/pump.h"
#include "../eeprom_data.h"
#include "../sensors_state.h"
#include <Arduino.h>


#define STEAM_TEMPERATURE         155.f
#define STEAM_WAND_HOT_WATER_TEMP 105.f
#define DELTA_RANGE             0.25f // % to apply as delta

extern eepromValues_t runningCfg;
extern SensorState currentState;

extern bool brewActive;
extern bool preinfusionFinished;

//delta stuff
inline static float TEMP_DELTA(float d) { return (d*DELTA_RANGE); }

void justDoCoffee(void);
void steamCtrl(void);

#endif

