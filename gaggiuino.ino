#include <EEPROM.h>
#include <Nextion.h>
#include <max6675.h>

// Define our pins
int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;
int relayPin = 8;

// EEPROM  stuff
int EEP_ADDR1 = 1, EEP_ADDR2 = 20, EEP_ADDR3 = 40, EEP_ADDR4 = 60;
unsigned long EEP_VALUE1, EEP_VALUE2, EEP_VALUE3, EEP_VALUE4;

uint32_t presetTemp, offsetTemp, brewTimeDelayTemp, brewTimeDelayDivider, realTempRead, waterTemp;
// uint32_t offsetTemp;
// uint32_t brewTimeDelayTemp;

// Heating Operational Vars
// uint32_t brewTimeDelayDivider;
float brewSwitchOnDetect, errCalc, overshootVariable, maxReachedTemp, lastMaxReachedTemp, sumOfAllMaxes, overshootErr;

//Define the temp reading vars
// uint32_t realTempRead;
// uint32_t waterTemp;

// Graph
// bool graph=false;


//Init the thermocouple with the appropriate pins
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

// Define the Nextion objects types
NexText waterTextBox0 = NexText(0, 2, "t0");
NexText boilerTextBox1 = NexText(0, 4, "t1");
NexText msgTextBox = NexText(2, 1, "t0");
NexVariable va0 = NexVariable(0, 7, "va0");
NexVariable va1 = NexVariable(0, 8, "va1");
NexVariable va2 = NexVariable(0, 9, "va2");
NexVariable va3 = NexVariable(0, 10, "va3");
NexVariable va_eeprom_0 = NexVariable(0, 12, "va4");
NexVariable va_eeprom_1 = NexVariable(0, 13, "va5");
NexVariable va_eeprom_2 = NexVariable(0, 14, "va6");
NexVariable va_eeprom_3 = NexVariable(0, 15, "va7");
NexNumber boilerTemp = NexNumber(1, 4, "n0");
NexNumber offTimeTemp = NexNumber(1, 6, "n1");
NexNumber brewTimeDelay = NexNumber(3, 6, "n0");
NexNumber brewTimeDivider = NexNumber(3, 10, "n1");
NexButton bPlus = NexButton(1, 3, "b2");
NexButton bMinus = NexButton(1, 2, "b1");
NexButton boffsetPlus = NexButton(1, 7, "b4");
NexButton boffsetMinus = NexButton(1, 5, "b3");
NexButton bSave = NexButton(1, 10, "b5");
NexPage msgPage = NexPage(2, 0, "popupMSG");
NexPage mainPage = NexPage(0, 0, "page0");
// NexWaveform graphPlotting = NexWaveform(5, 1, "s0");

//Register object n0, b0, b1, to the touch event list.
NexTouch *nex_listen_list[] =
{
  &bPlus,
  &bMinus,
  &boffsetPlus,
  &boffsetMinus,
  &bSave,
  NULL  
};

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

//Nextion stuff
// void waterTextBox0PushCallback(void *ptr)
// {
//   graphPage.show();
// }
void bPlusPushCallback(void *ptr)
{
  uint32_t presetTemp;
  boilerTemp.getValue(&presetTemp); 
  presetTemp++;
  boilerTemp.setValue(presetTemp);
}
void bMinusPushCallback(void *ptr)
{
  uint32_t presetTemp;
  boilerTemp.getValue(&presetTemp);
  presetTemp--;
  boilerTemp.setValue(presetTemp);
}
void boffsetPlusPushCallback(void *ptr)
{
  uint32_t offsetTemp;
  offTimeTemp.getValue(&offsetTemp); 
  offsetTemp++;
  offTimeTemp.setValue(offsetTemp);
}
void boffsetMinusPushCallback(void *ptr)
{
  uint32_t offsetTemp;
  offTimeTemp.getValue(&offsetTemp);
  offsetTemp--;
  offTimeTemp.setValue(offsetTemp);
}
void bSavePushCallback(void *ptr)
{
  // set desired temp values and save them to EEPROM
  int savedOffsetTemp;
  int savedBoilerTemp;
  int savedBrewTimeDelay;
  int savedbrewTimeDivider;
  boilerTemp.getValue(&EEP_VALUE1);
  offTimeTemp.getValue(&EEP_VALUE2);
  brewTimeDelay.getValue(&EEP_VALUE3);
  brewTimeDivider.getValue(&EEP_VALUE4);
  savedBoilerTemp = EEP_VALUE1 / 4;
  savedOffsetTemp = EEP_VALUE2 / 4;
  savedBrewTimeDelay = EEP_VALUE3 / 4;
  savedbrewTimeDivider = EEP_VALUE4 / 4;
  if (savedBoilerTemp != 0) {
    writeIntIntoEEPROM(EEP_ADDR1, savedBoilerTemp);
  }
  else {
    msgTextBox.setText("ERROR!");
    msgPage.show();
  }
  if (savedOffsetTemp != 0) {
    writeIntIntoEEPROM(EEP_ADDR2, savedOffsetTemp);
  }
  else {
    msgTextBox.setText("ERROR!");
    msgPage.show();
  }
  if (savedBrewTimeDelay != 0) {
    writeIntIntoEEPROM(EEP_ADDR3, savedBrewTimeDelay);
  }
  else {
    msgTextBox.setText("ERROR!");
    msgPage.show();
  }
  if (savedbrewTimeDivider != 0) {
    writeIntIntoEEPROM(EEP_ADDR4, savedbrewTimeDivider);
  }
  else {
    msgTextBox.setText("ERROR!");
    msgPage.show();
  }
}

// void drawGraph() {
//   while ( graph == true ) {
//     graphPlotting.addValue(0, waterTemp);
//     graphPlotting.addValue(1, (thermocouple.readCelsius()-10));
//   }
// }

// The precise temp control logic
void doCoffee() {
  int i=1;
  double brewTimeStartValue, brewTimeStopValue, previousBrewTimeDetectValue;
  float CurrentTempReadValue, PreviousTempReadValue;

  CurrentTempReadValue = thermocouple.readCelsius();
  // Getting the right values from the nextion lib, the whole Nextion thing is quite unstable might need to rewrite it later
  while (presetTemp == 0 || offsetTemp == 0 || brewTimeDelayTemp == 0 || brewTimeDelayDivider == 0 ) {
    va0.getValue(&presetTemp);
    va1.getValue(&offsetTemp);
    va2.getValue(&brewTimeDelayTemp);
    va3.getValue(&brewTimeDelayDivider);
  }

  // Calculating the overshoot value
  if (CurrentTempReadValue > presetTemp+1 && CurrentTempReadValue < presetTemp+3 && CurrentTempReadValue > maxReachedTemp) {
    maxReachedTemp = CurrentTempReadValue;
    i++;
    sumOfAllMaxes = maxReachedTemp + lastMaxReachedTemp; //193
    lastMaxReachedTemp = maxReachedTemp; //96
    overshootVariable = maxReachedTemp - presetTemp;//97-94=3 
    overshootErr = sumOfAllMaxes/presetTemp/i;//193/94=2.05
    errCalc = overshootVariable-overshootErr;//3-2.05=0.9
  }

  // some logic to keep the boiler as close to the desired set temp as possible 
  previousBrewTimeDetectValue = brewTimeStopValue - brewTimeStartValue;
  brewTimeStartValue = millis();
  waterTemp=presetTemp-float(offsetTemp);
  if ((relayPin != HIGH) && (CurrentTempReadValue - PreviousTempReadValue > 0) && (CurrentTempReadValue < float(presetTemp-10))) {
    digitalWrite(relayPin, HIGH);
  }
  else if ((CurrentTempReadValue - PreviousTempReadValue > 0) && (CurrentTempReadValue >= float(presetTemp-10)) && (CurrentTempReadValue < float(presetTemp)-overshootVariable)) { //errCalc
    digitalWrite(relayPin, HIGH);
    delay(brewTimeDelayTemp);
    digitalWrite(relayPin, LOW);
  }
  else if ((CurrentTempReadValue - PreviousTempReadValue < 0) && (CurrentTempReadValue >= float(presetTemp-10)) && (CurrentTempReadValue <= float(presetTemp)-overshootVariable)) { //2
    digitalWrite(relayPin, HIGH);
    delay(brewTimeDelayTemp);
    digitalWrite(relayPin, LOW);
  }
  else if ((CurrentTempReadValue - PreviousTempReadValue > 0) && (CurrentTempReadValue >= float(presetTemp)-errCalc) && (CurrentTempReadValue < float(presetTemp))) {
    digitalWrite(relayPin, HIGH);
    delay(brewTimeDelayTemp/brewTimeDelayDivider);
    digitalWrite(relayPin, LOW);
    delay(brewTimeDelayTemp);
  }
  else if ((CurrentTempReadValue - PreviousTempReadValue < 0) && (CurrentTempReadValue - PreviousTempReadValue > -0.5) && (CurrentTempReadValue >= float(presetTemp)-errCalc) && (CurrentTempReadValue < float(presetTemp))) {
    digitalWrite(relayPin, HIGH);
    delay(brewTimeDelayTemp/brewTimeDelayDivider);
    digitalWrite(relayPin, LOW);
    delay(brewTimeDelayTemp);
  }
  else if ((CurrentTempReadValue - PreviousTempReadValue < -1) && (CurrentTempReadValue >= float(presetTemp)-errCalc) && (CurrentTempReadValue < float(presetTemp+0.2))) {
    digitalWrite(relayPin, HIGH);
    delay(brewTimeDelayTemp/(brewTimeDelayDivider/2));
    digitalWrite(relayPin, LOW);
    delay(brewTimeDelayTemp);
  }
  // else if (PreviousTempReadValue - CurrentTempReadValue > 1 && CurrentTempReadValue >= presetTemp-0.2 && CurrentTempReadValue <= presetTemp+0.2 && millis() - brewTimeStartValue < float(brewTimeDelayTemp)/float(brewTimeDelayDivider)) {
  //   digitalWrite(relayPin, HIGH);
  //   delay(brewTimeDelayTemp);
  //   digitalWrite(relayPin, LOW);
  // }
  else {
    digitalWrite(relayPin, LOW);
  }
  brewTimeStopValue = millis();
  PreviousTempReadValue = CurrentTempReadValue;
}
void update_t0_t1() {
  float tmp = thermocouple.readCelsius();
  String displayTemp;
  String realTemp;
  displayTemp = tmp - float(offsetTemp);
  // realTemp = tmp;
  realTemp = errCalc;
  char const* waterTempPrint = displayTemp.c_str();
  char const* realTempPrint = realTemp.c_str();
  waterTextBox0.setText(waterTempPrint);
  boilerTextBox1.setText(realTempPrint);
}
void setup() {
  Serial.begin(9600);  
  nexInit();
  delay(500);
  //Registering a higher baud for hopefully more responsive touch controls
  Serial.print("baud=115200");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.end();
  Serial.begin(115200);

  bPlus.attachPush(bPlusPushCallback, &bPlus);
  bMinus.attachPush(bMinusPushCallback, &bMinus);
  boffsetPlus.attachPush(boffsetPlusPushCallback, &boffsetPlus);
  boffsetMinus.attachPush(boffsetMinusPushCallback, &boffsetMinus);
  bSave.attachPush(bSavePushCallback, &bSave);

  // port setup
  pinMode(relayPin, OUTPUT);  
  // port init with - starts with the relay decoupled just in case
  digitalWrite(relayPin, LOW);

  // wait for EEPROM and other chip to stabilize  
  uint32_t tmp1 = readIntFromEEPROM(EEP_ADDR1);
  uint32_t tmp2 = readIntFromEEPROM(EEP_ADDR2);
  uint32_t tmp3 = readIntFromEEPROM(EEP_ADDR3);
  uint32_t tmp4 = readIntFromEEPROM(EEP_ADDR4);
  if ((tmp1 != 0) && (tmp2 != 0) && (tmp3 != 0) && (tmp4 != 0)) {
    va_eeprom_0.setValue(tmp1);
    va_eeprom_1.setValue(tmp2);
    va_eeprom_2.setValue(tmp3);
    va_eeprom_3.setValue(tmp4);
  }
}
void loop() {
  nexLoop(nex_listen_list);
  doCoffee();
  update_t0_t1();
  delay(250);
}
