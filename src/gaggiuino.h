#ifndef GAGGIUINO_H
#define GAGGIUINO_H

#include "profiling_phases.h"
#include "log.h"
#include "eeprom_data.h"
#include "lcd/lcd.h"
#include "peripherals/pump.h"
#include "peripherals/pressure_sensor.h"
#include "peripherals/scales.h"
#include "peripherals/peripherals.h"
#include "sensors_state.h"
#include "functional/descale.h"
#include "functional/just_do_coffee.h"

// Define some const values
#define GET_KTYPE_READ_EVERY    250 // thermocouple data read interval not recommended to be changed to lower than 250 (ms)
#define GET_PRESSURE_READ_EVERY 10
#define GET_SCALES_READ_EVERY   100
#define REFRESH_SCREEN_EVERY    150 // Screen refresh interval (ms)
#define REFRESH_FLOW_EVERY      500



typedef enum {
    OPMODE_straight9Bar,
    OPMODE_justPreinfusion,
    OPMODE_justPressureProfile,
    OPMODE_manual,
    OPMODE_preinfusionAndPressureProfile,
    OPMODE_flush,
    OPMODE_descale,
    OPMODE_empty,
    OPMODE_justFlowBasedProfiling,
    OPMODE_steam,
    OPMODE_justFlowBasedPreinfusion,
    OPMODE_everythingFlowProfiled,
    OPMODE_pressureBasedPreinfusionAndFlowProfile
} OPERATION_MODES;

extern eepromValues_t runningCfg;
extern SensorState currentState;

//Timers
extern unsigned long pressureTimer;
extern unsigned long thermoTimer;
extern unsigned long scalesTimer;
extern unsigned long flowTimer;
extern unsigned long pageRefreshTimer;
extern unsigned long brewingTimer;

//scales vars
extern float shotWeight;
extern float previousWeight;
extern bool tareDone;

// brew detection vars
extern bool brewActive;

//PP&PI variables
extern int preInfusionFinishedPhaseIdx;
extern bool preinfusionFinished;
extern bool homeScreenScalesEnabled;

// Other util vars
extern float pressureTargetComparator;
#endif
