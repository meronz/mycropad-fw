/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "tusb_config.h"
#include "tusb.h"
#include "bsp/board.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "usb_descriptors.h"
#include "enums.h"
#include "rotary_encoder.h"
#include "keymap.h"

static RotaryEncoder *encoder = nullptr;
static Keymap *keymap = nullptr;

#define DEBUG 0
#if DEBUG
#define LOG cdc_send
#else
#define LOG
#endif

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
void cdc_task(void);
void mp_gpio_init(void);
void mp_gpio_callback(uint gpio, uint32_t events);

void cdc_send(const char *fmt, ...)
{
    static char cdc_send_buf[4096];
    va_list argptr;
    va_start(argptr, fmt);
    vfprintf(stderr, fmt, argptr);
    int n = vsnprintf(cdc_send_buf, 4096, fmt, argptr);
    va_end(argptr);

    // tud_cdc_write(cdc_send_buf, n);
    // tud_cdc_write_flush();
}

int main()
{
    keymap = new Keymap();
    encoder = new RotaryEncoder(GPIO_ROT_CLK, GPIO_ROT_DATA, RotaryEncoder::LatchMode::TWO03);
    //stdio_init_all();
    while (2000 > to_ms_since_boot(get_absolute_time()))
    {
    }

    mp_gpio_init();
    board_init();
    tusb_init();

    while (1)
    {
        tud_task();
        led_blinking_task();
        hid_task();
        //cdc_task();
    }

    return 0;
}

//--------------------------------------------------------------------+
// GPIO Related functions
//--------------------------------------------------------------------+

// #define ON_RELEASE (true)
// static volatile uint32_t gpio_events = 0;

// void mp_gpio_callback(uint gpio, uint32_t events)
// {
//     //gpio_put(PICO_DEFAULT_LED_PIN, gpio_events);

//     uint key_evt = 0;
//     switch (gpio)
//     {
//         case GPIO_SW_1:
//             key_evt = 1 << Keys::Key1;
//             break;
//         case GPIO_SW_2:
//             key_evt = 1 << Keys::Key2;
//             break;
//         case GPIO_SW_3:
//             key_evt = 1 << Keys::Key3;
//             break;
//         case GPIO_SW_4:
//             key_evt = 1 << Keys::Key4;
//             break;
//         case GPIO_SW_5:
//             key_evt = 1 << Keys::Key5;
//             break;
//         case GPIO_SW_6:
//             key_evt = 1 << Keys::Key6;
//             break;
//         case GPIO_SW_7:
//             key_evt = 1 << Keys::Key7;
//             break;
//         case GPIO_SW_8:
//             key_evt = 1 << Keys::Key8;
//             break;
//         case GPIO_ROT_SW:
//             key_evt = 1 << Keys::RotClick;
//             break;
//         case GPIO_ROT_CLK:
//         case GPIO_ROT_DATA:
//         // {
//         //     encoder->tick();
//         //     RotaryEncoder::Direction dir = encoder->getDirection();
//         //     if (dir == RotaryEncoder::Direction::CLOCKWISE)
//         //     {
//         //         key_evt = 1 << Keys::RotCW;
//         //     }
//         //     else if (dir == RotaryEncoder::Direction::COUNTERCLOCKWISE)
//         //     {
//         //         key_evt = 1 << Keys::RotCCW;
//         //     }
//         //     break;
//         // }
//         default:
//             return;
//     }

//     //TODO: Add debounce logic
//     // if ((ON_RELEASE && (events & 1 <<GPIO_IRQ_EDGE_RISE)) ||
//     //     (!ON_RELEASE && (events & GPIO_IRQ_EDGE_FALL)))
//     // {
//         //gpio_events |= key_evt;
//     // }
//     // #if ON_RELEASE
//     // if(events & (uint)GPIO_IRQ_EDGE_RISE)
//     // #else
//     // if(events & (uint)GPIO_IRQ_EDGE_FALL)
//     // #endif
//     // {
//     //     gpio_events |= key_evt;
//     // }
// }

static const uint8_t gpios[] = {
    // GPIO_SW_1,
    // GPIO_SW_2,
    // GPIO_SW_3,
    // GPIO_SW_4,
    // GPIO_SW_5,
    // GPIO_SW_6,
    // GPIO_SW_7,
    // GPIO_SW_8,
    GPIO_ROT_SW,
    // GPIO_ROT_CLK,
    // GPIO_ROT_DATA,
};

void mp_gpio_init()
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // gpio_set_irq_enabled_with_callback(GPIO_SW_1, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    // gpio_set_irq_enabled_with_callback(GPIO_SW_2, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    // gpio_set_irq_enabled_with_callback(GPIO_SW_3, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    // gpio_set_irq_enabled_with_callback(GPIO_SW_4, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    // gpio_set_irq_enabled_with_callback(GPIO_SW_5, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    // gpio_set_irq_enabled_with_callback(GPIO_SW_6, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    // gpio_set_irq_enabled_with_callback(GPIO_SW_7, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    // gpio_set_irq_enabled_with_callback(GPIO_SW_8, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    // gpio_set_irq_enabled_with_callback(GPIO_ROT_SW, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    // gpio_set_irq_enabled_with_callback(GPIO_ROT_CLK, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);
    // gpio_set_irq_enabled_with_callback(GPIO_ROT_DATA, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &mp_gpio_callback);

    for (int i = 0; i < sizeof(gpios); i++)
    {
        gpio_init(gpios[i]);
        gpio_set_dir(gpios[i], GPIO_IN);
        gpio_set_pulls(gpios[i], true, false);    
    }
}

static uint32_t get_gpio_events()
{
    uint32_t key_evt = 0;
    //key_evt |= gpio_get(GPIO_SW_1) << Keys::Key1;
    //key_evt |= gpio_get(GPIO_SW_2) << Keys::Key2;
    //key_evt |= gpio_get(GPIO_SW_3) << Keys::Key3;
    //key_evt |= gpio_get(GPIO_SW_4) << Keys::Key4;
    //key_evt |= gpio_get(GPIO_SW_5) << Keys::Key5;
    //key_evt |= gpio_get(GPIO_SW_6) << Keys::Key6;
    //key_evt |= gpio_get(GPIO_SW_7) << Keys::Key7;
    //key_evt |= gpio_get(GPIO_SW_8) << Keys::Key8;
    key_evt |= gpio_get(GPIO_ROT_SW) << Keys::RotClick;

    encoder->tick();
    RotaryEncoder::Direction dir = encoder->getDirection();
    if (dir == RotaryEncoder::Direction::CLOCKWISE)
    {
        key_evt |= 1 << Keys::RotCW;
    }
    else if (dir == RotaryEncoder::Direction::COUNTERCLOCKWISE)
    {
        key_evt |= 1 << Keys::RotCCW;
    }
    return key_evt;
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

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
enum
{
    ITF_KEYBOARD = 0
};

void hid_task(void)
{
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms)
        return; // not enough time
    start_ms += interval_ms;

    uint32_t const btn = board_button_read();

    // Remote wakeup
    if (tud_suspended() && btn)
    {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    }

    /*------------- Keyboard -------------*/
    if (tud_hid_n_ready(ITF_KEYBOARD))
    {
        // use to avoid send multiple consecutive zero report for keyboard
        static bool has_key = false;

        uint32_t gpio_events = get_gpio_events();
        if (gpio_events)
        {
            const uint8_t *keycode = nullptr;
            for (int i = 0; i < 11; i++)
            {
                // We have an event
                if ((gpio_events >> i) & 1)
                {
                    // Lets clear it up
                    gpio_events &= ~(1UL << i);
                    keycode = keymap->GetKey(i);
                    break;
                }
            }

            if (keycode == nullptr)
                return;

            uint8_t key_input[6] = {keycode[0], 0, 0, 0, 0, 0};
            tud_hid_n_keyboard_report(ITF_KEYBOARD, 0, 0, key_input);
            has_key = true;
        }
        else
        {
            // send empty key report if previously has key pressed
            if (has_key)
                tud_hid_n_keyboard_report(ITF_KEYBOARD, 0, 0, NULL);
            has_key = false;
        }
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
//char response_buf[255];
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
    // This example doesn't use multiple report and report ID
    (void)itf;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)bufsize;
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
// void cdc_task(void)
// {
//     // connected() check for DTR bit
//     // Most but not all terminal client set this when making connection
//     // if ( tud_cdc_connected() )
//     {
//         // connected and there are data available
//         if (tud_cdc_available())
//         {
//             // read data
//             bool ok = false;
//             static uint8_t buf[4096];
//             static bool stx_received = false;
//             uint32_t count = tud_cdc_read(buf, sizeof(buf));

//             // parse config message
//             if (count == 0)
//             {
//                 return;
//             }

//             if (!stx_received)
//             {
//                 if (buf[0] == 0x02) //stx
//                 {
//                     stx_received = true;
//                 }
//             }

//             switch (buf[0])
//             {
//             case MessageTypes::NewKeymap:
//                 ok = keymap->SetKeymap(buf + 1, count - 1);
//                 break;

//             default:
//                 break;
//             }

//             //send CmdStatus
//             StaticJsonDocument<32> cmdStatus;
//             cmdStatus["Ok"] = ok;
//             int len = serializeJson(cmdStatus, buf, sizeof(buf));
//             tud_cdc_write(buf, len);
//             tud_cdc_write_flush();
//         }
//     }
// }

// // Invoked when cdc when line state changed e.g connected/disconnected
// void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
// {
//     (void)itf;
//     (void)rts;

//     // TODO set some indicator
//     if (dtr)
//     {
//         // Terminal connected
//     }
//     else
//     {
//         // Terminal disconnected
//     }
// }

// // Invoked when CDC interface received data from host
// void tud_cdc_rx_cb(uint8_t itf)
// {
//     (void)itf;
// }

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
    LOG(".");
}