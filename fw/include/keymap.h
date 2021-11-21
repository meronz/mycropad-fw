#ifndef KEYMAP_H
#define KEYMAP_H

#include "tusb.h"

class Keymap
{
public:

    enum Keys
    {
        None = 0,
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
    };
    static const int MaxKeyNum = 11;

    Keymap();
    uint8_t* GetKeys(Keymap::Keys key);
    bool SetKeymap(uint8_t const *new_keymap_cmd_json, uint16_t len);



private:
    uint8_t* _keymap[MaxKeyNum];
    uint8_t _default_keymap[MaxKeyNum][13] = {
        { 2,  HID_KEY_A, HID_KEY_A },
        { 3,  HID_KEY_B, HID_KEY_B, HID_KEY_B },
        { 4,  HID_KEY_C, HID_KEY_C, HID_KEY_C, HID_KEY_C },
        { 5,  HID_KEY_D, HID_KEY_D, HID_KEY_D, HID_KEY_D, HID_KEY_D  },
        { 6,  HID_KEY_E, HID_KEY_E, HID_KEY_E, HID_KEY_E, HID_KEY_E, HID_KEY_E },
        { 7,  HID_KEY_F, HID_KEY_F, HID_KEY_F, HID_KEY_F, HID_KEY_F, HID_KEY_F, HID_KEY_F },
        { 8,  HID_KEY_G, HID_KEY_G, HID_KEY_G, HID_KEY_G, HID_KEY_G, HID_KEY_G, HID_KEY_G, HID_KEY_G },
        { 9,  HID_KEY_H, HID_KEY_H, HID_KEY_H, HID_KEY_H, HID_KEY_H, HID_KEY_H, HID_KEY_H, HID_KEY_H, HID_KEY_H },
        { 10, HID_KEY_I, HID_KEY_I, HID_KEY_I, HID_KEY_I, HID_KEY_I, HID_KEY_I, HID_KEY_I, HID_KEY_I, HID_KEY_I, HID_KEY_I },
        { 11, HID_KEY_L, HID_KEY_L, HID_KEY_L, HID_KEY_L, HID_KEY_L, HID_KEY_L, HID_KEY_L, HID_KEY_L, HID_KEY_L, HID_KEY_L, HID_KEY_L },
        { 12, HID_KEY_M, HID_KEY_M, HID_KEY_M, HID_KEY_M, HID_KEY_M, HID_KEY_M, HID_KEY_M, HID_KEY_M, HID_KEY_M, HID_KEY_M, HID_KEY_M, HID_KEY_M }
    };
};

#endif