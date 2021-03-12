/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HID_KEYCODE__H__
#define __HID_KEYCODE__H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mhscpu.h"

// key codes
enum
{
    KEY_A             = 4,
    KEY_B             = 5,
    KEY_C             = 6,
    KEY_D             = 7,
    KEY_E             = 8,
    KEY_F             = 9,
    KEY_G             = 10,
    KEY_H             = 11,
    KEY_I             = 12,
    KEY_J             = 13,
    KEY_K             = 14,
    KEY_L             = 15,
    KEY_M             = 16,
    KEY_N             = 17,
    KEY_O             = 18,
    KEY_P             = 19,
    KEY_Q             = 20,
    KEY_R             = 21,
    KEY_S             = 22,
    KEY_T             = 23,
    KEY_U             = 24,
    KEY_V             = 25,
    KEY_W             = 26,
    KEY_X             = 27,
    KEY_Y             = 28,
    KEY_Z             = 29,
    KEY_1             = 30,
    KEY_2             = 31,
    KEY_3             = 32,
    KEY_4             = 33,
    KEY_5             = 34,
    KEY_6             = 35,
    KEY_7             = 36,
    KEY_8             = 37,
    KEY_9             = 38,
    KEY_0             = 39,
    KEY_ENTER         = 40,
    KEY_ESCAPE        = 41,
    KEY_BACKSPACE     = 42,
    KEY_TAB           = 43,
    KEY_SPACEBAR      = 44,
    KEY_UNDERSCORE    = 45,
    KEY_PLUS          = 46,
    KEY_OPEN_BRACKET  = 47,
    KEY_CLOSE_BRACKET = 48, //
    KEY_BACKSLASH     = 49, // backslash and pipe
    KEY_HASH          = 50, // hash and tilde
    KEY_COLON         = 51, // semicolon
    KEY_QUOTE         = 52, // quote and single quote
    KEY_TILDE         = 53, // grave accent
    KEY_COMMA         = 54, // , >
    KEY_DOT           = 55, // . >
    KEY_SLASH         = 56, // / ?
    KEY_CAPS_LOCK     = 57,
    KEY_F1            = 58,
    KEY_F2            = 59,
    KEY_F3            = 60,
    KEY_F4            = 61,
    KEY_F5            = 62,
    KEY_F6            = 63,
    KEY_F7            = 64,
    KEY_F8            = 65,
    KEY_F9            = 66,
    KEY_F10           = 67,
    KEY_F11           = 68,
    KEY_F12           = 69,

    KEY_PRINTSCREEN                = 70,
    KEY_SCROLL_LOCK                = 71,
    KEY_PAUSE                      = 72,
    KEY_INSERT                     = 73,
    KEY_HOME                       = 74,
    KEY_PAGEUP                     = 75,
    KEY_DELETE                     = 76,
    KEY_END                        = 77,
    KEY_PAGEDOWN                   = 78,
    KEY_RIGHT                      = 79,
    KEY_LEFT                       = 80,
    KEY_DOWN                       = 81,
    KEY_UP                         = 82,
    KEY_KP_NUM_LOCK                = 83,
    KEY_KP_DIVIDE                  = 84,
    KEY_KP_AT                      = 85,
    KEY_KP_MULTIPLY                = 85,
    KEY_KP_MINUS                   = 86,
    KEY_KP_PLUS                    = 87,
    KEY_KP_ENTER                   = 88,
    KEY_KP_1                       = 89,
    KEY_KP_2                       = 90,
    KEY_KP_3                       = 91,
    KEY_KP_4                       = 92,
    KEY_KP_5                       = 93,
    KEY_KP_6                       = 94,
    KEY_KP_7                       = 95,
    KEY_KP_8                       = 96,
    KEY_KP_9                       = 97,
    KEY_KP_0                       = 98,
    KEY_KP_DOT_AND_DELETE          = 99,
    KEY_NON_US_BACKSLASH_AND_SLASH = 100,

    KEY_APPLICATION = 0x65,
    KEY_POWER       = 0x66,

    KEY_F13 = 0x68,
    KEY_F14 = 0x69,
    KEY_F15 = 0x6a,
    KEY_F16 = 0x6b,
    KEY_F17 = 0x6c,
    KEY_F18 = 0x6d,
    KEY_F19 = 0x6e,
    KEY_F20 = 0x6f,
    KEY_F21 = 0x70,
    KEY_F22 = 0x71,
    KEY_F23 = 0x72,
    KEY_F24 = 0x73,

    KEY_EXECUTE     = 0x74,
    KEY_HELP        = 0x75,
    KEY_MENU        = 0x76,
    KEY_SELECT      = 0x77,
    KEY_STOP        = 0x78,
    KEY_AGAIN       = 0x79,
    KEY_UNDO        = 0x7a,
    KEY_CUT         = 0x7b,
    KEY_COPY        = 0x7c,
    KEY_PASTE       = 0x7d,
    KEY_FIND        = 0x7e,
    KEY_MUTE        = 0x7f,
    KEY_VOLUME_UP   = 0x80,
    KEY_VOLUME_DOWN = 0x81,

    // modifier bit sets
    KEY_MODIFIER_NONE        = 0x00,
    KEY_MODIFIER_LEFT_CTRL   = 0x01,
    KEY_MODIFIER_LEFT_SHIFT  = 0x02,
    KEY_MODIFIER_LEFT_ALT    = 0x04,
    KEY_MODIFIER_LEFT_UI     = 0x08,
    KEY_MODIFIER_RIGHT_CTRL  = 0x10,
    KEY_MODIFIER_RIGHT_SHIFT = 0x20,
    KEY_MODIFIER_RIGHT_ALT   = 0x40,
    KEY_MODIFIER_RIGHT_UI    = 0x80,

    // media keys
    KEY_MEDIA_SCAN_NEXT   = 0x01,
    KEY_MEDIA_SCAN_PREV   = 0x02,
    KEY_MEDIA_STOP        = 0x04,
    KEY_MEDIA_EJECT       = 0x08,
    KEY_MEDIA_PAUSE       = 0x10,
    KEY_MEDIA_MUTE        = 0x20,
    KEY_MEDIA_VOLUME_UP   = 0x40,
    KEY_MEDIA_VOLUME_DOWN = 0x80,
};

extern const uint8_t KeyCodeArray[97][3];

#ifdef __cplusplus
}
#endif

#endif
