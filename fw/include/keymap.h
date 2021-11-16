#include <cstdint>
#include <cstring>
#include "tusb.h"
#include <ArduinoJson.h>

class Keymap
{
public:
    Keymap();

    static const int MaxKeyNum = 11;

    const int GetKey(int key);
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

Keymap::Keymap()
{
    //TODO: load from flash and check CRC
    memcpy(_keymap, _default_keymap, sizeof(_keymap));
}

const int Keymap::GetKey(int key)
{
    if (key > Keymap::MaxKeyNum)
    {
        return -1;
    }

    return _keymap[key];
}

bool Keymap::SetKeymap(uint8_t const *new_keymap_cmd_json, uint16_t len)
{
    if (new_keymap_cmd_json == nullptr || len != sizeof(_keymap))
    {
        return false;
    }

    // DynamicJsonDocument doc(1024);
    // DeserializationError error = deserializeJson(doc, (char *)new_keymap_cmd_json); //the cast is a bad idea
    // if (error)
    // {
    //     printf(error.c_str());
    //     return false;
    // }

    // for (int keyNumber = 0; keyNumber < MaxKeyNum; keyNumber++)
    // {
    //     if (_keymap[keyNumber])
    //     {
    //         delete _keymap[keyNumber];
    //     }

    //     int mappingType = doc[keyNumber]["Type"];
    //     switch (mappingType)
    //     {
    //     case MappingTypes::Macro:
    //     {
    //         int len = doc[keyNumber]["Length"];
    //         if (len < 0)
    //             continue;

    //         _keymap[keyNumber] = new uint8_t[len + 1];
    //         JsonArray keycodes = doc[keyNumber]["Keycodes"];
    //         for (int kIndex = 0; kIndex < len; kIndex++)
    //         {
    //             _keymap[keyNumber][kIndex] = keycodes[kIndex];
    //         }
    //         _keymap[keyNumber][len] = 0; // end the array

    //         break;
    //     }
    //     case MappingTypes::Delay:
    //         break;
    //     default:
    //         break;
    //     }
    // }

    // //TODO: ADD CRC
    // //TODO: write to flash
    return true;
}