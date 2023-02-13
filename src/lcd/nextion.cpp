#include "lcd.h"
#include "pindef.h"
#include "log.h"

EasyNex myNex(USART_LCD);
volatile int lcdCurrentPageId;
volatile int lcdLastCurrentPageId;

void lcdInit(void) {
  myNex.begin(115200);
  while (myNex.readNumber("safetyTempCheck") != 100 )
  {
    LOG_VERBOSE("Connecting to Nextion LCD");
    delay(100);
  }
  myNex.writeStr("splash.build_version.txt", AUTO_VERSION);
  lcdCurrentPageId = myNex.currentPageId;
  lcdLastCurrentPageId = myNex.currentPageId;
}

void lcdListen(void) {
  myNex.NextionListen();
  lcdCurrentPageId = myNex.currentPageId;
}

void lcdUploadCfg(eepromValues_t &eepromCurrentValues) {

  myNex.writeNum("setPoint", eepromCurrentValues.setpoint);
  myNex.writeNum("moreTemp.n1.val", eepromCurrentValues.setpoint-eepromCurrentValues.offsetTemp);

  myNex.writeNum("steamSetPoint", eepromCurrentValues.steamSetPoint);
  myNex.writeNum("moreTemp.n6.val", eepromCurrentValues.steamSetPoint-eepromCurrentValues.offsetTemp);

  myNex.writeNum("offSet", eepromCurrentValues.offsetTemp);
  myNex.writeNum("moreTemp.n2.val", eepromCurrentValues.offsetTemp);

  myNex.writeNum("hpwr", eepromCurrentValues.hpwr);
  myNex.writeNum("moreTemp.n3.val", eepromCurrentValues.hpwr);

  myNex.writeNum("mDiv", eepromCurrentValues.mainDivider);
  myNex.writeNum("moreTemp.n4.val", eepromCurrentValues.mainDivider);

  myNex.writeNum("bDiv", eepromCurrentValues.brewDivider);
  myNex.writeNum("moreTemp.n5.val", eepromCurrentValues.brewDivider);

  myNex.writeNum("ppStart", eepromCurrentValues.pressureProfilingStart * 10.f);
  myNex.writeNum("brewAuto.pStartBar.val", eepromCurrentValues.pressureProfilingStart * 10.f);

  myNex.writeNum("ppFlowStart", eepromCurrentValues.flowProfileStart * 10.f);
  myNex.writeNum("brewAuto.flowStartBox.val", eepromCurrentValues.flowProfileStart * 10.f);

  myNex.writeNum("ppFin", eepromCurrentValues.pressureProfilingFinish * 10.f);
  myNex.writeNum("brewAuto.pEndBar.val", eepromCurrentValues.pressureProfilingFinish * 10.f);

  myNex.writeNum("ppFlowFinish", eepromCurrentValues.flowProfileEnd * 10.f);
  myNex.writeNum("brewAuto.flowEndBox.val", eepromCurrentValues.flowProfileEnd * 10.f);

  myNex.writeNum("ppHold", eepromCurrentValues.pressureProfilingHold);
  myNex.writeNum("brewAuto.n5.val", eepromCurrentValues.pressureProfilingHold);

  myNex.writeNum("ppFlowPressure", eepromCurrentValues.flowProfilePressureTarget * 10.f);
  myNex.writeNum("brewAuto.flowBarBox.val", eepromCurrentValues.flowProfilePressureTarget * 10.f);

  myNex.writeNum("ppLength", eepromCurrentValues.pressureProfilingLength);
  myNex.writeNum("brewAuto.n6.val", eepromCurrentValues.pressureProfilingLength);

  myNex.writeNum("ppFlowCurveSpeed", eepromCurrentValues.flowProfileCurveSpeed);
  myNex.writeNum("brewAuto.flowRampBox.val", eepromCurrentValues.flowProfileCurveSpeed);

  myNex.writeNum("piState", eepromCurrentValues.preinfusionState);
  myNex.writeNum("brewAuto.bt0.val", eepromCurrentValues.preinfusionState);

  myNex.writeNum("ppState", eepromCurrentValues.pressureProfilingState);
  myNex.writeNum("brewAuto.bt1.val", eepromCurrentValues.pressureProfilingState);

  myNex.writeNum("ppFlowState", eepromCurrentValues.flowProfileState);
  myNex.writeNum("brewAuto.bt2.val", eepromCurrentValues.flowProfileState);

  myNex.writeNum("piFlowState", eepromCurrentValues.preinfusionFlowState);
  myNex.writeNum("brewAuto.bt3.val", eepromCurrentValues.preinfusionFlowState);


  myNex.writeNum("piSec", eepromCurrentValues.preinfusionSec);
  myNex.writeNum("brewAuto.n0.val", eepromCurrentValues.preinfusionSec);

  myNex.writeNum("piFlow", eepromCurrentValues.preinfusionFlowVol * 10.f);
  myNex.writeNum("brewAuto.flowPIbox.val", eepromCurrentValues.preinfusionFlowVol * 10.f);

  myNex.writeNum("piBar", eepromCurrentValues.preinfusionBar * 10.f);
  myNex.writeNum("brewAuto.piTargetBar.val", eepromCurrentValues.preinfusionBar * 10.f);

  myNex.writeNum("piFlowTime", eepromCurrentValues.preinfusionFlowTime);
  myNex.writeNum("brewAuto.flowPiSecBox.val", eepromCurrentValues.preinfusionFlowTime);

  myNex.writeNum("piSoak", eepromCurrentValues.preinfusionSoak);
  myNex.writeNum("brewAuto.n4.val", eepromCurrentValues.preinfusionSoak);

  myNex.writeNum("piFlowSoak", eepromCurrentValues.preinfusionFlowSoakTime);
  myNex.writeNum("brewAuto.flowPiSoakBox.val", eepromCurrentValues.preinfusionFlowSoakTime);

  myNex.writeNum("piFlowPressure", eepromCurrentValues.preinfusionFlowPressureTarget * 10.f);
  myNex.writeNum("brewAuto.flowPiBarBox.val", eepromCurrentValues.preinfusionFlowPressureTarget * 10.f);

  myNex.writeNum("piRamp", eepromCurrentValues.preinfusionRamp);
  myNex.writeNum("brewAuto.rampSpeed.val", eepromCurrentValues.preinfusionRamp);

  myNex.writeNum("regHz", eepromCurrentValues.powerLineFrequency);

  myNex.writeNum("systemSleepTime", eepromCurrentValues.lcdSleep*60);
  myNex.writeNum("morePower.n1.val", eepromCurrentValues.lcdSleep);

  myNex.writeNum("morePower.lc1.val", eepromCurrentValues.scalesF1);
  myNex.writeNum("morePower.lc2.val", eepromCurrentValues.scalesF2);
  myNex.writeNum("morePower.pump_zero.val", eepromCurrentValues.pumpFlowAtZero * 10000.f);

  myNex.writeNum("homeOnBrewFinish", eepromCurrentValues.homeOnShotFinish);
  myNex.writeNum("brewSettings.btGoHome.val", eepromCurrentValues.homeOnShotFinish);

  myNex.writeNum("basketPrefill", eepromCurrentValues.basketPrefill);
  myNex.writeNum("brewSettings.btPrefill.val", eepromCurrentValues.basketPrefill);

  myNex.writeNum("warmupState", eepromCurrentValues.warmupState);
  myNex.writeNum("brewSettings.btWarmup.val", eepromCurrentValues.warmupState);

  myNex.writeNum("deltaState", eepromCurrentValues.brewDeltaState);
  myNex.writeNum("brewSettings.btTempDelta.val", eepromCurrentValues.brewDeltaState);

  myNex.writeNum("switchPhaseOnThreshold", eepromCurrentValues.switchPhaseOnThreshold);
  myNex.writeNum("brewSettings.btPhaseSwitch.val", eepromCurrentValues.switchPhaseOnThreshold);

  myNex.writeNum("shotState", eepromCurrentValues.stopOnWeightState);
  myNex.writeNum("shotDose", eepromCurrentValues.shotDose * 10.f);
  myNex.writeNum("shotPreset", eepromCurrentValues.shotPreset);
  myNex.writeNum("shotCustomVal", eepromCurrentValues.shotStopOnCustomWeight * 10.f);
}

eepromValues_t lcdDownloadCfg(void) {
  eepromValues_t lcdCfg = {};

  lcdCfg.preinfusionState               = myNex.readNumber("piState");
  lcdCfg.preinfusionSec                 = myNex.readNumber("piSec");
  lcdCfg.preinfusionBar                 = myNex.readNumber("piBar") / 10.f;
  lcdCfg.preinfusionSoak                = myNex.readNumber("piSoak");
  lcdCfg.preinfusionRamp                = myNex.readNumber("piRamp");

  lcdCfg.pressureProfilingState         = myNex.readNumber("ppState");
  lcdCfg.pressureProfilingStart         = myNex.readNumber("ppStart") / 10.f;
  lcdCfg.pressureProfilingFinish        = myNex.readNumber("ppFin") / 10.f;
  lcdCfg.pressureProfilingHold          = myNex.readNumber("ppHold");
  lcdCfg.pressureProfilingLength        = myNex.readNumber("ppLength");

  lcdCfg.preinfusionFlowState           = myNex.readNumber("piFlowState");
  lcdCfg.preinfusionFlowVol             = myNex.readNumber("piFlow") / 10.f;
  lcdCfg.preinfusionFlowTime            = myNex.readNumber("piFlowTime" );
  lcdCfg.preinfusionFlowSoakTime        = myNex.readNumber("piFlowSoak");
  lcdCfg.preinfusionFlowPressureTarget  = myNex.readNumber("piFlowPressure") / 10.f;
  lcdCfg.flowProfileState               = myNex.readNumber("ppFlowState");
  lcdCfg.flowProfileStart               = myNex.readNumber("ppFlowStart") / 10.f;
  lcdCfg.flowProfileEnd                 = myNex.readNumber("ppFlowFinish") / 10.f;
  lcdCfg.flowProfilePressureTarget      = myNex.readNumber("ppFlowPressure") / 10.f;
  lcdCfg.flowProfileCurveSpeed          = myNex.readNumber("ppFlowCurveSpeed");

  lcdCfg.stopOnWeightState              = myNex.readNumber("shotState");
  lcdCfg.shotDose                       = myNex.readNumber("shotDose") / 10.f;
  lcdCfg.shotStopOnCustomWeight         = myNex.readNumber("shotCustomVal") / 10.f;
  lcdCfg.shotPreset                     = myNex.readNumber("shotPreset");

  lcdCfg.setpoint                       = myNex.readNumber("setPoint");
  lcdCfg.steamSetPoint                  = myNex.readNumber("steamSetPoint");
  lcdCfg.offsetTemp                     = myNex.readNumber("offSet");
  lcdCfg.hpwr                           = myNex.readNumber("hpwr");
  lcdCfg.mainDivider                    = myNex.readNumber("mDiv");
  lcdCfg.brewDivider                    = myNex.readNumber("bDiv");
  lcdCfg.powerLineFrequency             = myNex.readNumber("regHz");
  lcdCfg.warmupState                    = myNex.readNumber("warmupState");

  lcdCfg.homeOnShotFinish               = myNex.readNumber("homeOnBrewFinish");
  lcdCfg.basketPrefill                  = myNex.readNumber("basketPrefill");
  lcdCfg.lcdSleep                       = myNex.readNumber("systemSleepTime") / 60;
  lcdCfg.brewDeltaState                 = myNex.readNumber("deltaState");
  lcdCfg.switchPhaseOnThreshold         = myNex.readNumber("switchPhaseOnThreshold");

  lcdCfg.scalesF1                       = myNex.readNumber("morePower.lc1.val");
  lcdCfg.scalesF2                       = myNex.readNumber("morePower.lc2.val");
  lcdCfg.pumpFlowAtZero                 = myNex.readNumber("morePower.pump_zero.val") / 10000.f;

  return lcdCfg;
}

int lcdGetManualFlowVol(void) {
  return myNex.readNumber("h0.val");
}

int lcdGetHomeScreenScalesEnabled(void) {
  return myNex.readNumber("scalesEnabled");
}

int lcdGetSelectedOperationalMode(void) {
  return  myNex.readNumber("modeSelect");
}

int lcdGetDescaleCycle(void) {
  return myNex.readNumber("j0.val");
}

void lcdSetDescaleCycle(int cycle) {
  myNex.writeNum("j0.val", cycle);
}

void lcdSetPressure(float val) {
  myNex.writeNum("pressure.val", val);
}

void lcdSetUpTime(float val) {
  myNex.writeNum("systemUpTime", val);
}

void lcdSetTemperature(int val) {
  myNex.writeNum("currentTemp", val);
}

void lcdSetWeight(float val) {
  char tmp[5];
  int check = snprintf(tmp, sizeof(tmp), "%.1f", static_cast<double>(val));
  if (check > 0 && check <= sizeof(tmp))
    myNex.writeStr("weight.txt", tmp);
}

void lcdSetFlow(int val) {
  myNex.writeNum("flow.val", val);
}

void lcdShowDebug(int val1, int val2) {
  myNex.writeNum("debug1",val1);
  myNex.writeNum("debug2",val2);
}

void lcdShowPopup(const char *msg) {
  static unsigned int timer;
  if(millis() > timer + 1000) {
    myNex.writeStr("popupMSG.t0.txt", msg);
    myNex.writeStr("page popupMSG");
    timer = millis();
  }
}

void lcdTargetState(int val) {
  myNex.writeNum("targetState", val);
}

void lcdBrewTimerStart(void) {
  myNex.writeNum("timerState", 1);
}

void lcdBrewTimerStop(void) {
  myNex.writeNum("timerState", 0);
}

void lcdSetBrewTimer(int seconds) {
  myNex.writeNum("activeBrewTime", seconds);
}

void lcdWarmupStateStop(void) {
  myNex.writeNum("warmupState", 0);
}

void trigger1(void) { lcdTrigger1(); }
void trigger2(void) { lcdTrigger2(); }
void trigger3(void) { lcdTrigger3(); }
void trigger4(void) { lcdTrigger4(); }
