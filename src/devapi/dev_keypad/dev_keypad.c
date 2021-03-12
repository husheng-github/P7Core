/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    ���ļ�����KEYPAD�������ӿ�
  
****************************************************************************/
#include "devglobal.h"
#include "drv_keypad.h"
#include "ddi/ddi_audio.h"
#include "ddi/ddi_key.h"

#if(KEYPAD_EXIST==1)
#define KEYPAD_VALUE_MAX 8

typedef struct _st_keypad
{ 
    u32 keytmp[2];
    u32 pressnum;
    u32 pressnum1;      //��¼long ��down��ʱ��
    u32 buf[KEYPAD_VALUE_MAX];
    u8 enable_f;
//    u8 newkey;
    u8 holdkey;
    u8 waitlose;
    u8 head;
    u8 tail;
    u8 keyscansn;//num;
    u8 timercnt;
//    u8 keyrow;
    u8 keycol;
    u8 keycol1;     //���������
    u8 keyevent;    //key event
}st_keypad;

static st_keypad   g_st_keypad;
static s32 g_keypad_fd = -1;
static volatile u8 g_keypad_pwr_en = 1;
static const u8 *gp_keypad_tab=NULL;
static u32 g_powerkey_flg = 0;

#define POWER_PHYSICS       31      //����POWER�������ֵ

#define KEYTAB_MAX          32
#if 0
const u8 keypad_tab[KEYTAB_MAX]=
{
  //0,        1,        2,        3,            4,          5  6  7
    FUNCTION,       ESC,      CLEAR,    ENTER,     0,       0, 0, 0,
    //  8, 
    ALPHA,       DIGITAL3, DIGITAL6, DIGITAL9,DOWN_OR_11  ,0, 0, 0,  
    // 16 
    F1,       DIGITAL2, DIGITAL5, DIGITAL8,  DIGITAL0,0, 0, 0,
    //24
    0,        DIGITAL1, DIGITAL4, DIGITAL7,UP_OR_10  ,   0, 0, POWER,
};
#endif
const u8 keypad_tab_t1[KEYTAB_MAX]=
{
  //0,        1,        2,        3,            4,          5  6  7
    FUNCTION, ESC,      CLEAR,    ENTER,     0,       0, 0, 0,
    //  8, 
    ALPHA,    DIGITAL3, DIGITAL6, DIGITAL9,DOWN_OR_11  ,0, 0, 0,  
    // 16 
    F1,       DIGITAL2, DIGITAL5, DIGITAL8,UP_OR_10, 0, 0, 0,
    //24
    DIGITAL0, DIGITAL1, DIGITAL4, DIGITAL7,  0,   0, 0, POWER,
};

const u8 keypad_tab_t3[KEYTAB_MAX]=
{
  //0,        1,        2,        3,            4,          5  6  7
    FUNCTION,       ESC,      CLEAR,    ENTER,  0,       0, 0, 0,
    //  8, 
    DOWN_OR_11,  DIGITAL3, DIGITAL6, DIGITAL9,  0,0, 0, 0,  
    // 16 
    UP_OR_10,    DIGITAL2, DIGITAL5, DIGITAL8,  0,0, 0, 0,
    //24
    DIGITAL0,    DIGITAL1, DIGITAL4, DIGITAL7,  0,   0, 0, POWER,
};

const u8 keypad_tab_p6[KEYTAB_MAX]=
{
  //0,        1,        2,        3,            4,          5  6  7
    0,         KEY_PLUS ,      KEY_MINUS ,      KEY_PLUS_MINUS, KEY_CFG, KEY_CFG_PLUS, KEY_CFG_MINUS, 0,
    //  8, 
    KEY_PAPER, KEY_PAPER_PLUS, KEY_PAPER_MINUS, 0,              KEY_CFG_PAPER, 0, 0, 0,  
    // 16 
    KEY_TBD, 0, 0, 0, 0, 0, 0, 0,
    //24
    0, 0, 0, 0, 0, 0, 0, POWER,
};

const u8 long_keypad_tab_p6[KEYTAB_MAX]=
{
  //0,        1,        2,        3,            4,          5  6  7
    0,       KEY_PLUS_LONG ,      KEY_MINUS_LONG ,0,     KEY_CFG_LONG, 0, 0, 0,
    //  8, 
    KEY_PAPER_LONG, 0, 0, 0, 0, 0, 0, 0,  
    // 16 
    KEY_TBD_LONG, 0, 0, 0, 0, 0, 0, 0,
    //24
    0, 0, 0, 0, 0, 0, 0, POWER,
};


const u8 bitcover[16] = 
{ 
    //0,    1,    2,    3,    4,    5,    6,    7
    0x0f, 0x00, 0x01, 0x0f, 0x02, 0x0f, 0x0f, 0x0f,
    //8,    9,   10,   11,    12,   13,   14,   15
    0x03, 0x0f, 0x0f, 0x0f, 0x02, 0x0f, 0x0f, 0x0f,
    
          
};

#endif
/****************************************************************************
**Description:       �����豸��ʼ��
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170524
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_keypad_init(void)
{
    u8 machineid;
    
    #if(KEYPAD_EXIST==1)
    g_st_keypad.enable_f = 0;
    g_st_keypad.timercnt = 0;
    g_st_keypad.head = 0;
    g_st_keypad.tail = 0;
    g_st_keypad.waitlose = 0;
    g_st_keypad.pressnum = 0;
    g_st_keypad.holdkey = 0;
    g_st_keypad.keytmp[0] = 0;
    g_st_keypad.keytmp[1] = 0xFFFFFFFF;
//    g_st_keypad.keyrow = 0;
    g_st_keypad.keycol = 0;
    g_keypad_pwr_en = 1;
    drv_keypad_init();
    #if 0
    machineid = dev_misc_getmachinetypeid();
    if(machineid == MACHINE_TYPE_ID_T3)
    {
        gp_keypad_tab = keypad_tab_t3;
    }
    else if(machineid == MACHINE_TYPE_ID_T1)
    {
        gp_keypad_tab = keypad_tab_t1;
    }
    else
    {
        gp_keypad_tab = keypad_tab_t1;
    }
    #else
    gp_keypad_tab = keypad_tab_p6;
    #endif
    #endif  
    
}
/****************************************************************************
**Description:       ������Դ��ɨ��ʹ�ܿ���
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170524
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_keypad_pwr_enable(u8 flg)
{
    #if(KEYPAD_EXIST==1)
    g_keypad_pwr_en = flg;
    #endif
}
/****************************************************************************
**Description:       ����ɨ��
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170524
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_keypad_insetbuf(u32 key)
{
    u8 i;

     //�����Դ��
    i = g_st_keypad.head+1;
    if(i>=KEYPAD_VALUE_MAX)i = 0;
    
    if(i != g_st_keypad.tail)
    {
        g_st_keypad.buf[g_st_keypad.head] = key;
        g_st_keypad.head = i;
    }
}

s32 dev_keypad_scan(u8 num)
{
    #if(KEYPAD_EXIST==1)  
    u8 i;// = 0;
    u8 keyrow;// = 0;
    u8 powerkey;
    u8 machineid;
    //dev_debug_printf("1");

    if(0 == g_st_keypad.enable_f)
    {
        return 0;
    }

    if(num == 0)
    {
        g_st_keypad.keyscansn = 1;
        return 0;
        
    }
    else if(num == 1)
    {
        if(g_st_keypad.keyscansn == 1)
        {

            //����ROW���,col����
            drv_keypad_colin();
            drv_keypad_rowout(0);
            g_st_keypad.keyscansn = 2;
        }
        else
        {
            g_st_keypad.keyscansn = 0;
            drv_keypad_colin();
            drv_keypad_rowin();
        }
        return 0;
    }
    else if(num == 2)
    {
        if(g_st_keypad.keyscansn == 2)
        {
            //��COL��ֵ,
            g_st_keypad.keycol = drv_keypad_colread();
            g_st_keypad.keyscansn = 3;
            drv_keypad_rowin();
            drv_keypad_colout(0);
        }            
        else
        {
            g_st_keypad.keyscansn = 0;
            drv_keypad_colin();
            drv_keypad_rowin();
        }
        return 0;
    }
    else if(num == 3)
    {
        if(g_st_keypad.keyscansn == 3)
        {
            //��ȡROW��ֵ
            keyrow = drv_keypad_rowread();
            g_st_keypad.keyscansn = 0;
            drv_keypad_rowin();
            drv_keypad_colin();


            if(g_keypad_pwr_en)
            {
                powerkey = drv_keypad_get_powerkey();
            }
            else
            {
                powerkey = 0;
            }
        }
        else
        {
            g_st_keypad.keyscansn = 0;
            drv_keypad_rowin();
            drv_keypad_colin();
            return 0;
        }
    }
    else
    {
        g_st_keypad.keyscansn = 0;
        drv_keypad_rowin();
        drv_keypad_colin();
        
        return -1;
    }
    if((keyrow==0) && (0 == powerkey)&&(0==g_st_keypad.keycol1))
    {
        //����up�¼�
        if((g_st_keypad.keyevent == KEY_LONG_PRESS) || (g_st_keypad.keyevent == KEY_EVENT_DOWN))
        {
            g_st_keypad.keyevent = KEY_EVENT_UP;
            dev_keypad_insetbuf((g_st_keypad.keytmp[1])|(g_st_keypad.keyevent<<8));
        }
        //û�м�����
        
        drv_keypad_irq_enable();
        g_st_keypad.pressnum=0;
        g_st_keypad.pressnum1=0;
        if(0 != g_st_keypad.waitlose)  
        {
            g_st_keypad.keytmp[0] = 0;
            g_st_keypad.holdkey = 0;
            g_st_keypad.waitlose = 0;   
        }
        
    }
    else
    {
        //if(0 == g_st_keypad.waitlose)//��ϼ���Ҫ
        {
            if(0 != powerkey)
            {
                //˵���е�Դ������
                g_st_keypad.keytmp[0] = POWER_PHYSICS;
            }
            else
            {

                if(keyrow!=0)
                {
                    g_st_keypad.keytmp[0] = keyrow;
                    //dev_debug_printf("i:%d\r\n",keyrow);
                }

            }
            if(0xff == g_st_keypad.keytmp[0])
            {
                return 0;
            }
        }
        if(g_st_keypad.pressnum == 0)
        {
            g_st_keypad.keytmp[1] = g_st_keypad.keytmp[0];
            g_st_keypad.pressnum++;    
        }
        else
        {
            if(g_st_keypad.keytmp[1]!=g_st_keypad.keytmp[0])
            {
                #if 1//��ϼ�
                if(!drv_keypad_row_ismultiplekey(g_st_keypad.keytmp[0]) && drv_keypad_row_ismultiplekey(g_st_keypad.keytmp[1]))
                {
                    g_st_keypad.pressnum1++;
                    //dev_debug_printf("long to down time:%d",g_st_keypad.pressnum1);
                    if(g_st_keypad.pressnum1 > LONGKEY_TIMER)
                    {
                        g_st_keypad.pressnum1 = 0;
                    }
                    else
                    {
                        return 0;
                    }
                    
                    
                }
                g_st_keypad.waitlose = 0;
                g_st_keypad.keytmp[1] = g_st_keypad.keytmp[0];
                g_st_keypad.pressnum++;  
                #else
                g_st_keypad.keytmp[0] = 0;
                g_st_keypad.keytmp[1] = 0xFFFFFFFF;
                g_st_keypad.pressnum = 0;
                #endif
                
            }
            {
                if(g_st_keypad.pressnum>2)
                {
                    if(g_st_keypad.waitlose==0)
                    {
                        g_st_keypad.keyevent = KEY_EVENT_DOWN;
                        dev_keypad_insetbuf((g_st_keypad.keytmp[1])|(g_st_keypad.keyevent<<8));
                        g_st_keypad.waitlose=1;
                    }
                }
                
                g_st_keypad.pressnum++;

                #if 0
                  //�ж�POWER����200�ĳ�300, 3��
                if(g_st_keypad.pressnum==300)    
                {
                    if(POWER_PHYSICS == g_st_keypad.keytmp[1])  
                    {
                      g_st_keypad.keyevent = KEY_LONG_PRESS;
                      dev_keypad_insetbuf(g_st_keypad.keytmp[1]);
                      //SP��λAP
                      //drv_power_set_latch(0);
                    }
                }
                #endif
                //��������
                if((g_st_keypad.pressnum>2)&&((g_st_keypad.pressnum%LONGKEY_TIMER)==0))
                {
                    g_st_keypad.holdkey = 1;
                    g_st_keypad.keyevent = KEY_LONG_PRESS;
                    dev_keypad_insetbuf((g_st_keypad.keytmp[1])|(g_st_keypad.keyevent<<8));
                }

            }
        }
    }
    #endif  
    return 0;
}
/****************************************************************************
**Description:       �򿪰����豸
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170525
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_keypad_open(void)
{
    #if(KEYPAD_EXIST == 1)  
    if(g_keypad_fd < 0)
    {
        g_keypad_fd = 0;
        dev_keypad_init();
        g_st_keypad.enable_f = 1;
        //drv_keypad_irq_enable();
    }
    return DEVSTATUS_SUCCESS;
    #else
    return DEVSTATUS_ERR_FAIL;
    #endif  
  
}
/****************************************************************************
**Description:       �򿪰����豸
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170525
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_keypad_close(void)
{
    #if(KEYPAD_EXIST==1)
    if(g_keypad_fd >= 0)
    {
        g_keypad_fd = -1;
        g_st_keypad.enable_f = 0;
        drv_keypad_colin();
        drv_keypad_rowin();
    }
    return DEVSTATUS_SUCCESS;
    #else
    return DEVSTATUS_ERR_FAIL;
    #endif  
}
/****************************************************************************
**Description:       �������
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170525
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_keypad_clear(void)
{
    #if(KEYPAD_EXIST==1)
    g_st_keypad.tail = g_st_keypad.head;
    #endif  
    return DEVSTATUS_SUCCESS;
}
/****************************************************************************
**Description:       ������
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170525
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_keypad_read(u32 *rkey)
{
    #if(KEYPAD_EXIST == 1)
    u32 key_code;
    u8 key;

//    dev_power_test();
    //drv_mag_decode();
    *rkey = 0;
    if(g_st_keypad.tail!=g_st_keypad.head)
    {
        key_code = g_st_keypad.buf[g_st_keypad.tail++];
        key = (key_code & 0xff);
        
        if(g_st_keypad.tail>=KEYPAD_VALUE_MAX)
        {
            g_st_keypad.tail = 0;
        }
        
        if(key < KEYTAB_MAX)
        {
            #if 1
            *rkey =(gp_keypad_tab[key] & 0xff) | (key_code & 0xff00);
            //*rkey =key_code; //(gp_keypad_tab[key])|(key_code >> 8);
            #else
            if(g_st_keypad.holdkey == 1)
            {
                //KEYPAD_DEBUG("long");
                *rkey = long_keypad_tab_p6[key];
            }
            else
            {
                *rkey = gp_keypad_tab[key];//keypad_tab[key]; 
            }
            #endif
            //KEYPAD_DEBUG("key index:%d,key = %02x,key_event:%d\r\n",key,gp_keypad_tab[key],(key_code >> 8));
            
            return 1;
        }
        else
        {
            *rkey = 0;
            return 0;
        }
    }    
       return 0;
    #else
    return 0;
    #endif  
}
/****************************************************************************
**Description:       ��������
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170525
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_keypad_read_beep(u32 *rkey)
{
    #if(KEYPAD_EXIST == 1)
    return 0;
    #endif  
}
/****************************************************************************
**Description:       ��ȡ��ǰ����ֵ
**Input parameters:    
**Output parameters: rbuf�����ֵ
**Returned value:   ���ذ�������
**Created by:        pengxuebin,20170525
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 dev_keypad_get_currkey(u8 *rbuf)
{
    u32 i;
    s32 ret;
    u8 tmp[4];

    if(g_keypad_fd < 0)
    {
        return 0;
    }

    ret = 0;
    if(g_st_keypad.pressnum>2)
    {
        i = g_st_keypad.keytmp[1];
        tmp[0] = (u8)(i);
        //tmp[1] = (i>>16);
        //tmp[2] = (i>>8);
        //tmp[3] = (i>>0);
        ret = 0;
        for(i=0; i<1; i++)
        {

            if(tmp[i]<KEYTAB_MAX)
            {
                tmp[i] = gp_keypad_tab[tmp[i]];//keypad_tab[tmp[i]];  
            }
            else
            {
                tmp[i] = 0;
            } 
            if(tmp[i] != 0)
            {
                rbuf[ret++] = tmp[i];
            }
        }
        return ret;
        
    }
    return 0;
}


/**
 * @brief ����power key ����״̬
 * @param[in] lParam 
 * @retval  DDI_OK ��
 */
s32 dev_keypad_set_powerkey_status(u32 lParam)
{
    g_powerkey_flg = lParam;
    return DDI_OK;
}

/**
 * @brief ��ȡpower key ����״̬
 * @param[in] ��
 * @retval  ����״̬
 */
s32 dev_keypad_get_powerkey_status(void)
{
    return g_powerkey_flg;
}


s32 dev_keypad_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    s32 ret = DDI_EINVAL;
    
    switch(nCmd)
    {
    case DDI_KEY_GET_CURRKEY:
        ret = dev_keypad_get_currkey((u8*)wParam);
        break;
    case DDI_KEY_CTL_POWERKEY_FLG:
        ret = dev_keypad_set_powerkey_status(lParam);
        break;
    case DDI_KEY_GET_POWERKEY_FLG:
        ret = dev_keypad_get_powerkey_status();
        break;
    default:
        break;
    }
    return ret;
}


