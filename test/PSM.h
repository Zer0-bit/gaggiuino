class PSM
{
public:
  PSM(unsigned char sensePin, unsigned char controlPin, unsigned int range, int mode);

  void set(unsigned int value);

  long getCounter();
  void resetCounter();

  void stopAfter(long counter);
};
