#ifndef GPIO_H
#define GPIO_H

#include "pico/stdlib.h"
#include "bsp/board.h"
#include "rotary_encoder.h"
#include "enums.h"

static const uint8_t _gpios[] = {
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
class Gpio
{
private:
    uint32_t _key_events;
    RotaryEncoder* _encoder;

public:
    Gpio();
    ~Gpio();
    void Tick();
    uint32_t GetKeyEvent();
};

#endif