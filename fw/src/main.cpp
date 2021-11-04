/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tusb_config.h"
#include "tusb.h"
#include "bsp/board.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "usb_descriptors.h"
#include "enums.h"
#include "rotary_encoder.h"

static RotaryEncoder *encoder = nullptr;

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum
{
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;
void led_blinking_task(void);
void hid_task(void);

void mp_gpio_init(void);
void mp_gpio_callback(uint gpio, uint32_t events);

int main()
{
    // int pos = 0;
    encoder = new RotaryEncoder(GPIO_ROT_CLK, GPIO_ROT_DATA, RotaryEncoder::LatchMode::TWO03);
    stdio_init_all();
    mp_gpio_init();
    board_init();

    tusb_init();

    while (1)
    {
        tud_task(); // tinyusb device task
        //led_blinking_task();
        hid_task();
    }

    // // Wait forever
    // while (1)
    // {

    //     int newPos = encoder->getPosition();
    //     if (pos != newPos)
    //     {
    //         printf("pos:");
    //         printf("%d", newPos);
    //         printf(" dir:");
    //         printf("%d\n", (int)(encoder->getDirection()));
    //         pos = newPos;
    //     }
    // }

    return 0;
}

//--------------------------------------------------------------------+
// GPIO Related functions
//--------------------------------------------------------------------+

static const bool ON_RELEASE = true;
static uint32_t gpio_events;

void mp_gpio_callback(uint gpio, uint32_t events)
{
    //gpio_put(PICO_DEFAULT_LED_PIN, 1);

    uint key_evt = 0;
    switch (gpio)
    {
    case GPIO_SW_1:
        key_evt = 1 << Key1;
        break;
    case GPIO_SW_2:
        key_evt = 1 << Key2;
        break;
    case GPIO_SW_3:
        key_evt = 1 << Key3;
        break;
    case GPIO_SW_4:
        key_evt = 1 << Key4;
        break;
    case GPIO_SW_5:
        key_evt = 1 << Key5;
        break;
    case GPIO_SW_6:
        key_evt = 1 << Key6;
        break;
    case GPIO_SW_7:
        key_evt = 1 << Key7;
        break;
    case GPIO_SW_8:
        key_evt = 1 << Key8;
        break;
    case GPIO_ROT_SW:
        key_evt = 1 << RotClick;
        break;
    case GPIO_ROT_CLK:
    case GPIO_ROT_DATA:
    default:
        encoder->tick();
        RotaryEncoder::Direction dir = encoder->getDirection();
        if (dir == RotaryEncoder::Direction::CLOCKWISE)
        {
            key_evt = 1 << RotCW;
        }
        else if (dir == RotaryEncoder::Direction::COUNTERCLOCKWISE)
        {
            key_evt = 1 << RotCCW;
        }
        return;
    }

    //TODO: Add debounce logic
    if ((ON_RELEASE && (events & GPIO_IRQ_EDGE_RISE)) ||
        (!ON_RELEASE && (events & GPIO_IRQ_EDGE_FALL)))
    {
        gpio_events |= key_evt;
    }
}

void mp_gpio_init()
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    gpio_set_irq_enabled_with_callback(GPIO_SW_1, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    gpio_set_irq_enabled_with_callback(GPIO_SW_2, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    gpio_set_irq_enabled_with_callback(GPIO_SW_3, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    gpio_set_irq_enabled_with_callback(GPIO_SW_4, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    gpio_set_irq_enabled_with_callback(GPIO_SW_5, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    gpio_set_irq_enabled_with_callback(GPIO_SW_6, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    gpio_set_irq_enabled_with_callback(GPIO_SW_7, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    gpio_set_irq_enabled_with_callback(GPIO_SW_8, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    gpio_set_irq_enabled_with_callback(GPIO_ROT_SW, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    gpio_set_irq_enabled_with_callback(GPIO_ROT_CLK, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    gpio_set_irq_enabled_with_callback(GPIO_ROT_DATA, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);

    gpio_set_pulls(GPIO_SW_1, true, false);
    gpio_set_pulls(GPIO_SW_2, true, false);
    gpio_set_pulls(GPIO_SW_3, true, false);
    gpio_set_pulls(GPIO_SW_4, true, false);
    gpio_set_pulls(GPIO_SW_5, true, false);
    gpio_set_pulls(GPIO_SW_6, true, false);
    gpio_set_pulls(GPIO_SW_7, true, false);
    gpio_set_pulls(GPIO_SW_8, true, false);
    gpio_set_pulls(GPIO_ROT_SW, true, false);
    gpio_set_pulls(GPIO_ROT_CLK, true, false);
    gpio_set_pulls(GPIO_ROT_DATA, true, false);
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void)remote_wakeup_en;
    blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static const uint8_t keys_to_keycodes[11][6] =
    {
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

static void
send_hid_report()
{
    // skip if hid is not ready yet
    if (!tud_hid_ready())
        return;

    // use to avoid send multiple consecutive zero report for keyboard
    static bool has_keyboard_key = false;
    if (gpio_events)
    {
        const uint8_t *keycode = NULL;
        for (int i = 0; i < 11; i++)
        {
            // We have an event
            if ((gpio_events >> i) & 1)
            {
                // Lets clear it up
                gpio_events &= ~(1UL << i);
                keycode = keys_to_keycodes[i];
                break;
            }
        }

        if (keycode == NULL)
            return;

        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, (uint8_t *)keycode);
        has_keyboard_key = true;
    }
    else
    {
        // send empty key report if previously has key pressed
        if (has_keyboard_key)
            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        has_keyboard_key = false;
    }
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms)
        return; // not enough time
    start_ms += interval_ms;

    // Remote wakeup
    if (tud_suspended() && gpio_events)
    {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    }
    else
    {
        // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
        send_hid_report();
    }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t itf, uint8_t const *report, uint8_t len)
{
    (void)itf;
    (void)report;
    (void)len;

    send_hid_report();
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

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
    static uint32_t start_ms = 0;
    static bool led_state = false;

    // Blink every interval ms
    if (board_millis() - start_ms < blink_interval_ms)
        return; // not enough time
    start_ms += blink_interval_ms;

    board_led_write(led_state);
    led_state = 1 - led_state; // toggle
}
