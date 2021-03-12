#ifndef __DEV_BT_H
#define __DEV_BT_H
#include "ddi/ddi_bt.h"
typedef struct _str_btparam
{
    u8 m_btmod;         //表示包含哪些项
                        //bit0=1: btname存在
                        //bit1=1: btmac存在
                        //bit2=1: nvram存在
    u8 m_btname[32];    //字符串,有效字符小于32Byte
    u8 m_btmac[6];      
    u8 m_nvram[120];    
}str_btparam_t;
#define STR_BTPARAM_LEN    sizeof(str_btparam_t) //STR_BTPARAM_LEN必须小于BTPARAMLEN_MAX
#define BTPARAM_NAME_BIT    (1<<0)
#define BTPARAM_MAC_BIT     (1<<1)
#define BTPARAM_NVRAM_BIT   (1<<2)


typedef struct _PARAM_BTFIRMWAREUPDATE
{ 
    u8 filename[20];          //0x60 for KEYA，0x61 for KEYB
    u32 offset;
}PARAM_BTFIRMWAREUPDATE;

void dev_bt_task_process(void);
s32 dev_bt_open(void);
s32 dev_bt_close(void);
s32 dev_bt_write(u8 *wbuf, s32 wlen);
s32 dev_bt_read(u8 *rbuf, s32 rlen);
s32 dev_bt_get_status(void);
s32 dev_bt_disconnect(const u8 *mac);
s32 dev_bt_ioctl(u32 nCmd, u32 lParam, u32 wParam);
s32 dev_bt_check(void);

#endif
