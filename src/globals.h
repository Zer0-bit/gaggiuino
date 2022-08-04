#ifndef GLOBALS_H
#define GLOBALS_H

#include "eeprom_data.h"

// Some vars are better global
//Timers
unsigned long pressureTimer = 0;
unsigned long thermoTimer = 0;
unsigned long scalesTimer = 0;
unsigned long flowTimer = 0;
unsigned long pageRefreshTimer = 0;
unsigned long brewingTimer = 0;

SensorState currentState;

//scales vars
float shotWeight;
float previousWeight;
bool tareDone = false;

// brew detection vars
bool brewActive;

//PP&PI variables
//default phases. Updated in updatePressureProfilePhases.
Phase phaseArray[6];
Phases phases {6,  phaseArray};
int preInfusionFinishedPhaseIdx = 3;
bool preinfusionFinished;

eepromValues_t runningCfg;

bool homeScreenScalesEnabled;

// Other util vars
float pressureTargetComparator;
#endif
