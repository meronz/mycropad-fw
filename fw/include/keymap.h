#ifndef KEYMAP_H
#define KEYMAP_H

#include "tusb.h"

class Keymap
{
public:
    Keymap();

    static const int MaxKeyNum = 11;

    int GetKey(int key);
    bool SetKeymap(uint8_t const *new_keymap_cmd_json, uint16_t len);

private:
    uint8_t _keymap[MaxKeyNum];
    uint8_t _default_keymap[MaxKeyNum] = {
        HID_KEY_A,
        HID_KEY_B,
        HID_KEY_C,
        HID_KEY_D,
        HID_KEY_E,
        HID_KEY_F,
        HID_KEY_G,
        HID_KEY_H,
        HID_KEY_I,
        HID_KEY_L,
        HID_KEY_M,
    };
};

#endif