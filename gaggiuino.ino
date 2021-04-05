#include <Nextion.h>
#include "max6675.h"

// Define our pins
int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;
int relayPin = 8;

//Define the buffer used for the buttons sent to Nextion
//char buffer[10] = {0};

//Define the temp reading vars
float realTemp;
//uint32_t presetTemp = 96;

//Init the thermocouple with the appropriate pins
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

// Define the Nextion objects types
NexNumber brewTemp = NexNumber(1, 4, "n0");
NexButton bPlus = NexButton(1, 3, "b2");
NexButton bMinus = NexButton(1, 2, "b1");

//Register object n0, b0, b1, to the touch event list.
NexTouch *nex_listen_list[] =
{
  //&brewTemp,
  &bPlus,
  &bMinus,
  NULL  
};


// void brewTempPopCallback(void *ptr)
// {
//   uint32_t presetTemp;
//     brewTemp.setValue(presetTemp);
// }
void bPlusPushCallback(void *ptr)
{
  uint32_t presetTemp;
  brewTemp.getValue(&presetTemp); 
  presetTemp++;
  brewTemp.setValue(presetTemp);
}
void bMinusPushCallback(void *ptr)
{
  uint32_t presetTemp;
  brewTemp.getValue(&presetTemp);
  presetTemp--;
  brewTemp.setValue(presetTemp);
}

void setup() {
  Serial.begin(9600);
  nexInit();
  bPlus.attachPush(bPlusPushCallback, &bPlus);
  bMinus.attachPush(bMinusPushCallback, &bMinus);
  pinMode(relayPin, OUTPUT);
  // wait for MAX chip to stabilize
  delay(500);
}

void loop() {
  nexLoop(nex_listen_list);

// some logic to keep the boiler as close to the desired set temp as possible
  uint32_t presetTemp;
  brewTemp.getValue(&presetTemp);
  if (thermocouple.readCelsius() < presetTemp) {
    digitalWrite(relayPin, HIGH);
  }
  // else if (thermocouple.readCelsius() < (brewTemp - 2.00)) {
  //   digitalWrite(relayPin, HIGH);
  // }
  else {
    digitalWrite(relayPin, LOW);
  }
  // basic readout test, just print the current temp
  // Serial.print("C = ");

  // displaying the realtime temperature readings
  realTemp = thermocouple.readCelsius();
  String tempPrint = "t0.txt=\""+String(realTemp,1)+"\"";
  Serial.print(tempPrint);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  delay(250);
}
