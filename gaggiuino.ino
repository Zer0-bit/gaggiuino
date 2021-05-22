#include <EEPROM.h>
#include <trigger.h>
#include <EasyNextionLibrary.h>
#include <max6675.h>
// #include <iostream>
// #include <string>

// Define our pins
int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;
int relayPin = 8;

// EEPROM  stuff
int EEP_ADDR1 = 1, EEP_ADDR2 = 20, EEP_ADDR3 = 40, EEP_ADDR4 = 60;
unsigned long EEP_VALUE1, EEP_VALUE2, EEP_VALUE3, EEP_VALUE4;

uint32_t presetTemp, offsetTemp, brewTimeDelayTemp, brewTimeDelayDivider, realTempRead, waterTemp;
float brewSwitchOnDetect, errCalc, overshootVariable, maxReachedTemp, lastMaxReachedTemp, sumOfAllMaxes, overshootErr, overshootDivider;



//Init the thermocouple with the appropriate pins
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
EasyNex myNex(Serial);

// EEPROM WRITE
void writeIntIntoEEPROM(int address, int number)
{ 
  EEPROM.update(address, number >> 8);
  EEPROM.update(address + 1, number & 0xFF);
}
//EEPROM READ
int readIntFromEEPROM(int address)
{
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}


void trigger1()
{
  uint32_t presetTemp;
  presetTemp = myNex.readNumber("page1.n0.val");
  presetTemp--;
  myNex.writeNum("page1.n0.val",presetTemp);
}
void trigger2()
{
  uint32_t presetTemp;
  presetTemp = myNex.readNumber("page1.n0.val");
  presetTemp++;
  myNex.writeNum("page1.n0.val",presetTemp);
}
void trigger3()
{
  uint32_t offsetTemp;
  offsetTemp = myNex.readNumber("page1.n1.val");
  offsetTemp--;
  myNex.writeNum("page1.n1.val",offsetTemp);
}
void trigger4()
{
  uint32_t offsetTemp;
  offsetTemp = myNex.readNumber("page1.n1.val");
  offsetTemp++;
  myNex.writeNum("page1.n1.val",offsetTemp);
}
void trigger5()
{
  // set desired temp values and save them to EEPROM
  int savedBoilerTemp = myNex.readNumber("page1.n0.val");
  int savedOffsetTemp = myNex.readNumber("page1.n1.val");
  int savedBrewTimeDelay = myNex.readNumber("page2.n0.val");
  int savedbrewTimeDivider = myNex.readNumber("page2.n1.val");

  writeIntIntoEEPROM(EEP_ADDR1, savedBoilerTemp);
  writeIntIntoEEPROM(EEP_ADDR2, savedOffsetTemp);
  writeIntIntoEEPROM(EEP_ADDR3, savedBrewTimeDelay);
  writeIntIntoEEPROM(EEP_ADDR4, savedbrewTimeDivider);
  myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
  myNex.writeStr("page popupMSG");
}

void trigger6()
{
  for (int i=0; i<=30; i++)
  {
    if (i < 25) {
      myNex.writeNum("page0.sec_number.pco", 65535);
      myNex.writeNum("page0.sec_number.val", i);
    }
    else if (i > 25)
    {
      myNex.writeNum("page0.sec_number.pco", 63488);
      myNex.writeNum("page0.sec_number.val", i);
    }
    delay(1000);
  }
}

// The precise temp control logic
void doCoffee() {
  int i=1;
  double brewTimeStartValue, brewTimeStopValue, previousBrewTimeDetectValue;
  float CurrentTempReadValue, PreviousTempReadValue;

  CurrentTempReadValue = thermocouple.readCelsius();
  
  presetTemp = myNex.readNumber("page1.n0.val");
  offsetTemp = myNex.readNumber("page1.n1.val");
  brewTimeDelayTemp = myNex.readNumber("page2.n0.val");
  brewTimeDelayDivider = myNex.readNumber("page2.n1.val");

  // Calculating the overshoot value
  if (CurrentTempReadValue > presetTemp+1 && CurrentTempReadValue < presetTemp+3 && CurrentTempReadValue > maxReachedTemp) {
    maxReachedTemp = CurrentTempReadValue;
    i++;
    sumOfAllMaxes = maxReachedTemp + lastMaxReachedTemp; //193
    lastMaxReachedTemp = maxReachedTemp; //96
    overshootVariable = maxReachedTemp - presetTemp;//97-94=3 
    overshootErr = sumOfAllMaxes/presetTemp/i;//193/94=2.05
    overshootDivider = overshootVariable / errCalc; // 3/2.05=1.46
    errCalc = overshootVariable-overshootErr;//3-2.05=0.9
  }

  // some logic to keep the boiler as close to the desired set temp as possible 
  //previousBrewTimeDetectValue = brewTimeStopValue - brewTimeStartValue;
  brewTimeStartValue = millis();
  waterTemp=presetTemp-float(offsetTemp);
  if ((relayPin != HIGH) && (CurrentTempReadValue < float(presetTemp)-10)) {
    digitalWrite(relayPin, HIGH);
  }
  else if ((CurrentTempReadValue - PreviousTempReadValue > 0) && (CurrentTempReadValue >= float(presetTemp)-10) && (CurrentTempReadValue < float(presetTemp)-overshootVariable)) {
    digitalWrite(relayPin, HIGH);
    delay(brewTimeDelayTemp);
    digitalWrite(relayPin, LOW);
  }
  else if ((CurrentTempReadValue - PreviousTempReadValue < 0) && (CurrentTempReadValue - PreviousTempReadValue > -1) && (CurrentTempReadValue >= float(presetTemp)-10) && (CurrentTempReadValue <= float(presetTemp)-overshootVariable)) {
    digitalWrite(relayPin, HIGH);
    delay(brewTimeDelayTemp);
    digitalWrite(relayPin, LOW);
  }
  else if ((CurrentTempReadValue - PreviousTempReadValue > 0) && (CurrentTempReadValue >= float(presetTemp)-overshootDivider) && (CurrentTempReadValue < float(presetTemp)-0.2)) {
    digitalWrite(relayPin, HIGH);
    delay(brewTimeDelayTemp/brewTimeDelayDivider/2);
    digitalWrite(relayPin, LOW);
    delay(brewTimeDelayTemp);
  }
  else if ((CurrentTempReadValue - PreviousTempReadValue < 0) && (CurrentTempReadValue - PreviousTempReadValue > -0.5) && (CurrentTempReadValue >= float(presetTemp)-overshootDivider) && (CurrentTempReadValue < float(presetTemp)-0.2)) {
    digitalWrite(relayPin, HIGH);
    delay(brewTimeDelayTemp/brewTimeDelayDivider/2);
    digitalWrite(relayPin, LOW);
    delay(brewTimeDelayTemp);
  }
  else if (((CurrentTempReadValue - PreviousTempReadValue < -1) || (CurrentTempReadValue - PreviousTempReadValue > 0.5)) && (CurrentTempReadValue >= float(presetTemp)-errCalc) && (CurrentTempReadValue < float(presetTemp)-0.2)) {
    digitalWrite(relayPin, HIGH);
    delay(brewTimeDelayTemp/(brewTimeDelayDivider));
    digitalWrite(relayPin, LOW);
    delay(brewTimeDelayTemp);
  }
  else {
    digitalWrite(relayPin, LOW);
  }
  brewTimeStopValue = millis();
  PreviousTempReadValue = CurrentTempReadValue;
}
void update_t0_t1() {
  float tmp = thermocouple.readCelsius();
  String waterTempPrint, displayTemp, overshootVariablePrint, overshootErrPrint, overshootDividerPrint, errCalcPrint;

  waterTempPrint = tmp - float(offsetTemp);
  overshootVariablePrint = overshootVariable;
  overshootErrPrint = overshootErr;
  overshootDividerPrint = overshootDivider;
  errCalcPrint = errCalc;
  // realTemp = tmp;
  // realTemp = errCalc;
  //char const* waterTempPrint = displayTemp.c_str();
  // char const* overshootVariableChar = overshootVariablePrint.c_str();
  // char const* overshootErrChar = overshootErrPrint.c_str();
  // char const* overshootDividerChar = overshootDividerPrint.c_str();
  // char const* errCalcChar = errCalcPrint.c_str();

  myNex.writeStr("page0.t0.txt", waterTempPrint);
  myNex.writeStr("page0.t1.txt", overshootVariablePrint);
  myNex.writeStr("page0.t2.txt", overshootErrPrint);
  myNex.writeStr("page0.t3.txt", overshootDividerPrint);
  myNex.writeStr("page0.t4.txt", errCalcPrint);
}
void setup() {
  
  myNex.begin(9600);  
  // nexInit();
  // delay(500);
  //Registering a higher baud for hopefully more responsive touch controls
  Serial.print("baud=115200");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.end();
  Serial.begin(115200);

  // port setup
  pinMode(relayPin, OUTPUT);  
  // port init with - starts with the relay decoupled just in case
  digitalWrite(relayPin, LOW);

  delay(1000);

  // wait for EEPROM and other chip to stabilize  
  uint32_t tmp1 = readIntFromEEPROM(EEP_ADDR1);
  uint32_t tmp2 = readIntFromEEPROM(EEP_ADDR2);
  uint32_t tmp3 = readIntFromEEPROM(EEP_ADDR3);
  uint32_t tmp4 = readIntFromEEPROM(EEP_ADDR4);
  if ((tmp1 != 0) && (tmp2 != 0) && (tmp3 != 0) && (tmp4 != 0)) {
    myNex.writeNum("page1.n0.val",tmp1);
    myNex.writeNum("page1.n1.val",tmp2);
    myNex.writeNum("page2.n0.val",tmp3);
    myNex.writeNum("page2.n1.val",tmp4);
  }
}
void loop() {
  myNex.NextionListen();
  doCoffee();
  update_t0_t1();
  delay(250);
}
