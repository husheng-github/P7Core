#ifndef __DEV_KEYPAD_H
#define __DEV_KEYPAD_H


#define POWER           (0x01) //系统关机
#define F1              (0x02)  // 0x02
#define F2              (0x03)        // 0x03
#define F3              (0x04)       // 0x04
#define FUNCTION        (0x05)       // 0x05
#define DIGITAL1        (0x06)       // 0x06
#define DIGITAL2        (0x07)  // 0x07
#define DIGITAL3        (0x08)  // 0x08
#define ALPHA           (0x09)  // 0x09
#define DIGITAL4        (0x0A)       // 0x0A
#define DIGITAL5        (0x0B)  // 0x0B
#define DIGITAL6        (0x0C)  // 0x0C
#define ESC             (0x0D)  // 0x0D
#define DIGITAL7        (0x0E)          // 0x0E
#define DIGITAL8        (0x0F)  // 0x0F
#define DIGITAL9        (0x10)  // 0x10
#define CLEAR           (0x11)  // 0x11
#define UP_OR_10        (0x12) //向上，星号  // 0x12
#define DIGITAL0        (0x13)             // 0x13
#define DOWN_OR_11      (0x14) //向下，井号 // 0x14
#define ENTER           (0x15)         // 0x15
#define IDLESLEEP       (0x16)         // 0x16
#define KEY_PLUS        (0x17)         // 0x17 key_+
#define KEY_MINUS       (0x18)         // 0x18 key_-
#define KEY_CFG         (0x19)         // 0x19
#define KEY_PAPER       (0x1A)         // 0x1A
#define KEY_TBD         (0x1B)         // 0x1B


#define KEY_CFG_PLUS    (0x40)          //key_cfg key_+
#define KEY_CFG_MINUS   (0x41)          //key_cfg key_-
#define KEY_CFG_PAPER   (0x42)          //key_cfg key_paper
#define KEY_PAPER_PLUS  (0x43)          //key_paper key_+
#define KEY_PAPER_MINUS (0x44)          //key_paper key_-
#define KEY_PLUS_MINUS  (0x45)          //key_+ key_-

#define KEY_PLUS_LONG        (0x80)         // 0x80 long key_+
#define KEY_MINUS_LONG       (0x81)         // 0x81 long key_-
#define KEY_CFG_LONG         (0x82)         // 0x82
#define KEY_PAPER_LONG       (0x83)         // 0x83
#define KEY_TBD_LONG         (0x84)         // 0x84





#define LONGKEY_TIMER       100//500     //5s

typedef enum _KEY_TYPE
{
    KEY_EVENT_DOWN = 1,
    KEY_EVENT_UP,
    KEY_LONG_PRESS,

}KEY_TYPE_T;


s32 dev_keypad_scan(u8 num);
void dev_keypad_pwr_enable(u8 flg);
void dev_keypad_init(void);
s32 dev_keypad_open(void);
s32 dev_keypad_close(void);
s32 dev_keypad_read(u32 *rkey);
s32 dev_keypad_scan(u8 num);
s32 dev_keypad_clear(void);
s32 dev_keypad_read_beep(u32 *rkey);
s32 dev_keypad_ioctl(u32 nCmd, u32 lParam, u32 wParam);

#endif
