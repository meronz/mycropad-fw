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

keycode_t *Keymap::GetKeys(Keys key)
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
        if (len == 0 || len > Keymap::MaxKeycodesNum)
        {
            printf("Len 0");
            return false;
        }

        keycode_t *keymap = _keymap[keyIndex];
        keymap[0] = len;

        size_t size = len * sizeof(keycode_t);
        memcpy(keymap, newKeymap + readOffset, size);
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
// | 4 bytes  | 4 bytes  |  data for the whole keymap size (number of keycodes * keycode size)
//
// Lenght is the number of words stored

void Keymap::Load()
{
    unsigned short off = 0;

    // XIP Flash can be read from memory,
    // lets start reading the crc and keymap lenght
    volatile uint32_t crc = *((uint32_t *)KEYMAP_FLASH_ADDR);
    volatile uint32_t keymapLength = *(uint32_t *)(KEYMAP_FLASH_ADDR + sizeof(uint32_t));

    if (keymapLength > (MaxKeycodesNum * MaxKeyNum))
    {
        printf("Insane size %d\n", keymapLength);
        LoadDefault();
        return;
    }

    // Do not include CRC
    off += sizeof(crc);
    uint32_t calculatedCrc = crc32(KEYMAP_FLASH_ADDR + off, sizeof(keymapLength) + sizeof(_keymap));
    if (crc != calculatedCrc)
    {
        printf("Wrong CRC %x != %x\n", crc, calculatedCrc);
        LoadDefault();
        return;
    }

    off += sizeof(keymapLength);
    memcpy(_keymap, KEYMAP_FLASH_ADDR + off, sizeof(_keymap));
    printf("Loaded keymap len %u\n", keymapLength);
}

void Keymap::LoadDefault()
{
    for (size_t i = 0; i < MaxKeyNum; i++)
    {
        uint8_t len = _default_keymap[i][0];
        memset(_keymap[i], 0, sizeof(_keymap[i]));
        memcpy(_keymap[i], _default_keymap[i], len * sizeof(keycode_t));
    }

    printf("Loaded default keymap\n");
    Save();
}

void Keymap::Save()
{
    uint32_t crc = 0;
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
    size_t dataSize = sizeof(crc) + sizeof(keymapLength) + sizeof(_keymap);
    size_t flashDataSize = NEXT_MULTIPLE(dataSize, FLASH_PAGE_SIZE);
    uint32_t *tmpBuf = (uint32_t *)malloc(flashDataSize);
    memset(tmpBuf, 0, flashDataSize);

    // write keymap length, will be included in CRC
    tmpBuf[1] = keymapLength;

    // start writing keymap data after crc + len
    memcpy(tmpBuf + 2, _keymap, sizeof(_keymap));

    // calculate crc
    crc = crc32(tmpBuf + 1, dataSize - sizeof(crc));
    tmpBuf[0] = crc;

    printf("Saving keymap: bytes %u (crc %x)\n", dataSize, crc);
    uint32_t ints = save_and_disable_interrupts();
    {
        // flash_range_erase count must be a multiple of FLASH_SECTOR_SIZE
        flash_range_erase(KEYMAP_FLASH_OFFSET, NEXT_MULTIPLE(flashDataSize, FLASH_SECTOR_SIZE));
        // flash_range_program count must be a multiple of FLASH_PAGE_SIZE
        flash_range_program(KEYMAP_FLASH_OFFSET, (const uint8_t *)tmpBuf, flashDataSize);
    }
    restore_interrupts(ints);

    printf("Saved keymap\n");
    free(tmpBuf);
}