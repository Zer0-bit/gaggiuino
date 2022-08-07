#ifndef JUST_DO_COFFEE_H
#define JUST_DO_COFFEE_H

#include <Arduino.h>
#include "../gaggiuino.h"

// #define BEAUTIFY_GRAPH
#define STEAM_TEMPERATURE         155.f
#define STEAM_WAND_HOT_WATER_TEMP 105.f
#define DELTA_RANGE             0.25f // % to apply as delta

//#############################################################################################
//#########################____NO_OPTIONS_ENABLED_POWER_CONTROL____############################
//#############################################################################################

//delta stuff
inline static float TEMP_DELTA(float d) { return (d*DELTA_RANGE); }

void justDoCoffee(void);
void steamCtrl(void);

#endif

