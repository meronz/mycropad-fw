#ifndef LEDS_H
#define LEDS_H
class Leds
{
private:
    Leds();
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
    void Tick();
};

#endif //LEDS_H