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
#include "peripherals/thermocouple.h"
#include "sensors_state.h"
#include "functional/descale.h"
#include "functional/just_do_coffee.h"

#include <Arduino.h>
#include <SimpleKalmanFilter.h>

// Define some const values
#ifdef SINGLE_BOARD
#define GET_KTYPE_READ_EVERY    70 // thermocouple data read interval not recommended to be changed to lower than 250 (ms)
#else
#define GET_KTYPE_READ_EVERY    250 // thermocouple data read interval not recommended to be changed to lower than 250 (ms)
#endif
#define GET_PRESSURE_READ_EVERY 10
#define GET_SCALES_READ_EVERY   100
#define REFRESH_SCREEN_EVERY    150 // Screen refresh interval (ms)
#define REFRESH_FLOW_EVERY      150
#define READ_TRAY_OFFSET_EVERY  1000
#define EMPTY_TRAY_WEIGHT       23456.f
#define TRAY_FULL_THRESHOLD     700.f



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


//Timers
unsigned long pageRefreshTimer = 0;
unsigned long pressureTimer = 0;
unsigned long brewingTimer = 0;
unsigned long thermoTimer = 0;
unsigned long scalesTimer = 0;
unsigned long flowTimer = 0;
unsigned long trayTimer = millis();

//scales vars
float previousWeight  = 0;
float brewStopWeight  = 0;
float shotWeight      = 0;
float shotTarget      = 0;
bool tareDone         = false;

// brew detection vars
bool brewActive = false;
bool nonBrewModeActive = false;

//PP&PI variables
int preInfusionFinishedPhaseIdx = 3;
bool preinfusionFinished = false;
bool homeScreenScalesEnabled = false;

// Other util vars
float smoothedPressure;
float previousSmoothedPressure;
float smoothedPumpFlow;
float previousSmoothedPumpFlow;
bool startupInitFinished;


#endif
