#include "just_do_coffee.h"

//delta stuff
inline static float TEMP_DELTA(float d) { return (d*DELTA_RANGE); }

void justDoCoffee(eepromValues_t &runningCfg, SensorState &currentState, bool brewActive, bool preinfusionFinished) {
  int HPWR_LOW = runningCfg.hpwr / runningCfg.mainDivider;
  static double heaterWave;
  static bool heaterState;
  float BREW_TEMP_DELTA;
  // Calculating the boiler heating power range based on the below input values
  int HPWR_OUT = mapRange(currentState.temperature, runningCfg.setpoint - 10, runningCfg.setpoint, runningCfg.hpwr, HPWR_LOW, 0);
  HPWR_OUT = constrain(HPWR_OUT, HPWR_LOW, runningCfg.hpwr);  // limits range of sensor values to HPWR_LOW and HPWR
  BREW_TEMP_DELTA = mapRange(currentState.temperature, runningCfg.setpoint, runningCfg.setpoint + TEMP_DELTA(runningCfg.setpoint), TEMP_DELTA(runningCfg.setpoint), 0, 0);
  BREW_TEMP_DELTA = constrain(BREW_TEMP_DELTA, 0, TEMP_DELTA(runningCfg.setpoint));

  if (brewActive) {
  // Applying the HPWR_OUT variable as part of the relay switching logic
    if (currentState.temperature > runningCfg.setpoint && currentState.temperature < runningCfg.setpoint + 0.25f && !preinfusionFinished ) {
      if (millis() - heaterWave > HPWR_OUT * runningCfg.brewDivider && !heaterState ) {
        setBoilerOff();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > HPWR_LOW * runningCfg.mainDivider && heaterState ) {
        setBoilerOn();
        heaterState=false;
        heaterWave=millis();
      }
    } else if (currentState.temperature > runningCfg.setpoint - 1.5f && currentState.temperature < runningCfg.setpoint + (runningCfg.brewDeltaState ? BREW_TEMP_DELTA : 0.f) && preinfusionFinished ) {
      if (millis() - heaterWave > runningCfg.hpwr * runningCfg.brewDivider && !heaterState ) {
        setBoilerOn();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > runningCfg.hpwr && heaterState ) {
        setBoilerOff();
        heaterState=false;
        heaterWave=millis();
      }
    } else if (runningCfg.brewDeltaState && currentState.temperature >= (runningCfg.setpoint + BREW_TEMP_DELTA) && currentState.temperature <= (runningCfg.setpoint + BREW_TEMP_DELTA + 2.5f)  && preinfusionFinished ) {
      if (millis() - heaterWave > runningCfg.hpwr * runningCfg.mainDivider && !heaterState ) {
        setBoilerOn();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > runningCfg.hpwr && heaterState ) {
        setBoilerOff();
        heaterState=false;
        heaterWave=millis();
      }
    } else if(currentState.temperature <= runningCfg.setpoint - 1.5f) {
      setBoilerOn();
    } else {
      setBoilerOff();
    }
  } else { //if brewState == 0
    if (currentState.temperature < ((float)runningCfg.setpoint - 10.00f)) {
      setBoilerOn();
    } else if (currentState.temperature >= ((float)runningCfg.setpoint - 10.f) && currentState.temperature < ((float)runningCfg.setpoint - 5.f)) {
      if (millis() - heaterWave > HPWR_OUT && !heaterState) {
        setBoilerOn();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > HPWR_OUT / runningCfg.brewDivider && heaterState ) {
        setBoilerOff();
        heaterState=false;
        heaterWave=millis();
      }
    } else if ((currentState.temperature >= ((float)runningCfg.setpoint - 5.f)) && (currentState.temperature <= ((float)runningCfg.setpoint - 0.25f))) {
      if (millis() - heaterWave > HPWR_OUT * runningCfg.brewDivider && !heaterState) {
        setBoilerOn();
        heaterState=!heaterState;
        heaterWave=millis();
      } else if (millis() - heaterWave > HPWR_OUT / runningCfg.brewDivider && heaterState ) {
        setBoilerOff();
        heaterState=!heaterState;
        heaterWave=millis();
      }
    } else {
      setBoilerOff();
    }
  }
}

//#############################################################################################
//################################____STEAM_POWER_CONTROL____##################################
//#############################################################################################

void steamCtrl(eepromValues_t &runningCfg, SensorState &currentState, bool brewActive) {
    // steam temp control, needs to be aggressive to keep steam pressure acceptable
  if ((currentState.temperature > runningCfg.setpoint - 10.f) && (currentState.temperature <= STEAM_WAND_HOT_WATER_TEMP)) {
    setBoilerOn();
    brewActive ? setPumpFullOn() : setPumpOff();
  }else if ((currentState.pressure <= 9.f) && (currentState.temperature > STEAM_WAND_HOT_WATER_TEMP) && (currentState.temperature <= STEAM_TEMPERATURE)) {
    setBoilerOn();
    if (currentState.pressure < 1.5f) {
      #ifndef SINGLE_BOARD
        openValve();
      #endif
      setPumpToRawValue(5);
    }
  } else {
    setBoilerOff();
    (currentState.pressure < 1.5f) ? setPumpToRawValue(5) : setPumpOff();
  }
}
