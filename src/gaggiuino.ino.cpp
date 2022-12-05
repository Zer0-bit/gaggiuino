# 1 "C:\\Users\\victo\\AppData\\Local\\Temp\\tmp5ta7ginj"
#include <Arduino.h>
# 1 "C:/GIT/gaggiuino/src/gaggiuino.ino"
#if defined(DEBUG_ENABLED)
  #include "dbg.h"
#endif

#include "gaggiuino.h"

SimpleKalmanFilter smoothPressure(2.f, 2.f, 0.5f);
SimpleKalmanFilter smoothPumpFlow(2.f, 2.f, 0.5f);
SimpleKalmanFilter smoothScalesFlow(2.f, 2.f, 0.5f);


Phase phaseArray[8];
Phases phases {8, phaseArray};
PhaseProfiler phaseProfiler{phases};

PredictiveWeight predictiveWeight;


SensorState currentState;

OPERATION_MODES selectedOperationalMode;

eepromValues_t runningCfg;
void setup(void);
void loop(void);
static void sensorsRead(void);
static void sensorsReadTemperature(void);
static void sensorsReadWeight(void);
static void sensorsReadPressure(void);
static void sensorsReadFlow(float elapsedTime);
static void calculateWeightAndFlow(void);
static void pageValuesRefresh(bool forcedUpdate);
static void modeSelect(void);
static void lcdRefresh(void);
void lcdTrigger1(void);
void lcdTrigger2(void);
void lcdTrigger3(void);
void lcdTrigger4(void);
static void updatePressureProfilePhases(void);
void setFillBasketPhase(int phaseIdx, float flowRate);
void setPressurePhase(int phaseIdx, float startBar, float endBar, float flowRestriction, int timeMs, float pressureTarget, float weightTarget);
void setFlowPhase(int phaseIdx, float startFlow, float endFlow, float pressureRestriction, int timeMs, float pressureTarget, float weightTarget);
void setPhase(int phaseIdx, PHASE_TYPE type, float startValue, float endValue, float startRestriction, float endRestriction, int timeMs, float pressureTarget, float weightTarget);
static void profiling(void);
static void manualFlowControl(void);
static void brewDetect(void);
static void brewParamsReset(void);
static void flushActivated(void);
static void flushDeactivated(void);
static void fillBoiler(float targetBoilerFullPressure);
static void systemHealthCheck(float pressureThreshold);
#line 25 "C:/GIT/gaggiuino/src/gaggiuino.ino"
void setup(void) {
  LOG_INIT();
  LOG_INFO("Gaggiuino (fw: %s) booting", AUTO_VERSION);


  pinInit();
  LOG_INFO("Pin init");

  setBoilerOff();
  LOG_INFO("Boiler turned off");


  setPumpOff();
  LOG_INFO("Pump turned off");


  closeValve();
  LOG_INFO("Valve closed");

  lcdInit();
  LOG_INFO("LCD Init");

#if defined(DEBUG_ENABLED)

  dbgInit();
  LOG_INFO("DBG init");
#endif


  eepromInit();
  eepromValues_t eepromCurrentValues = eepromGetCurrentValues();
  LOG_INFO("EEPROM Init");

  thermocoupleInit();
  LOG_INFO("Thermocouple Init");

  lcdUploadCfg(eepromCurrentValues);
  LOG_INFO("LCD cfg uploaded");

  adsInit();
  LOG_INFO("Pressure sensor init");


  scalesInit(eepromCurrentValues.scalesF1, eepromCurrentValues.scalesF2);
  LOG_INFO("Scales init");


  pumpInit(eepromCurrentValues.powerLineFrequency, eepromCurrentValues.pumpFlowAtZero);
  LOG_INFO("Pump init");

  pageValuesRefresh(true);
  LOG_INFO("Setup sequence finished");
}







void loop(void) {
  pageValuesRefresh(false);
  lcdListen();
  sensorsRead();
  brewDetect();
  modeSelect();
  lcdRefresh();
  systemHealthCheck(0.8f);
}






static void sensorsRead(void) {
  sensorsReadTemperature();
  sensorsReadWeight();
  sensorsReadPressure();
  calculateWeightAndFlow();
  fillBoiler(2.0f);
}

static void sensorsReadTemperature(void) {
  if (millis() > thermoTimer) {
    currentState.temperature = thermocouple.readCelsius();
    thermoTimer = millis() + GET_KTYPE_READ_EVERY;
  }
}

static void sensorsReadWeight(void) {
  if (scalesIsPresent() && millis() > scalesTimer) {
    if(!tareDone) {
      scalesTare();
      if (!nonBrewModeActive && (scalesGetWeight() < -0.3f || scalesGetWeight() > 0.3f)) tareDone = false;
      else tareDone = true;
    }
    currentState.weight = scalesGetWeight();
    scalesTimer = millis() + GET_SCALES_READ_EVERY;
  }
}

static void sensorsReadPressure(void) {
  if (millis() > pressureTimer) {
    previousSmoothedPressure = currentState.smoothedPressure;
    currentState.pressure = getPressure();
    currentState.isPressureRising = isPressureRaising();
    currentState.isPressureRisingFast = currentState.smoothedPressure >= previousSmoothedPressure + 0.06f;
    currentState.isPressureFalling = isPressureFalling();
    currentState.isPressureFallingFast = isPressureFallingFast();
    currentState.smoothedPressure = smoothPressure.updateEstimate(currentState.pressure);
    pressureTimer = millis() + GET_PRESSURE_READ_EVERY;
  }
}

static void sensorsReadFlow(float elapsedTime) {
    long pumpClicks = getAndResetClickCounter();
    float cps = 1000.f * (float)pumpClicks / elapsedTime;
    currentState.pumpFlow = getPumpFlow(cps, currentState.smoothedPressure);

    previousSmoothedPumpFlow = currentState.smoothedPumpFlow;
    currentState.smoothedPumpFlow = smoothPumpFlow.updateEstimate(currentState.pumpFlow);
}

static void calculateWeightAndFlow(void) {
  long elapsedTime = millis() - flowTimer;

  if (brewActive) {
    if (scalesIsPresent()) {
      currentState.shotWeight = currentState.weight;
    }

    if (elapsedTime > REFRESH_FLOW_EVERY) {
      flowTimer = millis();
      sensorsReadFlow(elapsedTime);
      currentState.isPumpFlowRisingFast = currentState.smoothedPumpFlow > previousSmoothedPumpFlow + 0.45f;
      currentState.isPumpFlowFallingFast = currentState.smoothedPumpFlow < previousSmoothedPumpFlow - 0.45f;

      CurrentPhase& phase = phaseProfiler.getCurrentPhase(millis() - brewingTimer, currentState);
      predictiveWeight.update(currentState, phase, runningCfg);

      if (scalesIsPresent()) {
        currentState.weightFlow = fmaxf(0.f, (currentState.shotWeight - previousWeight) * 1000.f / (float)elapsedTime);
        currentState.weightFlow = smoothScalesFlow.updateEstimate(currentState.weightFlow);
        previousWeight = currentState.shotWeight;
      } else if (predictiveWeight.isOutputFlow()) {
        currentState.shotWeight += currentState.smoothedPumpFlow * (float)elapsedTime / 1000.f;
      }
      currentState.liquidPumped += currentState.smoothedPumpFlow * (float)elapsedTime / 1000.f;
    }
  } else {
    if (elapsedTime > REFRESH_FLOW_EVERY) {
      flowTimer = millis();
      sensorsReadFlow(elapsedTime);
    }
  }
}





static void pageValuesRefresh(bool forcedUpdate) {

  if ( lcdCurrentPageId != lcdLastCurrentPageId || forcedUpdate == true ) {
    runningCfg = lcdDownloadCfg();


    homeScreenScalesEnabled = lcdGetHomeScreenScalesEnabled();
    selectedOperationalMode = (OPERATION_MODES) lcdGetSelectedOperationalMode();

    updatePressureProfilePhases();

    lcdLastCurrentPageId = lcdCurrentPageId;
  }
}




static void modeSelect(void) {
  if (!startupInitFinished) return;

  switch (selectedOperationalMode) {

    case OPMODE_straight9Bar:
    case OPMODE_justPreinfusion:
    case OPMODE_justPressureProfile:
    case OPMODE_preinfusionAndPressureProfile:
    case OPMODE_justFlowBasedProfiling:
    case OPMODE_justFlowBasedPreinfusion:
    case OPMODE_everythingFlowProfiled:
    case OPMODE_pressureBasedPreinfusionAndFlowProfile:
      nonBrewModeActive = false;
      if (!steamState()) profiling();
      else steamCtrl(runningCfg, currentState, brewActive);
      break;
    case OPMODE_manual:
      nonBrewModeActive = false;
      manualFlowControl();
      break;
    case OPMODE_flush:
      nonBrewModeActive = true;
      brewActive ? flushActivated() : flushDeactivated();
      justDoCoffee(runningCfg, currentState, brewActive, preinfusionFinished);
      break;
    case OPMODE_steam:
      nonBrewModeActive = true;
      if (!steamState()) {
        brewActive ? flushActivated() : flushDeactivated();
        justDoCoffee(runningCfg, currentState, brewActive, preinfusionFinished);
      } else {
        steamCtrl(runningCfg, currentState, brewActive);
      }
      break;
    case OPMODE_descale:
      nonBrewModeActive = true;
      deScale(runningCfg, currentState);
      break;
    case OPMODE_empty:
      break;
    default:
      pageValuesRefresh(true);
      break;
  }
}





static void lcdRefresh(void) {

  if (millis() > pageRefreshTimer) {

    #ifdef BEAUTIFY_GRAPH
      lcdSetPressure(currentState.smoothedPressure * 10.f);
    #else
      lcdSetPressure(
        currentState.pressure > 0.f
          ? currentState.pressure * 10.f
          : 0.f
      );
    #endif


    lcdSetTemperature(currentState.temperature - runningCfg.offsetTemp);


    if (lcdCurrentPageId == 0 && homeScreenScalesEnabled) {
      lcdSetWeight(currentState.weight);
    } else {
      if (scalesIsPresent()) {
        lcdSetWeight(currentState.weight);
      } else if (currentState.shotWeight) {
        lcdSetWeight(currentState.shotWeight);
      }
    }


    if (lcdCurrentPageId == 1 || lcdCurrentPageId == 2 || lcdCurrentPageId == 8 ) {
      lcdSetFlow(
        currentState.weight > 0.4f
          ? currentState.weightFlow * 10.f
          : currentState.smoothedPumpFlow * 10.f
      );
    }

  #if defined DEBUG_ENABLED && defined stm32f411xx
    lcdShowDebug(readTempSensor(), getAdsError());
  #endif


    if (brewActive) {
      lcdSetBrewTimer((millis() > brewingTimer) ? (int)((millis() - brewingTimer) / 1000) : 0);
      lcdBrewTimerStart();
      lcdWarmupStateStop();
    } else {
      lcdBrewTimerStop();
    }

    pageRefreshTimer = millis() + REFRESH_SCREEN_EVERY;
  }
}




void lcdTrigger1(void) {
  LOG_VERBOSE("Saving values to EEPROM");
  bool rc;
  eepromValues_t eepromCurrentValues = eepromGetCurrentValues();
  eepromValues_t lcdValues = lcdDownloadCfg();

  switch (lcdCurrentPageId){
    case 1:
      break;
    case 2:
      break;
    case 3:

      eepromCurrentValues.pressureProfilingStart = lcdValues.pressureProfilingStart;
      eepromCurrentValues.pressureProfilingFinish = lcdValues.pressureProfilingFinish;
      eepromCurrentValues.pressureProfilingHold = lcdValues.pressureProfilingHold;
      eepromCurrentValues.pressureProfilingLength = lcdValues.pressureProfilingLength;
      eepromCurrentValues.pressureProfilingState = lcdValues.pressureProfilingState;
      eepromCurrentValues.preinfusionState = lcdValues.preinfusionState;
      eepromCurrentValues.preinfusionSec = lcdValues.preinfusionSec;
      eepromCurrentValues.preinfusionBar = lcdValues.preinfusionBar;
      eepromCurrentValues.preinfusionSoak = lcdValues.preinfusionSoak;
      eepromCurrentValues.preinfusionRamp = lcdValues.preinfusionRamp;


      eepromCurrentValues.preinfusionFlowState = lcdValues.preinfusionFlowState;
      eepromCurrentValues.preinfusionFlowVol = lcdValues.preinfusionFlowVol;
      eepromCurrentValues.preinfusionFlowTime = lcdValues.preinfusionFlowTime;
      eepromCurrentValues.preinfusionFlowSoakTime = lcdValues.preinfusionFlowSoakTime;
      eepromCurrentValues.preinfusionFlowPressureTarget = lcdValues.preinfusionFlowPressureTarget;
      eepromCurrentValues.flowProfileState = lcdValues.flowProfileState;
      eepromCurrentValues.flowProfileStart = lcdValues.flowProfileStart;
      eepromCurrentValues.flowProfileEnd = lcdValues.flowProfileEnd;
      eepromCurrentValues.flowProfilePressureTarget = lcdValues.flowProfilePressureTarget;
      eepromCurrentValues.flowProfileCurveSpeed = lcdValues.flowProfileCurveSpeed;
      break;
    case 4:
      eepromCurrentValues.homeOnShotFinish = lcdValues.homeOnShotFinish;
      eepromCurrentValues.graphBrew = lcdValues.graphBrew;
      eepromCurrentValues.brewDeltaState = lcdValues.brewDeltaState;
      eepromCurrentValues.warmupState = lcdValues.warmupState;
      eepromCurrentValues.switchPhaseOnThreshold = lcdValues.switchPhaseOnThreshold;
      break;
    case 5:
      eepromCurrentValues.stopOnWeightState = lcdValues.stopOnWeightState;
      eepromCurrentValues.shotDose = lcdValues.shotDose;
      eepromCurrentValues.shotPreset = lcdValues.shotPreset;
      eepromCurrentValues.shotStopOnCustomWeight = lcdValues.shotStopOnCustomWeight;
      break;
    case 6:
      eepromCurrentValues.setpoint = lcdValues.setpoint;
      eepromCurrentValues.steamSetPoint = lcdValues.steamSetPoint;
      eepromCurrentValues.offsetTemp = lcdValues.offsetTemp;
      eepromCurrentValues.hpwr = lcdValues.hpwr;
      eepromCurrentValues.mainDivider = lcdValues.mainDivider;
      eepromCurrentValues.brewDivider = lcdValues.brewDivider;
      break;
    case 7:
      eepromCurrentValues.powerLineFrequency = lcdValues.powerLineFrequency;
      eepromCurrentValues.lcdSleep = lcdValues.lcdSleep;
      eepromCurrentValues.scalesF1 = lcdValues.scalesF1;
      eepromCurrentValues.scalesF2 = lcdValues.scalesF2;
      eepromCurrentValues.pumpFlowAtZero = lcdValues.pumpFlowAtZero;
      break;
    default:
      break;
  }

  rc = eepromWrite(eepromCurrentValues);
  if (rc == true) {
    lcdShowPopup("UPDATE SUCCESSFUL!");
  } else {
    lcdShowPopup("ERROR!");
  }
}

void lcdTrigger2(void) {
  LOG_VERBOSE("Tare scales");
  if (scalesIsPresent()) scalesTare();
}

void lcdTrigger3(void) {
  LOG_VERBOSE("Scales enabled or disabled");
  homeScreenScalesEnabled = lcdGetHomeScreenScalesEnabled();
}

void lcdTrigger4(void) {
  LOG_VERBOSE("Predictive scales tare action completed!");
  if (!scalesIsPresent()) {
    if (currentState.shotWeight > 0.f) {
      currentState.shotWeight = 0.f;
      predictiveWeight.setIsForceStarted(true);
    } else predictiveWeight.setIsForceStarted(true);
  } else scalesTare();
}




static void updatePressureProfilePhases(void) {
  int phaseCount = 0;
  float preInfusionFinishBar = 0.f;
  float shotTarget = -1.f;

  if (runningCfg.stopOnWeightState) {
    shotTarget = (runningCfg.shotStopOnCustomWeight < 1.f)
      ? runningCfg.shotDose * runningCfg.shotPreset
      : runningCfg.shotStopOnCustomWeight;
  }


  setFillBasketPhase(phaseCount++, 6.f);


  if (runningCfg.preinfusionState) {
    if (runningCfg.preinfusionFlowState) {
      setFlowPhase(phaseCount++, runningCfg.preinfusionFlowVol, runningCfg.preinfusionFlowVol, runningCfg.preinfusionFlowPressureTarget, runningCfg.preinfusionFlowTime * 1000, runningCfg.preinfusionFlowPressureTarget, -1);
      setFlowPhase(phaseCount++,0 , 0, 0, runningCfg.preinfusionFlowSoakTime * 1000, -1, -1);
    } else {
      setPressurePhase(phaseCount++, runningCfg.preinfusionBar, runningCfg.preinfusionBar, 4.f, runningCfg.preinfusionSec * 1000, runningCfg.preinfusionBar, -1);
      setPressurePhase(phaseCount++, 0, 0, -1, runningCfg.preinfusionSoak * 1000, -1, -1);
      preInfusionFinishBar = runningCfg.preinfusionBar;
    }
  }
  preInfusionFinishedPhaseIdx = phaseCount;


  if (runningCfg.pressureProfilingState) {
    if (runningCfg.flowProfileState) {
      setFlowPhase(phaseCount++, runningCfg.flowProfileStart, runningCfg.flowProfileEnd, runningCfg.flowProfilePressureTarget, runningCfg.flowProfileCurveSpeed * 1000, -1, shotTarget);
      setFlowPhase(phaseCount++, runningCfg.flowProfileEnd, runningCfg.flowProfileEnd, runningCfg.flowProfilePressureTarget, -1, -1, shotTarget);
    } else {
      setPressurePhase(phaseCount++, preInfusionFinishBar, runningCfg.pressureProfilingStart, -1, runningCfg.preinfusionRamp * 1000, -1, -1);
      setPressurePhase(phaseCount++, runningCfg.pressureProfilingStart, runningCfg.pressureProfilingStart, -1, runningCfg.pressureProfilingHold * 1000, -1, shotTarget);
      setPressurePhase(phaseCount++, runningCfg.pressureProfilingStart, runningCfg.pressureProfilingFinish, -1, runningCfg.pressureProfilingLength * 1000, -1, shotTarget);
      setPressurePhase(phaseCount++, runningCfg.pressureProfilingFinish, runningCfg.pressureProfilingFinish, -1, -1, -1, shotTarget);
    }
  } else {
    setPressurePhase(phaseCount++, preInfusionFinishBar, 9, -1, runningCfg.preinfusionRamp * 1000, -1, shotTarget);
    setPressurePhase(phaseCount++, 9, 9, -1, -1, -1, shotTarget);
  }

  phases.count = phaseCount;
}

void setFillBasketPhase(int phaseIdx, float flowRate) {
  float pressureAbove = 0.1f;
  setFlowPhase(phaseIdx, flowRate, flowRate, -1, -1, pressureAbove, -1);
}

void setPressurePhase(int phaseIdx, float startBar, float endBar, float flowRestriction, int timeMs, float pressureTarget, float weightTarget) {
  setPhase(phaseIdx, PHASE_TYPE_PRESSURE, startBar, endBar, flowRestriction, flowRestriction, timeMs, pressureTarget, weightTarget);
}

void setFlowPhase(int phaseIdx, float startFlow, float endFlow, float pressureRestriction, int timeMs, float pressureTarget, float weightTarget) {
  setPhase(phaseIdx, PHASE_TYPE_FLOW, startFlow, endFlow, pressureRestriction, pressureRestriction, timeMs, pressureTarget, weightTarget);
}

void setPhase(int phaseIdx, PHASE_TYPE type, float startValue, float endValue, float startRestriction, float endRestriction, int timeMs, float pressureTarget, float weightTarget) {
  StopConditions stopConditions = StopConditions{
    timeMs,
    weightTarget,
    (runningCfg.switchPhaseOnThreshold) ? pressureTarget : -1,
    -1,
    -1,
    -1
  };

  phases.phases[phaseIdx].type = type;
  phases.phases[phaseIdx].startValue = startValue;
  phases.phases[phaseIdx].endValue = endValue;
  phases.phases[phaseIdx].startRestriction = startRestriction;
  phases.phases[phaseIdx].endRestriction = endRestriction;
  phases.phases[phaseIdx].stopConditions = stopConditions;
}

static void profiling(void) {
  if (brewActive) {
    long timeInShot = millis() - brewingTimer;
    CurrentPhase& currentPhase = phaseProfiler.getCurrentPhase(timeInShot, currentState);
    preinfusionFinished = currentPhase.getIndex() >= preInfusionFinishedPhaseIdx;

    if (phaseProfiler.isFinished()) {
      closeValve();
      setPumpOff();
      brewActive = false;
    } else if (currentPhase.getType() == PHASE_TYPE_PRESSURE) {
      float newBarValue = currentPhase.getTarget();
      float flowRestriction = currentPhase.getRestriction();
      openValve();
      setPumpPressure(newBarValue, flowRestriction, currentState);
    } else {
      float newFlowValue = currentPhase.getTarget();
      float pressureRestriction = currentPhase.getRestriction();
      openValve();
      setPumpFlow(newFlowValue, pressureRestriction, currentState);
    }
  } else {
    setPumpOff();
    closeValve();
  }

  justDoCoffee(runningCfg, currentState, brewActive, preinfusionFinished);
}

static void manualFlowControl(void) {
  if (brewActive) {
    openValve();
    float flow_reading = lcdGetManualFlowVol() / 10 ;
    setPumpFlow(flow_reading, 0.f, currentState);
    justDoCoffee(runningCfg, currentState, brewActive, preinfusionFinished);
  } else {
    closeValve();
    setPumpOff();
  }
}





static void brewDetect(void) {
  static bool paramsReset = true;

  if (brewState()) {
    if(!paramsReset) {
      brewParamsReset();
      paramsReset = true;
      brewActive = true;
    }


    systemHealthTimer = millis() + HEALTHCHECK_EVERY;
  } else {
    brewActive = false;
    if(paramsReset) {
      brewParamsReset();
      paramsReset = false;
    }
  }
}

static void brewParamsReset(void) {
  tareDone = false;
  currentState.shotWeight = 0.f;
  currentState.pumpFlow = 0.f;
  previousWeight = 0.f;
  currentState.weight = 0.f;
  currentState.liquidPumped = 0.f;
  preinfusionFinished = false;
  brewingTimer = millis();
  flowTimer = millis() + REFRESH_FLOW_EVERY;
  systemHealthTimer = millis() + HEALTHCHECK_EVERY;

  predictiveWeight.reset();
  phaseProfiler.reset();
}


static void flushActivated(void) {
  #if defined SINGLE_BOARD || defined LEGO_VALVE_RELAY
      openValve();
  #endif
  setPumpFullOn();
}

static void flushDeactivated(void) {
  #if defined SINGLE_BOARD || defined LEGO_VALVE_RELAY
      closeValve();
  #endif
  setPumpOff();
}

static void fillBoiler(float targetBoilerFullPressure) {
#if defined LEGO_VALVE_RELAY || defined SINGLE_BOARD
  static long elapsedTimeSinceStart = millis();
  lcdSetUpTime((millis() > elapsedTimeSinceStart) ? (int)((millis() - elapsedTimeSinceStart) / 1000) : 0);
  if (!startupInitFinished && lcdCurrentPageId == 0 && millis() - elapsedTimeSinceStart >= 3000) {
    unsigned long timePassed = millis() - elapsedTimeSinceStart;

    if (currentState.smoothedPressure < targetBoilerFullPressure && timePassed <= BOILER_FILL_TIMEOUT) {
      lcdShowPopup("Filling boiler!");
      openValve();
      setPumpToRawValue(80);
    } else if (!startupInitFinished) {
      closeValve();
      setPumpToRawValue(0);
      startupInitFinished = true;
    }
  }
#else
  startupInitFinished = true;
#endif
}

static void systemHealthCheck(float pressureThreshold) {
  #if defined LEGO_VALVE_RELAY || defined SINGLE_BOARD
  if (!brewState() && !steamState()) {
    if (millis() >= systemHealthTimer) {
      while (currentState.smoothedPressure >= pressureThreshold && currentState.temperature < STEAM_WAND_HOT_WATER_TEMP)
      {
        lcdShowPopup("Releasing pressure!");
        setPumpOff();
        setBoilerOff();
        openValve();
        sensorsRead();
      }
      closeValve();
      systemHealthTimer = millis() + HEALTHCHECK_EVERY;
    }
  }
  #endif



  while (currentState.temperature <= 0.0f || currentState.temperature == NAN || currentState.temperature >= 170.0f) {


    setPumpOff();
    setBoilerOff();
    if (millis() > thermoTimer) {
      LOG_ERROR("Cannot read temp from thermocouple (last read: %.1lf)!", static_cast<double>(currentState.temperature));
      lcdShowPopup("TEMP READ ERROR");
      currentState.temperature = thermocouple.readCelsius();
      thermoTimer = millis() + GET_KTYPE_READ_EVERY;
    }
  }
}