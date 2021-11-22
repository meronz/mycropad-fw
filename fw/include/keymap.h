#ifndef KEYMAP_H
#define KEYMAP_H

#include "tusb.h"

#define KEYMAP_KEYCODE(x)    ((uint8_t)(x       & 0xFF))
#define KEYMAP_MODIFIER(x)   ((uint8_t)(x >> 8  & 0xFF))
#define KEYMAP_RESERVED(x)   ((uint8_t)(x >> 16 & 0xFF))
#define KEYMAP_RESERVEDD(x)  ((uint8_t)(x >> 24 & 0xFF))

#define KEYMAP_ENTRY(mod, code)   ((uint32_t)(\
    ((mod & 0xFF) << 8) | \
    (code & 0xFF)\
    ))

typedef uint32_t keycode_t;

class Keymap
{
public:
    static const int MaxKeyNum = 11;

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

    enum MessageTypes
    {
        NewKeymap = 1
    };

    enum MappingTypes
    {
        Macro,
        Delay
    };

    static Keymap* Instance()
    {
        static Keymap *instance = nullptr;
        if(instance == nullptr) {
            instance = new Keymap();
        }
        return instance;
    }

    keycode_t* GetKeys(Keymap::Keys key);
    bool SetKeymap(uint8_t const *new_keymap_cmd_json, uint16_t len);
    void Save();
    void Load();
    void LoadDefault();

private:
    Keymap();
    keycode_t* _keymap[MaxKeyNum];
    keycode_t _default_keymap[MaxKeyNum][2] = {
        /* Key1 */       { 1 + 1, KEYMAP_ENTRY(KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_X) },
        /* Key2 */       { 1 + 1, KEYMAP_ENTRY(KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_C) },
        /* Key3 */       { 1 + 1, KEYMAP_ENTRY(KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_V) },
        /* Key4 */       { 1 + 1, KEYMAP_ENTRY(KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_D) },
        /* Key5 */       { 1 + 1, KEYMAP_ENTRY(KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_E) },
        /* Key6 */       { 1 + 1, KEYMAP_ENTRY(KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_F) },
        /* Key7 */       { 1 + 1, KEYMAP_ENTRY(KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_G) },
        /* Key8 */       { 1 + 1, KEYMAP_ENTRY(KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_H) },
        /* RotCW */      { 1 + 1, KEYMAP_ENTRY(KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_Y) },
        /* RotCCW */     { 1 + 1, KEYMAP_ENTRY(KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_Z) },
        /* RotClick */   { 1 + 1, KEYMAP_ENTRY(KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_X) },
    };
};

#endif