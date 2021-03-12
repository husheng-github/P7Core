

#ifndef _DDI_KEY_H_
#define _DDI_KEY_H_


#include "ddi_common.h"


//键值定义
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
#define KEY_CFG_PAPER   (0x42)          //key_cfg paper
#define KEY_PAPER_PLUS  (0x43)          //key_paper key_+
#define KEY_PAPER_MINUS (0x44)          //key_paper key_-
#define KEY_PLUS_MINUS  (0x45)          //key_+ key_-




#define KEY_PLUS_LONG        (0x80)         // 0x80 long key_+
#define KEY_MINUS_LONG       (0x81)         // 0x81 long key_-
#define KEY_CFG_LONG         (0x82)         // 0x82
#define KEY_PAPER_LONG       (0x83)         // 0x83
#define KEY_TBD_LONG         (0x84)         // 0x84


/*按键控制命令*/
#define     DDI_KEY_CTL_LONGPRESS           (0) //设置长按键
#define     DDI_KEY_CTL_BKLIGHT             (1) //控制按键背光
#define     DDI_KEY_CTL_BKLIGHTTIME         (2) //设置背光时间
#define     DDI_KEY_CTL_BKLIGHT_CTRL         (3) //控制键盘背光亮灭
#define     DDI_KEY_GET_CURRKEY             (4)  //获取当前键值
#define     DDI_KEY_CTL_POWERKEY_FLG        (5)  //设置power key flg
#define     DDI_KEY_GET_POWERKEY_FLG        (6)  //获取power key flg


//============================================
//对外函数声明
extern s32 ddi_key_open (void);
extern s32 ddi_key_close (void);
extern s32 ddi_key_clear (void);
extern s32 ddi_key_read (u32 *lpKey);
extern s32 ddi_key_read_withoutdelay(u32 *lpKey);
extern s32 ddi_key_ioctl(u32 nCmd, u32 lParam, u32 wParam);

//=========================================
typedef s32 (*core_ddi_key_open)(void);
typedef s32 (*core_ddi_key_close)(void);
typedef s32 (*core_ddi_key_clear)(void);
typedef s32 (*core_ddi_key_read)(u32 *lpKey);
typedef s32 (*core_ddi_key_read_withoutdelay)(u32 *lpKey);
typedef s32 (*core_ddi_key_ioctl)(u32 nCmd, u32 lParam, u32 wParam);
//typedef s32 (*core_ddi_powerkey_check)(u8 *lpKey);
#endif
