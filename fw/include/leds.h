#ifndef LEDS_H
#define LEDS_H

class Leds
{
private:
    Leds() { }
    int _gpio;
    bool _isRGBW;
    int _numLeds;
    unsigned int _currentPattern;
    const uint _interval_ms = 50;

public:
    static Leds *Instance()
    {
        static Leds *instance = nullptr;
        if (instance == nullptr)
        {
            instance = new Leds();
        }
        return instance;
    }

    void Init();
    void Tick();
    bool SwitchPattern(int pattern);
    bool SetFixedMap(uint8_t *map);
};

#endif //LEDS_H