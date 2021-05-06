#include <EEPROM.h>
#include <Nextion.h>
#include <max6675.h>

// Define our pins
int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;
int relayPin = 8;

// EEPROM  stuff
int EEP_ADDR1 = 1;
int EEP_ADDR2 = 20;
int EEP_ADDR3 = 40;
unsigned long EEP_VALUE1;
unsigned long EEP_VALUE2;
unsigned long EEP_VALUE3;
uint32_t presetTemp;
uint32_t offsetTemp;
uint32_t brewTimeDelayTemp;

//Define the temp reading vars
uint32_t lastSetBoilerTemp;
uint32_t lastSetOffsetTemp;
uint32_t waterTemp;


//Init the thermocouple with the appropriate pins
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

// Define the Nextion objects types
NexText waterTextBox0 = NexText(0, 2, "t0");
NexText boilerTextBox1 = NexText(0, 4, "t1");
NexVariable va0 = NexVariable(0, 7, "va0");
NexVariable va1 = NexVariable(0, 8, "va1");
NexVariable va2 = NexVariable(0, 9, "va2");
NexVariable va3 = NexVariable(0, 10, "va3");
NexNumber boilerTemp = NexNumber(1, 4, "n0");
NexNumber offTimeTemp = NexNumber(1, 6, "n1");
NexNumber brewTimeDelay = NexNumber(1, 11, "n2");
NexButton bPlus = NexButton(1, 3, "b2");
NexButton bMinus = NexButton(1, 2, "b1");
NexButton boffsetPlus = NexButton(1, 7, "b4");
NexButton boffsetMinus = NexButton(1, 5, "b3");
NexButton bSave = NexButton(1, 10, "b5");
NexPage startPage = NexPage(1, 0, "page1");
NexPage mainPage = NexPage(0, 0, "page0");

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
  boilerTemp.getValue(&EEP_VALUE1);
  offTimeTemp.getValue(&EEP_VALUE2);
  brewTimeDelay.getValue(&EEP_VALUE3);
  savedBoilerTemp = EEP_VALUE1 / 4;
  savedOffsetTemp = EEP_VALUE2 / 4;
  savedBrewTimeDelay = EEP_VALUE3 / 4;
  if ((savedBoilerTemp != 0) && (savedOffsetTemp != 0) && (savedBrewTimeDelay != 0)) {
    writeIntIntoEEPROM(EEP_ADDR1, savedBoilerTemp);
    writeIntIntoEEPROM(EEP_ADDR2, savedOffsetTemp);
    writeIntIntoEEPROM(EEP_ADDR3, savedBrewTimeDelay);
  }
  else {}
}

// The precise temp control logic
void doCoffee() {
  // Getting the right values from the nextion lib, the whole Nextion thing is quite unstable might need to rewrite it later
  while (presetTemp == 0 || offsetTemp == 0 || brewTimeDelayTemp == 0 ) {
    va0.getValue(&presetTemp);
    va1.getValue(&offsetTemp);
    va2.getValue(&brewTimeDelayTemp);
  }
 // some logic to keep the boiler as close to the desired set temp as possible 
  waterTemp=presetTemp-float(offsetTemp);
  if (thermocouple.readCelsius() < float(presetTemp-10)) {
    digitalWrite(relayPin, HIGH);
  }
  else if (thermocouple.readCelsius() >= float(presetTemp-10) && thermocouple.readCelsius() < float(presetTemp-0.5)) {
    digitalWrite(relayPin, HIGH);
    delay(brewTimeDelayTemp);
    digitalWrite(relayPin, LOW);
  }
  else if (thermocouple.readCelsius() > float(presetTemp) && thermocouple.readCelsius() <= float(presetTemp+0.5)) {
    digitalWrite(relayPin, HIGH);
    delay(brewTimeDelayTemp/2);
    digitalWrite(relayPin, LOW);
  }
  else if (thermocouple.readCelsius() > float(presetTemp+0.5) && thermocouple.readCelsius() <= float(presetTemp+1)) {
    digitalWrite(relayPin, HIGH);
    delay(brewTimeDelayTemp/3);
    digitalWrite(relayPin, LOW);
  }
  else {
    digitalWrite(relayPin, LOW);
  }
}
void update_t0_t1() {
  float tmp = thermocouple.readCelsius();
  String displayTemp;
  String realTemp;
  displayTemp = tmp - float(offsetTemp);
  realTemp = tmp;
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
  if ((tmp1 != 0) && (tmp2 != 0) && (tmp3 != 0)) {
    boilerTemp.setValue(tmp1);
    offTimeTemp.setValue(tmp2);
    brewTimeDelay.setValue(tmp3);
  }
  //startPage.show();
}
void loop() {
  doCoffee();
  update_t0_t1();
  delay(250);
  nexLoop(nex_listen_list);
}
