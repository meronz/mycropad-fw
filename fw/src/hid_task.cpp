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
#include "usb_descriptors.h"

#define ITF_KEYBOARD 0

uint16_t keycodeToConsumerCode(uint8_t keycode)
{
  switch (keycode)
  {
  case HID_KEY_MEDIA_PLAYPAUSE:
    return HID_USAGE_CONSUMER_PLAY_PAUSE;
  case HID_KEY_MEDIA_STOPCD:
    return HID_USAGE_CONSUMER_STOP;
  case HID_KEY_MEDIA_PREVIOUSSONG:
    return HID_USAGE_CONSUMER_SCAN_PREVIOUS;
  case HID_KEY_MEDIA_NEXTSONG:
    return HID_USAGE_CONSUMER_SCAN_NEXT;
  case HID_KEY_MEDIA_VOLUMEUP:
    return HID_USAGE_CONSUMER_VOLUME_INCREMENT;
  case HID_KEY_MEDIA_VOLUMEDOWN:
    return HID_USAGE_CONSUMER_VOLUME_DECREMENT;
  case HID_KEY_MEDIA_MUTE:
    return HID_USAGE_CONSUMER_MUTE;
  case HID_KEY_MEDIA_BACK:
    return HID_USAGE_CONSUMER_AC_BACK;
  case HID_KEY_MEDIA_FORWARD:
    return HID_USAGE_CONSUMER_AC_FORWARD;
  default:
    return 0;
  }
}

void hid_task()
{
  // Poll every X ms
  const uint32_t intervalMs = 5;
  static uint32_t startMs = 0;
  static bool isRepeating = false;
  static ulong lastEventMs = 0;
  static bool hasKeyboardKey = false;
  static bool hasConsumerKey = false;
  static Keymap::Keys oldEvent = Keymap::Keys::None;

  if (board_millis() - startMs < intervalMs)
    return; // not enough time
  startMs += intervalMs;

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

  // send empty key report if previously has key pressed
  if (hasKeyboardKey)
  {
    tud_hid_n_keyboard_report(ITF_KEYBOARD, REPORT_ID_KEYBOARD, 0, NULL);
    hasKeyboardKey = false;
    oldEvent = Keymap::Keys::None;
    return;
  }
  else if (hasConsumerKey)
  {
    uint16_t key = 0;
    tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &key, 2);
    hasConsumerKey = false;
    oldEvent = Keymap::Keys::None;
    return;
  }

  // Key repeat
  // Rotary encoder keys are not subject to repeat-rate
  uint msSinceLastPress = board_millis() - lastEventMs;
  if (msSinceLastPress > (isRepeating ? KEY_REPEAT_DELAY_MS : KEY_REPEAT_DELAY_MS) ||
      keyEvent == Keymap::Keys::RotCCW ||
      keyEvent == Keymap::Keys::RotCW)
  {
    oldEvent = Keymap::Keys::None;
    isRepeating = true;
  }
  else
  {
    isRepeating = false;
  }

  // use to avoid send multiple consecutive zero report for keyboard
  static keycode_t *kcArray = nullptr;
  static uint8_t kcLen = 0;
  static uint8_t kcIndex = 0;

  // sending a keycode sequence
  uint8_t kc = 0;
  uint8_t kmod = 0;

  if (kcArray != nullptr && kcLen >= kcIndex)
  {
    // We are iterating on a kcArray, get the next element.
    printf("next %d/%d\n", kcIndex, kcLen);
    kc = KEYMAP_KEYCODE(kcArray[kcIndex]);
    kmod = KEYMAP_MODIFIER(kcArray[kcIndex]);
    kcIndex++;
  }
  else if (keyEvent != Keymap::Keys::None && keyEvent != oldEvent)
  {
    // Sending an event
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

    printf("key %d/%d\n", kcIndex, kcLen);
    kc = KEYMAP_KEYCODE(kcArray[kcIndex]);
    kmod = KEYMAP_MODIFIER(kcArray[kcIndex]);
    kcIndex++;
  }

  if (!kc)
  {
    kcArray = nullptr;
    kcIndex = 0;
    kcLen = 0;
    return;
  }

  // Send media keys as REPORT
  if (kc >= HID_KEY_MEDIA_PLAYPAUSE && kc <= HID_KEY_MEDIA_CALC)
  {
    uint16_t key = keycodeToConsumerCode(kc);
    tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &key, 2);
    hasConsumerKey = true;
  }
  else
  {
    printf("kc %x, mod %x\n", kc, kmod);
    uint8_t key_input[6] = {kc, 0, 0, 0, 0, 0};
    tud_hid_n_keyboard_report(ITF_KEYBOARD, REPORT_ID_KEYBOARD, kmod, key_input);
    hasKeyboardKey = true;
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
