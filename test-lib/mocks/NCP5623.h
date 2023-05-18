#ifndef NCP5623_H
#define NCP5623_H
#include <stdint.h> // for uint8_t

class NCP5623
{
  public:
    NCP5623();
    void begin();
    void setColor(uint8_t red, uint8_t green, uint8_t blue);
};

#endif
