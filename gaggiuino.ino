#include <EEPROM.h>
#include <trigger.h>
#include <EasyNextionLibrary.h>
#include <max6675.h>

#define MAX_TEMP 165

// Define our pins
int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;
int relayPin = 8;

//Init the thermocouple with the appropriate pins
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
EasyNex myNex(Serial);

// EEPROM  stuff
const int EEP_ADDR1 = 1, EEP_ADDR2 = 20, EEP_ADDR3 = 40, EEP_ADDR4 = 60;
float currentTempReadValue = 0;
long timer = 0;
const unsigned long interval = 250;
const float STEAM_START = 105;




void setup() {
  myNex.begin(9600);  
  //Registering a higher baud for hopefully more responsive touch controls
  Serial.print("baud=115200");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.end();
  Serial.begin(115200);

  // relay port init and set initial operating mode
  pinMode(relayPin, OUTPUT);  
  digitalWrite(relayPin, LOW);

  // Small delay so the LCD fully initializes before passing the EEPROM values
  delay(800);

  // Applying the EEPROM saved values  
  uint32_t tmp1 = 0;
  uint32_t tmp2 = readIntFromEEPROM(EEP_ADDR2);
  uint32_t tmp3 = readIntFromEEPROM(EEP_ADDR3);
  uint32_t tmp4 = readIntFromEEPROM(EEP_ADDR4);

  // Making sure we're getting the right values and sending them to the display   
  tmp1 = readIntFromEEPROM(EEP_ADDR1);
  if(tmp1 != 777777){       // 777777is the return value if the code fails to read the new value
    myNex.writeNum("page1.n0.val",tmp1);
  } 
  else if(tmp1 == 777777){
    tmp1 = readIntFromEEPROM(EEP_ADDR1);
    delay(5); //Small delay to get full serial
    myNex.writeNum("page1.n0.val",tmp1);
    delay(5); //Small delay to get full serial
  }
  tmp2 = readIntFromEEPROM(EEP_ADDR2);
  if(tmp2 != 777777){       // 777777is the return value if the code fails to read the new value
    myNex.writeNum("page1.n1.val",tmp2);
  } 
  else if(tmp2 == 777777){
    tmp2 = readIntFromEEPROM(EEP_ADDR2);
    delay(5); //Small delay to get full serial
    myNex.writeNum("page1.n1.val",tmp2);
    delay(5); //Small delay to get full serial
  }
  tmp3 = readIntFromEEPROM(EEP_ADDR3);
  if(tmp3 != 777777){       // 777777is the return value if the code fails to read the new value
    myNex.writeNum("page2.n0.val",tmp3);
  } 
  else if(tmp3 == 777777){
    tmp3 = readIntFromEEPROM(EEP_ADDR3);
    delay(5); //Small delay to get full serial
    myNex.writeNum("page2.n0.val",tmp3);
    delay(5); //Small delay to get full serial
  }
  tmp4 = readIntFromEEPROM(EEP_ADDR4);
  if(tmp4 != 777777){       // 777777is the return value if the code fails to read the new value
    myNex.writeNum("page2.n1.val",tmp4);
  } 
  else if(tmp4 == 777777){
    tmp4 = readIntFromEEPROM(EEP_ADDR4);
    delay(5); //Small delay to get full serial
    myNex.writeNum("page2.n1.val",tmp4);
    delay(5); //Small delay to get full serial
  }
}


//Main loop where all the bellow logic is continuously run
void loop() {
  // Reading the temperature just once ever 250ms between the loops while making sure we're getting a value
  if (millis() - timer >= interval) {
    timer += interval;
    currentTempReadValue = thermocouple.readCelsius();
    if (currentTempReadValue == NAN || currentTempReadValue < 0) currentTempReadValue = thermocouple.readCelsius();
  }
  myNex.NextionListen();
  doCoffee();
  updateLCD();
  // delay(250);  //delay so max6675 has a chance to read the values
}


//  ALL used functions declared bellow
// The *precise* temp control logic
void doCoffee() {
  int setPoint = 0;
  int offsetTemp = 0;
  int brewTimeDelayTemp = 0;
  int brewTimeDelayDivider = 0;

  // Making sure the serial communication finishes sending all the values
  setPoint = myNex.readNumber("page1.n0.val");
  if (setPoint != 777777) {
    delay(1);
  }
  else if (setPoint == 777777) {
    setPoint = myNex.readNumber("page1.n0.val");
  }

  offsetTemp = myNex.readNumber("page1.n1.val");
  if (offsetTemp != 777777) {
    delay(1);
  }
  else if (offsetTemp == 777777) {
    offsetTemp = myNex.readNumber("page1.n1.val");
  }

  brewTimeDelayTemp = myNex.readNumber("page2.n0.val");
  if (brewTimeDelayTemp != 777777) {
    delay(1);
  }
  else if (brewTimeDelayTemp == 777777) {
    brewTimeDelayTemp = myNex.readNumber("page2.n0.val");
  }

  brewTimeDelayDivider = myNex.readNumber("page2.n1.val");
  if (brewTimeDelayDivider != 777777) {
    delay(1);
  }
  else if (brewTimeDelayDivider == 777777) {
    brewTimeDelayDivider = myNex.readNumber("page2.n1.val");
  }
  // Calculating the boiler heating power
  int powerOutput = map(currentTempReadValue, setPoint-10, setPoint, brewTimeDelayTemp, brewTimeDelayTemp/brewTimeDelayDivider);
  if (powerOutput > brewTimeDelayTemp){
    powerOutput = brewTimeDelayTemp;
  }
  else if (powerOutput < brewTimeDelayTemp/brewTimeDelayDivider) {
    powerOutput = brewTimeDelayTemp/brewTimeDelayDivider;
  }

  // Applying the powerOutput variable as part of the relay logic
  if (currentTempReadValue < float(setPoint-10) && !(currentTempReadValue < 0) && currentTempReadValue != NAN) {
    digitalWrite(relayPin, HIGH);
  }
  else if (currentTempReadValue >= float(setPoint-10) && currentTempReadValue < float(setPoint-3)) {
    digitalWrite(relayPin, HIGH);
    delay(powerOutput);
    digitalWrite(relayPin, LOW);
  }
  else if (currentTempReadValue >= float(setPoint-3) && currentTempReadValue <= float(setPoint-1)) {
    digitalWrite(relayPin, HIGH);
    delay(powerOutput);
    digitalWrite(relayPin, LOW);
    delay(powerOutput);
  }
  else if (currentTempReadValue >= float(setPoint-1) && currentTempReadValue < float(setPoint-0.2)) {
    digitalWrite(relayPin, HIGH);
    delay(powerOutput);
    digitalWrite(relayPin, LOW);
    delay(powerOutput);
  }
  else {
    digitalWrite(relayPin, LOW);
  }
}

void updateLCD() {
  // Declaring the local vars to keep the temperature and boiler_power values
  String waterTempPrint;
  int hPwrPrint = 0;
  int setPoint = 0;
  int offsetTemp = 0;
  int brewTimeDelayTemp = 0;
  int brewTimeDelayDivider = 0;

  // Making sure the serial communication finishes sending all the values
  setPoint = myNex.readNumber("page1.n0.val");
  if (setPoint != 777777) {
    delay(1);
  }
  else if (setPoint == 777777) {
    setPoint = myNex.readNumber("page1.n0.val");
  }

  offsetTemp = myNex.readNumber("page1.n1.val");
  if (offsetTemp != 777777) {
    delay(1);
  }
  else if (offsetTemp == 777777) {
    offsetTemp = myNex.readNumber("page1.n1.val");
  }

  brewTimeDelayTemp = myNex.readNumber("page2.n0.val");
  if (brewTimeDelayTemp != 777777) {
    delay(1);
  }
  else if (brewTimeDelayTemp == 777777) {
    brewTimeDelayTemp = myNex.readNumber("page2.n0.val");
  }

  brewTimeDelayDivider = myNex.readNumber("page2.n1.val");
  if (brewTimeDelayDivider != 777777) {
    delay(1);
  }
  else if (brewTimeDelayDivider == 777777) {
    brewTimeDelayDivider = myNex.readNumber("page2.n1.val");
  }
  
  // Calculating the boiler heating power to apply
  int powerOutput = map(currentTempReadValue, setPoint-10, setPoint, brewTimeDelayTemp, brewTimeDelayTemp/brewTimeDelayDivider);
  if (currentTempReadValue < STEAM_START) {
    if (powerOutput > brewTimeDelayTemp) {
      powerOutput = brewTimeDelayTemp;
    }
    else if (powerOutput < brewTimeDelayTemp/brewTimeDelayDivider) {
      powerOutput = brewTimeDelayTemp/brewTimeDelayDivider;
    }
    else {
      myNex.writeNum("page0.n0.val", powerOutput);
      delay(10); //Small delay to get full serial
    }
    
    myNex.writeNum("page0.n0.val", powerOutput);
    delay(10); //Small delay to get full serial
  }
  else if (currentTempReadValue > float(setPoint+10)) {
    // for( uint32_t tStart = millis();  (millis()-tStart) < 100;  ){
    if (millis() - timer >= interval) {
      timer += interval;
      myNex.writeStr("popupMSG.t0.txt", "STEAMING!");
      myNex.writeStr("popupMSG.t0.pco=RED");
      myNex.writeStr("page popupMSG");
    }
    // for( uint32_t tStart = millis();  (millis()-tStart) < 1000;  ){ 
    // }
  }
  else {
      powerOutput = NAN;
      myNex.writeNum("page0.n0.val", powerOutput);
      delay(10); //Small delay to get full serial
  }
  //Printing the current values to the display
  waterTempPrint = String(currentTempReadValue-offsetTemp, 2);
  myNex.writeStr("page0.t0.txt", waterTempPrint);
  delay(10); //Small delay to get full serial
}
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

// Save the desired temp values to EEPROM
void trigger1()
{
  int savedBoilerTemp = 0;
  int savedOffsetTemp = 0;
  int savedBrewTimeDelay = 0;
  int savedbrewTimeDivider = 0;

  savedBoilerTemp = myNex.readNumber("page1.n0.val");
  if(savedBoilerTemp != 777777){       // 777777is the return value if the code fails to read the new value
    writeIntIntoEEPROM(EEP_ADDR1, savedBoilerTemp);
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
  } 
  else if(savedBoilerTemp == 777777){
    savedBoilerTemp = myNex.readNumber("page1.n0.val");
    delay(5); //Small delay to get full serial
    writeIntIntoEEPROM(EEP_ADDR1, savedBoilerTemp);
    delay(5); //Small delay to get full serial
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
  }


  savedOffsetTemp = myNex.readNumber("page1.n1.val");
  if(savedOffsetTemp != 777777){       // 777777is the return value if the code fails to read the new value
    writeIntIntoEEPROM(EEP_ADDR2, savedOffsetTemp);
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
  } 
  else if(savedOffsetTemp == 777777){
    savedOffsetTemp = myNex.readNumber("page1.n1.val");
    delay(5); //Small delay to get full serial
    writeIntIntoEEPROM(EEP_ADDR2, savedOffsetTemp);
    delay(5); //Small delay to get full serial
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
  }


  savedBrewTimeDelay = myNex.readNumber("page2.n0.val");
  if(savedBrewTimeDelay != 777777){       // 777777is the return value if the code fails to read the new value
    writeIntIntoEEPROM(EEP_ADDR3, savedBrewTimeDelay);
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
  } 
  else if(savedBrewTimeDelay == 777777){
    savedBrewTimeDelay = myNex.readNumber("page2.n0.val");
    delay(5); //Small delay to get full serial
    writeIntIntoEEPROM(EEP_ADDR3, savedBrewTimeDelay);
    delay(5); //Small delay to get full serial
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
  }


  savedbrewTimeDivider = myNex.readNumber("page2.n1.val");
  if(savedbrewTimeDivider != 777777){       // 777777is the return value if the code fails to read the new value
    writeIntIntoEEPROM(EEP_ADDR4, savedbrewTimeDivider);
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
  } 
  else if(savedbrewTimeDivider == 777777){
    savedbrewTimeDivider = myNex.readNumber("page2.n1.val");
    delay(5); //Small delay to get full serial
    writeIntIntoEEPROM(EEP_ADDR4, savedbrewTimeDivider);
    delay(5); //Small delay to get full serial
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("popupMSG.t0.pco=BLACK");
    myNex.writeStr("page popupMSG");
  }
}
