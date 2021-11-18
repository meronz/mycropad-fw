#include "keymap.h"

#include <cstdint>
#include <cstring>
#include <ArduinoJson.h>

Keymap::Keymap()
{
    //TODO: load from flash and check CRC
    memcpy(_keymap, _default_keymap, sizeof(_keymap));
}

int Keymap::GetKey(int key)
{
    if (key<=0 || key > Keymap::MaxKeyNum)
    {
        return -1;
    }

    return _keymap[key-1];
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