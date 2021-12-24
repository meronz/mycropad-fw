#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"

#include "consts.h"
#include "keymap.h"
#include "leds.h"
#include "gpio.h"

#define ITF_KEYBOARD 0

void hid_task()
{
  // Poll every X ms
  const uint32_t interval_ms = 5;
  static uint32_t start_ms = 0;
  static bool is_repeating = false;
  static ulong lastEventMs = 0;
  static Keymap::Keys oldEvent = Keymap::Keys::None;

  if (board_millis() - start_ms < interval_ms)
    return; // not enough time
  start_ms += interval_ms;

  Keymap::Keys keyEvent = Gpio::Instance()->GetKeyEvent();

  // Remote wakeup
  if (tud_suspended() && keyEvent)
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }

  if (!tud_hid_n_ready(ITF_KEYBOARD))
  {
    return;
  }


  // Key repeat
  // Rotary encoder keys are not subject to repeat-rate
  uint msSinceLastPress = board_millis() - lastEventMs;
  if (msSinceLastPress > (is_repeating ? KEY_REPEAT_RATE_MS : KEY_REPEAT_RATE_MS) ||
      keyEvent == Keymap::Keys::RotCCW ||
      keyEvent == Keymap::Keys::RotCW)
  {
    oldEvent = Keymap::Keys::None;
    is_repeating = true;
  }
  else
  {
    is_repeating = false;
  }

  // use to avoid send multiple consecutive zero report for keyboard
  static bool hasKey = false;
  static keycode_t *kcArray = nullptr;
  static uint8_t kcLen = 0;
  static uint8_t kcIndex = 0;

  // sending a keycode sequence
  uint8_t kc = 0;
  uint8_t kmod = 0;
  // send empty key report if previously has key pressed
  if (hasKey)
  {
    tud_hid_n_keyboard_report(ITF_KEYBOARD, 0, 0, NULL);
    hasKey = false;
    return;
  }

  if (kcArray != nullptr && kcLen >= kcIndex)
  {
    printf("Next %d/%d\n", kcIndex, kcLen);
    kc = KEYMAP_KEYCODE(kcArray[kcIndex]);
    kmod = KEYMAP_MODIFIER(kcArray[kcIndex]);
    kcIndex++;
  }
  else if (keyEvent != Keymap::Keys::None && keyEvent != oldEvent)
  {
    oldEvent = keyEvent;
    lastEventMs = board_millis();
    printf("event: %d\n", (int)keyEvent);
    kcArray = Keymap::Instance()->GetKeys(keyEvent);
    kcLen = kcArray[0] - 1;
    kcIndex = 1;

    if (kcArray == nullptr)
    {
      printf("keycodes ptr: %p\n", kcArray);
      kcIndex = 0;
      kcLen = 0;
      return;
    }

    printf("Key %d/%d\n", kcIndex, kcLen);
    kc = KEYMAP_KEYCODE(kcArray[kcIndex]);
    kmod = KEYMAP_MODIFIER(kcArray[kcIndex]);
    kcIndex++;
  }

  if (kc)
  {
    printf("KC %x, mod %x\n", kc, kmod);
    uint8_t key_input[6] = {kc, 0, 0, 0, 0, 0};
    tud_hid_n_keyboard_report(ITF_KEYBOARD, 0, kmod, key_input);
    hasKey = true;
  }
  else
  {
    kcArray = nullptr;
    kcIndex = 0;
    kcLen = 0;
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void)itf;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
  // TODO set LED based on CAPLOCK, NUMLOCK etc...
  (void)itf;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)bufsize;
}
