extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hardware/pio.h"
#include "ws2812.pio.h"
}

#include "leds.h"
#include "consts.h"
#include "misc.h"


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

static volatile bool fixed_map_changed = true;
static uint32_t fixed_map[LEDS_NUM];

void pattern_fixed(uint len)
{
    if(fixed_map_changed)
    {
        for (size_t i = 0; i < len; i++)
        {
            put_pixel(fixed_map[i]);
        }
        fixed_map_changed = false;
    }
}

void pattern_rainbow(uint len)
{
    static uint8_t step = 0;
    static float r = 0;
    static float g = 0;
    static float b = 0;
    static const float speed = 1.0;
    uint8_t maxBrightness = 128; // max 255

    switch(step)
    {
        case 0:
            if(b > 0) b -= speed;
            r >= 255 - speed ? step++ : r += speed;
            break;
        case 1:
            if(r > 0) r -= speed;
            g >= 255 - speed ? step++ : g += speed;
            break;
        case 2:
            if(g > 0) g -= speed;
            b >= 255 - speed ? step = 0 : b += speed;
            break;
    }

    for (uint i = 0; i < len; i++)
    {
        put_pixel(urgb_u32(
            (int)((r * maxBrightness) / 0xFF) & 0xFF,
            (int)((g * maxBrightness) / 0xFF) & 0xFF,
            (int)((b * maxBrightness) / 0xFF) & 0xFF));
    }
}

#define PATTERNS_NUM (2)
typedef void (*pattern)(uint len);
pattern patterns[PATTERNS_NUM] = { pattern_fixed, pattern_rainbow };

void Leds::Init()
{
    _gpio = GPIO_LEDS;
    _isRGBW = LEDS_RGBW;
    _numLeds = LEDS_NUM;
    _currentPattern = 1;

    memset(fixed_map, 0, sizeof(fixed_map));

    // init PIO application
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, _gpio, 800000, _isRGBW);
}

void Leds::Tick()
{
    static uint32_t start_ms = 0;
    if (millis() - start_ms < _interval_ms) return;
    start_ms += _interval_ms;

    patterns[_currentPattern](_numLeds);
}

bool Leds::SwitchPattern(int pattern)
{
    if(pattern < 0 || pattern > PATTERNS_NUM)
    {
        return false;
    }

    _currentPattern = pattern;
    fixed_map_changed = true;
    return true;
}

bool Leds::SetFixedMap(uint8_t* map)
{
    if(map == nullptr)
    {
        return false;
    }

    memcpy(fixed_map, map, sizeof(fixed_map));
    fixed_map_changed = true;
    return true;
}