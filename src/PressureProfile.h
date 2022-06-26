#ifndef PRESSURE_PROFILER_H
#define PRESSURE_PROFILER_H

#include "utils.h"

struct Phase
{
    float startPressure;
    float endPressure;
    unsigned int durationMs;

    float getPressure(unsigned long timeInPhase)
    {
        return mapRange(timeInPhase, 0, durationMs, startPressure, endPressure, 1);
    }
};

struct CurrentPhase
{
    short phaseIndex;
    unsigned long timeInPhase;
};

struct Phases
{
    short count;
    Phase *phases;

    CurrentPhase getCurrentPhase(unsigned long timeInPP)
    {
        short phase = 0;
        unsigned long accumulatedTime = 0;

        while (phase < count - 1 && timeInPP >= accumulatedTime + (phases[phase].durationMs))
        {
            accumulatedTime += phases[phase].durationMs;
            phase += 1;
        }
        return CurrentPhase{phase, timeInPP - accumulatedTime};
    }
};

#endif
