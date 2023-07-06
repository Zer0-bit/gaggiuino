/* 09:32 15/03/2023 - change triggering comment */
#pragma GCC optimize ("Ofast")
// #define STM32F4 // This define has to be here otherwise the include of FlashStorage_STM32.h bellow fails.
#if defined(DEBUG_ENABLED)
  #include "dbg.h"
#endif
#include "gaggiuino.h"

SimpleKalmanFilter smoothPressure(0.6f, 0.6f, 0.06f);
SimpleKalmanFilter smoothPumpFlow(0.1f, 0.1f, 0.01f);
SimpleKalmanFilter smoothScalesFlow(0.5f, 0.5f, 0.01f);
SimpleKalmanFilter smoothConsideredFlow(0.1f, 0.1f, 0.1f);

Profile manualProfile;
Profile activeProfile;
PhaseProfiler phaseProfiler;

PredictiveWeight predictiveWeight;

SensorState currentState;

GaggiaSettings runningCfg;

SystemState systemState;

LED led;
TOF tof;

void setup(void) {
  LOG_INIT();
  LOG_INFO("Gaggiuino (fw: %s) booting", AUTO_VERSION);

  // Various pins operation mode handling
  pinInit();
  LOG_INFO("Pin init");

  setBoilerOff();  // relayPin LOW
  setSteamValveRelayOff();
  setSteamBoilerRelayOff();
  LOG_INFO("Boiler turned off");

  //Pump
  setPumpOff();
  LOG_INFO("Pump turned off");

  // Valve
  closeValve();
  LOG_INFO("Valve closed");

#if defined(DEBUG_ENABLED)
  // Debug init if enabled
  dbgInit();
  LOG_INFO("DBG init");
#endif

  // Initialise comms library for talking to the ESP mcu
  espCommsInit();

  // Initialize LED
  led.begin();
  led.setColor(9u, 0u, 9u); // WHITE
  // Init the tof sensor
  tof.init(systemState);

  cpsInit(runningCfg);
  LOG_INFO("CPS Init");

  thermocoupleInit();
  LOG_INFO("Thermocouple Init");

  adsInit();
  LOG_INFO("Pressure sensor init");

  // Scales handling
  scalesInit(runningCfg.system.scalesF1, runningCfg.system.scalesF2);
  LOG_INFO("Scales init");

  // Pump init
  pumpInit(currentState.powerLineFrequency, runningCfg.system.pumpFlowAtZero);
  LOG_INFO("Pump init");

  // Change LED colour on setup exit.
  led.setColor(9u, 0u, 9u); // 64171

  iwdcInit();
}

//##############################################################################################################################
//############################################________________MAIN______________################################################
//##############################################################################################################################


//Main loop where all the logic is continuously run
void loop(void) {
  fillBoiler();
  sensorsRead();
  brewDetect();
  modeSelect();
  espUpdateState();
  sysHealthCheck(SYS_PRESSURE_IDLE);
}

//##############################################################################################################################
//#############################################___________SENSORS_READ________##################################################
//##############################################################################################################################


static void sensorsRead(void) {
  sensorReadSwitches();
  espCommsReadData();
  sensorsReadTemperature();
  sensorsReadWeight();
  sensorsReadPressure();
  calculateWeightAndFlow();
  updateStartupTimer();
  readTankWaterLevel();
  doLed();
}

static void sensorReadSwitches(void) {
  currentState.brewSwitchState = brewState();
  currentState.steamSwitchState = steamState();
  currentState.hotWaterSwitchState = waterPinState() || (currentState.brewSwitchState && currentState.steamSwitchState); // use either an actual switch, or the GC/GCP switch combo
}

static void sensorsReadTemperature(void) {
  if (millis() > thermoTimer) {
    currentState.temperature = thermocoupleRead() - runningCfg.boiler.offsetTemp;

    float brewTempSetPoint = activeProfile.waterTemperature + runningCfg.boiler.offsetTemp;

    currentState.waterTemperature = (currentState.temperature > activeProfile.waterTemperature && currentState.brewSwitchState)
      ? currentState.temperature / (float)brewTempSetPoint + activeProfile.waterTemperature
      : currentState.temperature;

    thermoTimer = millis() + GET_KTYPE_READ_EVERY;
  }
}

static Measurement handleTaringAndReadWeight() {
  if (!currentState.tarePending) { // No tare needed just get weight
    return scalesGetWeight();
  }

  // Tare is required. Invoke it.
  scalesTare();
  weightMeasurements.clear();
  Measurement weight = scalesGetWeight();

  if (fabsf(weight.value < 0.3f) && fabsf(weight.value > -0.2f)) { // Tare was successful. return reading
    currentState.tarePending = false;
    return weight;
  } else {  // Tare was unsuccessful. return 0 weight.
    return Measurement{ .value=0.f, .millis = millis()};
  }
}

static void sensorsReadWeight(void) {
  uint32_t elapsedTime = millis() - scalesTimer;

  if (elapsedTime > GET_SCALES_READ_EVERY) {
    systemState.scalesPresent = scalesIsPresent();
    if (systemState.scalesPresent) {
      if (currentState.tarePending) {
        scalesTare();
        weightMeasurements.clear();
        weightMeasurements.add(scalesGetWeight());
        currentState.tarePending = false;
      }
      else {
        weightMeasurements.add(scalesGetWeight());
      }
      currentState.weight = weightMeasurements.latest().value;

      if (brewActive) {
        currentState.shotWeight = currentState.tarePending ? 0.f : currentState.weight;
        currentState.weightFlow = fmax(0.f, weightMeasurements.measurementChange().changeSpeed());
        currentState.smoothedWeightFlow = smoothScalesFlow.updateEstimate(currentState.weightFlow);
      }
    }
    scalesTimer = millis();
  }
}

static void sensorsReadPressure(void) {
  uint32_t elapsedTime = millis() - pressureTimer;
  if (elapsedTime > GET_PRESSURE_READ_EVERY) {
    float elapsedTimeSec = elapsedTime / 1000.f;
    currentState.pressure = getPressure();
    previousSmoothedPressure = currentState.smoothedPressure;
    currentState.smoothedPressure = smoothPressure.updateEstimate(currentState.pressure);
    currentState.pressureChangeSpeed = (currentState.smoothedPressure - previousSmoothedPressure) / elapsedTimeSec;
    pressureTimer = millis();
  }
}

static long sensorsReadFlow(float elapsedTimeSec) {
  long pumpClicks = getAndResetClickCounter();
  currentState.pumpClicks = (float) pumpClicks / elapsedTimeSec;

  currentState.pumpFlow = getPumpFlow(currentState.pumpClicks, currentState.smoothedPressure);

  previousSmoothedPumpFlow = currentState.smoothedPumpFlow;
  // Some flow smoothing
  currentState.smoothedPumpFlow = smoothPumpFlow.updateEstimate(currentState.pumpFlow);
  currentState.pumpFlowChangeSpeed = (currentState.smoothedPumpFlow - previousSmoothedPumpFlow) / elapsedTimeSec;
  return pumpClicks;
}

static void calculateWeightAndFlow(void) {
  uint32_t elapsedTime = millis() - flowTimer;

  if (brewActive) {
    // Marking for tare in case smth has gone wrong and it has exited tare already.
    if (currentState.weight < -.3f) currentState.tarePending = true;

    if (elapsedTime > REFRESH_FLOW_EVERY) {
      flowTimer = millis();
      float elapsedTimeSec = elapsedTime / 1000.f;
      long pumpClicks = sensorsReadFlow(elapsedTimeSec);
      float consideredFlow = currentState.smoothedPumpFlow * elapsedTimeSec;
      // Update predictive class with our current phase
      const CurrentPhase& phase = phaseProfiler.getCurrentPhase();
      predictiveWeight.update(currentState, phase, runningCfg);

      // Start the predictive weight calculations when conditions are true
      if (predictiveWeight.isOutputFlow() || currentState.weight > 0.4f) {
        float flowPerClick = getPumpFlowPerClick(currentState.smoothedPressure);
        float actualFlow = (consideredFlow > pumpClicks * flowPerClick) ? consideredFlow : pumpClicks * flowPerClick;
        /* Probabilistically the flow is lower if the shot is just started winding up and we're flow profiling,
        once pressure stabilises around the setpoint the flow is either stable or puck restriction is high af. */
        // if ((ACTIVE_PROFILE(runningCfg).mfProfileState || ACTIVE_PROFILE(runningCfg).tpType) && currentState.pressureChangeSpeed > 0.15f) {
        //   if ((currentState.smoothedPressure < ACTIVE_PROFILE(runningCfg).mfProfileStart * 0.9f)
        //   || (currentState.smoothedPressure < ACTIVE_PROFILE(runningCfg).tfProfileStart * 0.9f)) {
        //     actualFlow *= 0.3f;
        //   }
        // }
        currentState.consideredFlow = smoothConsideredFlow.updateEstimate(actualFlow);
        if (!systemState.scalesPresent) {
          currentState.shotWeight = currentState.shotWeight + actualFlow;
        }
      }
      currentState.waterPumped += consideredFlow;
    }
  } else {
    currentState.consideredFlow = 0.f;
    currentState.pumpClicks = getAndResetClickCounter();
    flowTimer = millis();
  }
}

// return the reading in mm of the tank water level.
static void readTankWaterLevel(void) {
  if (!brewActive) {
    currentState.waterLvl = tof.readLvl();
  }
}

// Function to track time since system has started
static unsigned long getTimeSinceInit(void) {
  static unsigned long startTime = millis();
  return millis() - startTime;
}

static void updateStartupTimer() {
  systemState.timeAlive = getTimeSinceInit() / 1000;
}

//#############################################################################################
//############################____OPERATIONAL_MODE_CONTROL____#################################
//#############################################################################################
static void modeSelect(void) {
  if (!systemState.startupInitFinished) return;

  switch (systemState.operationMode) {
    //REPLACE ALL THE BELOW WITH OPMODE_auto_profiling
    case OperationMode::BREW_AUTO:
      if (currentState.hotWaterSwitchState) hotWaterMode(currentState);
      else if (currentState.steamSwitchState) steamCtrl(runningCfg, currentState, systemState);
      else {
        profiling();
        steamTime = millis();
      }
      break;
    case OperationMode::BREW_MANUAL:
      if (!currentState.steamSwitchState) steamTime = millis();
      profiling();
      break;
    case OperationMode::FLUSH:
      if (!currentState.steamSwitchState) steamTime = millis();
      backFlush(currentState);
      brewActive ? setBoilerOff() : justDoCoffee(runningCfg, currentState, activeProfile.waterTemperature, false);
      break;
    case OperationMode::STEAM:
      steamCtrl(runningCfg, currentState, systemState);

      if (!currentState.steamSwitchState) {
        brewActive ? flushActivated() : flushDeactivated();
        steamCtrl(runningCfg, currentState, systemState);
      }
      break;
    case OperationMode::DESCALE:
      if (!currentState.steamSwitchState) steamTime = millis();
      deScale(runningCfg, currentState, systemState);
      break;
    default:
      break;
  }
}

//#############################################################################################
//################################____EPS_COMMS_CONTROL___###################################
//#############################################################################################

static void espUpdateState(void) {
  if (millis() > pageRefreshTimer) {
    espCommsSendSystemState(systemState, 1000);
    espCommsSendSensorData(currentState, 500);
 
    if (brewActive) {
      espCommsSendShotData(buildShotSnapshot(millis() - brewingTimer, currentState, phaseProfiler), 100);
    }
    pageRefreshTimer = millis() + REFRESH_ESP_DATA_EVERY;
  }
}

void onProfileReceived(const Profile& newProfile) {
  activeProfile = newProfile;
}

void onGaggiaSettingsReceived(const GaggiaSettings& newSettings) {
  runningCfg = newSettings;
  if (systemState.startupInitFinished) {
    pumpInit(currentState.powerLineFrequency, runningCfg.system.pumpFlowAtZero);
  }
}

void onManualBrewPhaseReceived(const Phase& phase) {
  if (manualProfile.phaseCount() != 1) {
    manualProfile.phases.resize(1);
  }
  manualProfile.phases[0] = phase;
}

void onOperationModeReceived(const OperationMode operationMode) {
  systemState.operationMode = operationMode;
}

void onBoilerSettingsReceived(const BoilerSettings& boilerSettings) {
  runningCfg.boiler = boilerSettings;
}
void onLedSettingsReceived(const LedSettings& ledSettings) {
  runningCfg.led = ledSettings;
}

void onSystemSettingsReceived(const SystemSettings& systemSettings) {
  runningCfg.system = systemSettings;
}

void onBrewSettingsReceived(const BrewSettings& brewSettings) {
  runningCfg.brew = brewSettings;
}

//#############################################################################################
//###############################____PROFILING_CONTROL____#####################################
//#############################################################################################

static void profiling(void) {
  if (brewActive) { //runs this only when brew button activated and pressure profile selected
    uint32_t timeInShot = millis() - brewingTimer;
    phaseProfiler.setProfile(systemState.operationMode == OperationMode::BREW_AUTO ? activeProfile : manualProfile);
    phaseProfiler.updatePhase(timeInShot, currentState);
    const CurrentPhase& currentPhase = phaseProfiler.getCurrentPhase();

    if (phaseProfiler.isFinished()) {
      setPumpOff();
      closeValve();
      brewActive = false;
    } else if (currentPhase.getType() == PhaseType::PRESSURE) {
      float newBarValue = currentPhase.getTarget();
      float flowRestriction =  currentPhase.getRestriction();
      openValve();
      setPumpPressure(newBarValue, flowRestriction, currentState);
    } else {
      float newFlowValue = currentPhase.getTarget();
      float pressureRestriction =  currentPhase.getRestriction();
      openValve();
      setPumpFlow(newFlowValue, pressureRestriction, currentState);
    }
  } else {
    setPumpOff();
    closeValve();
  }
  // Keep that water at temp
  // TODO: If active phase overrides the water temperature, then send the active phase's temp
  justDoCoffee(runningCfg, currentState, activeProfile.waterTemperature, brewActive);
}

//#############################################################################################
//###################################____BREW DETECT____#######################################
//#############################################################################################

static void brewDetect(void) {
  // Do not allow brew detection while system reports not ready.
  if (!sysReadinessCheck()) {
    return;
  }

  static bool paramsReset = true;
  if (currentState.brewSwitchState) {
    if (!paramsReset) {
      brewParamsReset();
      paramsReset = true;
      brewActive = true;
    }
    // needs to be here as it creates a locking state soemtimes if not kept up to date during brew
    // mainly when shotWeight restriction kick in.
    systemHealthTimer = millis() + HEALTHCHECK_EVERY;
  } else {
    brewActive = false;
    currentState.pumpClicks = getAndResetClickCounter();
    if (paramsReset) {
      brewParamsReset();
      paramsReset = false;
    }
  }
}

static void brewParamsReset(void) {
  currentState.tarePending = true;
  currentState.shotWeight  = 0.f;
  currentState.pumpFlow    = 0.f;
  currentState.weight      = 0.f;
  currentState.waterPumped = 0.f;
  brewingTimer             = millis();
  flowTimer                = brewingTimer;
  systemHealthTimer        = brewingTimer + HEALTHCHECK_EVERY;

  weightMeasurements.clear();
  predictiveWeight.reset();
  phaseProfiler.reset();
}

static bool sysReadinessCheck(void) {
  // Startup procedures not finished
  if (!systemState.startupInitFinished) {
    return false;
  }
  // If there's not enough water in the tank
  if (!brewActive && currentState.waterLvl < MIN_WATER_LVL) {
    espCommsSendNotification(Notification::warn("Fill the water tank!"));
    return false;
  }

  return true;
}

static inline void sysHealthCheck(float pressureThreshold) {
  //Reloading the watchdog timer, if this function fails to run MCU is rebooted
  watchdogReload();

  /* This *while* is here to prevent situations where the system failed to get a temp reading and temp reads as 0 or -7(cause of the offset)
  If we would use a non blocking function then the system would keep the SSR in HIGH mode which would most definitely cause boiler overheating */
  while (currentState.temperature <= 0.0f || currentState.temperature == NAN || currentState.temperature >= 170.0f) {
    //Reloading the watchdog timer, if this function fails to run MCU is rebooted
    watchdogReload();
    /* In the event of the temp failing to read while the SSR is HIGH
    we force set it to LOW while trying to get a temp reading - IMPORTANT safety feature */
    setPumpOff();
    setBoilerOff();
    setSteamBoilerRelayOff();
    if (millis() > thermoTimer) {
      LOG_ERROR("Cannot read temp from thermocouple (last read: %.1lf)!", static_cast<double>(currentState.temperature));
      currentState.steamSwitchState ? espCommsSendNotification(Notification::warn("COOLDOWN!")) : espCommsSendNotification(Notification::warn("TEMP READ ERROR"));  // writing a LCD message
      currentState.temperature  = thermocoupleRead() - runningCfg.boiler.offsetTemp;  // Making sure we're getting a value
      thermoTimer = millis() + GET_KTYPE_READ_EVERY;
    }
  }

  /*Shut down heaters if steam has been ON and unused fpr more than 10 minutes.*/
  while (systemState.isSteamForgottenON) {
    //Reloading the watchdog timer, if this function fails to run MCU is rebooted
    watchdogReload();
    espCommsSendNotification(Notification::warn("TURN STEAM OFF NOW!"));
    setPumpOff();
    setBoilerOff();
    setSteamBoilerRelayOff();
    systemState.isSteamForgottenON = currentState.steamSwitchState;
  }

  // No point going through the whole thing if this first condition isn't met.
  if (currentState.brewSwitchState || currentState.steamSwitchState || currentState.hotWaterSwitchState) {
    systemHealthTimer = millis() + HEALTHCHECK_EVERY;
    return;
  }
  // Should enter the block every "systemHealthTimer" seconds
  // TODO Find a way to not run this when we're still showing the brewGraph
  if (millis() >= systemHealthTimer) {
    while (!brewActive && currentState.smoothedPressure >= pressureThreshold && currentState.temperature < 100.f)
    {
      sensorsRead();
      espCommsSendNotification(Notification::info("Releasing pressure!"));
      setPumpOff();
      setBoilerOff();
      setSteamValveRelayOff();
      setSteamBoilerRelayOff();
      openValve();
    }
    closeValve();
    systemHealthTimer = millis() + HEALTHCHECK_EVERY;
  }
  // Throwing a pressure release countodown.
  if (brewActive) return;

  if (currentState.smoothedPressure >= pressureThreshold && currentState.temperature < 100.f) {
    if (millis() >= systemHealthTimer - 3500ul && millis() <= systemHealthTimer - 500ul) {
      int countdown = (int)(systemHealthTimer-millis())/1000;
      espCommsSendNotification(Notification::info("Dropping beats in: " + std::to_string(countdown)));
    }
  }
}

static void fillBoiler(void) {
  if (systemState.startupInitFinished) {
    return;
  }

  if (currentState.temperature > BOILER_FILL_SKIP_TEMP) {
    systemState.startupInitFinished = true;
    return;
  }

  if (isBoilerFillPhase(getTimeSinceInit()) && !isSwitchOn()) {
    fillBoilerUntilThreshod(getTimeSinceInit());
  }
  else if (isSwitchOn()) {
    espCommsSendNotification(Notification::warn("Brew Switch ON!"));
  }
}

static bool isBoilerFillPhase(unsigned long elapsedTime) {
  return elapsedTime >= BOILER_FILL_START_TIME;
}

static bool isBoilerFull(unsigned long elapsedTime) {
  bool boilerFull = false;
  if (elapsedTime > BOILER_FILL_START_TIME + 1000UL) {
    boilerFull =  (previousSmoothedPressure - currentState.smoothedPressure > -0.02f)
                &&
                  (previousSmoothedPressure - currentState.smoothedPressure < 0.001f);
  }

  return elapsedTime >= BOILER_FILL_TIMEOUT || boilerFull;
}

// Checks if Brew switch is ON
static bool isSwitchOn(void) {
  return currentState.brewSwitchState;
}

static void fillBoilerUntilThreshod(unsigned long elapsedTime) {
  if (elapsedTime >= BOILER_FILL_TIMEOUT) {
    systemState.startupInitFinished = true;
    return;
  }

  if (isBoilerFull(elapsedTime)) {
    closeValve();
    setPumpOff();
    systemState.startupInitFinished = true;
    return;
  }

  espCommsSendNotification(Notification::info("Filling boiler!"));
  openValve();
  setPumpToRawValue(35);
}

// TODO: 
// static void updateStartupTimer(void) {
//   lcdSetUpTime(getTimeSinceInit() / 1000);
// }

static void cpsInit(GaggiaSettings &runningCfg) {
  int cps = getCPS();
  if (cps > 110) { // double 60 Hz
    currentState.powerLineFrequency = 60u;
  } else if (cps > 80) { // double 50 Hz
    currentState.powerLineFrequency = 50u;
  } else if (cps > 55) { // 60 Hz
    currentState.powerLineFrequency = 60u;
  } else if (cps > 0) { // 50 Hz
    currentState.powerLineFrequency = 50u;
  }
}

static void doLed(void) {
  if (runningCfg.led.disco && brewActive) {
    switch(systemState.operationMode) {
      case OperationMode::BREW_AUTO:
      case OperationMode::BREW_MANUAL:
        led.setDisco(led.CLASSIC);
        break;
      case OperationMode::FLUSH:
        led.setDisco(led.STROBE);
        break;
      case OperationMode::DESCALE:
        led.setDisco(led.DESCALE);
        break;
      default:
        led.setColor(0, 0, 0);
        break;
    }
  } else {
    led.setColor(runningCfg.led.color.R, runningCfg.led.color.G, runningCfg.led.color.B);
  }
}
