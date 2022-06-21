#ifndef PERIPHERALS_H
#define PERIPHERALS_H

void pinInit(void);

bool brewState(void);
bool steamState(void);

void setBoilerOn(void);
void setBoilerOff(void);

void closeValve(void);
void openValve(void);

#endif
