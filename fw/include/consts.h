#ifndef CONSTS_H
#define CONSTS_H

#ifdef __cplusplus
extern "C"
{
#endif

#define GPIO_SW_1 (27)
#define GPIO_SW_2 (26)
#define GPIO_SW_3 (22)
#define GPIO_SW_4 (21)
#define GPIO_SW_5 (17)
#define GPIO_SW_6 (16)
#define GPIO_SW_7 (14)
#define GPIO_SW_8 (15)
#define GPIO_ROT_CLK (19)
#define GPIO_ROT_DATA (20)
#define GPIO_ROT_SW (18)
#define GPIO_LEDS (28)

#define LEDS_RGBW (false)
#define LEDS_NUM  (8)

#define KEY_REPEAT_DELAY_MS (500)
#define KEY_REPEAT_RATE_MS (100)

// HID media keys not defined in tusb
#define HID_KEY_MEDIA_PLAYPAUSE           0XE8
#define HID_KEY_MEDIA_STOPCD              0XE9
#define HID_KEY_MEDIA_PREVIOUSSONG        0XEA
#define HID_KEY_MEDIA_NEXTSONG            0XEB
#define HID_KEY_MEDIA_EJECTCD             0XEC
#define HID_KEY_MEDIA_VOLUMEUP            0XED
#define HID_KEY_MEDIA_VOLUMEDOWN          0XEE
#define HID_KEY_MEDIA_MUTE                0XEF
#define HID_KEY_MEDIA_WWW                 0XF0
#define HID_KEY_MEDIA_BACK                0XF1
#define HID_KEY_MEDIA_FORWARD             0XF2
#define HID_KEY_MEDIA_STOP                0XF3
#define HID_KEY_MEDIA_FIND                0XF4
#define HID_KEY_MEDIA_SCROLLUP            0XF5
#define HID_KEY_MEDIA_SCROLLDOWN          0XF6
#define HID_KEY_MEDIA_EDIT                0XF7
#define HID_KEY_MEDIA_SLEEP               0XF8
#define HID_KEY_MEDIA_COFFEE              0XF9
#define HID_KEY_MEDIA_REFRESH             0XFA
#define HID_KEY_MEDIA_CALC                0XFB

#ifdef __cplusplus
}
#endif

#endif