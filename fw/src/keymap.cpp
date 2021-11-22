#include "keymap.h"

#include <cstdint>
#include <cstring>
#include <ArduinoJson.h>
#include "consts.h"
#include "misc.h"

#include <hardware/flash.h>

Keymap::Keymap()
{
    for (size_t i = 0; i < MaxKeyNum; i++)
    {
        _keymap[i] = nullptr;
    }
}

uint32_t *Keymap::GetKeys(Keys key)
{
    if ((int)key <= 0 || (int)key > Keymap::MaxKeyNum)
    {
        return nullptr;
    }

    return _keymap[key - 1];
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

#define RESERVED_SPACE (16 * 1024)
#define KEYMAP_FLASH_OFFSET ((256 * 1024) - RESERVED_SPACE)
#define KEYMAP_FLASH_ADDR ((const uint8_t *)(XIP_BASE + KEYMAP_FLASH_OFFSET))

#define NEXT_MULTIPLE(n, x) (n % x ? (n + (x - n % x)) : n)


// Keymap is stored in the following format
// |   CRC    |  Length  |  Data
// | 4 bytes  | 4 bytes  |  data for lenght * 4 bytes
// 
// Lenght is the number of words stored

void Keymap::Load()
{
    unsigned short off = 0;

    // XIP Flash can be read from memory,
    // lets start reading the crc and keymap lenght
    volatile uint32_t crc = *((uint32_t*)KEYMAP_FLASH_ADDR);
    volatile uint32_t keymapLength = *(uint32_t*)(KEYMAP_FLASH_ADDR + sizeof(uint32_t));

    if (keymapLength > (255 * MaxKeyNum))
    {
        printf("Insane size %d\n", keymapLength);
        LoadDefault();
        return;
    }

    off += sizeof(crc);
    uint32_t calculatedCrc = crc32(
        KEYMAP_FLASH_ADDR + off,            // Do not include CRC
        sizeof(keymapLength)
        + (keymapLength * sizeof(keycode_t)));
    if (crc != calculatedCrc)
    {
        printf("Wrong CRC %x != %x\n", crc, calculatedCrc);
        LoadDefault();
        return;
    }

    off += sizeof(keymapLength);
    for (size_t i = 0; i < MaxKeyNum; i++)
    {
        if (_keymap[i] != nullptr)
        {
            delete _keymap[i];
        }

        uint32_t* keycodes = (uint32_t *)(KEYMAP_FLASH_ADDR + off);
        volatile uint8_t len = keycodes[0];
        volatile uint8_t size = len * sizeof(keycode_t);
        _keymap[i] = new keycode_t[len];
        memcpy(_keymap[i], KEYMAP_FLASH_ADDR + off, size);
        off += size;
        printf("Loaded keymap[%u] of len %u\n", i, len);
    }
}

void Keymap::LoadDefault()
{
    for (size_t i = 0; i < MaxKeyNum; i++)
    {
        if (_keymap[i] != nullptr)
        {
            delete _keymap[i];
        }
        uint8_t len = _default_keymap[i][0];
        _keymap[i] = new keycode_t[len];
        memcpy(_keymap[i], _default_keymap[i], len * sizeof(keycode_t));
    }

    printf("Loaded default keymap\n");
    Save();
}

void Keymap::Save()
{
    // calculate total keymap length
    uint32_t keymapLength = 0;
    for (size_t i = 0; i < MaxKeyNum; i++)
    {
        keymapLength = keymapLength + _keymap[i][0];
    }

    // allocate a temporary buffer to write all at once
    // flash_range_program will write a multiple of the page size, so
    // we allocate a buffer big enough to store our keymap rounded
    // to the next page.
    uint32_t dataSize = NEXT_MULTIPLE((keymapLength + 2) * sizeof(keycode_t), FLASH_PAGE_SIZE);
    uint8_t *tmpBuf = new uint8_t[dataSize];
    memset(tmpBuf, 0, dataSize);

    // write keymap length, will be included in CRC
    *((uint32_t *)(tmpBuf + sizeof(keymapLength))) = keymapLength;

    // start writing keymap data after crc + len
    uint32_t off = sizeof(uint32_t) + sizeof(keymapLength);
    for (size_t i = 0; i < MaxKeyNum; i++)
    {
        uint8_t size = _keymap[i][0] * sizeof(keycode_t);
        memcpy(((uint8_t *)tmpBuf) + off, (const uint8_t *)_keymap[i], size);
        off += size;
    }

    // calculate crc
    uint32_t crc = crc32(tmpBuf + sizeof(crc), 
        sizeof(keymapLength)
        + (keymapLength * sizeof(keycode_t)));
    *((uint32_t *)tmpBuf) = crc;

    printf("Saving keymap: bytes %u (crc %u)\n", dataSize, crc);
    uint32_t ints = save_and_disable_interrupts();
    {
        // flash_range_erase count must be a multiple of FLASH_SECTOR_SIZE
        flash_range_erase(KEYMAP_FLASH_OFFSET, NEXT_MULTIPLE(dataSize, FLASH_SECTOR_SIZE));
        // flash_range_program count must be a multiple of FLASH_PAGE_SIZE
        flash_range_program(KEYMAP_FLASH_OFFSET, tmpBuf, dataSize);
    }
    restore_interrupts(ints);

    printf("Saved keymap\n");
    delete tmpBuf;
}