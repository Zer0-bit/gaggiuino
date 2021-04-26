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
  boilerTemp.getValue(&presetTemp);
  offTimeTemp.getValue(&offsetTemp);
  brewTimeDelay.getValue(&brewTimeDelayTemp);

 // some logic to keep the boiler as close to the desired set temp as possible 
  waterTemp=presetTemp-float(offsetTemp);
  if (thermocouple.readCelsius() < float(presetTemp-10)) {
    digitalWrite(relayPin, HIGH);
  }
  else if (thermocouple.readCelsius() >= float(presetTemp-10) && thermocouple.readCelsius() < float(presetTemp)) {
    digitalWrite(relayPin, HIGH);
    delay(brewTimeDelayTemp);
    digitalWrite(relayPin, LOW);
  }
  else {
    digitalWrite(relayPin, LOW);
  }
}
void update_t0_water() {
  float tmp = thermocouple.readCelsius();
  String displayTemp;
  displayTemp = tmp - float(offsetTemp);
  //waterTempPrint = "t0.txt=\""+String(displayTemp,1)+"\"";
  char const* waterTempPrint = displayTemp.c_str();
  waterTextBox0.setText(waterTempPrint);
}
void update_t1_boiler() {
  String realTemp;
  float tmp = thermocouple.readCelsius();
  realTemp = tmp;
  char const* realTempPrint = realTemp.c_str();
  boilerTextBox1.setText(realTempPrint);
}
void setup() {
  Serial.begin(9600);  
  nexInit();
  startPage.show();
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
}

void loop() {
  doCoffee();
  update_t0_water();
  update_t1_boiler();
  delay(250);
  nexLoop(nex_listen_list);
}
