#include "keymap.h"

#include <cstdint>
#include <cstring>
#include <ArduinoJson.h>
#include "consts.h"
#include "misc.h"

#include <hardware/flash.h>

Keymap::Keymap()
{

}

keycode_t* Keymap::GetKeys(Keys key)
{
    if ((int)key <= 0 || (int)key > Keymap::MaxKeyNum)
    {
        return nullptr;
    }

    return _keymap[key - 1];
}

bool Keymap::SetKeymap(uint8_t const *newKeymap)
{
    if (newKeymap == nullptr)
    {
        return false;
    }

    uint32_t readOffset = 0;
    for (int keyIndex = 0; keyIndex < Keymap::MaxKeyNum; keyIndex++)
    {
        uint8_t len = newKeymap[readOffset];
        if(len == 0 || len > Keymap::MaxKeycodesNum)
        {
            printf("Len 0");
            return false;
        }

        keycode_t *keymap = _keymap[keyIndex];
        keymap[0] = len;

        size_t size = len * sizeof(keycode_t);
        memcpy(keymap, newKeymap+readOffset, size);
        printf("Set keymap of size %u\n", len);
        readOffset += size;
    }

    Save();
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

    if (keymapLength > (MaxKeycodesNum * MaxKeyNum))
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
        uint32_t* keycodes = (uint32_t *)(KEYMAP_FLASH_ADDR + off);
        uint8_t len = keycodes[0];
        if(len == 0 || len > Keymap::MaxKeycodesNum)
        {
            printf("Bad keymap[%u] len %u\n", i, len);
            LoadDefault();
            return;
        }
        uint8_t size = len * sizeof(keycode_t);
        memcpy(_keymap[i], keycodes, size);
        off += size;
        printf("Loaded keymap[%u] of len %u\n", i, len);
    }
}

void Keymap::LoadDefault()
{
    for (size_t i = 0; i < MaxKeyNum; i++)
    {
        uint8_t len = _default_keymap[i][0];
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
    delete[] tmpBuf;
}