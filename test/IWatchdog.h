class IWatchdogClass {

  public:
    void begin(uint32_t timeout, uint32_t window);
    void set(uint32_t timeout, uint32_t window);
    void get(uint32_t *timeout, uint32_t *window = NULL);
    void reload(void);
    bool isEnabled(void)
    {
      return _enabled;
    };
    bool isReset(bool clear = false);
    void clearReset(void);

  private:
    static bool _enabled;
};

extern IWatchdogClass IWatchdog;
