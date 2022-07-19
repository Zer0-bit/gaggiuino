#ifndef GAGGIUINO_H
#define GAGGIUINO_H

// Define some const values
#define GET_KTYPE_READ_EVERY    250 // thermocouple data read interval not recommended to be changed to lower than 250 (ms)
#define GET_PRESSURE_READ_EVERY 10
#define GET_SCALES_READ_EVERY   100
#define REFRESH_SCREEN_EVERY    150 // Screen refresh interval (ms)
#define REFRESH_FLOW_EVERY      1000
#define DESCALE_PHASE1_EVERY    500 // short pump pulses during descale
#define DESCALE_PHASE2_EVERY    5000 // short pause for pulse effficience activation
#define DESCALE_PHASE3_EVERY    120000 // long pause for scale softening
#define DELTA_RANGE             0.25f // % to apply as delta
// #define BEAUTIFY_GRAPH
#define STEAM_TEMPERATURE         155.f
#define STEAM_WAND_HOT_WATER_TEMP 105.f


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

#endif
