//################################__PSM_BY_VAS__########################################
class PSM {
public:
    PSM(unsigned char sensePin, unsigned char controlPin, unsigned int range, int mode = RISING);

    void set(unsigned int value);
    unsigned int getValue();

    long getCounter();
    void resetCounter();

    void stopAfter(long counter);

    unsigned int cps();

private:
    static inline void onInterrupt();
    void calculateSkip();
    void updateControl();

    unsigned char _sensePin;
    unsigned char _controlPin;
    unsigned int _range;
    volatile unsigned int _value;
    volatile unsigned int _a;
    volatile bool _skip;
    volatile bool _secondZero;
    volatile long _counter;
    volatile long _stopAfter;
};
PSM *_thePSM;

enum OperationalModes {
    straight9Bar,
    justPreinfusion,
    justProfile,
    manual,
    preinfProfile,
    flush,
    descale,
    backflush,
    _empty,
    steam,
    exceptions
};

enum NextionPages {
    home,
    cleanBackflush,
    brewingManual,
    bApre,
    brewSettings,
    cleanDescale,
    moreTemp,
    cleanFlush,
    brewGraph,
    steamGraph,
    splash,
    scales,
    empty,
    bAprofile,
    bAflow,
    bAmore
};