/* 09:32 15/03/2023 - change triggering comment */
#include "just_do_coffee.h"
#include "../lcd/lcd.h"

extern unsigned long steamTime;
// inline static float TEMP_DELTA(float d) { return (d*DELTA_RANGE); }
inline static float TEMP_DELTA(float d, const SensorState &currentState) {
  return (
    d * (currentState.pumpFlow < 1.f
      ? currentState.pumpFlow / 9.f
      : currentState.pumpFlow / 10.f
    )
  );
}

void justDoCoffee(const eepromValues_t &runningCfg, const SensorState &currentState, const bool brewActive, const bool preinfusionFinished) {
  lcdTargetState(0); // setting the target mode to "brew temp"
  float sensorTemperature = currentState.temperature + runningCfg.offsetTemp;

  if (brewActive) { //if brewState == true
    if(sensorTemperature <= runningCfg.setpoint - 5.f) {
      setBoilerOn();
    } else {
      float deltaOffset = 0.f;
      if (runningCfg.brewDeltaState) {
        float tempDelta = TEMP_DELTA(runningCfg.setpoint, currentState);
        float BREW_TEMP_DELTA = mapRange(sensorTemperature, runningCfg.setpoint, runningCfg.setpoint + tempDelta, tempDelta, 0, 0);
        deltaOffset = constrain(BREW_TEMP_DELTA, 0, tempDelta);
      }
      if (sensorTemperature <= runningCfg.setpoint + deltaOffset) {
        pulseHeaters(runningCfg.hpwr, runningCfg.mainDivider, runningCfg.brewDivider, brewActive);
      } else {
        setBoilerOff();
      }
    }
  } else { //if brewState == false
    if (sensorTemperature <= ((float)runningCfg.setpoint - 10.f)) {
      setBoilerOn();
    } else {
      int HPWR_LOW = runningCfg.hpwr / runningCfg.mainDivider;
      // Calculating the boiler heating power range based on the below input values
      int HPWR_OUT = mapRange(sensorTemperature, runningCfg.setpoint - 10, runningCfg.setpoint, runningCfg.hpwr, HPWR_LOW, 0);
      HPWR_OUT = constrain(HPWR_OUT, HPWR_LOW, runningCfg.hpwr);  // limits range of sensor values to HPWR_LOW and HPWR

      if (sensorTemperature <= ((float)runningCfg.setpoint - 5.f)) {
        pulseHeaters(HPWR_OUT, 1, runningCfg.mainDivider, brewActive);
      } else if (sensorTemperature < ((float)runningCfg.setpoint)) {
        pulseHeaters(HPWR_OUT,  runningCfg.brewDivider, runningCfg.brewDivider, brewActive);
      } else {
        setBoilerOff();
      }
    }
  }
  if (brewActive || !currentState.brewSwitchState) { // keep steam boiler supply valve open while steaming/descale only
    setSteamValveRelayOff();
  }
  setSteamBoilerRelayOff();
}

void pulseHeaters(const uint32_t pulseLength, const int factor_1, const int factor_2, const bool brewActive) {
  static uint32_t heaterWave;
  static bool heaterState;
  if (!heaterState && ((millis() - heaterWave) > (pulseLength * factor_1))) {
    brewActive ? setBoilerOff() : setBoilerOn();
    heaterState=!heaterState;
    heaterWave=millis();
  } else if (heaterState && ((millis() - heaterWave) > (pulseLength / factor_2))) {
    brewActive ? setBoilerOn() : setBoilerOff();
    heaterState=!heaterState;
    heaterWave=millis();
  }
}

//#############################################################################################
//################################____STEAM_POWER_CONTROL____##################################
//#############################################################################################

#if not defined SINGLE_BOARD && not defined INDEPENDENT_DIMMER
#define PUMP_NEEDS_OPEN_VALVE  // not ENABLED if using the PCB or the dimmer wired separate from relay
#endif

void steamCtrl(const eepromValues_t &runningCfg, SensorState &currentState) {
  lcdTargetState(1); // setting the target mode to "steam temp"
  // steam temp control, needs to be aggressive to keep steam pressure acceptable
  float sensorTemperature = currentState.temperature + runningCfg.offsetTemp;

  if (currentState.smoothedPressure > 9.f || sensorTemperature < runningCfg.setpoint - 10.f) {
    setBoilerOff();
    setSteamBoilerRelayOff();
    setSteamValveRelayOff();
    setPumpOff();
  } else {
    if (sensorTemperature < runningCfg.steamSetPoint) {
      setBoilerOn();
    } else {
      setBoilerOff();
    }
    setSteamValveRelayOn();
    setSteamBoilerRelayOn();
    #ifndef DREAM_STEAM_DISABLED // disabled for bigger boilers which have no  need of adjusting the pressure
      if (currentState.smoothedPressure < 1.8f) {
        #ifdef PUMP_NEEDS_OPEN_VALVE
          openValve();
        #endif
        setPumpToRawValue(3);
      } else {
        setPumpOff();
        #ifdef PUMP_NEEDS_OPEN_VALVE
          closeValve();
        #endif
      }
    #endif
  }

  /*In case steam is forgotten ON for more than 15 min*/
  if (currentState.smoothedPressure > 3.f) {
    currentState.isSteamForgottenON = millis() - steamTime >= STEAM_TIMEOUT;
  } else steamTime = millis();
}

/*Water mode and all that*/
void hotWaterMode(const SensorState &currentState) {
  #ifdef PUMP_NEEDS_OPEN_VALVE
  openValve();
  #else
  closeValve();
  #endif
  setPumpToRawValue(80);
  setBoilerOn();
  if (currentState.temperature < MAX_WATER_TEMP) setBoilerOn();
  else setBoilerOff();
}
