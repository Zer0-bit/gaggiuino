#include "Arduino.h"

class EasyNex {
  public:
    EasyNex(int serial);
    void begin(unsigned long baud = 9600);
    void writeNum(String, uint32_t);
    void writeStr(String, String txt = "cmd");
    void NextionListen(void);
    uint32_t readNumber(String);
    String readStr(String);
    int currentPageId;
    int lastCurrentPageId;
};
