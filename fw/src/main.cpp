/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"

#include "enums.h"
#include "rotary_encoder.h"
#include "keymap.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

// Interface index depends on the order in configuration descriptor
enum {
  ITF_KEYBOARD = 0,
  ITF_MOUSE = 1
};

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};


static uint32_t _key_events = 0;
static uint32_t _blink_interval_ms = BLINK_NOT_MOUNTED;
static RotaryEncoder* _encoder = nullptr;
static Keymap* _keymap = nullptr;

void led_blinking_task(void);
void hid_task(void);
void mp_gpio_init(void);
uint32_t get_gpio_events();

/*------------- MAIN -------------*/
int main(void)
{
  _keymap = new Keymap();
  _encoder = new RotaryEncoder(GPIO_ROT_CLK, GPIO_ROT_DATA, RotaryEncoder::LatchMode::TWO03);

  board_init();
  mp_gpio_init();
  tusb_init();

  while (1)
  {
    tud_task(); // tinyusb device task
    led_blinking_task();

    hid_task();
    get_gpio_events();
  }

  return 0;
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  _blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  _blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  _blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  _blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

void hid_task(void)
{
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  // Remote wakeup
  if ( tud_suspended() && _key_events )
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }

  /*------------- Keyboard -------------*/
  if ( tud_hid_n_ready(ITF_KEYBOARD) )
  {
    // use to avoid send multiple consecutive zero report for keyboard
    static bool has_key = false;

    if ( _key_events )
    {
      uint8_t keycode = -1;
      for (int i = 0; i < 11; i++)
      {
          // We have an event
          if ((_key_events >> i) & 1)
          {
              // Lets clear it up
              _key_events &= ~(1UL << i);
              keycode = _keymap->GetKey(i);
              break;
          }
      }

      if (keycode == -1)
        return;

      uint8_t key_input[6] = {keycode, 0, 0, 0, 0, 0};
      tud_hid_n_keyboard_report(ITF_KEYBOARD, 0, 0, key_input);

      has_key = true;
      _key_events = false;
    }
    else
    {
      // send empty key report if previously has key pressed
      if (has_key) tud_hid_n_keyboard_report(ITF_KEYBOARD, 0, 0, NULL);
      has_key = false;
    }
  }
}


// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) itf;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  // TODO set LED based on CAPLOCK, NUMLOCK etc...
  (void) itf;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) bufsize;
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // Blink every interval ms
  if ( board_millis() - start_ms < _blink_interval_ms) return; // not enough time
  start_ms += _blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}

static const uint8_t gpios[] = {
    GPIO_SW_1,
    GPIO_SW_2,
    GPIO_SW_3,
    GPIO_SW_4,
    GPIO_SW_5,
    GPIO_SW_6,
    GPIO_SW_7,
    GPIO_SW_8,
    GPIO_ROT_SW,
    GPIO_ROT_CLK,
    GPIO_ROT_DATA,
};

void mp_gpio_init()
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    for (uint i = 0; i < sizeof(gpios); i++)
    {
        gpio_init(gpios[i]);
        gpio_set_dir(gpios[i], GPIO_IN);
        gpio_set_pulls(gpios[i], true, false);
    }
}

uint32_t get_gpio_events()
{
  //TODO: Debounce
  uint32_t gpio_state = gpio_get_all();

  _key_events |= !(gpio_state & (1<< GPIO_SW_1)) << Keys::Key1;
  _key_events |= !(gpio_state & (1<< GPIO_SW_2)) << Keys::Key2;
  _key_events |= !(gpio_state & (1<< GPIO_SW_3)) << Keys::Key3;
  _key_events |= !(gpio_state & (1<< GPIO_SW_4)) << Keys::Key4;
  _key_events |= !(gpio_state & (1<< GPIO_SW_5)) << Keys::Key5;
  _key_events |= !(gpio_state & (1<< GPIO_SW_6)) << Keys::Key6;
  _key_events |= !(gpio_state & (1<< GPIO_SW_7)) << Keys::Key7;
  _key_events |= !(gpio_state & (1<< GPIO_SW_8)) << Keys::Key8;
  _key_events |= !(gpio_state & (1<< GPIO_ROT_SW)) << Keys::RotClick;

  int clk = !(gpio_state & (1<< GPIO_ROT_CLK));
  int data = !(gpio_state & (1<< GPIO_ROT_DATA));
  _encoder->tick(clk, data);
  RotaryEncoder::Direction dir = _encoder->getDirection();
  if (dir == RotaryEncoder::Direction::CLOCKWISE)
  {
      _key_events |= 1 << Keys::RotCW;
  }
  else if (dir == RotaryEncoder::Direction::COUNTERCLOCKWISE)
  {
      _key_events |= 1 << Keys::RotCCW;
  }

  if (_key_events)
  {
    board_led_write(1);
  }

  return _key_events;
}