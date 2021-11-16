#include <cstdint>
#include <cstring>
#include "tusb.h"
#include <ArduinoJson.h>

class Keymap
{
public:
    Keymap();

    static const int MaxKeyNum = 11;

    const uint8_t *GetKey(int key);
    bool SetKeymap(uint8_t const *new_keymap_cmd_json, uint16_t len);

private:
    uint8_t *_keymap[MaxKeyNum];
    uint8_t _default_keymap[MaxKeyNum][6] = {
        {HID_KEY_A, 0, 0, 0, 0, 0},
        {HID_KEY_B, 0, 0, 0, 0, 0},
        {HID_KEY_C, 0, 0, 0, 0, 0},
        {HID_KEY_D, 0, 0, 0, 0, 0},
        {HID_KEY_E, 0, 0, 0, 0, 0},
        {HID_KEY_F, 0, 0, 0, 0, 0},
        {HID_KEY_G, 0, 0, 0, 0, 0},
        {HID_KEY_H, 0, 0, 0, 0, 0},
        {HID_KEY_I, 0, 0, 0, 0, 0},
        {HID_KEY_L, 0, 0, 0, 0, 0},
        {HID_KEY_M, 0, 0, 0, 0, 0}};
};

Keymap::Keymap()
{
    //TODO: load from flash and check CRC
    memcpy(_keymap, _default_keymap, sizeof(_keymap));
}

const uint8_t *Keymap::GetKey(int key)
{
    if (key > Keymap::MaxKeyNum)
    {
        return nullptr;
    }

    return (const uint8_t *)_keymap[key];
}

bool Keymap::SetKeymap(uint8_t const *new_keymap_cmd_json, uint16_t len)
{
    if (new_keymap_cmd_json == nullptr || len != sizeof(_keymap))
    {
        return false;
    }

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, (char *)new_keymap_cmd_json); //the cast is a bad idea
    if (error)
    {
        printf(error.c_str());
        return false;
    }

    for (int keyNumber = 0; keyNumber < MaxKeyNum; keyNumber++)
    {
        if (_keymap[keyNumber])
        {
            delete _keymap[keyNumber];
        }

        int mappingType = doc[keyNumber]["Type"];
        switch (mappingType)
        {
        case MappingTypes::Macro:
        {
            int len = doc[keyNumber]["Length"];
            if (len < 0)
                continue;

            _keymap[keyNumber] = new uint8_t[len + 1];
            JsonArray keycodes = doc[keyNumber]["Keycodes"];
            for (int kIndex = 0; kIndex < len; kIndex++)
            {
                _keymap[keyNumber][kIndex] = keycodes[kIndex];
            }
            _keymap[keyNumber][len] = 0; // end the array

            break;
        }
        case MappingTypes::Delay:
            break;
        default:
            break;
        }
    }

    //TODO: ADD CRC
    //TODO: write to flash
    return true;
}