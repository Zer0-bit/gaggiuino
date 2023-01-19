class ADS1115
{
public:
  ADS1115(int);
  void begin();
  void setGain(int);
  float getValue();
  int getError();
  void readADC(int);
  void setMode(int);
  void setDataRate(int);
  bool isReady(void);
  bool isConnected(void);
};
