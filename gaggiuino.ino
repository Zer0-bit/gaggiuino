#include <RBDdimmer.h>
#include <EEPROM.h>
#include <trigger.h>
#include <EasyNextionLibrary.h>
#include <max6675.h>
#include <ACS712.h>




// Define our pins
#define thermoDO 4
#define thermoCS 5
#define thermoCLK 6
#define brewSwitchPin A0 // PD7
#define relayPin 8  // PB0
#define dimmerPin 9

// Define some const values
#define GET_KTYPE_READ_EVERY 350
#define dimmerMinValue 90
#define dimmerMaxValue 97
#define dimmerDescaleMinValue 1
#define dimmerDescaleMaxValue 5

//RAM debug
extern unsigned int __bss_end;
extern unsigned int __heap_start;
extern void *__brkval;


//Init the thermocouple with the appropriate pins defined above with the prefix "thermo"
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
// EasyNextion object init
EasyNex myNex(Serial);
//Init the ACS712 hall sensor
ACS712 sensor(ACS712_20A, brewSwitchPin);
// RobotDYN Dimmer object init
dimmerLamp dimmer(dimmerPin); //initialase port for dimmer for MEGA, Leonardo, UNO, Arduino M0, Arduino Zero




// Global var  - just defined globally to avoid reading the temp sensor in every function
float currentTempReadValue = 0.0;
float lastReadTempValue = 0.0;
bool  descaleCheckBox = 0;
bool  preinfusionCheckBox = 0;
bool  pressureProfileCheckBox = 0;
bool  valuesRefreshOnPageChange = 0;
bool  pProfileRefresh;
uint16_t  HPWR = 0;
uint16_t  setPoint = 0;
uint16_t  offsetTemp = 0;
uint16_t  MainCycleDivider = 0;
uint16_t  BrewCycleDivider = 0;
uint16_t  preinfusionCounter = 0;
unsigned long thermoTimer = millis();

// Declaring local vars
// EEPROM  stuff
uint16_t  EEP_SETPOINT = 1;
uint16_t  EEP_OFFSET = 20;
uint16_t  EEP_HPWR = 40;
uint16_t  EEP_M_DIVIDER = 60;
uint16_t  EEP_B_DIVIDER = 80;
uint16_t  EEP_P_START = 100;
uint16_t  EEP_P_FINISH = 120;
uint16_t  EEP_P_AUTO = 140;
uint16_t  EEP_P_MANUAL = 160;


void setup() {
  
  Serial.begin(115200); // switching our board to the new serial speed

  // To debug correct work of the bellow feature later
  dimmer.begin(NORMAL_MODE, ON); //dimmer initialisation: name.begin(MODE, STATE)

  // Calibrating the hall current sensor
  sensor.calibrate();

  // relay port init and set initial operating mode
  pinMode(relayPin, OUTPUT);
  pinMode(brewSwitchPin, INPUT);

  // Chip side  HIGH/LOW  specification
  PORTB &= ~_BV(PB0);  // relayPin LOW

  // Will wait hereuntil full serial is established, this is done so the LCD fully initializes before passing the EEPROM values
  delay(500);


  //If it's the first boot we'll need to set some defaults
  if (EEPROM.read(0) != 253 || EEPROM.read(EEP_SETPOINT) == NULL || EEPROM.read(EEP_SETPOINT) == 65535) {
    EEPROM.put(0, 253);
    EEPROM.put(EEP_SETPOINT, 101);
    EEPROM.put(EEP_OFFSET, 8);
    EEPROM.put(EEP_HPWR, 550);
    EEPROM.put(EEP_M_DIVIDER, 5);
    EEPROM.put(EEP_B_DIVIDER, 2);
    EEPROM.put(EEP_P_START, 97);
    EEPROM.put(EEP_P_FINISH, 30);
    EEPROM.put(EEP_P_AUTO, 0);
    EEPROM.put(EEP_P_MANUAL, 0);
  }
  // Applying our saved EEPROM saved values
  uint16_t init_val;
  // Making sure we're getting the right values and sending them to the LCD module

  EEPROM.get(EEP_SETPOINT, init_val);
  if ( init_val > 0 && init_val != NULL ) myNex.writeNum("page1.n0.val", init_val);

  EEPROM.get(EEP_OFFSET, init_val);
  if ( init_val > 0 && init_val != NULL ) myNex.writeNum("page1.n1.val", init_val);

  EEPROM.get(EEP_HPWR, init_val);
  if (  init_val > 0 && init_val != NULL ) myNex.writeNum("page1.n2.val", init_val);

  EEPROM.get(EEP_M_DIVIDER, init_val);
  if ( init_val > 1 && init_val != NULL ) myNex.writeNum("page1.n4.val", init_val);

  EEPROM.get(EEP_B_DIVIDER, init_val);
  if (  init_val > 1 && init_val != NULL ) myNex.writeNum("page1.n3.val", init_val);

  EEPROM.get(EEP_P_START, init_val);
  if (  init_val > 1 && init_val != NULL ) myNex.writeNum("page2.n0.val", init_val);

  EEPROM.get(EEP_P_FINISH, init_val);
  if (  init_val > 1 && init_val != NULL ) myNex.writeNum("page2.n1.val", init_val);

  EEPROM.get(EEP_P_AUTO, init_val);
  if (  !(init_val < 0) && init_val < 2 && init_val != NULL ) myNex.writeNum("page2.c3.val", init_val);

  EEPROM.get(EEP_P_MANUAL, init_val);
  if (  !(init_val < 0) && init_val < 2 && init_val != NULL ) myNex.writeNum("page2.c4.val", init_val);

 myNex.writeStr("page 0");
 delay(10);
 myNex.lastCurrentPageId = 1;
}


//Main loop where all the bellow logic is continuously run
void loop() {
  pageValuesRefresh();
  myNex.NextionListen();
  kThermoRead();
  screenRefresh();
  doCoffee();

  // if (preinfusionCheckBox == 1 && descaleCheckBox == 0) { //Preinfusion
  //   preInfusion();
  // } else if (pressureProfileCheckBox == 1 && preinfusionCheckBox == 1 && descaleCheckBox == 0) { //Pressure profile
  //   pressureProfile(); //Applied if passing conditions
  // } else if(pressureProfileCheckBox == 1 && preinfusionCheckBox == 1 && descaleCheckBox == 0) { //pressure profile with preinfusion
  //   //logic to run preinfusion first
  //   // then switch to the main event
  //   preInfusion();
  // } else if (descaleCheckBox == 1) { // Descale function
  //   deScale(descaleCheckBox); 
  // } else {
  //   doCoffee(); //"normal" cycle
  // }
}


//  ALL used functions declared bellow
// The temperature, preinfusion, dimming, LCD update, etc control logic is all in the bellow functions
void doCoffee() {
  // Calculating the boiler heating power range based on the bellow input values
  uint16_t HPWR_OUT = constrain(HPWR_OUT, HPWR/MainCycleDivider, HPWR);  // limits range of sensor values to between 10 and 150
  HPWR_OUT = map(currentTempReadValue, setPoint - 10, setPoint, HPWR, HPWR / MainCycleDivider);


  if (brewState() == true && descaleCheckBox == 0) {
    if (preinfusionCheckBox == 0 && pressureProfileCheckBox == 0) dimmer.setPower(dimmerMaxValue);
    brewTimer(1);

  // Applying the HPWR_OUT variable as part of the relay switching logic
    if (currentTempReadValue < setPoint+0.5) {
      PORTB |= _BV(PB0);   // relayPIN -> HIGH
      delay(HPWR_OUT/BrewCycleDivider);  // delaying the relayPin state change
      PORTB &= ~_BV(PB0);  // relayPIN -> LOW
      delay(HPWR_OUT); 
    }
  } else if (brewState() == true && descaleCheckBox == 1) {
    if (currentTempReadValue < (float)setPoint - 10.00 && !(currentTempReadValue < 0.00) && currentTempReadValue != NAN) {
      PORTB |= _BV(PB0);  // relayPIN -> HIGH
    } else if (currentTempReadValue >= (float)setPoint - 10.00 && currentTempReadValue < (float)setPoint - 3.00) {
      PORTB |= _BV(PB0);   // relayPIN -> HIGH
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
      PORTB &= ~_BV(PB0);  // relayPIN -> LOW
    } else if (currentTempReadValue >= (float)setPoint - 3.00 && currentTempReadValue <= float(setPoint - 1.00)) {
      PORTB |= _BV(PB0);   // relayPIN -> HIGH
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
      PORTB &= ~_BV(PB0);  // relayPIN -> LOW
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
    } else if (currentTempReadValue >= (float)setPoint - 1.00 && currentTempReadValue < (float)setPoint - 0.50) {
      PORTB |= _BV(PB0);   // relayPIN -> HIGH
      delay(HPWR_OUT/2);  // delaying the relayPin state for <HPWR_OUT> ammount of time
      PORTB &= ~_BV(PB0);  // relayPIN -> LOW
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
    } else {
      PORTB &= ~_BV(PB0);  // relayPIN -> LOW
    }
  } else {
    brewTimer(0);
    if (currentTempReadValue < (float)setPoint - 10.00 && !(currentTempReadValue < 0.00) && currentTempReadValue != NAN) {
      PORTB |= _BV(PB0);  // relayPIN -> HIGH
    } else if (currentTempReadValue >= (float)setPoint - 10.00 && currentTempReadValue < (float)setPoint - 3.00) {
      PORTB |= _BV(PB0);   // relayPIN -> HIGH
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
      PORTB &= ~_BV(PB0);  // relayPIN -> LOW
    } else if (currentTempReadValue >= (float)setPoint - 3.00 && currentTempReadValue <= float(setPoint - 1.00)) {
      PORTB |= _BV(PB0);   // relayPIN -> HIGH
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
      PORTB &= ~_BV(PB0);  // relayPIN -> LOW
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
    } else if (currentTempReadValue >= (float)setPoint - 1.00 && currentTempReadValue < (float)setPoint - 0.50) {
      PORTB |= _BV(PB0);   // relayPIN -> HIGH
      delay(HPWR_OUT/2);  // delaying the relayPin state for <HPWR_OUT> ammount of time
      PORTB &= ~_BV(PB0);  // relayPIN -> LOW
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
    } else {
      PORTB &= ~_BV(PB0);  // relayPIN -> LOW
    }
  } 
}

void kThermoRead() {
  // Reading the temperature every 350ms between the loops
  if ((millis() - thermoTimer) > GET_KTYPE_READ_EVERY){
    currentTempReadValue = thermocouple.readCelsius();
    if (0 > currentTempReadValue == NAN) currentTempReadValue = thermocouple.readCelsius();  // Making sure we're getting a value
    thermoTimer = millis();
  }
}
void screenRefresh() {
  // myNex.currentPageId = myNex.readNumber("dp");
  // Updating the LCD every 300ms
    char waterTempPrint[6];
    static unsigned long pageRefreshTimer = millis();
    static bool steam_reset = 0;
    
  if (millis() - pageRefreshTimer > 350) {
    if (currentTempReadValue < 115.00 ) { //LCD update behaviour  when temps under 115C
      if (steam_reset == 1) {
        myNex.writeNum("page0.var_steam.val", 0); //Resetting the "STEAMING!" message
        steam_reset = 0;
      }
      uint16_t HPWR_OUT = constrain(HPWR_OUT, HPWR/MainCycleDivider, HPWR);
      myNex.writeNum("page0.n0.val", HPWR_OUT);      
    } else if (currentTempReadValue > 115.00 && currentTempReadValue < 170.00) { //Updating the LCD during steaming
      if (steam_reset == 0) {
        myNex.writeNum("page0.var_steam.val", 1);
        steam_reset = 1;
      }
    }
    // float tmp1 = currentTempReadValue - (float)offsetTemp;
    dtostrf(currentTempReadValue - (float)offsetTemp, 6, 2, waterTempPrint); // converting values with floating point to string
    myNex.writeStr("page0.t0.txt", waterTempPrint);  // Printing the current water temp values to the display
    myNex.writeNum("page0.n1.val", getFreeSram());
    pageRefreshTimer = millis();
    lastReadTempValue = currentTempReadValue;
  }
}

void pageValuesRefresh() {

  myNex.currentPageId = myNex.readNumber("dp");

  if (myNex.currentPageId != myNex.lastCurrentPageId) {
    valuesRefreshOnPageChange == false; //Making this false to update the values if a new page was loaded
    // Getting the latest LCD side set temp settings 
    if ( valuesRefreshOnPageChange == false ) {
      // Making sure the serial communication finishes sending all the values
      setPoint = myNex.readNumber("page1.n0.val");  // reading the setPoint value from the lcd
      if ( setPoint == NULL || setPoint < 0 ) setPoint = myNex.readNumber("page1.n0.val");

      offsetTemp = myNex.readNumber("page1.n1.val");  // reading the offset value from the lcd
      if (offsetTemp ==  NULL || offsetTemp < 0 ) offsetTemp = myNex.readNumber("page1.n1.val");

      HPWR = myNex.readNumber("page1.n2.val");  // reading the brew time delay used to apply heating in waves
      if ( HPWR == NULL || HPWR < 0 ) HPWR = myNex.readNumber("page1.n2.val");

      MainCycleDivider = myNex.readNumber("page1.n4.val");  // reading the delay divider
      if ( MainCycleDivider == NULL || MainCycleDivider < 1 ) MainCycleDivider = myNex.readNumber("page1.n4.val");

      BrewCycleDivider = myNex.readNumber("page1.n3.val");  // reading the delay divider
      if ( BrewCycleDivider == NULL || BrewCycleDivider < 1  ) BrewCycleDivider = myNex.readNumber("page1.n3.val");

      // reding the descale value which should be 0 or 1
      descaleCheckBox = myNex.readNumber("page2.c1.val");
      if ( descaleCheckBox < 0 || descaleCheckBox > 1 || descaleCheckBox == NULL ) descaleCheckBox = myNex.readNumber("page2.c1.val");

      // reding the preinfusion value which should be 0 or 1
      preinfusionCheckBox = myNex.readNumber("page2.c0.val");
      if ( preinfusionCheckBox < 0 || preinfusionCheckBox > 1 ) preinfusionCheckBox = myNex.readNumber("page2.c0.val");

      pressureProfileCheckBox = myNex.readNumber("page2.c2.val");
      if ( pressureProfileCheckBox < 0 || pressureProfileCheckBox > 1 ) pressureProfileCheckBox = myNex.readNumber("page2.c2.val");

      // some boolean logic controling values setting
      // powerBackOnDescaleFinish = true; // setting this to true so we make sure the pump has power after we turn off the descale mode
    }
    valuesRefreshOnPageChange == true; // setting this to TRUE so the condition is skipped on subsequent iterations
    myNex.lastCurrentPageId = myNex.currentPageId;
  }
}

// Save the desired temp values to EEPROM
void trigger1() {
  
  uint16_t savedBoilerTemp = myNex.readNumber("page1.n0.val");
  uint8_t savedOffsetTemp = myNex.readNumber("page1.n1.val");
  uint16_t savedHPWR = myNex.readNumber("page1.n2.val");
  uint8_t savedMainCycleDivider = myNex.readNumber("page1.n4.val");
  uint8_t savedBrewCycleDivider = myNex.readNumber("page1.n3.val");
  uint8_t savedPPStart = myNex.readNumber("page2.n0.val");
  uint8_t savedPPFinish = myNex.readNumber("page2.n1.val");
  bool savedAutoPP = myNex.readNumber("page2.c3.val");
  bool savedManPP = myNex.readNumber("page2.c4.val");
  uint8_t allValuesUpdated = 0;

  if (EEP_SETPOINT >= 19 ) EEP_SETPOINT = 1;
  if (EEP_OFFSET >= 39 ) EEP_OFFSET = 20;
  if (EEP_HPWR >= 59 ) EEP_HPWR = 40;
  if (EEP_M_DIVIDER >= 79 ) EEP_M_DIVIDER = 60;
  if (EEP_B_DIVIDER >= 99 ) EEP_B_DIVIDER = 80;
  if (EEP_P_START >= 119) EEP_P_START = 100;
  if (EEP_P_FINISH >= 139) EEP_P_FINISH = 120;
  if (EEP_P_AUTO >= 159) EEP_P_AUTO = 140;
  if (EEP_P_MANUAL >= 179) EEP_P_MANUAL = 160;

  // Reading our values from the LCD and checking whether we got proper serial communication
  if (savedBoilerTemp != NULL && savedBoilerTemp > 0) { 
    EEPROM.put(EEP_SETPOINT, savedBoilerTemp);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt","BOILER TEMP ERROR!");
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (savedOffsetTemp != NULL && savedOffsetTemp > 0) {
    EEPROM.put(EEP_OFFSET, savedOffsetTemp);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", (" OFFSET ERROR!"));
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (savedHPWR != NULL && savedHPWR > 0) {
    EEPROM.put(EEP_HPWR, savedHPWR);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", ("HPWR ERROR!"));
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (savedMainCycleDivider != NULL && savedMainCycleDivider >= 1) {
    EEPROM.put(EEP_M_DIVIDER, savedMainCycleDivider);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", (" M_DIV ERROR!"));
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (savedBrewCycleDivider != NULL && savedBrewCycleDivider >= 1) {
    EEPROM.put(EEP_B_DIVIDER, savedBrewCycleDivider);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", (" B_DIV ERROR!"));
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (savedPPStart != NULL && savedPPStart >= 1) {
    EEPROM.put(EEP_P_START, savedPPStart);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", ("PP Start ERROR!"));
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (savedPPFinish != NULL && savedPPFinish >= 1) {
    EEPROM.put(EEP_P_FINISH, savedPPFinish);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", ("PP Finish ERROR!"));
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (!(savedAutoPP < 0) || savedAutoPP != NULL ) {
    EEPROM.put(EEP_P_AUTO, (byte)savedAutoPP);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", ("AUTO PP ERROR!"));
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if ( !(savedManPP < 0) || savedManPP != NULL ) {
    EEPROM.put(EEP_P_MANUAL, (byte)savedManPP);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", ("MANUAL PP ERROR!"));
    myNex.writeStr("page popupMSG");
    delay(5);
  }

  if (allValuesUpdated == 9) {
    myNex.writeStr("popupMSG.t0.txt", ("UPDATE SUCCESSFUL!"));
    myNex.writeStr("page popupMSG");
    delay(5);
  }
}

void trigger2() {
  pProfileRefresh = false;
}


//Function to get the state of the brew switch button
//returns true or false based on the read P(power) value
bool brewState() {
  //Monitors the current flowing through the ACS712 circuit
  float P = 230 * sensor.getCurrentAC()*10;
  //When it exceeds the set limit 
  if ( P >= 400.00 ) return true;
  else return false;
}


void deScale(bool c) {
  static bool blink = true, value_set = false, dimmerModeSet = false;
  static unsigned long timer = millis();
  static int i = 0;
  if (c == true) {
    if (i < 6) { // descale in cycles for 5 times then wait according to the bellow condition
      // Logic that switches between modes depending on the $blink value
      if (blink == true) {
        if (value_set == false) { // making sure we're not constantly setting the dimmer value over serial
          dimmer.setPower(dimmerDescaleMaxValue);
          value_set = true;
        }
        if (millis() - timer > 10000UL) { //set dimmer power to max descale value for 10 sec
          blink = false;
          value_set = false;
          timer = millis();
        }
      }else {
        if (value_set == false) { // making sure we're not constantly setting the dimmer value over serial
          dimmer.setPower(dimmerDescaleMinValue);
          value_set = true;
        }
        if (millis() - timer > 20000UL) { //set dimmer power to min descale value for 20 sec
          blink = true;
          value_set = false;
          i+=1;
          timer = millis();
        }
      }
    }else {
      if (value_set == false) { // making sure we're not constantly setting the dimmer value over serial
        dimmer.setPower(0);
        value_set = true;
      }
      if (millis() - timer > 300000UL) { //do nothing for 5 minutes
        dimmer.setPower(dimmerDescaleMinValue);
        value_set = false;
        i=0;
        timer = millis();
      } 
    }
  }
  doCoffee(); //keeping it at temp
}


//#############################################################################################
//###############################____PRESSURE CONTROL____######################################
//#############################################################################################


// Pressure profiling function, uses dimmer to dim the pump 
// as time passes, starts dimming at about 15 seconds mark 
// goes from 9bar to the lower threshold set in settings(default 4bar)
void pressureProfile() {
  static bool setPerformed = 0, phase_1 = 1, phase_2 = 0, auto_p, man_p, pProfileActive;
  static unsigned long timer = millis();
  static uint8_t dimmerInput, start_p ,finish_p;
  
  pProfileActive = myNex.readNumber("page2.c2.val");

  if ( pProfileActive == true) {
    // if ( pProfileRefresh == false ) { //updating the values once to avoid serial spamming
      auto_p =  myNex.readNumber("page2.c3.val");
      man_p =  myNex.readNumber("page2.c4.val");
      start_p = myNex.readNumber("page2.n0.val");
      finish_p = myNex.readNumber("page2.n1.val");
      // pProfileRefresh = true;
    // }
    
    // bool tmp = myNex.readNumber("page2.c2.val");
    //DEBUG READ VALUES
    myNex.writeNum("page0.n2.val", start_p);
    //END DEBUG 

    if (brewState() == true && auto_p == true ) { //runs this only when brew button activated and pressure profile selected  
      if (phase_1 == true) { //enters phase 1
        if (millis() - timer >  15000UL) { // the actions of this if block are run after 15 seconds have passed since starting brewing
          phase_1 = 0;
          phase_2 = 1;
        }
        if (setPerformed == false) { // sets the start dimmer value once
          dimmer.setPower(start_p);
          setPerformed = true;
        }
      } else if (phase_2 == true) { //enters pahse 2
        if (millis() - timer > 1000UL) { // runs the bellow block every second
          dimmerInput+=7;
          if (start_p > finish_p) {
            uint8_t dimmerNewPowerVal = constrain(dimmerNewPowerVal, start_p, finish_p);  // limits range of sensor values to between start_p and finish_p
            dimmerNewPowerVal = map(dimmerInput, 0, 100, start_p, finish_p); //calculates a new dimmer power value every second given the max and min
            dimmer.setPower(dimmerNewPowerVal);
          }else if (start_p < finish_p) {
            uint8_t dimmerNewPowerVal = constrain(dimmerNewPowerVal, start_p, finish_p);  // limits range of sensor values to between start_p and finish_p
            dimmerNewPowerVal = map(dimmerInput, 0, 100, finish_p, start_p); //calculates a new dimmer power value every second given the max and min
            dimmer.setPower(dimmerNewPowerVal);
          }
          timer = millis();
        } 
      }
    }else if ( brewState() == true && man_p == true && myNex.currentPageId == 0 ) { //Manual preinfusion control
      if (setPerformed == false) {
        myNex.writeStr("vis h0,1");
        setPerformed == true;
      }
      dimmer.setPower(myNex.readNumber("page0.h0.val"));
    }
  }
  doCoffee(); // Keep that water at temp
}


// Pump dimming during brew for preinfusion
void preInfusion() {

  static unsigned long  timer = millis(),
                        preinfuseTime = myNex.readNumber("page2.n3.val");
  int8_t  start_val = myNex.readNumber("page2.n0.val");
  int8_t finish_val = myNex.readNumber("page2.n1.val");
  static bool preinfusionFinished = 0;

  myNex.writeNum("page0.n2.val", preinfuseTime);

  if (brewState() == true && (preinfusionCounter%2 == 0)) { //runs this only when brew button activated and pressure profile selected  
    if (preinfusionFinished == 0) { //enters phase 1
      if (millis() - timer > preinfuseTime*1000UL) { // keeping track of the elapsing time
        preinfusionFinished = 1;
      }
      brewTimer(1);
      // if (setPerformed == false) { // sets the start dimmer value once
      dimmer.setPower(start_val);
      myNex.writeNum("page0.n2.val", start_val);
      // setPerformed = true;
    } else if(preinfusionFinished == 1) {
      brewTimer(0);
      preinfusionCounter++;
      preinfusionFinished = 0;
      dimmer.setPower(finish_val);
      myNex.writeNum("page0.n2.val", finish_val);
    }
  }else if(brewState() == false && (preinfusionCounter%2 != 0)) {
    preinfusionCounter++;
  }
  doCoffee(); //more warmth into the masses
}

bool brewTimer(bool c) {
  if (myNex.currentPageId == 0 && c == 1) {  
    if (brewState() == true) {
      myNex.writeNum("page0.tim_v.val", 1);
    }
  }else if(myNex.currentPageId == 0 && c == 0) { 
    myNex.writeNum("page0.tim_v.val", 0);
  }
}


uint16_t getFreeSram() {
  uint8_t newVariable;
  // heap is empty, use bss as start memory address
  if ((uint16_t)__brkval == 0)
    return (((uint16_t)&newVariable) - ((uint16_t)&__bss_end));
  // use heap end as the start of the memory address
  else
    return (((uint16_t)&newVariable) - ((uint16_t)__brkval));
}
