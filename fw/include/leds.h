#ifndef LEDS_H
#define LEDS_H

extern "C" {
    #include <stdio.h>
    #include <stdlib.h>
    #include "hardware/pio.h"
    #include "hardware/clocks.h"
    #include "ws2812.pio.h"
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


static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

void pattern_snakes(uint len, uint t) {
    for (uint i = 0; i < len; ++i) {
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

void pattern_random(uint len, uint t) {
    if (t % 8)
        return;
    for (uint i = 0; i < len; ++i)
        put_pixel(rand());
}

void pattern_sparkle(uint len, uint t) {
    if (t % 8)
        return;
    for (uint i = 0; i < len; ++i)
        put_pixel(rand() % 16 ? 0 : 0xffffffff);
}

void pattern_greys(uint len, uint t) {
    uint max = 60; // let's not draw too much current!
    t %= max;
    for (uint i = 0; i < len; ++i) {
        put_pixel(t * 0x10101);
        if (++t >= max) t = 0;
    }
}

typedef void (*pattern)(uint len, uint t);
const struct {
    pattern pat;
    const char *name;
} pattern_table[] = {
        {pattern_snakes,  "Snakes!"},
        {pattern_random,  "Random data"},
        {pattern_sparkle, "Sparkles"},
        {pattern_greys,   "Greys"},
};

Leds::Leds(int gpio, bool isRGBW, int numLeds) {
    _gpio = gpio;
    _isRGBW = isRGBW;
    _numLeds = numLeds;
    _t = 0;
    _currentPattern = 1;
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
    static uint pattern_i = 0;

    if ( board_millis() - start_ms < _refresh_interval_ms) return; // not enough time
    start_ms += _refresh_interval_ms;
    
    if(pattern_i < 1000) {
        pattern_table[_currentPattern].pat(_numLeds, _t);
        _t += _currentDirection;
        pattern_i++;
    } else {
        pattern_i = 0;
        _currentDirection = (rand() >> 30) & 1 ? 1 : -1;
        //_currentPattern = rand() % count_of(pattern_table);
    }
}


#endif //LEDS_H