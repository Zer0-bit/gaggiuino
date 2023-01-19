#include "just_do_coffee.h"
#include "../lcd/lcd.h"

//delta stuff
inline static float TEMP_DELTA(float d) { return (d*DELTA_RANGE); }

void justDoCoffee(eepromValues_t &runningCfg, SensorState &currentState, bool brewActive, bool preinfusionFinished) {
  int HPWR_LOW = runningCfg.hpwr / runningCfg.mainDivider;
  static double heaterWave;
  static bool heaterState;
  float BREW_TEMP_DELTA;
  float sensorTemperature = currentState.temperature + runningCfg.offsetTemp;
  // Calculating the boiler heating power range based on the below input values
  int HPWR_OUT = mapRange(sensorTemperature, runningCfg.setpoint - 10, runningCfg.setpoint, runningCfg.hpwr, HPWR_LOW, 0);
  HPWR_OUT = constrain(HPWR_OUT, HPWR_LOW, runningCfg.hpwr);  // limits range of sensor values to HPWR_LOW and HPWR
  BREW_TEMP_DELTA = mapRange(sensorTemperature, runningCfg.setpoint, runningCfg.setpoint + TEMP_DELTA(runningCfg.setpoint), TEMP_DELTA(runningCfg.setpoint), 0, 0);
  BREW_TEMP_DELTA = constrain(BREW_TEMP_DELTA, 0, TEMP_DELTA(runningCfg.setpoint));
  lcdTargetState(0); // setting the target mode to "brew temp"

  if (brewActive) {
    // Applying the HPWR_OUT variable as part of the relay switching logic
    if (sensorTemperature > runningCfg.setpoint && sensorTemperature < runningCfg.setpoint + 0.25f && !preinfusionFinished ) {
      if (millis() - heaterWave > HPWR_OUT * runningCfg.brewDivider && !heaterState ) {
        setBoilerOff();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > HPWR_LOW * runningCfg.mainDivider && heaterState ) {
        setBoilerOn();
        heaterState=false;
        heaterWave=millis();
      }
    } else if (sensorTemperature > runningCfg.setpoint - 1.5f && sensorTemperature < runningCfg.setpoint + (runningCfg.brewDeltaState ? BREW_TEMP_DELTA : 0.f) && preinfusionFinished ) {
      if (millis() - heaterWave > runningCfg.hpwr * runningCfg.brewDivider && !heaterState ) {
        setBoilerOn();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > runningCfg.hpwr && heaterState ) {
        setBoilerOff();
        heaterState=false;
        heaterWave=millis();
      }
    } else if (runningCfg.brewDeltaState && sensorTemperature >= (runningCfg.setpoint + BREW_TEMP_DELTA) && sensorTemperature <= (runningCfg.setpoint + BREW_TEMP_DELTA + 2.5f)  && preinfusionFinished ) {
      if (millis() - heaterWave > runningCfg.hpwr * runningCfg.mainDivider && !heaterState ) {
        setBoilerOn();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > runningCfg.hpwr && heaterState ) {
        setBoilerOff();
        heaterState=false;
        heaterWave=millis();
      }
    } else if(sensorTemperature <= runningCfg.setpoint - 1.5f) {
      setBoilerOn();
    } else {
      setBoilerOff();
    }
  } else { //if brewState == 0
    if (sensorTemperature < ((float)runningCfg.setpoint - 10.00f)) {
      setBoilerOn();
    } else if (sensorTemperature >= ((float)runningCfg.setpoint - 10.f) && sensorTemperature < ((float)runningCfg.setpoint - 5.f)) {
      if (millis() - heaterWave > HPWR_OUT && !heaterState) {
        setBoilerOn();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > HPWR_OUT / runningCfg.brewDivider && heaterState ) {
        setBoilerOff();
        heaterState=false;
        heaterWave=millis();
      }
    } else if ((sensorTemperature >= ((float)runningCfg.setpoint - 5.f)) && (sensorTemperature <= ((float)runningCfg.setpoint - 0.25f))) {
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

void steamCtrl(eepromValues_t &runningCfg, SensorState &currentState, bool brewActive, unsigned long steamTime) {
  lcdTargetState(1); // setting the target mode to "steam temp"
  // steam temp control, needs to be aggressive to keep steam pressure acceptable
  float sensorTemperature = currentState.temperature + runningCfg.offsetTemp;
  if (steamState() && brewState()) {
    closeValve();
    setPumpToRawValue(80);
    setBoilerOn();
  } else if ((currentState.smoothedPressure <= 9.f)
  && (sensorTemperature > runningCfg.setpoint - 10.f)
  && (sensorTemperature <= runningCfg.steamSetPoint))
  {
    setBoilerOn();
    if (currentState.smoothedPressure < 1.8f) {
      #if not defined (SINGLE_BOARD) // not ENABLED if using the PCB
        #if not defined (DREAM_STEAM_DISABLED) // disabled for bigger boilers which have no  need of adjusting the pressure
        openValve();
        #endif
      #endif

      #ifndef DREAM_STEAM_DISABLED
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
