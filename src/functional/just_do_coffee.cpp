#include "just_do_coffee.h"
#include "../lcd/lcd.h"

//delta stuff
// inline static float TEMP_DELTA(float d) { return (d*DELTA_RANGE); }
inline static float TEMP_DELTA(float d, SensorState &currentState) {
  return (
    d * (currentState.pumpFlow < 1.f
      ? currentState.pumpFlow / 9.f
      : currentState.pumpFlow / 10.f
    )
  );
}

void justDoCoffee(eepromValues_t &runningCfg, SensorState &currentState, bool brewActive, bool preinfusionFinished) {
  int HPWR_LOW = runningCfg.hpwr / runningCfg.mainDivider;
  float BREW_TEMP_DELTA;
  float sensorTemperature = currentState.temperature + runningCfg.offsetTemp;
  // Calculating the boiler heating power range based on the below input values
  int HPWR_OUT = mapRange(sensorTemperature, runningCfg.setpoint - 10, runningCfg.setpoint, runningCfg.hpwr, HPWR_LOW, 0);
  HPWR_OUT = constrain(HPWR_OUT, HPWR_LOW, runningCfg.hpwr);  // limits range of sensor values to HPWR_LOW and HPWR
  BREW_TEMP_DELTA = mapRange(sensorTemperature, runningCfg.setpoint, runningCfg.setpoint + TEMP_DELTA(runningCfg.setpoint, currentState), TEMP_DELTA(runningCfg.setpoint, currentState), 0, 0);
  BREW_TEMP_DELTA = constrain(BREW_TEMP_DELTA, 0, TEMP_DELTA(runningCfg.setpoint, currentState));
  lcdTargetState(0); // setting the target mode to "brew temp"

  if (brewActive) { //if brewState == true
    if(sensorTemperature <= runningCfg.setpoint) {
      setBoilerOn();
    }
    else if (sensorTemperature <= runningCfg.setpoint + (runningCfg.brewDeltaState ? BREW_TEMP_DELTA : 0.f)) {
      pulseHeaters(runningCfg.hpwr, runningCfg.mainDivider, runningCfg.brewDivider, brewActive);
    }
    else {
      setBoilerOff();
    }
  } else { //if brewState == false
    if (sensorTemperature <= ((float)runningCfg.setpoint - 10.00f)) {
      setBoilerOn();
    }
    else if (sensorTemperature <= ((float)runningCfg.setpoint - 5.f)) {
      pulseHeaters(HPWR_OUT, 1, runningCfg.mainDivider, brewActive);
    }
    else if (sensorTemperature < ((float)runningCfg.setpoint)) {
      pulseHeaters(HPWR_OUT,  runningCfg.brewDivider, runningCfg.brewDivider, brewActive);
    }
    else {
      setBoilerOff();
    }
  }
}

void pulseHeaters(uint32_t pulseLength, int factor_1, int factor_2, bool brewActive) {
  static uint32_t heaterWave;
  static bool heaterState;
  if (millis() - heaterWave > pulseLength * factor_1 && !heaterState ) {
    brewActive ? setBoilerOff() : setBoilerOn();
    heaterState=!heaterState;
    heaterWave=millis();
  } else if (millis() - heaterWave > pulseLength / factor_2 && heaterState ) {
    brewActive ? setBoilerOn() : setBoilerOff();
    heaterState=!heaterState;
    heaterWave=millis();
  }
}

//#############################################################################################
//################################____STEAM_POWER_CONTROL____##################################
//#############################################################################################

void steamCtrl(eepromValues_t &runningCfg, SensorState &currentState, bool brewActive, unsigned long steamTime) {
  lcdTargetState(1); // setting the target mode to "steam temp"
  // steam temp control, needs to be aggressive to keep steam pressure acceptable
  float sensorTemperature = currentState.temperature + runningCfg.offsetTemp;
  if (steamState() && brewState()) {
    hotWaterMode();
  }
  else if ((currentState.smoothedPressure <= 9.f)
            && (sensorTemperature > runningCfg.setpoint - 10.f)
            && (sensorTemperature <= runningCfg.steamSetPoint)
          ) {
    setBoilerOn();
    if (currentState.smoothedPressure < 1.8f) {
      #if not defined (SINGLE_BOARD) // not ENABLED if using the PCB
        #if not defined (DREAM_STEAM_DISABLED) // disabled for bigger boilers which have no  need of adjusting the pressure
        openValve();
        #endif
      #endif
      #ifndef DREAM_STEAM_DISABLED //
        setPumpToRawValue(3);
      #endif
    } else setPumpOff();
  } else {
    setBoilerOff();
    #ifndef DREAM_STEAM_DISABLED
      (currentState.smoothedPressure < 1.8f) ? setPumpToRawValue(3) : setPumpOff();
    #endif
  }

  /*In case steam is forgotten ON for more than 15 min*/
  if (currentState.smoothedPressure > 3.f) {
    long steamTimeout = millis() - steamTime;
    (steamTimeout >= STEAM_TIMEOUT)
      ? currentState.isSteamForgottenON = true
      : currentState.isSteamForgottenON = false;
  } else steamTime = millis();
}
/*Water mode and all that*/
void hotWaterMode() {
  #ifndef SINGLE_BOARD
  openValve();
  #else
  closeValve();
  #endif
  setPumpToRawValue(80);
  setBoilerOn();
}

