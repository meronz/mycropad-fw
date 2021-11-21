#include "keymap.h"

#include <cstdint>
#include <cstring>
#include <ArduinoJson.h>
#include "consts.h"

Keymap::Keymap()
{
    //TODO: load from flash and check CRC
    for (size_t i = 0; i < MaxKeyNum; i++)
    {
        uint8_t size = _default_keymap[i][0] + 1;
        _keymap[i] = new uint8_t[size];
        memcpy(_keymap[i], _default_keymap[i], size);
    }
}

uint8_t* Keymap::GetKeys(Keys key)
{
    if ((int)key <= 0 || (int)key > Keymap::MaxKeyNum)
    {
        return nullptr;
    }

    return _keymap[key-1];
}

bool Keymap::SetKeymap(uint8_t const *new_keymap_cmd_json, uint16_t len)
{
    if (new_keymap_cmd_json == nullptr || len != sizeof(_keymap)) // fix sizeof
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