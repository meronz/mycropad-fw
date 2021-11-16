#ifndef ENUMS_H
#define ENUMS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define GPIO_SW_1 (1)
#define GPIO_SW_2 (2)
#define GPIO_SW_3 (3)
#define GPIO_SW_4 (4)
#define GPIO_SW_5 (5)
#define GPIO_SW_6 (6)
#define GPIO_SW_7 (7)
#define GPIO_SW_8 (8)
#define GPIO_ROT_CLK (9)
#define GPIO_ROT_DATA (10)
#define GPIO_ROT_SW (11)
#define GPIO_LEDS (28)

    typedef enum Keys
    {
        Key1,
        Key2,
        Key3,
        Key4,
        Key5,
        Key6,
        Key7,
        Key8,
        RotCW,
        RotCCW,
        RotClick,
    } Keys;

    typedef enum MessageTypes
    {
        NewKeymap = 1
    } MessageTypes;

    typedef enum MappingTypes
    {
        Macro,
        Delay
    } MappingTypes;

#ifdef __cplusplus
}
#endif

#endif