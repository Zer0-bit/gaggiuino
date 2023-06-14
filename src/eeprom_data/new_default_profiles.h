/* 09:32 15/03/2023 - change triggering comment */
#ifndef DEFAULT_PROFILES_H
#define DEFAULT_PROFILES_H

#include <Arduino.h>
#include <vector>
#include "profiling_phases.h"

const uint8_t DEFAULT_PROFILES_SIZE = 5;

const Profile defaultProfiles[DEFAULT_PROFILES_SIZE] = {
  Profile{
    .name = "IUIUIU Classic",
    .phases = {
    /*pre-infuse*/ Phase{.type = PhaseType::FLOW, .target = Transition(3.f), .restriction = 4.f, .stopConditions = {.time = 20000, .pressureAbove = 4.f, .weight = 4.f, .waterPumpedInPhase = 60.f}},
    /*soak      */ Phase{.type = PhaseType::FLOW, .target = Transition(0.f), .stopConditions = {.time = 30000, .pressureBelow = 2.f }},
    /*ramp      */ Phase{.type = PhaseType::PRESSURE, .target = Transition(7.5, TransitionCurve::EASE_OUT, 5000), .stopConditions = {.time = 5000}},
    /*adv-hold  */ Phase{.skip = true }, //off
    /*adv-slope */ Phase{.skip = true }, //off
    /*main-slope*/ Phase{.type = PhaseType::PRESSURE, .target = Transition(7.5f, 6.f, TransitionCurve::EASE_IN_OUT, 4000), .restriction = 3.f},
    },
    .globalStopConditions = GlobalStopConditions{.weight = 36.f},
    .waterTemperature = 93.f,
    .recipe = {.coffeeIn = 18.f, .ratio = 2.f},
  },
  Profile{
    .name = "Londinium",
    .phases = {
    /*pre-infuse*/ Phase{.type = PhaseType::FLOW, .target = Transition(9.f), .restriction = 4.f, .stopConditions = {.time = 10000, .pressureAbove = 4.f, .waterPumpedInPhase = 65.f}},
    /*soak      */ Phase{.type = PhaseType::FLOW, .target = Transition(0.f), .stopConditions = {.time = 10000, .pressureBelow = 0.7f }},
    /*ramp      */ Phase{.type = PhaseType::PRESSURE, .target = Transition(9.f, TransitionCurve::EASE_OUT, 1000), .stopConditions = {.time = 1000}},
    /*adv-hold  */ Phase{.type = PhaseType::PRESSURE, .target = Transition(9.f), .restriction = 3.f, .stopConditions = {.time = 4000}},
    /*adv-slope */ Phase{.type = PhaseType::PRESSURE, .skip = true }, //off
    /*main-slope*/ Phase{.type = PhaseType::PRESSURE, .target = Transition(9.f, 3.f, TransitionCurve::EASE_IN_OUT, 20000), .restriction = 3.f},
    },
    .globalStopConditions = GlobalStopConditions{.weight = 40.f},
    .waterTemperature = 92.f,
    .recipe = {.coffeeIn = 20.f, .ratio = 2.f},
  },
  Profile{
    .name = "Adaptive",
    .phases = {
    /*pre-infuse*/ Phase{.type = PhaseType::FLOW, .target = Transition(7.f), .restriction = 3.f, .stopConditions = {.time = 20000, .pressureAbove = 3.f, .waterPumpedInPhase = 60.f}},
    /*soak      */ Phase{.type = PhaseType::PRESSURE, .target = Transition(3.f), .stopConditions = {.time = 6000}},
    /*ramp      */ Phase{.skip = true }, //off
    /*adv-hold  */ Phase{.skip = true }, //off
    /*adv-slope */ Phase{.type = PhaseType::PRESSURE, .target = Transition(9.f, TransitionCurve::LINEAR, 5000), .stopConditions = {.time = 5000 }},
    /*main-slope*/ Phase{.type = PhaseType::FLOW, .target = Transition(2.5f, TransitionCurve::LINEAR, 25000), .restriction = 9.f},
    },
    .globalStopConditions = GlobalStopConditions{.weight = 33.f},
    .waterTemperature = 93.f,
    .recipe = {.coffeeIn = 15.f, .coffeeOut = 33.f},
  },
  Profile{
    .name = "Filter 2.1",
    .phases = {
    /*pre-infuse*/ Phase{.type = PhaseType::FLOW, .target = Transition(4.f), .restriction = 1.f, .stopConditions = {.time = 15000, .pressureAbove = 1.f, .waterPumpedInPhase = 60.f}},
    /*soak      */ Phase{.type = PhaseType::FLOW, .target = Transition(0.2f), .stopConditions = {.time = 90000, .weight = 45.f}},
    /*ramp      */ Phase{.skip = true }, //off
    /*adv-hold  */ Phase{.skip = true }, //off
    /*adv-slope */ Phase{.skip = true }, //off
    /*main-slope*/ Phase{.type = PhaseType::FLOW, .target = Transition(0.2f, 3.f, TransitionCurve::EASE_IN_OUT, 10000), .restriction = 9.f},
    },
    .globalStopConditions = GlobalStopConditions{.weight = 36.f},
    .waterTemperature = 89.f,
    .recipe = {.coffeeIn = 18.f, .ratio = 2.f},
  },
  Profile{
    .name = "Blooming espresso",
    .phases = {
    /*pre-infuse*/ Phase{.type = PhaseType::FLOW, .target = Transition(4.f), .restriction = 7.f, .stopConditions = {.time = 20000, .pressureAbove = 7.f, .waterPumpedInPhase = 65.f}},
    /*soak      */ Phase{.type = PhaseType::FLOW, .target = Transition(0.f), .stopConditions = {.time = 30000, .pressureBelow = 0.6f, .weight = 5.f}},
    /*ramp      */ Phase{.type = PhaseType::FLOW, .target = Transition(2.f, TransitionCurve::EASE_OUT, 5000), .stopConditions = {.time = 5000}},
    /*adv-hold  */ Phase{.skip = true }, //off
    /*adv-slope */ Phase{.skip = true }, //off
    /*main-slope*/ Phase{.type = PhaseType::FLOW, .target = Transition(2.f), .restriction = 9.f},
    },
    .globalStopConditions = GlobalStopConditions{.weight = 36.f},
    .waterTemperature = 93.f,
    .recipe = {.coffeeIn = 18.f, .ratio = 2.f},
  },
};

#endif
