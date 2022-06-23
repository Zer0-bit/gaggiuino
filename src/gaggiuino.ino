#include "Adafruit_VL53L0X.h"
#include "config.h"
#include "utils.h"
#include <Adafruit_ADS1X15.h>
#include <Arduino.h>
#include <EasyNextionLibrary.h>
#include <HX711_2.h>
#include <PID_v2.h>
#include <SimpleKalmanFilter.h>
#include <Wire.h>
#include <digitalWriteFast.h>

#if WATCHDOG_ACTIVE
#if defined(ARDUINO_ARCH_AVR)
#include <Adafruit_Sleepydog.h>
#define Watchdog_feed() Watchdog.reset()
#elif defined(ARDUINO_ARCH_STM32)
#include <IWatchdog.h>
#define Watchdog_feed() IWatchdog.reload()
#endif
#endif

#if HEATER_CONTROL
#include <TimerOne.h>
#endif

#if BIG_BOILER
#define STEAM_PRESSURE 1.7
#define STEAM_TEMP 120
#define TYPICAL_PURGE_LOSS -0.05
#define TYPICAL_STEAM_RANGE 0.05
#else
#define STEAM_PRESSURE 2.1
#define STEAM_TEMP 160
#define TYPICAL_PURGE_LOSS -0.75
#define TYPICAL_STEAM_RANGE 0.2
#endif

//########################__GLOBAL_VARIABLES__DECLARATION__#####################################
// volatile vars
volatile float tempC; // temp val
volatile float livePressure;
volatile float liquidPumped;
volatile float outputEstimate;
volatile float flowEstVal;
volatile float flowVal;

volatile bool adcFailure;

unsigned long debugTime;
// scales vars
float currentWeight;
float previousWeight;
float finalWeight;
float pressureDelta;
float offset, offset2;
float absoluteWeight, absoluteWeight2;
float preinfuseFillFlow;
float ppLimit;
float fpStartFlow;
float fpFinishFlow;

float flowAtZero, flowAtFour, flowAtTen, flowAtFifteen;
bool cycle;

bool POWER_ON;
bool descaleCheckBox;
bool preinfusionState;
bool pressureProfileState;
bool flowProfileState;
bool warmupEnabled;
bool flushEnabled;
bool descaleEnabled;
bool preinfusionFinished;
bool adcPresent;
bool tofSensorPresent;
bool scalesPresent;
bool stopOnWeight;
bool stopBrew;
bool isTempRead;
bool isBrewing;
bool released;
bool cameFromPressure;
bool cameFromFlow;
bool deactivateBoiler;

float waterDistance;
uint16_t setPoint;
uint16_t offsetTemp;
float pBrew = 0;
float iBrew;
float dBrew;
uint8_t MainCycleDivider;
uint8_t BrewCycleDivider;
uint8_t preinfuseTime;
uint8_t preinfuseBar;
uint8_t preinfuseSoak;
uint8_t preinfuseSoakBar;
uint8_t ppStartBar;
uint8_t ppFinishBar;
uint8_t ppHold;
uint8_t ppLength;
uint8_t fpHold;
uint8_t fpLength;
uint8_t fpLimit;
uint8_t regionHz;
uint8_t stopWeight;
uint8_t inputWeight;
uint8_t tempOffset;
uint8_t brewDelta;
uint8_t localACFrequency;

unsigned int selectedOperationalMode;

volatile uint8_t timeStep;

volatile boolean zero_cross = false;
volatile int dimValue;

//########################__GLOBAL_VARIABLES__END__#####################################

//########################__NEXTION__#####################################
EasyNex display(USART_CH);

//#######################__Banoz_PSM__#################################### - for more cool shit visit https://github.com/banoz  and don't forget to star
PSM pump(zcPin, pumpPin, PUMP_RANGE);

//#######################__HX711_stuff__##################################
HX711_2 LoadCell;

//#######################__ADC_temp_press__###############################
Adafruit_ADS1115 ads;

//######################__Sensor_Filtering__##############################
SimpleKalmanFilter pressureEst(2, 2, 1.25);
SimpleKalmanFilter tempEst(2, 2, 10);
SimpleKalmanFilter weightEst(2, 2, 0.5);
SimpleKalmanFilter waterEst(2, 2, 0.01);

//#######################__PID_Boiler_Control__###########################
PID_v2 boilerControl(0, 0, 0, PID::Direct, PID::P_On::Measurement);

//######################__Water_Level_Sensing__###########################
Adafruit_VL53L0X waterLevel;

#if defined(ARDUINO_ARCH_STM32)
TwoWire Wire2(PB3, PB10);
#endif

//##############################################################################################################################
//############################################________________INIT______________################################################
//##############################################################################################################################
void setup() {
#if DEBUG_ENABLED
    Serial.begin(250000);
#endif

    USART_CH.begin(250000); // switching our board to the new serial speed
    // Will wait hereuntil full serial is established, this is done so the LCD fully initializes before passing the EEPROM values
    while (display.readNumber("safetyTempCheck") != 100) {
        delay(20);
        display.writeNum("currentTempHigh", 100);
    }

    sensorInit(); // Init adc and tof sensor

    pinInit(); // relay port init and set initial operating mode
    setBoiler(LOW);
    pump.set(0);
    POWER_ON = true;
    stopBrew = false;
    isTempRead = true;
    adcFailure = false;
    deactivateBoiler = false;

    scalesInit(); // Scales handling
    pageValuesRefresh();

    do {
        display.NextionListen();
        display.lastCurrentPageId = display.currentPageId;
    } while (display.currentPageId == splash);

    if (adcPresent) {
        ads.startADCReading(MUX_BY_CHANNEL[TEMP_ON_ADC], /*continuous=*/false);
        while (millis() < 1500) { // Initial Kalman-iterations need to be discarded
            sensorsRead();
        }
    } else {
        tempC = 0;
        livePressure = 0;
    }

    boilerControl.SetOutputLimits(0, PID_OUTPUT_MAX); // PID Init
    if (tempC < 60 && tempC > 0) {
        boilerControl.SetMode(PID::Automatic);
    } else {
        boilerControl.Start(tempC, 0, setPoint);
    }
    Serial.print(setPoint);
    Serial.print(" ");
    Serial.println(tempC);
#if WATCHDOG_ACTIVE
#if defined(ARDUINO_ARCH_AVR)
    Watchdog.enable(1000);
#elif defined(ARDUINO_ARCH_STM32)
    IWatchdog.begin(1000000);
#endif
#endif

#if HEATER_CONTROL
    Timer1.initialize();
    Timer1.attachInterrupt(dimCheck);
#endif

#if DEBUG_ENABLED
    Serial.println(ads.getDataRate());
    Serial.println(ads.getGain());
    Serial.println("Setup done!");
#endif
}
//##############################################################################################################################
//############################################________________MAIN______________################################################
//##############################################################################################################################

// Main loop where all the logic is continuously run

void loop() {
    pageValuesRefresh();
    display.NextionListen();
    sensorsRead();
    modeSelect();
    controlHeater();
    lcdRefresh();
    refreshWeight();

#if DEBUG_ENABLED
    debugOutput(2000);
#endif
#if WATCHDOG_ACTIVE
    Watchdog_feed();
#endif
}

#if DEBUG_ENABLED
void debugOutput(unsigned int interval) {

    static unsigned long timer = millis();
    if (millis() - timer > interval) { // debug output every 2 seconds
        Serial.print("Weight output: ");
        Serial.print(currentWeight);
        Serial.print(stopOnWeight ? "; Yes" : "; No");
        Serial.print("; Abs. Weight: ");
        Serial.print((absoluteWeight));
        Serial.print("; Offset: ");
        Serial.print(offset);
        Serial.print("; Abs. Weight 2: ");
        Serial.print((absoluteWeight2));
        Serial.print("; Offset 2: ");
        Serial.print(isBrewing);
        Serial.print("; dbgtime: ");
        Serial.print(display.currentPageId);

        Serial.print("; mode: ");
        Serial.print(selectedOperationalMode);
        Serial.print("; waterLevel: ");
        Serial.println(waterDistance);
        timer = millis();
    }
}
#endif

//##############################################################################################################################
//#############################################___________SENSORS_READ________##################################################
//##############################################################################################################################

void sensorsRead() {
    static unsigned long waterTimer = millis(), adcErrorTimer = millis();
    static float oldTemp = tempC;
    static float oldPres = livePressure;
    if (isTempRead && adcPresent) { // Get oscillating pressure / temp readings
        tempC = readTemp();
    } else if (adcPresent) {
        livePressure = getPressure();
    } else {
        tempC = 0;
        livePressure = 0;
    }

    if (oldTemp != tempC || oldPres != livePressure) { // Safety Fallback: If pressure / temp readings don't change for 2 seconds...
        adcErrorTimer = millis();
        oldTemp = tempC;
        oldPres = livePressure;
        adcFailure = false;
    } else if (millis() - adcErrorTimer > 2000 && adcPresent) {
        adcFailure = true; // ...something seems to be wrong with the adc
    }

    if (millis() - waterTimer > 25 && tofSensorPresent) { // reading the tof pin is slow, so only check in intervals of 25ms
        waterDistance = readWaterDistance();
        waterTimer = millis();
    } else if (!tofSensorPresent) {
        waterDistance = WATERDISTANCE_MID;
    }
}

float readTemp() {
    float tempResultEstimate;
    if (ads.conversionComplete()) {                       // If a temp read is available...
        int16_t results = ads.getLastConversionResults(); // ...get it..
        float volts = ads.computeVolts(results);
        tempResultEstimate = tempEst.updateEstimate(volts);          // ...and update Kalman Values
        ads.startADCReading(MUX_BY_CHANNEL[PRESSURE_ON_ADC], false); // then start a pressure read.
        isTempRead = false;
    } else {
        return tempC; // Otherwise, return old temp Value
    }

    // Do simplified Steinhart/Hart Math to get from NTC voltage to temp
    double widerstandNTC = SERIESRESISTOR * ((tempResultEstimate / 5) / (1 - (tempResultEstimate / 5)));
    double Tn = 273.15 + TEMPERATURENOMINAL;
    double TKelvin = 1 / ((1 / Tn) + ((double)1 / BCOEFFICIENT) * log((double)widerstandNTC / THERMISTORNOMINAL));

    float tempReading = TKelvin - 273.15; // convert absolute temp to C

    return tempReading - tempOffset;
}

float getPressure() {
    float pressureResultEstimate;
    if (ads.conversionComplete()) {
        int16_t results = ads.getLastConversionResults();
        float reading = ads.computeVolts(results);
        pressureResultEstimate = pressureEst.updateEstimate(reading);
        ads.startADCReading(MUX_BY_CHANNEL[TEMP_ON_ADC], false); // Start a temp read
        isTempRead = true;
        refreshPressureDelta();
    } else {
        return livePressure;
    }

    return mapFloat(pressureResultEstimate, TRANSDUCER_VOLTAGE_0_BAR, TRANSDUCER_VOLTAGE_12_BAR, 0.0, 12.0);
}

float readWaterDistance() {
    if (tofSensorPresent) {
        if (waterLevel.isRangeComplete()) {
            int result = waterLevel.readRange();
            float newDistance = waterEst.updateEstimate(result);

            if (newDistance < WATERDISTANCE_MIN)
                newDistance = WATERDISTANCE_MIN; // OOR readings get corrected, so the mapping to the screen works
            if (newDistance > WATERDISTANCE_MAX)
                newDistance = WATERDISTANCE_MAX;

            return newDistance;
        }
    }
    return waterDistance;
}

//##############################################################################################################################
//#################################################______BOILER_CONTROL_____####################################################
//##############################################################################################################################
void controlHeater() {
    if (adcPresent && !adcFailure && tempC >= 0.0 && tempC != NAN && !deactivateBoiler) { // Safety measures
        if (steamState() && boilerControl.GetMode()) {                                    // Manual control for steam mode
            boilerControl.SetMode(PID::Manual);
        } else if (tempC > 60 && !steamState() && selectedOperationalMode != steam && !boilerControl.GetMode()) { // Otherwise automatic control
            if (tempC < 70)                                                                                       // On startup
                boilerControl.Start(tempC, PID_OUTPUT_MAX, setPoint);
            else
                boilerControl.SetMode(PID::Automatic);
        }
        float output = boilerControl.Run(tempC);
        if (!boilerControl.GetMode()) {
            manualHeatControl();
        } else {
            automaticHeatControl(output);
        }
    } else {                              // If safety measure triggered
        digitalWriteFast(relayPin, LOW);  // turn the boiler off
        if (adcFailure && !brewState()) { // If the adc is not responding
            resetMCU();                   // reset the MCU while not in brew mode
        }
    }
}

void manualHeatControl() {
    if (tempC < 60 || (steamState() && (tempC < (STEAM_TEMP - 10) || livePressure < STEAM_PRESSURE) && tempC < 160)) { // Hard cutoff in steamMode at 160 degrees
        digitalWriteFast(relayPin, HIGH);
    } else {
        digitalWriteFast(relayPin, LOW);
    }
}

void automaticHeatControl(float output) {
    static unsigned long windowStartTime = millis();
    setPIDParameters(25000);
    if (millis() - windowStartTime > PID_OUTPUT_MAX) { // time to shift the Relay Window
        windowStartTime += PID_OUTPUT_MAX;
    }
    if (tempC < boilerControl.GetSetpoint() + 10) { // Hard stop at 10 degrees above setpoint. This will throw off PID, but only happen if parameters are set incorrectly
        if (output > millis() - windowStartTime) {
            digitalWriteFast(relayPin, HIGH);
        } else {
            digitalWriteFast(relayPin, LOW);
        }
    } else {
        digitalWriteFast(relayPin, LOW);
    }
}

void setPIDParameters(unsigned int afterBrewTime) {
    static unsigned long limitAfterBrew = millis() - afterBrewTime;
    static bool brewDetect = false;
    if (!brewState() && millis() - limitAfterBrew >= afterBrewTime && !brewDetect) { // If a brew starts...
        boilerControl.SetTunings(pBrew, iBrew, dBrew);
    } else if (brewState() || selectedOperationalMode == backflush) { // ...no more integral control + more reaction to RoC
        brewDetect = true;
        boilerControl.SetTunings(pBrew / 2, 0, dBrew + 15);
    } else if (brewDetect) { // Leave new parameters for specified amount of time (ms) to smooth the transition
        limitAfterBrew = millis();
        brewDetect = false;
    }
}

//##############################################################################################################################
//############################################______PAGE_CHANGE_VALUES_REFRESH_____#############################################
//##############################################################################################################################

void pageValuesRefresh() { // Refreshing our values on page changes (this is slow, so skipping it at the brew start. There might be some problems arising that I can't foresee, should be fine, though)

    if ((display.currentPageId != brewGraph && display.currentPageId != display.lastCurrentPageId) || (POWER_ON == true && display.currentPageId == home)) {

        if (POWER_ON) {
#if DEBUG_ENABLED
            display.writeNum("home.n7.aph", 127);  // Write debug Value
            display.writeNum("home.t26.aph", 127); // Write debug Value
#else
            display.writeNum("home.n7.aph", 0);  // Write debug Value
            display.writeNum("home.t26.aph", 0); // Write debug Value
#endif
        }
        unsigned long timera = millis(); // debug timers to check the blocking time

        preinfusionState = display.readNumber("bApre.bt0.val");         // reding the preinfusion state value which should be 0 or 1
        pressureProfileState = display.readNumber("bAprofile.bt1.val"); // reding the pressure profile state value which should be 0 or 1
        flowProfileState = display.readNumber("bAflow.bt1.val");
        preinfuseTime = display.readNumber("bApre.n0.val");
        preinfuseBar = display.readNumber("bApre.n1.val");
        preinfuseSoak = display.readNumber("bApre.n4.val"); // pre-infusion soak value
        preinfuseFillFlow = display.readNumber("bApre.n2.val") / 10.0;
        preinfuseSoakBar = display.readNumber("bApre.n3.val");

        unsigned long timerb = millis();
        ppStartBar = display.readNumber("bAprofile.n2.val");
        ppFinishBar = display.readNumber("bAprofile.n3.val");
        ppHold = display.readNumber("bAprofile.n5.val");   // pp start pressure hold
        ppLength = display.readNumber("bAprofile.n6.val"); // pp shot length
        ppLimit = display.readNumber("bAprofile.n0.val") / 10.0;

        fpStartFlow = display.readNumber("bAflow.x0.val") / 10.0;
        fpFinishFlow = display.readNumber("bAflow.x1.val") / 10.0;
        fpLength = display.readNumber("bAflow.n6.val"); // pp start pressure hold
        fpHold = display.readNumber("bAflow.n5.val");   // pp shot length
        fpLimit = display.readNumber("bAflow.n1.val");

        flushEnabled = display.readNumber("flushState");
        descaleEnabled = display.readNumber("descaleState");

        unsigned long timerc = millis();
        setPoint = display.readNumber("setPoint");             // reading the setPoint value from the lcd
        pBrew = display.readNumber("moreTemp.n2.val") / 100.0; // reading the proportional gain
        iBrew = display.readNumber("moreTemp.n3.val") / 100.0; // reading the integral gain
        dBrew = display.readNumber("moreTemp.n7.val") / 100.0; // reading the derivative gain
        tempOffset = display.readNumber("moreTemp.n0.val");
        brewDelta = display.readNumber("moreTemp.n4.val");
        localACFrequency = display.readNumber("moreTemp.n5.val");

        warmupEnabled = display.readNumber("warmupState");
        inputWeight = display.readNumber("bAmore.n7.val");
        stopWeight = display.readNumber("bAmore.n9.val");

        unsigned long timerd = millis();
        stopOnWeight = display.readNumber("bAmore.bt2.val");
        flowAtZero = display.readNumber("brewSettings.n10.val") / (60.0 * localACFrequency);
        flowAtFour = display.readNumber("brewSettings.n11.val") / (60.0 * localACFrequency);
        flowAtTen = display.readNumber("brewSettings.n12.val") / (60.0 * localACFrequency);
        flowAtFifteen = display.readNumber("brewSettings.n13.val") / (60.0 * localACFrequency);

        // MODE_SELECT should always be last
        uint32_t mode = display.readNumber("modeSelect");
        if (mode != 777777)
            selectedOperationalMode = mode;
        display.lastCurrentPageId = display.currentPageId;

        boilerControl.SetTunings(pBrew, iBrew, dBrew);
        POWER_ON = false;
        unsigned long timere = millis();

#if DEBUG_ENABLED
        int a = timerb - timera;
        int b = timerc - timerb;
        int c = timerd - timerc;
        int d = timere - timerb;
        if (a >= 25 || b >= 25 || c >= 25 || d >= 25) { // debug printout in case of high blocking time
            Serial.print(a);
            Serial.print("; ");
            Serial.print(b);
            Serial.print("; ");
            Serial.print(c);
            Serial.print("; ");
            Serial.print(d);
            Serial.println("; ");
        }

#endif
    } else if (display.currentPageId != display.lastCurrentPageId) {
        display.lastCurrentPageId = display.currentPageId;
    }
}

//#############################################################################################
//############################____OPERATIONAL_MODE_CONTROL____#################################
//#############################################################################################
void modeSelect() {
    switch (selectedOperationalMode) {
    case straight9Bar: // state = STRAIGHT 9 bar
        if (!steamState())
            justDoCoffee();
        else
            steamCtrl();
        break;
    case justPreinfusion: // state = PREINFUSION
        if (!steamState()) {
            preInfusion();
        } else
            steamCtrl();
        break;
    case justProfile: // state = PRESSURE PROFILING
        if (!steamState())
            automaticProfile(pressureProfileState);
        else
            steamCtrl();
        break;
    case manual: // state = MANUAL PP
        manualPressureProfile();
        break;
    case preinfProfile: // state = PP (with or without PI)
        if (!steamState()) {
            if (!preinfusionFinished) {
                preInfusion();
            } else
                automaticProfile(pressureProfileState);
        } else
            steamCtrl();
        break;
    case flush: // state = FLUSH
        if (!steamState())
            justDoCoffee();
        else
            steamCtrl();
        break;
    case descale: // state = DESCALE
        deScale(descaleCheckBox);
        break;
    case backflush: // state = BACKFLUSH
        automaticBackflush();
        break;
    case _empty: // state = N/A
        break;
    case steam: // state = STEAMING
        if (!steamState())
            justDoCoffee();
        else
            steamCtrl();
        break;
    default: // state = default state in case of logical omission
        if (!steamState())
            justDoCoffee();
        else
            steamCtrl();
        break;
    }
}

//#############################################################################################
//###############################____PUMP_OUTPUT_CONTROL____###################################
//#############################################################################################

void setPump(bool setPressure, float targetValue, float restriction) {
    updateOutputEstimates(getFlowAtPressure());

    if (restriction != 0) { // Stop pump if restriction is zero
        if (setPressure) {
            cameFromFlow = false;
            setPumpForPressure(targetValue, restriction);
        } else { // setFlow
            cameFromPressure = false;
            setPumpForFlow(targetValue, restriction);
        }
    } else {
        pump.set(0);
    }
}

void setPumpForPressure(float targetValue, float flowRestrict) {
    static bool restricted = false;
    if (((flowEstVal >= flowRestrict && flowVal > 0.05) || restricted)) {
        if (cameFromFlow) {
            pump.set(0);
        } else {
            restricted = true;
            cameFromPressure = true;
            setPumpForFlow(flowRestrict, targetValue);
        }
        if ((flowEstVal < flowRestrict && targetValue < livePressure + 1) || livePressure > targetValue) {
            restricted = false;
        }
    } else {
        int pumpValue;

        if (targetValue == 0 || livePressure > targetValue) {
            pumpValue = 0;
        } else {
            float diff = targetValue - livePressure;
            pumpValue = PUMP_RANGE / (1.f + exp(1.7f - diff / 0.9f));
        }
        pump.set(pumpValue);
    }
}

void setPumpForFlow(float targetValue, float pressureRestrict) {
    static bool restricted = false;
    if (livePressure >= pressureRestrict || restricted) {
        if (cameFromPressure) {
            pump.set(0);
        } else {
            restricted = true;
            cameFromFlow = true;
            setPumpForPressure(pressureRestrict, targetValue);
        }
        if (flowEstVal >= targetValue) {
            restricted = false;
        }
    } else {
        int pumpValue;

        float maxFlow = getFlowAtPressure() * localACFrequency;
        if (maxFlow < targetValue)
            targetValue = maxFlow;

        pumpValue = mapFloat(targetValue, 0, maxFlow, 0, PUMP_RANGE);

        pump.set(pumpValue);
    }
}

//#############################################################################################
//#########################____NO_OPTIONS_ENABLED_POWER_CONTROL____############################
//#############################################################################################
void justDoCoffee() {
    static bool coolingDone = true;
    if (scalesPresent) {
        refreshWeightStats();
    }
    if (brewState()) {
        if (warmupEnabled) {
            display.writeNum("warmupState", 0);
            warmupEnabled = false;
        }

        if (selectedOperationalMode != flush && selectedOperationalMode != steam) {
            boilerControl.Setpoint(setPoint + brewDelta);
            if (brewDelta > 0) {
                boilerControl.SetTunings(boilerControl.GetKp(), boilerControl.GetKi(), boilerControl.GetKd(), PID::P_On::Error);
            }
            if (stopOnWeight) {
                checkForStop();
            }
        }
        if (selectedOperationalMode == straight9Bar && coolingDone) {
            setPump(true, 9, 8);
            brewTimer(1);
            isBrewing = true;
        } else if (selectedOperationalMode == flush && coolingDone) {
            setPump(true, 9, 8);
        } else if (selectedOperationalMode == steam) { // If flushing to cool down, display message
            setPump(true, 9, 8);
            coolingDone = false;
            static unsigned long timer = millis() - 500;
            if (millis() - timer > 500) {
                display.writeNum("returnPage", 1);
                display.writeNum("currentTemp", (int)tempC);
                display.writeStr("popupMSG.t0.txt", String("Cooling. Current temp: ") + String(tempC) + String(" C"));
                display.writeStr("page popupMSG");
                timer = millis();
            }
        } else if (!coolingDone) {
            stopBrew = true;
        }
    }
#if defined(hotWaterPin)
    else if (hotWaterState()) {
        boilerControl.Setpoint(100);
        digitalWriteFast(solenoidPin, LOW);
        setPump(true, 9, 8);
    }
#endif
    else {
        coolingDone = true;
        if (deactivateBoiler) {
            boilerControl.Setpoint(0);
        } else {
            boilerControl.Setpoint(setPoint);

            if (brewDelta > 0) {
                boilerControl.SetTunings(boilerControl.GetKp(), boilerControl.GetKi(), boilerControl.GetKd(), PID::P_On::Measurement);
            }
        }
        brewTimer(0);
        isBrewing = false;
        setPump(true, 0, 0);
#if HAS_NO_OPV
        if (currentWeight < 20 && (livePressure > 13 || released)) { // Pressure-release through the group in case of high pressures, because I don't have an opv
            releasePressure();
        }
#endif
    }
}

//#############################################################################################
//################################____STEAM_POWER_CONTROL____##################################
//#############################################################################################

void steamCtrl() {
    if (!brewState()) {
        if (!deactivateBoiler)
            boilerControl.Setpoint(STEAM_TEMP);
        else
            boilerControl.Setpoint(0);

        static bool purged = false, startReady = false, started = false; // Detection of steam wand position to display steam graph
        if (pressureDelta < TYPICAL_PURGE_LOSS && !purged) {
            purged = true;
        } else if (pressureDelta > TYPICAL_STEAM_RANGE && purged && !startReady) {
            startReady = true;
        } else if (pressureDelta < TYPICAL_PURGE_LOSS && startReady) {
            started = true;
            brewTimer(1);
        } else if (pressureDelta > TYPICAL_STEAM_RANGE && started) {
            brewTimer(0);
            started = false;
            startReady = false;
            purged = false;
        }

    } else { // added to cater for hot water from steam wand functionality
        boilerControl.Setpoint(100);
        setPump(true, 9, 6);
    }
}

//#############################################################################################
//################################____LCD_REFRESH_CONTROL___###################################
//#############################################################################################

void lcdRefresh() {
    static unsigned long pageRefreshTimer = millis();
    unsigned long dbgtma = millis();
    if (millis() - pageRefreshTimer > REFRESH_SCREEN_EVERY) {
        display.writeNum("pressure.val", livePressure * 100);
        display.writeNum("currentTempHigh", int(tempC * 100));

        if (display.currentPageId == scales) { // On scales page
            (currentWeight < 0.2 && currentWeight > -0.2) ? display.writeStr("weight.txt", "0.00") : display.writeStr("weight.txt", String(currentWeight, 2));
        }

        if (display.currentPageId == brewingManual || display.currentPageId == brewGraph || isBrewing) { // If during brew
            if (flowEstVal >= 0 && flowEstVal < 20)
                display.writeNum("flowEst.val", int(flowEstVal * 10));
            else
                display.writeNum("flowEst.val", 0);

            if (liquidPumped != 0)
                display.writeStr("water.txt", String(liquidPumped, 1));
            if (scalesPresent) {
                if (flowVal >= 0 && flowVal < 20)
                    display.writeNum("flow.val", int(flowVal * 10));
                else
                    display.writeNum("flow.val", 0);
                if (currentWeight >= 0) {
                    if (!isBrewing) {
#if BIG_BOILER
                        display.writeStr("weight.txt", String(currentWeight, 1));
#else
                        if (finalWeight != 0)
                            display.writeStr("weight.txt", String(finalWeight, 1));
#endif
                    } else {
                        display.writeStr("weight.txt", String(currentWeight, 1));
                    }
                } else {
                    display.writeStr("weight.txt", "0.0");
                }
            }
        }

        if (display.currentPageId == home) { // If on home page
            display.writeNum("home.n5.val", boilerControl.GetSetpoint());
            (currentWeight < 0.2 && currentWeight > -0.2) ? display.writeStr("home.b2.txt", String("0.00") + String(" g")) : display.writeStr("home.b2.txt", String(currentWeight, 2) + String(" g"));

            if (tofSensorPresent) {
                display.writeNum("home.bar_water.val", map(waterDistance, WATERDISTANCE_MAX, WATERDISTANCE_MIN, 0, 100));
            }
            if ((boilerControl.GetSetpoint() == 160 && tempC > 145) || (boilerControl.GetSetpoint() == 120 && livePressure > 1.5)) {
                display.writeNum("steamReadyVal", 1);
            }
        }

        uint32_t mode = display.readNumber("modeSelect");
        if (mode != 777777)
            selectedOperationalMode = mode;
#if DEBUG_ENABLED
        display.writeNum("home.n7.val", pressureDelta * 100); // Write debug Value
#endif

        pageRefreshTimer = millis();
        unsigned long dbgtmb = millis();
    }
}

//#############################################################################################
//################################____EEPROM_HANDLING____######################################
//#############################################################################################

// intelligent Nextion has onboard EEPROM

//#############################################################################################
//##############################____DEACTIVATE_BOILER____######################################
//#############################################################################################

void trigger1() {
    deactivateBoiler = !deactivateBoiler;
    Serial.println("trigger");
}

//#############################################################################################
//###################################_____SCALES_TARE____######################################
//#############################################################################################

void trigger2() {
    tare();
}

void tare() {
    offset = absoluteWeight;
    offset2 = absoluteWeight2;
}

//#############################################################################################
//###############################_____HELPER_FUCTIONS____######################################
//#############################################################################################
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void refreshWeight() {
    if (scalesPresent) {
        if (LoadCell.is_ready()) { // Non-blocking weight read
            static bool firstTime = true;
            float values[2];
            LoadCell.get_units(values);
            absoluteWeight = values[0];
            absoluteWeight2 = values[1];
            float currentWeightUnfiltered = absoluteWeight - offset + absoluteWeight2 - offset2;

            currentWeight = weightEst.updateEstimate(currentWeightUnfiltered);
            if (firstTime) {
                tare();
                firstTime = false;
            }
        }
    } else {
        currentWeight = 0;
    }
}

void refreshWeightStats() {
    static unsigned long scalesRefreshTimer = millis(), flowRefreshTimer = millis();
    static bool tareDone = false;
    static float fWghtEntryVal, flowEstEntryVal;

    if (brewState()) {
        if (!tareDone) { // Tare at the start of brew
            tare();
            calcFlow(4096, 0); // Reset Flow and estimate values
            calcFlowEst(4096, 0);
            tareDone = true;
            flowRefreshTimer = millis();
        }

        if (millis() - scalesRefreshTimer > 100) {
            if (currentWeight <= -1.5 || (currentWeight - previousWeight) >= 30.0) { // If there are mayor weight fluctuations or weight is negative...
                tareDone = false;                                                    // ...tare again
            }
            previousWeight = currentWeight;
            scalesRefreshTimer = millis();
        }

        // FLow calc
        unsigned int timeVal = millis() - flowRefreshTimer;
        if (timeVal >= 250) {
            flowVal = calcFlow(fWghtEntryVal, timeVal);
            fWghtEntryVal = currentWeight;
            flowEstVal = calcFlowEst(flowEstEntryVal, timeVal);
            flowEstEntryVal = liquidPumped;
            flowRefreshTimer = millis();
        }
    } else if (display.currentPageId == scales) { // scales screen updating
        if (millis() - scalesRefreshTimer > 200) {
            if (!tareDone) {
                tare();
                tareDone = true;
            }
            scalesRefreshTimer = millis();
        }
    } else {
        tareDone = false;

        previousWeight = 0;
        fWghtEntryVal = 0;
        flowEstEntryVal = 0;
    }
}

void refreshPressureDelta() { // Pressure delta for steam detection
    static unsigned long now = millis();
    static float oldPressure = livePressure;
    if (millis() - now > 500) {
        pressureDelta = livePressure - oldPressure;
        oldPressure = livePressure;
        if (steamState()) {
            Serial.println(pressureDelta);
        }
        now = millis();
    }
}

#if defined(hotWaterPin)
bool hotWaterState() {
    return !(digitalReadFast(hotWaterPin));
}
#endif

bool steamState() {
    return !(digitalReadFast(steamPin));
}
// Function to get the state of the brew switch button

bool brewState() {
    static bool prevState = false;
    bool state = digitalReadFast(brewPin);
    if (prevState != state) {
        display.writeNum("brewButtonState", (int)!state);
        prevState = state;
    }
    bool retVal = !(state || (stopOnWeight && stopBrew));
    if (!steamState()) {
        if (retVal == 1) {
            digitalWriteFast(solenoidPin, HIGH);
        } else if (selectedOperationalMode != backflush && !released) {
            digitalWriteFast(solenoidPin, LOW);

#if DEBUG_ENABLED
            if (liquidPumped != 0) {
                Serial.print(outputEstimate);
                Serial.print("; ");
                Serial.println(liquidPumped);
            }
#endif

            if (state && stopBrew) {
                stopBrew = false;
                display.writeStr("page home");
                finalWeight = 0;
#if defined(lampPin)
                digitalWriteFast(lampPin, LOW);
            } else if (!state) {
                blinkLED(500);
            }
#else
            }
#endif
            liquidPumped = 0;
            outputEstimate = 0;
        }
    }
    return retVal;
}

#if defined(lampPin)
void blinkLED(unsigned int interval) {
    static unsigned long blinkTimer = millis();
    if (millis() - blinkTimer >= interval) {
        if (digitalReadFast(lampPin)) {
            digitalWriteFast(lampPin, LOW);
        } else {
            digitalWriteFast(lampPin, HIGH);
        }
        blinkTimer = millis();
    }
}
#endif

// Returns HIGH when switch is OFF and LOW when ON

void brewTimer(bool on) { // small function for easier timer start/stop
    if (on)
        display.writeNum("timerState", 1);
    else
        display.writeNum("timerState", 0);
}

// Actuating the heater element
void setBoiler(uint8_t val) {
    if (val == HIGH) {
        digitalWriteFast(relayPin, HIGH); // boilerPin -> HIGH
    } else {
        digitalWriteFast(relayPin, LOW); // boilerPin -> LOW
    }
}

float mapRange(float sourceNumber, float fromA, float fromB, float toA, float toB, int decimalPrecision) {
    float deltaA = fromB - fromA;
    float deltaB = toB - toA;
    float scale = deltaB / deltaA;
    float negA = -1 * fromA;
    float offset = (negA * scale) + toA;
    float finalNumber = (sourceNumber * scale) + offset;
    int calcScale = (int)pow(10, decimalPrecision);
    return (float)round(finalNumber * calcScale) / calcScale;
}

void checkForStop() {
    if (currentWeight + flowVal > stopWeight && outputEstimate > stopWeight - 10) {
        setPump(true, 0, 0);
        stopBrew = true;
        finalWeight = currentWeight + flowVal;
    }
}

void releasePressure() {
    static unsigned long releaseTime = millis();
    if (!released && millis() - releaseTime > 750) {
        digitalWriteFast(solenoidPin, HIGH);
        releaseTime = millis();
        released = true;
    }
    if (released && millis() - releaseTime > 750) {
        digitalWriteFast(solenoidPin, LOW);
        releaseTime = millis();
        released = false;
#if DEBUG_ENABLED
        Serial.println("release");
#endif
    }
}

float calcFlow(float lastWeight, unsigned int timeDelay) {
    static float vals[4] = {0};
    static int i = 0;
    if (lastWeight == 4096) { // reset
        for (uint8_t i = 0; i < 4; i++) {
            vals[i] = 0;
        }
    } else {
        float currentValue = (currentWeight - lastWeight);
        if (timeDelay > 250) {
            currentValue *= (250.0 / timeDelay); // corrective factor in case of time delays
        }
        vals[i++] = currentValue;
    }

    if (i == 4)
        i = 0;

    return (vals[0] + vals[1] + vals[2] + vals[3]);
}

float calcFlowEst(float lastEstimate, unsigned int timeDelay) {
    static float vals[4] = {0};
    static int i = 0;
    if (lastEstimate == 4096) { // reset
        for (uint8_t i = 0; i < 4; i++) {
            vals[i] = 0;
        }
    } else {
        float currentValue = (liquidPumped - lastEstimate);
        if (timeDelay > 250) {
            currentValue *= (250.0 / timeDelay); // corrective factor in case of time delays
        }
        vals[i++] = currentValue;
    }
    if (i == 4)
        i = 0;

    return (vals[0] + vals[1] + vals[2] + vals[3]);
}

float getFlowAtPressure() {
    if (livePressure < 4) {
        return mapFloat(livePressure, 0, 4, flowAtZero, flowAtFour);
    } else if (livePressure < 10) {
        return mapFloat(livePressure, 4, 10, flowAtFour, flowAtTen);
    } else {
        return mapFloat(livePressure, 10, 15, flowAtTen, flowAtFifteen);
    }
}

void updateOutputEstimates(float flowAtPressure) {
    static long oldPumpCounts = 0;
    long newPumpCounts = pump.getCounter();
    float addLiquid = ((newPumpCounts - oldPumpCounts) * flowAtPressure);
    liquidPumped += addLiquid;
    if (flowVal > 0 && (preinfusionFinished || selectedOperationalMode != preinfProfile))
        outputEstimate += addLiquid;

    oldPumpCounts = newPumpCounts;
}

void resetMCU() {
    static unsigned long now = millis();
    uint8_t seconds = 5 - (millis() - now) / 1000;
    display.writeStr("popupMSG.t0.txt", String("ADC does not respond. MCU reset in ") + String(seconds));
    display.writeStr("page popupMSG");
    if (seconds == 1) {
#if defined(ARDUINO_ARCH_AVR)
        Watchdog.enable(100);
        delay(200);
#elif defined(ARDUINO_ARCH_STM32)
        NVIC_SystemReset();
#endif
    }
}

//#############################################################################################
//###############################____DESCALE__CONTROL____######################################
//#############################################################################################

void deScale(bool c) {
    if (display.currentPageId == cleanDescale) {
        static bool blink = true;
        static unsigned long timer = millis();
        static uint8_t currentCycleRead = display.readNumber("j0.val");
        static uint8_t lastCycleRead = 10;
        static bool descaleFinished = false;
        if (brewState() == 1 && descaleFinished == false) {
            brewTimer(1);
            if (currentCycleRead < lastCycleRead) { // descale in cycles for 5 times then wait according to the below condition
                if (blink == true) {                // Logic that switches between modes depending on the $blink value
                    setPump(true, 2, 5);
                    if (millis() - timer > DESCALE_PHASE1_EVERY) { // set dimmer power to max descale value for 10 sec
                        if (currentCycleRead >= 100)
                            descaleFinished = true;
                        blink = false;
                        currentCycleRead = display.readNumber("j0.val");
                        timer = millis();
                    }
                } else {
                    setPump(true, 1, 5);
                    if (millis() - timer > DESCALE_PHASE2_EVERY) { // set dimmer power to min descale value for 20 sec
                        blink = true;
                        currentCycleRead++;
                        if (currentCycleRead < 100)
                            display.writeNum("j0.val", currentCycleRead);
                        timer = millis();
                    }
                }
            } else {
                setPump(true, 0, 0);
                if ((millis() - timer) > DESCALE_PHASE3_EVERY) { // nothing for 5 minutes
                    if (currentCycleRead * 3 < 100)
                        display.writeNum("j0.val", currentCycleRead * 3);
                    else {
                        display.writeNum("j0.val", 100);
                        descaleFinished = true;
                    }
                    lastCycleRead = currentCycleRead * 3;
                    timer = millis();
                }
            }
        } else if (brewState() == 1 && descaleFinished == true) {
            setPump(true, 0, 0);
            if ((millis() - timer) > 1000) {
                brewTimer(0);
                display.writeStr("t14.txt", "FINISHED!");
                timer = millis();
            }
        } else if (brewState() == 0) {
            currentCycleRead = 0;
            lastCycleRead = 10;
            descaleFinished = false;
            timer = millis();
        }
        // keeping it at temp
        justDoCoffee();
    }
}

void automaticBackflush() {
    static unsigned long backflushCount, backflushPressure;
    static unsigned int progressStep, progress;
    static unsigned int totalBackflushes;
    static bool backflushStart = false;
    static unsigned long releaseTimer = millis(), waitTimer = millis();
    unsigned int request = display.readNumber("cleanBackflush.bt0.val");
    bool startRequest;
    if (request == 777777 || request == 0) {
        startRequest = false;
    } else if (request == 1) {
        startRequest = true;
    }
    if (startRequest) {
        if (!backflushStart) {
            unsigned long now = millis();
            do {
                backflushCount = display.readNumber("cleanBackflush.n2.val");
                backflushPressure = display.readNumber("cleanBackflush.n3.val");
            } while ((backflushCount == 777777 || backflushPressure == 777777) && millis() - now < 500);
            if (backflushCount != 777777 && backflushPressure != 777777) {
                if (backflushCount > 20) {
                    backflushCount = 20;
                }
                if (backflushPressure > 12) {
                    backflushPressure = 10;
                }
                totalBackflushes = backflushCount;
                progressStep = 100 / backflushCount;
                progress = 0;
                backflushStart = true;
            } else {
                display.writeNum("returnPage", 16);
                display.writeStr("popupMSG.t0.txt", "Failed to read Backflush parameters");
                display.writeStr("page popupMSG");
                backflushCount = 0;
            }
        }
        if (backflushCount) {
            static bool decreasedCount = true;
            if (livePressure < backflushPressure && millis() - releaseTimer > 1000) {
                if (decreasedCount) {
                    display.writeStr("cleanBackflush.t3.txt", String((totalBackflushes - backflushCount) + 1) + String("/") + String(totalBackflushes));
                }
                digitalWriteFast(solenoidPin, HIGH);
                setPump(true, 9, 8);
                decreasedCount = false;
                waitTimer = millis();
            } else {
                setPump(true, 0, 0);
                if (millis() - waitTimer > 500) {
                    if (!decreasedCount) {
                        backflushCount--;
                        progress += progressStep;
                        display.writeNum("j0.val", progress);
                        decreasedCount = true;
                        released = true;
                        releaseTimer = millis();
                    }
                    releasePressure();
                }
            }
        } else {
            display.writeNum("bt0.val", 0);
            display.writeStr("bt0.txt", "Start");
        }

    } else if (backflushStart) {
        digitalWriteFast(solenoidPin, LOW);
        setPump(true, 0, 0);
        display.writeNum("n1.val", 0);
        display.writeNum("n0.val", 0);
        display.writeNum("j0.val", 0);
        display.writeStr("cleanBackflush.t3.txt", "Progress");
        backflushStart = false;
    }
}
//#############################################################################################
//###############################____PRESSURE_CONTROL____######################################
//#############################################################################################

// Pressure profiling function, uses dimmer to dim the pump
// Linear dimming as time passes, goes from pressure start to end incrementally or decrementally
void automaticProfile(bool pressure) {
    static bool phase_1 = true, phase_2 = false, updateTimer = true;
    static unsigned long timer;
    static float newSetValue;
    if (brewState()) { // runs this only when brew button activated and pressure profile selected

        if (updateTimer) {
            timer = millis();
            updateTimer = false;
        }
        if (phase_1) {                                                        // enters phase 1
            if ((millis() - timer) > ((pressure ? ppHold : fpHold) * 1000)) { // the actions of this if block are run after 4 seconds have passed since starting brewing
                phase_1 = false;
                phase_2 = true;
                timer = millis();
            }
            brewTimer(1);
            isBrewing = true;
            newSetValue = pressure ? ppStartBar : fpStartFlow;
            setPump(pressure, newSetValue, pressure ? ppLimit : fpLimit);
        } else if (phase_2) {
            float startValue = pressure ? ppStartBar : fpStartFlow;
            float endValue = pressure ? ppFinishBar : fpFinishFlow;
            unsigned long timera = micros();
            newSetValue = mapRange(millis(), timer, timer + ((pressure ? ppLength : fpLength) * 1000), startValue, endValue, 1);
            unsigned long timerb = micros();
            // debugTime = timerb - timera;
            if (startValue <= endValue) {
                if (newSetValue < startValue)
                    newSetValue = startValue;
                else if (newSetValue > endValue)
                    newSetValue = endValue;
            } else if (startValue > endValue) {
                if (newSetValue > startValue)
                    newSetValue = startValue;
                else if (newSetValue < endValue)
                    newSetValue = endValue;
            }
            setPump(pressure, newSetValue, pressure ? ppLimit : fpLimit);
        }
    } else {
        brewTimer(0);

        isBrewing = false;
        if (selectedOperationalMode == justPreinfusion) {
            setPump(true, ppStartBar, ppLimit);
        }
        if (preinfusionFinished)
            preinfusionFinished = false;
        timer = millis();
        phase_2 = false;
        phase_1 = true;
        updateTimer = true;
        newSetValue = 0.0;
    }

    // Keep that water at temp
    justDoCoffee();
}

void manualPressureProfile() {

    if (display.currentPageId == brewingManual) {
        float power_reading = display.readNumber("h0.val") / 10.0;
        if (brewState()) {
            brewTimer(1);
            isBrewing = true;
            setPump(true, power_reading, 8);
        } else {
            brewTimer(0);
            isBrewing = false;
            setPump(true, 0, 0);
        }
    }

    justDoCoffee();
}

//#############################################################################################
//###############################____PREINFUSION_CONTROL____###################################
//#############################################################################################

// Pump dimming during brew for preinfusion
void preInfusion() {
    static bool blink = true;
    static bool exitPreinfusion;
    static unsigned long timer = millis();

    if (brewState()) {
        if (exitPreinfusion == false) { // main preinfusion body
            if (blink == true) {        // Logic that switches between modes depending on the $blink value
                brewTimer(1);
                isBrewing = true;
                setPump(false, preinfuseFillFlow, preinfuseBar);

                if ((millis() - timer) >= (preinfuseTime * 1000) || (livePressure >= preinfuseBar && liquidPumped > 10)) {

                    blink = false;
                    timer = millis();
                }
            } else {
                setPump(true, preinfuseSoakBar, 1);

                if ((millis() - timer) >= (preinfuseSoak * 1000)) {
                    exitPreinfusion = true;
                    blink = true;
                    timer = millis();
                }
            }

        } else if (exitPreinfusion == true && selectedOperationalMode == justPreinfusion) {
            setPump(true, 9, 9);

        } else if (exitPreinfusion == true && selectedOperationalMode == preinfProfile) {
            preinfusionFinished = true;
            setPump(true, ppStartBar, ppLimit);
        }
    } else if (!brewState()) { // resetting all the values
        brewTimer(0);
        isBrewing = false;
        setPump(true, 0, 0);

        exitPreinfusion = false;
        timer = millis();
    }
    // keeping it at temp
    justDoCoffee();
}

//#############################################################################################
//###############################____INIT_AND_ADMIN_CTRL____###################################
//#############################################################################################
void pinInit() {
    pinMode(relayPin, OUTPUT);
    pinMode(solenoidPin, OUTPUT);
    pinMode(brewPin, INPUT_PULLUP);
    pinMode(steamPin, INPUT_PULLUP);
    pinMode(HX711_dout_1, INPUT_PULLUP);
    pinMode(HX711_dout_2, INPUT_PULLUP);
#if defined(lampPin)
    pinMode(lampPin, OUTPUT);
#endif
#if defined(hotWaterPin)
    pinMode(hotWaterPin, INPUT_PULLUP);
#endif
}

void sensorInit() {
#if USE_SECOND_I2C
    tofSensorPresent = waterLevel.begin(0x29, &Wire2);
#else
    tofSensorPresent = waterLevel.begin();
#endif
    if (tofSensorPresent) {
        waterLevel.startRangeContinuous();

        display.writeNum("home.bar_water.aph", 127);
    } else {
        display.writeNum("home.bar_water.val", 50);
        display.writeNum("home.bar_water.aph", 0);
    }
    waterDistance = WATERDISTANCE_MID;

#if USE_SECOND_I2C
    adcPresent = ads.begin(0x48, &Wire2);
#else
    adcPresent = ads.begin();
#endif
    if (adcPresent) {
        ads.setGain(GAIN_TWOTHIRDS);
        ads.setDataRate(475);
    }
}

void scalesInit() {
    LoadCell.begin(HX711_dout_1, HX711_dout_2, HX711_sck_1);
    LoadCell.power_up();
    if (LoadCell.wait_ready_timeout(1000)) {
        LoadCell.set_scale(SCALES_F1, SCALES_F2); // calibrated val1
        scalesPresent = true;
        cycle = false;
    } else {
        scalesPresent = false;
    }
}

//#############################################################################################
//##############################____INTERRUPT_HANDLING____#####################################
//#############################################################################################
#if HEATER_CONTROL
void dimCheck() {
    if (!zero_cross) {
        digitalWriteFast(relayPin, LOW);
        Timer1.stop();
    } else {
        digitalWriteFast(relayPin, HIGH);
        zero_cross = false;
        Timer1.setPeriod(TRIAC_PULSE_MICROS);
    }
}
#endif

//#############################################################################################
//##################################____PSM_BY_VAS____#########################################
//#############################################################################################

// This is here so using the z-c interrupt for heat control is easy

PSM::PSM(unsigned char sensePin, unsigned char controlPin, unsigned int range, int mode) {
    _thePSM = this;
    _secondZero = false;

    pinMode(sensePin, INPUT_PULLUP);
    PSM::_sensePin = sensePin;

    pinMode(controlPin, OUTPUT);
    PSM::_controlPin = controlPin;

    attachInterrupt(digitalPinToInterrupt(PSM::_sensePin), onInterrupt, mode);

    PSM::_range = range;
}

void PSM::onInterrupt() {
#if HEATER_CONTROL
    zero_cross = true;
    if (dimValue > TRIAC_MAX_DIM) {
        digitalWriteFast(relayPin, LOW);
    } else
        Timer1.setPeriod(dimValue);
#endif

    _thePSM->calculateSkip();
}

void PSM::set(unsigned int value) {
    if (value < PSM::_range) {
        PSM::_value = value;
    } else {
        PSM::_value = PSM::_range;
    }
}

unsigned int PSM::getValue() {
    return PSM::_value;
}

long PSM::getCounter() {
    return PSM::_counter;
}

void PSM::resetCounter() {
    PSM::_counter = 0;
}

void PSM::stopAfter(long counter) {
    PSM::_stopAfter = counter;
}

void PSM::calculateSkip() {
    if (!_secondZero) {
        PSM::_a += PSM::_value;

        if (PSM::_a >= PSM::_range) {
            PSM::_a -= PSM::_range + 1;
            PSM::_skip = false;
        } else {
            PSM::_skip = true;
        }

        if (PSM::_a > PSM::_range) {
            PSM::_a = 0;
            PSM::_skip = false;
        }

        if (!PSM::_skip) {
            PSM::_counter++;
        }

        if (!PSM::_skip && PSM::_stopAfter > 0 && PSM::_counter > PSM::_stopAfter) {
            PSM::_skip = true;
        }
    }

    _secondZero = !_secondZero;
    updateControl();
}

void PSM::updateControl() {
    if (PSM::_skip) {
        digitalWriteFast(PSM::_controlPin, LOW);
    } else {
        digitalWriteFast(PSM::_controlPin, HIGH);
    }
}

unsigned int PSM::cps() {
    unsigned int range = PSM::_range;
    unsigned int value = PSM::_value;

    PSM::_range = 0xFFFF;
    PSM::_value = 1;
    PSM::_a = 0;

    unsigned long stopAt = millis() + 1000;

    while (millis() < stopAt) {
        delay(0);
    }

    unsigned int result = PSM::_a;

    PSM::_range = range;
    PSM::_value = value;
    PSM::_a = 0;

    return result;
}