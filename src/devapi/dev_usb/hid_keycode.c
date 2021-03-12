#include "hid_keycode.h"

const uint8_t KeyCodeArray[97][3] = {
    /* ascii  CtrlValue  UsageID  */
    {'a', 0x00, KEY_A},
    {'b', 0x00, KEY_B},
    {'c', 0x00, KEY_C},
    {'d', 0x00, KEY_D},
    {'e', 0x00, KEY_E},
    {'f', 0x00, KEY_F},
    {'g', 0x00, KEY_G},
    {'h', 0x00, KEY_H},
    {'i', 0x00, KEY_I},
    {'j', 0x00, KEY_J},
    {'k', 0x00, KEY_K},
    {'l', 0x00, KEY_L},
    {'m', 0x00, KEY_M},
    {'n', 0x00, KEY_N},
    {'o', 0x00, KEY_O},
    {'p', 0x00, KEY_P},
    {'q', 0x00, KEY_Q},
    {'r', 0x00, KEY_R},
    {'s', 0x00, KEY_S},
    {'t', 0x00, KEY_T},
    {'u', 0x00, KEY_U},
    {'v', 0x00, KEY_V},
    {'w', 0x00, KEY_W},
    {'x', 0x00, KEY_X},
    {'y', 0x00, KEY_Y},
    {'z', 0x00, KEY_Z},
    {'A', 0x02, KEY_A},
    {'B', 0x02, KEY_B},
    {'C', 0x02, KEY_C},
    {'D', 0x02, KEY_D},
    {'E', 0x02, KEY_E},
    {'F', 0x02, KEY_F},
    {'G', 0x02, KEY_G},
    {'H', 0x02, KEY_H},
    {'I', 0x02, KEY_I},
    {'J', 0x02, KEY_J},
    {'K', 0x02, KEY_K},
    {'L', 0x02, KEY_L},
    {'M', 0x02, KEY_M},
    {'N', 0x02, KEY_N},
    {'O', 0x02, KEY_O},
    {'P', 0x02, KEY_P},
    {'Q', 0x02, KEY_Q},
    {'R', 0x02, KEY_R},
    {'S', 0x02, KEY_S},
    {'T', 0x02, KEY_T},
    {'U', 0x02, KEY_U},
    {'V', 0x02, KEY_V},
    {'W', 0x02, KEY_W},
    {'X', 0x02, KEY_X},
    {'Y', 0x02, KEY_Y},
    {'Z', 0x02, KEY_Z},
    {'1', 0x00, KEY_1},
    {'2', 0x00, KEY_2},
    {'3', 0x00, KEY_3},
    {'4', 0x00, KEY_4},
    {'5', 0x00, KEY_5},
    {'6', 0x00, KEY_6},
    {'7', 0x00, KEY_7},
    {'8', 0x00, KEY_8},
    {'9', 0x00, KEY_9},
    {'0', 0x00, KEY_0},
    {'!', 0x02, KEY_1},
    {'@', 0x02, KEY_2},
    {'#', 0x02, KEY_3},
    {'$', 0x02, KEY_4},
    {'%', 0x02, KEY_5},
    {'^', 0x02, KEY_6},
    {'&', 0x02, KEY_7},
    {'*', 0x02, KEY_8},
    {'(', 0x02, KEY_9},
    {')', 0x02, KEY_0},
    {'\n', 0x00, KEY_ENTER},
    {0x1B, 0x00, KEY_ESCAPE},
    {' ', 0x00, KEY_SPACEBAR},
    {'-', 0x00, KEY_UNDERSCORE},
    {'_', 0x02, KEY_UNDERSCORE},
    {'=', 0x00, KEY_PLUS},
    {'+', 0x02, KEY_PLUS},
    {'[', 0x00, KEY_OPEN_BRACKET},
    {'{', 0x02, KEY_OPEN_BRACKET},
    {']', 0x00, KEY_CLOSE_BRACKET},
    {'}', 0x02, KEY_CLOSE_BRACKET},
    {'\\', 0x00, KEY_BACKSLASH},
    {'|', 0x02, KEY_BACKSLASH},
    {';', 0x00, KEY_COLON},
    {':', 0x02, KEY_COLON},
    {'\'', 0x00, KEY_QUOTE},
    {'"', 0x02, KEY_QUOTE},
    {',', 0x00, KEY_COMMA},
    {'<', 0x02, KEY_COMMA},
    {'.', 0x00, KEY_DOT},
    {'>', 0x02, KEY_DOT},
    {'.', 0x00, KEY_DOT},
    {'>', 0x02, KEY_DOT},
    {'/', 0x00, KEY_SLASH},
    {'?', 0x02, KEY_SLASH},
};