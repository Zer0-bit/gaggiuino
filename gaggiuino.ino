#include <EEPROM.h>
#include <Nextion.h>
#include <max6675.h>

// Define our pins
int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;
int relayPin = 8;

// EEPROM  stuff
const uint32_t COUNT_ADDR1 = 0;
const uint32_t COUNT_ADDR2 = 20;
float E_ADDR1 = 0;
float E_ADDR2 = 0;
uint32_t presetTemp;
uint32_t offsetTemp;

//Define the buffer used for the buttons sent to Nextion
//char buffer[10] = {0};

//Define the temp reading vars
float realTemp;
float displayTemp;
uint32_t lastSetBoilerTemp;
uint32_t lastSetOffsetTemp;
uint32_t savedOffsetTemp;
uint32_t savedBoilerTemp;
uint32_t waterTemp;


//Init the thermocouple with the appropriate pins
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

// Define the Nextion objects types
NexNumber boilerTemp = NexNumber(1, 4, "n0");
NexNumber offTemp = NexNumber(1, 6, "n1");
NexButton bPlus = NexButton(1, 3, "b2");
NexButton bMinus = NexButton(1, 2, "b1");
NexButton boffsetPlus = NexButton(1, 7, "b4");
NexButton boffsetMinus = NexButton(1, 5, "b3");
NexButton bSave = NexButton(1, 10, "b5");
NexPage startPage = NexPage(1, 0, "page1");

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
  offTemp.getValue(&offsetTemp); 
  offsetTemp++;
  offTemp.setValue(offsetTemp);
}
void boffsetMinusPushCallback(void *ptr)
{
  uint32_t offsetTemp;
  offTemp.getValue(&offsetTemp);
  offsetTemp--;
  offTemp.setValue(offsetTemp);
}
void bSavePushCallback(void *ptr)
{
  //Get current set desired temp values and save them to EEPROM
  boilerTemp.getValue(&savedBoilerTemp);
  offTemp.getValue(&savedOffsetTemp);
  E_ADDR1 = savedBoilerTemp;
  E_ADDR2 = savedOffsetTemp;
  EEPROM.update(COUNT_ADDR1, E_ADDR1);
  EEPROM.update(COUNT_ADDR2, E_ADDR2);
  
}
void setup() {
  Serial.begin(9600);
  nexInit();
  pinMode(relayPin, OUTPUT);
  // wait for EEPROM and other chip to stabilize
  startPage.show();
  delay(2000);
  bPlus.attachPush(bPlusPushCallback, &bPlus);
  bMinus.attachPush(bMinusPushCallback, &bMinus);
  boffsetPlus.attachPush(boffsetPlusPushCallback, &boffsetPlus);
  boffsetMinus.attachPush(boffsetMinusPushCallback, &boffsetMinus);
  bSave.attachPush(bSavePushCallback, &bSave);
  boilerTemp.setValue(EEPROM.read(COUNT_ADDR1));
  offTemp.setValue(EEPROM.read(COUNT_ADDR2));  
}

void loop() {
  nexLoop(nex_listen_list);
  
// Readig the preset and offset temperature values set on the Nextion Menu page
  boilerTemp.getValue(&presetTemp);
  offTemp.getValue(&offsetTemp);
  
// some logic to keep the boiler as close to the desired set temp as possible 
  waterTemp=presetTemp-float(offsetTemp);
  if (thermocouple.readCelsius() < float(presetTemp-10)) {
    digitalWrite(relayPin, HIGH);
  }
  else if (thermocouple.readCelsius() >= float(presetTemp-10) && thermocouple.readCelsius() < float(presetTemp)) {
    digitalWrite(relayPin, HIGH);
    delay(700);
    digitalWrite(relayPin, LOW);
  }
  else {
    digitalWrite(relayPin, LOW);
  }
  
  // Writing the temp values on the Nextion textboxes
  realTemp = thermocouple.readCelsius();
  String realTempPrint = "t1.txt=\""+String(realTemp,1)+"\"";
  Serial.print(realTempPrint);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  displayTemp = realTemp - float(offsetTemp);
  String waterTempPrint = "t0.txt=\""+String(displayTemp,1)+"\"";
  Serial.print(waterTempPrint);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  
  // Needed so the thermocouple has time to do the temps calculations
  delay(250);
}
