#ifndef LEDS_H
#define LEDS_H

extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "misc.h"
}

class Leds
{
private:
    int _gpio;
    bool _isRGBW;
    int _numLeds;
    uint _t;
    uint _currentPattern;
    uint _currentDirection;
    const uint _refresh_interval_ms = 100;

public:
    Leds(int gpio, bool isRGBW, int numLeds);
    void Tick();
};

static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) |
           ((uint32_t)(g) << 16) |
           (uint32_t)(b);
}

void pattern_snakes(uint len, uint t)
{
    for (uint i = 0; i < len; ++i)
    {
        uint x = (i + (t >> 1)) % 64;
        if (x < 10)
            put_pixel(urgb_u32(0xff, 0, 0));
        else if (x >= 15 && x < 25)
            put_pixel(urgb_u32(0, 0xff, 0));
        else if (x >= 30 && x < 40)
            put_pixel(urgb_u32(0, 0, 0xff));
        else
            put_pixel(0);
    }
}

void pattern_random(uint len, uint t)
{
    if (t % 8)
        return;
    for (uint i = 0; i < len; ++i)
        put_pixel(rand());
}

void pattern_sparkle(uint len, uint t)
{
    if (t % 8)
        return;
    for (uint i = 0; i < len; ++i)
        put_pixel(rand() % 16 ? 0 : 0xffffffff);
}

void pattern_greys(uint len, uint t)
{
    uint max = 60; // let's not draw too much current!
    t %= max;
    for (uint i = 0; i < len; ++i)
    {
        put_pixel(t * 0x10101);
        if (++t >= max)
            t = 0;
    }
}

void pattern_rainbow(uint len, uint t)
{
    // Strip ID: 0 - Effect: Rainbow - LEDS: 8
    // Steps: 38 - Delay: 50
    // Colors: 3 (255.0.0, 0.255.0, 0.0.255)
    // Options: rainbowlen=24, toLeft=false,
    static uint8_t effStep = 0;
    const uint8_t steps = 39;

    float factor1, factor2;
    uint16_t ind;
    const float magicNum1 = 4.875;
    const float magicNum2 = 13;
    const float magicNum3 = 26;
    for (uint16_t j = 0; j < len; j++)
    {
        ind = steps - (uint16_t)(effStep - j * magicNum1) % steps;
        switch ((int)((ind % steps) / magicNum2))
        {
        case 0:
            factor1 = 1.0 - ((float)(ind % steps - 0 * magicNum2) / magicNum2);
            factor2 = (float)((int)(ind - 0) % steps) / magicNum2;
            put_pixel(urgb_u32(255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2));
            break;
        case 1:
            factor1 = 1.0 - ((float)(ind % steps - 1 * magicNum2) / magicNum2);
            factor2 = (float)((int)(ind - magicNum2) % steps) / magicNum2;
            put_pixel(urgb_u32(0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2));
            break;
        case 2:
            factor1 = 1.0 - ((float)(ind % steps - 2 * magicNum2) / magicNum2);
            factor2 = (float)((int)(ind - magicNum3) % steps) / magicNum2;
            put_pixel(urgb_u32(0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2));
            break;
        }
    }

    if (effStep >= steps)
    {
        effStep = 0;
    }
    else
    {
        effStep++;
    }
}

typedef void (*pattern)(uint len, uint t);
const struct
{
    pattern pat;
    const char *name;
} pattern_table[] = {
    {pattern_snakes, "Snakes!"},
    {pattern_random, "Random data"},
    {pattern_sparkle, "Sparkles"},
    {pattern_greys, "Greys"},
    {pattern_rainbow, "Rainbow"}};

Leds::Leds(int gpio, bool isRGBW, int numLeds)
{
    _gpio = gpio;
    _isRGBW = isRGBW;
    _numLeds = numLeds;
    _t = 0;
    _currentPattern = 4;
    _currentDirection = 1;

    // init PIO application
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, _gpio, 800000, _isRGBW);
}

void Leds::Tick()
{
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < _refresh_interval_ms)
        return; // not enough time

    start_ms += _refresh_interval_ms;
    pattern_table[_currentPattern].pat(_numLeds, _t);
    _t += 1;
}

#endif //LEDS_H