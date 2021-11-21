#ifndef GPIO_H
#define GPIO_H

#include "pico/time.h"
#include "pico/stdlib.h"
#include "bsp/board.h"
#include "rotary_encoder.h"
#include "enums.h"
#include "keymap.h"

#define CHECK_MSEC    5         // Read hardware every 5 msec
#define PRESS_MSEC    10        // Stable time before registering pressed
#define RELEASE_MSEC  100       // Stable time before registering released 

class Gpio
{
private:
    Gpio();
    ~Gpio();

    uint32_t _key_events;
    RotaryEncoder* _encoder;
    static const int GpioNum = 11;
    const uint8_t _gpios[GpioNum] = {
        GPIO_SW_1,
        GPIO_SW_2,
        GPIO_SW_3,
        GPIO_SW_4,
        GPIO_SW_5,
        GPIO_SW_6,
        GPIO_SW_7,
        GPIO_SW_8,
        GPIO_ROT_SW,
        GPIO_ROT_CLK,
        GPIO_ROT_DATA,
    };

    const uint32_t _debounceTimes[GpioNum] = { 0 };
public:
    static Gpio *Instance()
    {
        static Gpio *instance = nullptr;
        if(instance == nullptr) { instance = new Gpio(); }
        return instance;
    }
    void Tick();
    void RotaryEncoderTick();
    Keymap::Keys GetKeyEvent();
    Keymap::Keys GpioToKey(uint gpio);
};

#endif