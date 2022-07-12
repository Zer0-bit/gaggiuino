#ifndef GAGGIUINO_H
#define GAGGIUINO_H

typedef enum {
    OPMODE_straight9Bar,
    OPMODE_justPreinfusion,
    OPMODE_justPressureProfile,
    OPMODE_manual,
    OPMODE_preinfusionAndPressureProfile,
    OPMODE_flush,
    OPMODE_descale,
    OPMODE_backflush,
    OPMODE__empty,
    OPMODE_steam
} OPERATION_MODES;

#endif
