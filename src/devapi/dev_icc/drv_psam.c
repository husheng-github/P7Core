/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    ���ļ�����psam���߼���������ӿ�
  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_psam_io.h"
#include "drv_psam.h"
#if(PSAM_EXIST==1)
typedef enum _PSAM_STATUS
{
    PSAM_STATUS_FREE = 0,       //����״̬
    PSAM_STATUS_WRITE = 1,      //����д
    PSAM_STATUS_READ  = 2,      //���ڶ�
    PSAM_STATUS_WAIT  = 3,      //��ʱ�ȴ�
}psam_status_t;
typedef struct _str_psam_status
{
    volatile psam_status_t m_busying;   //��¼��ǰ����      
    volatile u8 m_bitstep;              //��¼��ǰ����
    u8 m_bit;                           //��¼bit��ǰֵ����Ҫд��λֵ                        
//    u8 m_psamid;                        //psam�����
    u8 m_psamT;                         //0xFF,��λTS,0xFE,��λ�����ֽ�,0����T0, 1����T1
    u8 m_currdata;                      //��ǰ����
    u8 m_errcnt;                        //�������
    u8 m_parity;                        //��żУ���־
    u8 m_result;                        //�������,0:�ɹ�,
                                        //         1:��żУ���, 
                                        //         
                                        //         20:������ʱ
                                        //         21:
    void (*m_func)(void);            //����ָ��
}str_psam_status_t;

static str_psam_status_t g_psam_status;   //psam����
#define PSAM_TIMER_ID       4           //��ʱ��4����psam���ж�

#define PSAM_TIMER_CLK      (SYSCTRL->PCLK_1MS_VAL*1000)
//#define PSAM_FD1_ETU        (372*1000000/PSAM_CLK_Frequency)    //FD=1ʱ,1etu=93us
static u32 g_psam_100clk;
static str_icc_info_t g_psam_info[PSAM_NUM_MAX];

static s32 g_psam_fd[PSAM_NUM_MAX];
static u8 g_curr_slot=0;    //��¼��ǰʹ�õĿ�Ƭ��

u8 const psam_calc_1_tab[16]={0,1,1,2,1,2,2,3,
                            1,2,2,3,2,3,3,4,
                            };
static icc_content_t *gp_icc_content=NULL;
/****************************************************************************
**Description:       ���õ�ǰ����
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170719
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_psam_setslot(u8 slot)
{
    if(slot < PSAM_NUM_MAX)
    {
        g_curr_slot = slot;
        return 0;
    }
    else
    {
        return -1;
    }
}
/****************************************************************************
**Description:       ����ʱ������
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170711
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static u32 drv_psam_timer_calc_clk(s32 fd, u32 netu)
{
    u32 i;
    
    i = (372*g_psam_100clk*netu)/100;
    if(fd == 1)
    {
        i >>= 1;
    }
    else if(fd == 1)
    {
        i >>= 2;
    }
    return i;
}
static u32 drv_psam_timer_calc_ms(s32 fd, u32 netu)
{
 //   u32 i;

 //   i = (drv_psam_timer_calc_clk(fd, netu));
//PSAM_DEBUG("g_clk=%d, fd=%d, netu=%d, i=%d\r\n", g_psam_100clk, fd, netu, i);    
    return ((drv_psam_timer_calc_clk(fd, netu)+SYSCTRL->PCLK_1MS_VAL-1)/(SYSCTRL->PCLK_1MS_VAL));
}
static u8 maths_calc_lrc(u8 initdata, u8 *src, s32 srclen)
{
    u8 data;
    s32 i;

    data = initdata;
    for(i=0; i<srclen; i++)
    {
        data ^= src[i];
    }
    return data;
}
/****************************************************************************
**Description:       ������ʱ��
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170617
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static void drv_psam_timer_run(void)
{
    dev_timer_int_clear(PSAM_TIMER_ID);
    dev_timer_run(PSAM_TIMER_ID);
}
/****************************************************************************
**Description:       �رն�ʱ��
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170628
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static void drv_psam_timer_stop(void)
{
    dev_timer_stop(PSAM_TIMER_ID);
}
/****************************************************************************
**Description:       ���ö�ʱ��
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170628
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static void drv_psam_timer_set(u32 nclk)
{
    dev_timer_setvalue_clk(PSAM_TIMER_ID, nclk);
}
/****************************************************************************
**Description:        PSAM1����ʱ������ȴ�����
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170711
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_psam_timer_handler_wait(void)
{
    drv_psam_timer_stop();
    g_psam_status.m_busying = PSAM_STATUS_FREE;     //ʱ�䵽
}
/****************************************************************************
**Description:        PSAM1����ʱ�������ʹ���
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170711
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
extern void drv_psam_timer_handler_send_b0_7(void);
extern void drv_psam_timer_handler_send_parity(void);
extern void drv_psam_timer_handler_send_stop(void);
extern void drv_psam_timer_handler_send_ACK(void);


void drv_psam_timer_handler_send_sta(void)
{
//    u32 i;

    //GPIO������Ϊ���0
    drv_psam_io_direction_set(g_curr_slot, 1, 0);
//drv_psam_testpin_xor(0);    
 #if 0  
    i = drv_psam_timer_calc_clk(g_psam_info[g_curr_slot].m_icfd, 1); 
    drv_psam_timer_stop();
    drv_psam_timer_set(i);
    drv_psam_timer_run();
  #endif 
    g_psam_status.m_bitstep = 0;
    g_psam_status.m_func = drv_psam_timer_handler_send_b0_7;
    if(g_psam_info[g_curr_slot].m_Nbit == 0)
    {   //����Լ��,�ߵ�ƽΪ1,�ȷ���λ���󷢸�λ
        if(g_psam_status.m_currdata&(1<<g_psam_status.m_bitstep))
        {
            g_psam_status.m_bit = 1;
        }
        else
        {
            g_psam_status.m_bit = 0;
        }
    }
    else
    {
        //����Լ��,�͵�ƽΪ1���ȷ���λ���󷢵�λ
        if(g_psam_status.m_currdata&(0x80>>g_psam_status.m_bitstep))
        {
            g_psam_status.m_bit = 0;
        }
        else
        {
            g_psam_status.m_bit = 1;
        }
    }
}
void drv_psam_timer_handler_send_b0_7(void)
{
    
    drv_psam_io_set_value(g_curr_slot, g_psam_status.m_bit);
//drv_psam_testpin_xor(0);    
  #if 0  
    u32 i;
    i = drv_psam_timer_calc_clk(g_psam_info[g_curr_slot].m_icfd, 1); 
    drv_psam_timer_stop();
    drv_psam_timer_set(i);
    drv_psam_timer_run();
  #endif   
    g_psam_status.m_bitstep++;
    if(g_psam_status.m_bitstep<8)
    {   //׼����һ������
        if(g_psam_info[g_curr_slot].m_Nbit == 0)
        {   //����Լ��,�ߵ�ƽΪ1,�ȷ���λ���󷢸�λ
            if(g_psam_status.m_currdata&(1<<g_psam_status.m_bitstep))
            {
                g_psam_status.m_bit = 1;
            }
            else
            {
                g_psam_status.m_bit = 0;
            }
        }
        else
        {
            //����Լ��,�͵�ƽΪ1���ȷ���λ���󷢵�λ
            if(g_psam_status.m_currdata&(0x80>>g_psam_status.m_bitstep))
            {
                g_psam_status.m_bit = 0;
            }
            else
            {
                g_psam_status.m_bit = 1;
            }
        }
    }
    else  
    {   //��һ������дУ��
        g_psam_status.m_func = drv_psam_timer_handler_send_parity;
        g_psam_status.m_bit = (psam_calc_1_tab[g_psam_status.m_currdata&0x0f]+psam_calc_1_tab[(g_psam_status.m_currdata>>4)&0x0f])&0x01;//������żУ��
        if(g_psam_info[g_curr_slot].m_Nbit != 0)   
        {
            g_psam_status.m_bit ^= 0x01;
        }
    }
}
void drv_psam_timer_handler_send_parity(void)
{
    
    drv_psam_io_set_value(g_curr_slot, g_psam_status.m_bit);
//drv_psam_testpin_xor(0);    
  #if 0  
    u32 i;
    i = drv_psam_timer_calc_clk(g_psam_info[g_curr_slot].m_icfd, 1); 
    drv_psam_timer_stop();
    drv_psam_timer_set(i);
    drv_psam_timer_run();
  #endif   
    g_psam_status.m_func = drv_psam_timer_handler_send_stop;
        
}
void drv_psam_timer_handler_send_stop(void)
{

    drv_psam_io_set_value(g_curr_slot, 1);
//drv_psam_testpin_xor(0);    
  #if 0  
    u32 i;
    i = drv_psam_timer_calc_clk(g_psam_info[g_curr_slot].m_icfd, 1); 
    drv_psam_timer_stop();
    drv_psam_timer_set(i);
    drv_psam_timer_run();
  #endif 
    if(g_psam_status.m_psamT == 0)
    {
        //����T0��, ���������
        g_psam_status.m_func = drv_psam_timer_handler_send_ACK;
        drv_psam_io_direction_set(g_curr_slot, 0, 0);    //����״̬
    }
    else
    {
//    g_psam_status.m_func = drv_psam_timer_handler_send_stop1;
    
        g_psam_status.m_result = ICC_PROC_OK;   //���ͳɹ�
        drv_psam_timer_stop();
        g_psam_status.m_func = NULL;
        g_psam_status.m_busying = PSAM_STATUS_FREE;     //�ֽڷ������
    }
}

void drv_psam_timer_handler_send_ACK(void)
{
//    int i;

    if(0 == drv_psam_io_get_value(g_curr_slot))
    {
        g_psam_status.m_result = ICC_PROC_T0WEITEERR;   //ACK����Ҫ�ط�
        
    }
    else
    {
        g_psam_status.m_result = ICC_PROC_OK;   //���ͳɹ�
    }
//drv_psam_testpin_xor(0);    
    drv_psam_timer_stop();
    g_psam_status.m_func = NULL;
    g_psam_status.m_busying = PSAM_STATUS_FREE;     //�ֽڷ������    
}
/****************************************************************************
**Description:        PSAM1����ʱ�������������λ
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170711
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
extern void drv_psam_timer_handler_recb1_7(void);
extern void drv_psam_timer_handler_recb8(void);
extern void drv_psam_timer_handler_rec_err1(void); 
extern void drv_psam_timer_handler_rec_err2(void);

void drv_psam_timer_handler_recb0(void)
{
    u32 i;

    g_psam_status.m_bit = drv_psam_io_get_value(g_curr_slot);
//drv_psam_testpin_xor(0);    
    i = drv_psam_timer_calc_clk(g_psam_info[g_curr_slot].m_icfd, 1); 
    drv_psam_timer_stop();
    drv_psam_timer_set(i);
    drv_psam_timer_run();
    g_psam_status.m_func = drv_psam_timer_handler_recb1_7;
    if(g_psam_info[g_curr_slot].m_Nbit == 0)
    {                           //����Լ��,�ߵ�ƽΪ1,�ȷ���λ���󷢸�λ
        if(g_psam_status.m_bit) 
        {
            g_psam_status.m_currdata = 0x80;
        }
        else
        {
            g_psam_status.m_currdata = 0x00;
        }
    }
    else    //����Լ��,�͵�ƽΪ1���ȷ���λ���󷢵�λ
    {
        if(0 == g_psam_status.m_bit) 
        {
            g_psam_status.m_currdata = 0x01;
        }
        else
        {
            g_psam_status.m_currdata = 0x00;
        }
    }
    g_psam_status.m_bitstep = 1;            //�յ�BIT0
}
void drv_psam_timer_handler_recb1_7(void)
{
    
    g_psam_status.m_bit = drv_psam_io_get_value(g_curr_slot);
//drv_psam_testpin_xor(0);    
  #if 0 
    u32 i; 
    i = drv_psam_timer_calc_clk(g_psam_info[g_curr_slot].m_icfd, 1); 
    drv_psam_timer_stop();
    drv_psam_timer_set(i);
    drv_psam_timer_run();
  #endif
    if(g_psam_info[g_curr_slot].m_Nbit == 0)
    {                           //����Լ��,�ߵ�ƽΪ1,�ȷ���λ���󷢸�λ
        g_psam_status.m_currdata>>=1;
        if(g_psam_status.m_bit) 
        {
            g_psam_status.m_currdata |= 0x80;
        }
        /*else
        {
            g_psam_status.m_currdata = 0x00;
        }*/
    }
    else    //����Լ��,�͵�ƽΪ1���ȷ���λ���󷢵�λ
    {
        g_psam_status.m_currdata<<=1;
        if(0 == g_psam_status.m_bit) 
        {
            g_psam_status.m_currdata |= 0x01;
        }
        /*else
        {
            g_psam_status.m_currdata = 0x00;
        }*/
    }
    g_psam_status.m_bitstep++;
    if(g_psam_status.m_bitstep>7)//˵���յ���B0����B7,
    {
        
        g_psam_status.m_func = drv_psam_timer_handler_recb8;    //������żУ��λ
    }
            
}
void drv_psam_timer_handler_recb8(void) 
{
    u32 j;
    
    g_psam_status.m_bit = drv_psam_io_get_value(g_curr_slot);
//drv_psam_testpin_xor(0);    
  #if 0  
    u32 i,
    i = drv_psam_timer_calc_clk(g_psam_info[g_curr_slot].m_icfd, 1); 
    drv_psam_timer_stop();
    drv_psam_timer_set(i);
    drv_psam_timer_run();
  #endif  
    j=(psam_calc_1_tab[g_psam_status.m_currdata&0x0f]+psam_calc_1_tab[(g_psam_status.m_currdata>>4)&0x0f])&0x01;//������żУ��
    if(g_psam_status.m_psamT != 0xFF)
    {   
        if(0 != g_psam_info[g_curr_slot].m_Nbit) //������
        {                          
            j ^= 0x01;
        }        
    }
    else
    {   //��λ��TS�ֽ�
        if(0x03 == g_psam_status.m_currdata)
        {
            j ^= 0x01;
            g_psam_info[g_curr_slot].m_Nbit = 1;     //TS = 0x3F
            g_psam_status.m_currdata = 0x3F;
        }
    }
    if(g_psam_status.m_bit)
    {
        j ^= 0x01;         
    }
    if(0 != j)
    {
        //��żУ���
        g_psam_status.m_result = ICC_PROC_T1PARITYERR;  //��żУ���
        if(g_psam_status.m_psamT == 0)
        {
            //����T1��,��żУ����跢Ӧ���ź�
            g_psam_status.m_func = drv_psam_timer_handler_rec_err1;
            return;
        }
    }
    else
    {
        g_psam_status.m_result = ICC_PROC_OK;
    }
    drv_psam_timer_stop();
    g_psam_status.m_func = NULL;
    g_psam_status.m_busying = PSAM_STATUS_FREE;     //�ֽڽ������
}
void drv_psam_timer_handler_rec_err1(void) 
{
    u32 i;
    
    //GPIO������Ϊ���0
    drv_psam_io_direction_set(g_curr_slot, 1, 0);
    //���ö�ʱ��1.5etu
    i = drv_psam_timer_calc_clk(g_psam_info[g_curr_slot].m_icfd, 15); 
    i = i/10;
    drv_psam_timer_stop();
    drv_psam_timer_set(i);
    drv_psam_timer_run();
    g_psam_status.m_func = drv_psam_timer_handler_rec_err2;
            
}
void drv_psam_timer_handler_rec_err2(void) 
{
//    u32 i;
    
    //GPIO������Ϊ���0
    drv_psam_io_set_value(g_curr_slot, 1);
    //GPIO����Ϊ����
    drv_psam_io_direction_set(g_curr_slot, 0, 0);
    drv_psam_timer_stop();
    g_psam_status.m_func = NULL;
    g_psam_status.m_result = ICC_PROC_T1PARITYERR;     //T0��żУ���
}
/****************************************************************************
**Description:        PSAM1����ʱ��������
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170628
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static void drv_psam_timer_handler(void *data)
{
    if(g_psam_status.m_func != NULL)
    {
        g_psam_status.m_func();
    }
}
/****************************************************************************
**Description:        PSAM1���ⲿ�жϴ�����
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170711
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static void drv_psam_slot0_handler(void *param)
{
    u32 i;
    
    if(g_curr_slot == 0)
    {
//drv_psam_testpin_set(0, 0);         
        g_psam_status.m_func = drv_psam_timer_handler_recb0;
        i = drv_psam_timer_calc_clk(g_psam_info[0].m_icfd, 135);    //1.40etu
        i = i/100;
        drv_psam_timer_stop();
        drv_psam_timer_set(i);
        drv_psam_timer_run();
//        g_psam_status.m_bitstep = 0;
        //��ֹ�ж�
        drv_psam_io_irq_enable(0, 0);
    }
    
}
/****************************************************************************
**Description:        PSAM2���ⲿ�жϴ�����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static void drv_psam_slot1_handler(void *param)
{
    u32 i;
    
    if(g_curr_slot == 1)
    {
        g_psam_status.m_func = drv_psam_timer_handler_recb0;
        i = drv_psam_timer_calc_clk(g_psam_info[1].m_icfd, 135);    //1.40etu
        i = i/100;
        drv_psam_timer_stop();
        drv_psam_timer_set(i);
        drv_psam_timer_run();
        //��ֹ�ж�
        drv_psam_io_irq_enable(1, 0);
    }
    
}
/****************************************************************************
**Description:       PSAM���ȴ�ETU
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170617
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_psam_wait_opt_ent(u32 ms)
{
    u32 timerid;

    timerid = dev_user_gettimeID();
    while(PSAM_STATUS_FREE != g_psam_status.m_busying)
    {
        if(dev_user_querrytimer(timerid, ms))
        {
            PSAM_DEBUG("timerout(ms=%d)\r\n", ms);
            drv_psam_timer_stop();
            //��ֹGPIO���ж�
            drv_psam_io_irq_enable(g_curr_slot, 0);
            return ICC_PROC_NORESPOND;
        }
        //�жϿ�Ƭ�γ�,psam��û���ж�
    }
    return 0;
}
/****************************************************************************
**Description:       PSAM���ȴ�ETU
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170617
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_psam_waitetu(u32 netu)
{
    u32 i;
    s32 ret=0;
    
    if(netu != 0)
    {
        i = drv_psam_timer_calc_clk(g_psam_info[g_curr_slot].m_icfd, netu);
        g_psam_status.m_busying = PSAM_STATUS_WAIT;
        g_psam_status.m_bitstep = 0;
        g_psam_status.m_func = drv_psam_timer_handler_wait;
        drv_psam_timer_stop();
        drv_psam_timer_set(i);
        drv_psam_timer_run();
        i = (i/SYSCTRL->PCLK_1MS_VAL);
        ret = drv_psam_wait_opt_ent(20+i);
    }
    return ret;
}
/****************************************************************************
**Description:       PSAM������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170628
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_psam_active(u8 mod)
{
//    s32 ret;
    
  #if 0  
    if(psamno >= PSAM_NUM_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
  #endif  
PSAM_DEBUG("mod=%d,slot=%d\r\n", mod, g_curr_slot);
    if(mod == 0)
    {
        //�临λ
        //�ϵ�
        drv_psam_power_ctl(g_curr_slot, 1);
        //GPIO���ó�����״̬
        drv_psam_io_direction_set(g_curr_slot, 0, 0);
        //
        dev_user_delay_us(93*3);
        drv_psam_clk_ctl(g_curr_slot, 1);    //��
    }  
    else
    {
        //�ȸ�λ
        drv_psam_rst_ctl(g_curr_slot, 0);
    }
    drv_psam_waitetu(113);
    drv_psam_rst_ctl(g_curr_slot, 1);
    return 0;
}
/****************************************************************************
**Description:       PSAM��ȥ����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170712
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_psam_inactive()
{
    //�µ�
    //GPIO��Ϊ����״̬
    drv_psam_io_direction_set(g_curr_slot, 0, 0);
    dev_user_delay_us(5);
    drv_psam_rst_ctl(g_curr_slot, 0);
    dev_user_delay_us(5);
    drv_psam_clk_ctl(g_curr_slot, 0);
    dev_user_delay_us(5);
    drv_psam_io_direction_set(g_curr_slot, 1, 0);
    drv_psam_power_ctl(g_curr_slot, 0);
    g_psam_info[g_curr_slot].m_resettype = 0;
    return 0;
}
/****************************************************************************
**Description:       PSAM�����ո�λ����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170717
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_psam_send_byte(u8 data)
{
    s32 i;
    s32 ret;
     
    i = drv_psam_timer_calc_clk(g_psam_info[g_curr_slot].m_icfd, 1);
    g_psam_status.m_busying = PSAM_STATUS_WRITE;
    g_psam_status.m_bitstep = 0;
    g_psam_status.m_func = drv_psam_timer_handler_send_sta;
    g_psam_status.m_currdata = data;
    drv_psam_timer_stop();
    drv_psam_timer_set(i);
    drv_psam_timer_run();
    ret = drv_psam_wait_opt_ent(10);   
    if(ret == 0)
    {
        ret = g_psam_status.m_result;
//dev_debug_printf("d=%02X,%d\r\n", g_psam_status.m_currdata, ret);          
        
    }  
    return ret;
}
/****************************************************************************
**Description:       PSAM��T0������������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170717
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_psam_send_nbytes_t0(u8 *wbuf, s32 wlen, u32 bgt, u32 cgt)
{
    s32 i;
    s32 ret;
    s32 errcnt;

//drv_psam_testpin_set(0, 1);  
    ret = drv_psam_waitetu(bgt);  
    for(i=0; i<wlen; i++)
    {
        errcnt=0;
        while(1)
        {
            ret = drv_psam_send_byte(wbuf[i]);
            if(ret == ICC_PROC_T1PARITYERR)
            {
                errcnt++;
                if(errcnt == 5)
                {
                    PSAM_DEBUG("PARAITYERR\r\n");
                    return ret;
                }
            }
            else if(ret != 0)
            {
                PSAM_DEBUG("ERR(%d)\r\n", ret);
                return ret;
            }
            break;
        }
        if(i != (wlen-1))
        {
            ret = drv_psam_waitetu(bgt);//cgt);
            if(ret != 0)
            {
                return ret;
            }
        }
    }
    return ret;
}
/****************************************************************************
**Description:       PSAM��T1������������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170717
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_psam_send_nbytes_t1(u8 *wbuf, s32 wlen, u32 bgt, u32 cgt)
{
    s32 i;
    s32 ret;

    drv_psam_waitetu(bgt);    
    for(i=0; i<wlen; i++)
    {
        while(1)
        {
            ret = drv_psam_send_byte(wbuf[i]);
          #if 0 
            if(ret == ICC_PROC_T1PARITYERR)
            {
                errcnt++;
                if(errcnt == 5)
                {
                    PSAM_DEBUG("PARAITYERR\r\n");
                    return ret;
                }
            }
            else 
          #endif      
            if(ret != 0)
            {
                PSAM_DEBUG("ERR(%d)\r\n", ret);
                return ret;
            }
            break;
        }
        if(i != (wlen-1))
        {
            ret = drv_psam_waitetu(cgt);
            if(ret != 0)
            {
                return ret;
            }
        }
    }
    return ret;
}
/****************************************************************************
**Description:       PSAM�����ո�λ����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170712
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_psam_rec_byte(u8 *data, u32 ms)
{
    s32 ret;
    
    //����GPIO���ж�
//drv_psam_testpin_set(0, 1);    
    drv_psam_io_direction_set(g_curr_slot, 0, 0);   //����״̬
    drv_psam_io_irq_enable(g_curr_slot, 1);
    g_psam_status.m_busying = PSAM_STATUS_READ;
//    g_psam_status.m_bitstep = 0;
    ret = drv_psam_wait_opt_ent(ms);
    if(ret == 0)
    {
        ret = g_psam_status.m_result;
//dev_debug_printf("d=%02X,%d\r\n", g_psam_status.m_currdata, ret);          
        if(ret == 0)
        {
            *data = g_psam_status.m_currdata;  
        }
    }
    return ret;
}
/****************************************************************************
**Description:       PSAM��������������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170712
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_psam_rec_nbytes(u8 *rbuf, u16 num, u32 bwt, u32 cwt)
{
    u8 data;
    s32 ret;
    s32 i;
    
    for(i=0; i<num; i++)
    {
        if(i==0)
        {
            ret = drv_psam_rec_byte(&data, bwt);
        }
        else
        {
            ret = drv_psam_rec_byte(&data, cwt);
        }
        if(ret == ICC_PROC_T1PARITYERR)
        {
//               parity_flg = 1; 
            PSAM_DEBUG("ParityErr\r\n");
            return ICC_PROC_T1PARITYERR;
        }
        else if(ret != 0)
        {
            PSAM_DEBUG("ret=%d\r\n", ret);
            //��ʱ,�µ�
            return ICC_PROC_NORESPOND;
        } 
        rbuf[i] = data;
    }
    return 0;
}
/****************************************************************************
**Description:       PSAM��T0 ���䴦��
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    1:
**Created by:        pengxuebin,20170719
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
#define T0_STEP_SENDHEAD    0
#define T0_STEP_RECSW1      1
#define T0_STEP_RECSW2      2
#define T0_STEP_SENDDATA    3
#define T0_STEP_RECDATA     4

//------------------------------------------------------
#define T0_SEND_BGT        16      //����T0�鱣��ʱ��
static s32 drv_psam_exchange_t0_block()
{
    s32 ret;
    u32 cwt;
    u32 bwt;
    u8 tmp8;
    
    
    gp_icc_content->m_step = T0_STEP_SENDHEAD; //
    gp_icc_content->m_txlen = 0; //
    gp_icc_content->m_txcnt = 0; //
    gp_icc_content->m_rxlen = 0;    //T0�Ľ��ճ����ɹ����ֽڿ���
    gp_icc_content->m_rxcnt = 0;
    gp_icc_content->m_result = 0;
    gp_icc_content->m_parityerr = 0;
    gp_icc_content->m_busy = 1;

//    bwt = g_psam_info[g_curr_slot].m_bwt;
//    cwt = g_psam_info[g_curr_slot].m_cwt;

    cwt = drv_psam_timer_calc_ms(g_psam_info[g_curr_slot].m_icfd, g_psam_info[g_curr_slot].m_cwt);
    bwt = drv_psam_timer_calc_ms(g_psam_info[g_curr_slot].m_icfd, g_psam_info[g_curr_slot].m_bwt);

    while(1)
    {
        switch(gp_icc_content->m_step)
        {
        case T0_STEP_SENDHEAD:    //0
            ret = drv_psam_send_nbytes_t0(gp_icc_content->m_txhead, gp_icc_content->m_txheadlen, T0_SEND_BGT, g_psam_info[g_curr_slot].m_gt);
            if(ret == 0)
            {
                //���ͳɹ�,׼�����ܹ����ֽڻ�״̬�ֽ�
                gp_icc_content->m_step = T0_STEP_RECSW1;
            }
            else
            {
                gp_icc_content->m_result = ret;//ICC_SCC_ERR_CONV_tab[ret];
                PSAM_DEBUG("ret=%d\r\n", ret);
                //ret = -1;
                //flg = 0;    //�˳�ѭ��
                return -1;
            }
            break;
        case T0_STEP_RECSW1:      //1
            ret = drv_psam_rec_nbytes(&tmp8, 1, bwt, cwt);
            if(ret != 0)
            {
                gp_icc_content->m_result = ret;//ICC_SCC_ERR_CONV_tab[ret];
                PSAM_DEBUG("ret=%d\r\n", ret);
                //ret = -1;
                //flg = 0;    //�˳�ѭ��
                return -1;
            }
            //�жϹ����ֽ�
            if(0x60 == tmp8)
            {
                //�������չ����ֽ�
                break;
            }
            //�����ֽ�INS,���ͻ����ʣ����ֽ�
            if(tmp8 == gp_icc_content->m_txhead[1])
            {
                if(gp_icc_content->m_lc!=0)
                {//����ʣ�������
                    gp_icc_content->m_txlen += gp_icc_content->m_lc;
                    gp_icc_content->m_lc = 0;
                    gp_icc_content->m_step = T0_STEP_SENDDATA;    //��������
                    break;
                }
                else
                {//����ʣ�����
                    if(0==gp_icc_content->m_le)
                    {
                        //��ʱ������
                    }
                    //else 
                    {
                        gp_icc_content->m_rxlen += gp_icc_content->m_le;
                        gp_icc_content->m_le = 0;
                        gp_icc_content->m_step = T0_STEP_RECDATA;    //��������
                        break;
                    }
                }
            }
            //�����ֽ�INS����,���ͻ����һ���ַ�
            if(tmp8 == (0xff-gp_icc_content->m_txhead[1]))
            {
                
                if(gp_icc_content->m_lc!=0)
                {//����1Byte
                    gp_icc_content->m_txlen ++;
                    gp_icc_content->m_lc --;
                    gp_icc_content->m_step = T0_STEP_SENDDATA;    //��������
                    break;
                }
                else
                {//��������1Byte
                    if(0==gp_icc_content->m_le)
                    {
                        //��ʱ������
                    }
                    //else 
                    {
                        gp_icc_content->m_rxlen ++;
                        gp_icc_content->m_le --;
                        gp_icc_content->m_step = T0_STEP_RECDATA;    //��������
                        break;
                    }
                }
            }
            //����״̬�ֽ�
            gp_icc_content->m_sw[0] = tmp8;
            tmp8 &= 0xf0;
            if((0x60==tmp8)||(0x90==tmp8))
            {
                gp_icc_content->m_step = T0_STEP_RECSW2;           //������һ�������ֽ�
            }
            else
            {   //�����ֽڴ�
                gp_icc_content->m_result = ICC_PROC_SWERR;  
                PSAM_DEBUG("ICC_PROC_SWERR\r\n");
                return -1;           //�����ֽڴ�  
            }
            break;
        case T0_STEP_RECSW2:      //2
            ret = drv_psam_rec_nbytes(&tmp8, 1, bwt, cwt);
            if(ret != 0)
            {
                gp_icc_content->m_result = ret;//ICC_SCC_ERR_CONV_tab[ret];
                PSAM_DEBUG("ret=%d\r\n", ret);
                //ret = -1;
                //flg = 0;    //�˳�ѭ��
                return -1;
            }
            else
            {
                gp_icc_content->m_sw[1] = tmp8;
                //������ɣ������ж����ж��⴦��
                gp_icc_content->m_result = ICC_PROC_OK;
                return 0;
            }
//            break;
        case T0_STEP_SENDDATA:    //3
            ret = drv_psam_send_nbytes_t0(&gp_icc_content->m_txbuf[gp_icc_content->m_txcnt], 
                  (gp_icc_content->m_txlen-gp_icc_content->m_txcnt), T0_SEND_BGT, g_psam_info[g_curr_slot].m_gt);
            if(ret == 0)
            {
                gp_icc_content->m_txcnt = gp_icc_content->m_txlen;
                //���ͳɹ�,׼�����չ����ֽڻ�״̬�ֽ�
                gp_icc_content->m_step = T0_STEP_RECSW1;
            }
            else
            {
                gp_icc_content->m_result = ret;//ICC_SCC_ERR_CONV_tab[ret];
                PSAM_DEBUG("ret=%d\r\n", ret);
                //ret = -1;
                //flg = 0;    //�˳�ѭ��
                return -1;
            }
            break;
        case T0_STEP_RECDATA:     //4
            ret = drv_psam_rec_nbytes(&gp_icc_content->m_rxbuf[gp_icc_content->m_rxcnt], 
                 (gp_icc_content->m_rxlen-gp_icc_content->m_rxcnt), bwt, cwt);
            if(ret != 0)
            {
                gp_icc_content->m_result = ret;//ICC_SCC_ERR_CONV_tab[ret];
                PSAM_DEBUG("ret=%d\r\n", ret);
                //ret = -1;
                //flg = 0;    //�˳�ѭ��
                return -1;
            }
            else
            {
                gp_icc_content->m_rxcnt = gp_icc_content->m_rxlen;
                //���ճɹ�,׼�����չ����ֽڻ�״̬�ֽ�
                gp_icc_content->m_step = T0_STEP_RECSW1;
            }
            break;
        default:
            gp_icc_content->m_result = ret;//ICC_SCC_ERR_CONV_tab[ret];
            PSAM_DEBUG("step=%d\r\n", gp_icc_content->m_step);
            return -1;    
//            break;
        }
    }
}
/****************************************************************************
**Description:       PSAM��T1 ���䴦��
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    1:
**Created by:        pengxuebin,20170719
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
#define T1_STEP_SENDBLOCK       0x10
#define T1_STEP_RECHEAD         0x11
#define T1_STEP_RECDATA         0x12

//---------------------------------------
#define T1_SEND_BGT             16

static s32 drv_psam_exchange_t1_block(void)
{
    s32 ret;
//    u8 flg;
    u32 cwt;
    u32 bwt;
//    u8 tmp8;
    s16 i;
    
    gp_icc_content->m_step = T1_STEP_SENDBLOCK; //
    //���㷢��LRC
    i = ((u16)gp_icc_content->m_txbuf[2])+3;
    gp_icc_content->m_txbuf[i] = dev_maths_getlrc(0, gp_icc_content->m_txbuf, i);
    i++;
    gp_icc_content->m_txlen = i; //
    gp_icc_content->m_txcnt = 0; //
    gp_icc_content->m_rxlen = 3;    //����ͷ��3�ֽ�
    gp_icc_content->m_rxcnt = 0;
    gp_icc_content->m_result = 0;
    gp_icc_content->m_parityerr = 0;
    gp_icc_content->m_busy = 1;

    //�ж��Ƿ�BWTӦ���
    if(0xe3 == gp_icc_content->m_txbuf[1])
    {
        bwt = g_psam_info[g_curr_slot].m_bwt*gp_icc_content->m_txbuf[3]
            + 100*(1<<g_psam_info[g_curr_slot].m_icfd);

    }
    else
    {
        bwt = g_psam_info[g_curr_slot].m_bwt + 100*(1<<g_psam_info[g_curr_slot].m_icfd);
    }
    //cwt = g_psam_info[g_curr_slot].m_cwt;
    cwt = drv_psam_timer_calc_ms(g_psam_info[g_curr_slot].m_icfd, g_psam_info[g_curr_slot].m_cwt);
    bwt = drv_psam_timer_calc_ms(g_psam_info[g_curr_slot].m_icfd, bwt);

//PSAM_DEBUG("bwt=%d, cwt=%d\r\n", bwt, cwt);  
    while(1)
    {
        switch(gp_icc_content->m_step)
        {
        case T1_STEP_SENDBLOCK:       //0x10
//PSAM_DEBUG("Send:%d\r\n", gp_icc_content->m_txlen);    
//dev_printformat(NULL, gp_icc_content->m_txbuf, gp_icc_content->m_txlen);
            ret = drv_psam_send_nbytes_t1(gp_icc_content->m_txbuf, gp_icc_content->m_txlen, T1_SEND_BGT, g_psam_info[g_curr_slot].m_gt);
            if(ret == 0)
            {
                //���ͳɹ�,׼�����ܹ����ֽڻ�״̬�ֽ�
                gp_icc_content->m_step = T1_STEP_RECHEAD;
            }
            else
            {
                gp_icc_content->m_result = ret;//ICC_SCC_ERR_CONV_tab[ret];
                PSAM_DEBUG("ret=%d\r\n", ret);
                //ret = -1;
                //flg = 0;    //�˳�ѭ��
                return 0;//-1;
            }
            break;
        case T1_STEP_RECHEAD:         //0x11
            //����ͷ��(NAD PCB LEN)
            ret = drv_psam_rec_nbytes(&gp_icc_content->m_rxbuf[0], 3, 
                  bwt, cwt);
//dev_debug_printf("r=%d, par=%d\r\n",ret, gp_icc_content->m_parityerr);  
//dev_printformat(NULL, &gp_icc_content->m_rxbuf[0], 3);
            if(ret != 0)
            {
                gp_icc_content->m_result = ret;//ICC_SCC_ERR_CONV_tab[ret];
                PSAM_DEBUG("ret=%d, bwt=%d,cwt=%d\r\n", ret, bwt, cwt);
                //ret = -1;
                //flg = 0;    //�˳�ѭ��
                return 0;//-1;
            }
            else
            {
                gp_icc_content->m_rxcnt = 3;
                //ͷ��������,׼�������������β��(EDC)
                gp_icc_content->m_rxlen = 3+gp_icc_content->m_rxbuf[2]+1;
                gp_icc_content->m_step = T1_STEP_RECDATA;
            }
            break;
        case T1_STEP_RECDATA:         //0x12
            ret = drv_psam_rec_nbytes(&gp_icc_content->m_rxbuf[gp_icc_content->m_rxcnt], 
                   (gp_icc_content->m_rxlen-gp_icc_content->m_rxcnt), cwt, cwt);

            if(ret != 0)
            {
                gp_icc_content->m_result = ret;//ICC_SCC_ERR_CONV_tab[ret];
                PSAM_DEBUG("ret=%d\r\n", ret);
                //ret = -1;
                //flg = 0;    //�˳�ѭ��
                return 0;//-1;
            }
            else
            {
                 
                gp_icc_content->m_result = ICC_PROC_OK;
                    
                gp_icc_content->m_rxcnt = 3+gp_icc_content->m_rxbuf[2]+1;
                return 0;
            }
//            break;
        default:
            gp_icc_content->m_result = ret;//ICC_SCC_ERR_CONV_tab[ret];
            PSAM_DEBUG("step=%d\r\n", gp_icc_content->m_step);
            return -1;    
//            break;
        }
    }
}
/****************************************************************************
**Description:       PSAM�����ո�λ����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170712
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_psam_rec_rst(u8 *rst, u16 *rstlen)
{
    str_icc_info_t *lp_icc_info;
    u32 i;
    u32 cwtms;
    u32 bwtms;
    s32 ret;
    u32 bwttimerid;
    u8 parity_flg;
    u8 num;
    u16 cnt;
    u8 protocol;
    u8 data;
    u8 TDtmp;
    
    *rstlen = 0;
    lp_icc_info = &g_psam_info[g_curr_slot];
    g_psam_status.m_psamT = 0xFF;
    lp_icc_info->m_Nbit = 0;
    lp_icc_info->m_icfd = 0;
    lp_icc_info->m_gt  = 0;
    lp_icc_info->m_cwt  = ICC_CWT_DEFAULT;
    lp_icc_info->m_bwt  = ICC_BWT_DEFAULT;
    cwtms = drv_psam_timer_calc_ms(lp_icc_info->m_icfd, lp_icc_info->m_cwt);
    bwtms = drv_psam_timer_calc_ms(lp_icc_info->m_icfd, lp_icc_info->m_bwt);
    i = drv_psam_timer_calc_ms(lp_icc_info->m_icfd, 113+5+10);
    parity_flg = 0;
    //����TS
    ret = drv_psam_rec_byte(&data, i+1);
    if(ret == ICC_PROC_T1PARITYERR)
    {
        parity_flg = 1; 
        return ICC_PROC_T1PARITYERR;
    }
    else if(ret != 0)
    {
        PSAM_DEBUG("ret=%d\r\n", ret);
        //��ʱ,�µ�
        return ICC_PROC_NORESPOND;
    } 
    if((data != 0x3F) && (data != 0x3B))
    {
        PSAM_DEBUG("TS=%02X\r\n", rst[0]);
        return ICC_PROC_TSERR;
    }
    rst[0] = data;
        
    bwttimerid = dev_user_gettimeID();
    //����T0
    ret = drv_psam_rec_byte(&data, cwtms);
    if(ret == ICC_PROC_T1PARITYERR)
    {
        parity_flg = 1; 
        return ICC_PROC_T1PARITYERR;
    }
    else if(ret != 0)
    {
        PSAM_DEBUG("ret=%d\r\n", ret);
        //��ʱ,�µ�
        return ICC_PROC_NORESPOND;
        
    } 
    if(data&0x80)
    {
        protocol = 1;
    }
    else
    {
        protocol = 0;
    }
    rst[1] = data;
    cnt = 2;
    TDtmp = data;
    while(1)
    {
      #if 0  
        num = psam_calc_1_tab[(TDtmp>>4)&0x0f];
        if(num == 0)
        {
            break;
        }
        //����TA1~TD1
        for(i=0; i<num; i++)
        {
            ret = drv_psam_rec_byte(&data, cwtms);
            if(ret == ICC_PROC_T1PARITYERR)
            {
                parity_flg = 1; 
                PSAM_DEBUG("ParityErr\r\n");
                return ICC_PROC_T1PARITYERR;
            }
            else if(ret != 0)
            {
                PSAM_DEBUG("ret=%d\r\n", ret);
                //��ʱ,�µ�
                return ICC_PROC_NORESPOND;
            } 
            rst[cnt++] = data;
        }
      #endif
        //����TA1~TD1
        num = psam_calc_1_tab[(TDtmp>>4)&0x0f];
        if(num == 0)
        {
            break;
        }
        ret = drv_psam_rec_nbytes(&rst[cnt], num, cwtms, cwtms);
        if(ret != 0)
        {
            return ret;
        }
        cnt += num;
        if(TDtmp&0x80)
        {
            data = rst[cnt-1];
            i = data&0x0F;
            if(0 == i)
            {
                protocol = 0;
            }
            else if(1 != i)
            {
                PSAM_DEBUG("ICRESETERR\r\n");
                return ICC_PROC_RESETERR;          //TD1��,����
            }
            TDtmp = data;
        }
        else
        {
            TDtmp = 0;
            break;
        }
        //TA2~TD2
        num = psam_calc_1_tab[(TDtmp>>4)&0x0f];
        if(num == 0)
        {
            break;
        }
        ret = drv_psam_rec_nbytes(&rst[cnt], num, cwtms, cwtms);
        if(ret != 0)
        {
            return ret;
        }
        cnt += num;
        if(TDtmp&0x80)
        {
            data = rst[cnt-1];
            i = data&0x0F;
            if((0==i) || (0x0f==i))
            {
                PSAM_DEBUG("ICRESETERR\r\n");
                return ICC_PROC_RESETERR;          //TD2��,����
            }
            if(0x0e==i)
            {
                if(1==(TDtmp&0x0f))
                {
                    PSAM_DEBUG("ICRESETERR\r\n");
                    return ICC_PROC_RESETERR;          //TD2��,����
                }
            }
            protocol = 1;
            TDtmp = data;
        }
        else
        {
            TDtmp = 0;
            break;
        }
        //TA3~TD3
        num = psam_calc_1_tab[(TDtmp>>4)&0x0f];
        if(num == 0)
        {
            break;
        }
        ret = drv_psam_rec_nbytes(&rst[cnt], num, cwtms, cwtms);
        if(ret != 0)
        {
            return ret;
        }
        cnt += num;
        if(TDtmp&0x80)
        {
            data = rst[cnt-1];
            if(data&0x80)
            {
                PSAM_DEBUG("ICRESETERR\r\n");
                return ICC_PROC_RESETERR;          //TD2��,����
            }
            i = data&0x0F;
            if((1==i) || (0x0E==i))
            {
                protocol =1 ;
            }
            else if(0 == i)
            {
                protocol = 0;
            }
            TDtmp = data;
        }
        else
        {
            TDtmp = 0;
            break;
        }
        //TA4~TD4
        num = psam_calc_1_tab[(TDtmp>>4)&0x0f];
        if(num == 0)
        {
            break;
        }
        ret = drv_psam_rec_nbytes(&rst[cnt], num, cwtms, cwtms);
        if(ret != 0)
        {
            return ret;
        }
        cnt += num;
        break;
    }
    //������ʷ�ַ���У��
    num = rst[1]&0x0f;
    if(protocol)
    {
        num ++;
    }
    ret = drv_psam_rec_nbytes(&rst[cnt], num, cwtms, cwtms);
    if(ret != 0)
    {
        return ret;
    }
    cnt += num;
    if(protocol)
    {
        //LRCУ��
        data = maths_calc_lrc(0, &rst[1], cnt-1);
        if(data != 0)
        {
            return ICC_PROC_T1CRCERR;      //crc��
        }
    }
    *rstlen = cnt;
PSAM_DEBUG("cnt=%d\r\n", cnt);
PSAM_DEBUGHEX(NULL, rst, cnt);
    return ret;
    
}
/****************************************************************************
**Description:       PSAM����λ����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    1:��Ҫ�����ȸ�λ
**Created by:        pengxuebin,20170712
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_psam_rst_phase(s8 type, u8 *rst, u16 rstlen)
{
    u8 protocol=0;
    s32 i;
    u8 m;
    s32 ret;
    s32 k, n, tc1;
    str_icc_info_t *lp_icc_info;
    
    if(rstlen<3)
    {
        PSAM_DEBUG("ERR(len=%d)!\r\n", rstlen);
        return 1;
    }
    lp_icc_info = &g_psam_info[g_curr_slot];
    ret = 0;
    while(1)
    {
        ret = 2;
        //T0��ʽ��
        if((rst[1]&0x80) == 0)
        {
            protocol = 0;
        }
        else
        {
            protocol = 1;
        }
        i = 2;
        m = rst[1];
        //TA1
        if((m&0x10) != 0)
        {
            lp_icc_info->m_icfd = rst[i];
            i++;
        }
        else
        {
            lp_icc_info->m_icfd = 0x11;
        }
        //TB1
        if((m&0x20) != 0)
        {
            if((type==0) && (rst[i]!=0x00))
            {
                //ret = 2;
                PSAM_DEBUG("ERR(TB1=%02X)!\r\n", rst[i]);
                break;
            }
            i++;            
        }
        else
        {
            if(type == 0)
            {
                //ret = 2;
                PSAM_DEBUG("ERR(TB1=NULL)!\r\n");
                break;
            }
        }
        //TC1
        if(0 != (m&0x40))
        {
            tc1 = rst[i];
            i++;
        }
        else
        {
            tc1 = 0;
        }
        //TD1
        if(0 != (m&0x80))
        {
            m = rst[i];
            if((m&0x0E) != 0)
            {
                //ret = 2;
                PSAM_DEBUG("ERR(TD1=%02X)!\r\n", rst[i]);
                break;
            }
            else if((m&0x0F) == 0)
            {
                protocol =0;
            }
            i++;
        }
        else
        {
            m = 0;
        }
        //TA2
        if((m&0x10) != 0)
        {
            if((lp_icc_info->m_icfd > 0x13) || (lp_icc_info->m_icfd < 0x11))
            {
                //ret = 2;
                PSAM_DEBUG("ERR(TA2=%02X)!\r\n", rst[i]);    
                break;
            }
            lp_icc_info->m_icfd = (lp_icc_info->m_icfd&0x03)-1;
            if((rst[i]&0x10) == 0x10)
            {
                //ret = 2;
                PSAM_DEBUG("ERR(TA2=%02X)!\r\n", rst[i]);   
                break;
            }
            else if((rst[i]&0x10) != 0x00)
            {
                lp_icc_info->m_icfd = 0;
            }
            i++;
        }
        else
        {
            lp_icc_info->m_icfd = 0;
        }
        //TB2
        if((m&0x20) != 0)
        {
            //ret = 2;
            PSAM_DEBUG("ERR(TB2=%02X)!\r\n", rst[i]);  
            break;
            //i++;
        }
        //TC2
        if((m&0x40) != 0)
        {
            if(rst[i] != 0x0A)
            {
                //ret = 2;
                PSAM_DEBUG("ERR(TC2=%02X)!\r\n", rst[i]);   
                break;
            }
            lp_icc_info->m_cwt = (960*rst[i]+480+100)*(0x01<<lp_icc_info->m_icfd);
            i++;
        }
        else
        {
            lp_icc_info->m_cwt = (10080 +100)*(0x01<<lp_icc_info->m_icfd);
        }
        lp_icc_info->m_bwt = lp_icc_info->m_cwt;
        //TD2
        if((m&0x80) != 0)
        {
            k = rst[i]&0x0F;
            if((k==0) || (k==0x0F) || (((m&0x0F)==0x01)&&(k==0x0E)))
            {
                //ret = 2;
                PSAM_DEBUG("ERR(TD2=%02X)!\r\n", rst[i]);   
                break;
            }
            m = rst[i];
            //�ܾ�TD2�Ͱ��ֽڲ�Ϊ1��E
            if(((m&0x0F)!=1) && ((m&0x0F)!=0x0E))
            {
                //ret = 2;
                PSAM_DEBUG("ERR(TD2=%02X)!\r\n", rst[i]);  
                break;
            }
            i++;
        }
        else
        {
            m = 0;
        }
        //TA3
        if((m&0x10) != 0)
        {
            if((rst[i]<0x10) || (rst[i]==0xFF))
            {
                //ret = 2;
                PSAM_DEBUG("ERR(TA3=%02X)!\r\n", rst[i]); 
                break;
            }
            else
            {
                lp_icc_info->m_ifsi = rst[i];
            }
            i++;
        }
        else
        {
            lp_icc_info->m_ifsi = 0x20;
        }
        //TB3
        if((m&0x20) != 0)
        {
            if(protocol != 0)
            {
                k = rst[i];
                if(((k&0x0F)>5) || (k>0x4F))
                {
                    //ret = 2;
                    PSAM_DEBUG("ERR(TB3=%02X)!\r\n", rst[i]); 
                    break;
                }
                else
                {
                    lp_icc_info->m_bwt = (960*((0x01<<(k>>4))+1)+100)*(0x01<<lp_icc_info->m_icfd);
                }
                k = 0x01<<(k&0x0F);
                if(tc1==0xFE)
                {
                    //ret = 2;
                    PSAM_DEBUG("ERR(TB3=%02X,TC1=%02X)!\r\n", rst[i], tc1); 
                    break;
                }
                n = (tc1+2)&0xff;
                if(k < n)
                {
                    //ret = 2;
                    PSAM_DEBUG("ERR(TB3=%02X,k=%d,n=%d)!\r\n", rst[i], k, n); 
                    break;
                }
                else
                {
                    lp_icc_info->m_cwt = k+10;
                }
            }
            i++;
        }
        else
        {
            if(protocol == 1)
            {
                //ret = 2;
                PSAM_DEBUG("ERR(TB3=NULL)!\r\n"); 
                break;
            }
        }
        //tc3
        if((m&0x40) != 0)
        {
            if(rst[i] != 0)
            {
                //ret = 2;
                PSAM_DEBUG("ERR(TC3=%02X)!\r\n", rst[i]); 
                break;
            }
            i++;
        }
        //TD3
        if((m&0x80) != 0)
        {
            m = rst[i];
            i++;
        }
        else
        {
            m = 0;
        }
        //TA4
        if((m&0x10) != 0)
        {
            i++;
        }
        //TB4
        if((m&0x20) != 0)
        {
            i++;
        }
        //TC4
        if((m&0x40) != 0)
        {
            i++;
        }
        if(protocol==1)
        {
            i++;
        }
        if((i+(rst[1]&0x0F)) > rstlen)
        {
            ret = 1;
            PSAM_DEBUG("ERR(i=(%d,%d), rstlen=%d)!\r\n", rst[i], i, (i+(rst[1]&0x0F)), rstlen); 
            break;
        }
        if(protocol == 0)
        {
            if(tc1 == 0xff)
            {
                lp_icc_info->m_gt = 1;
            }
            else
            {
                lp_icc_info->m_gt = tc1+1;
            }
        }
        else
        {
             lp_icc_info->m_gt = tc1+1;
        }
        ret = 0;
        lp_icc_info->m_protocolType = protocol;
        break;
    }
    return ret;
}
/****************************************************************************
**Description:       ������Ϣ���С����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    1:��Ҫ�����ȸ�λ
**Created by:        pengxuebin,20170712
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_psam_exchange_t1_IFSC(void)
{
    s32 flg=1;
    s32 i;
    s32 ret;

    for(i=0; i<3; i++)
    {
        memcpy(gp_icc_content->m_txbuf, "\x00\xc1\x01\xfe\x3e", 5);
        gp_icc_content->m_txheadlen = 4;
        ret = drv_psam_exchange_t1_block();
        if(ret < 0)
        {
            flg = 1;    
        }
        else if((ICC_PROC_CWTOVER == gp_icc_content->m_result)
         ||(ICC_PROC_BWTOVER == gp_icc_content->m_result))  
        {
            flg = 1;       //��ʱ�µ�
//dev_debug_printf("OVER=%d\r\n", gp_icc_content->m_result);
           break;
        }
        else  if((ICC_PROC_BWTOVER < gp_icc_content->m_result)
            ||(gp_icc_content->m_parityerr)
            ||(0 != (memcmp(gp_icc_content->m_rxbuf, "\x00\xE1\x01\xFE\x1E", 5))))
        {
//dev_debug_printf("ret=%d, parity=%d\r\n", gp_icc_content->m_result, gp_icc_content->m_parityerr);
//dev_printformat(NULL, gp_icc_content->m_rxbuf, gp_icc_content->m_rxlen);
//dev_delay_ms(5);
            //�ط�
            continue;
        }
        else
        {   //��λ�ɹ�
            g_psam_info[g_curr_slot].m_ipcb = 0x40;
            g_psam_info[g_curr_slot].m_nrpcb = 0x80;
            flg = 0;
            break;
        }
    }
    if(flg==1)
    {
        //����,�µ�
        drv_psam_inactive();
        ret = DEVSTATUS_ERR_FAIL;
    }
    return ret;
    
}
/****************************************************************************
**Description:       PSAM��T0 exchange����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    1:
**Created by:        pengxuebin,20170719
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_psam_exchange_t0(u8 *wbuf, u16 wlen, u8 *rbuf, u16 *rlen, u16 rxmax)
{
    s32 ret;
    u8 sw_bak[2];
    u8 casetype;

    *rlen = 0;
    sw_bak[0] = 0;

    gp_icc_content->m_txheadlen = 5;
    if(4 == wlen)
    {
        //case 1
        gp_icc_content->m_lc = 0;
        gp_icc_content->m_le = 0;
        casetype = 1;
        memcpy(gp_icc_content->m_txhead, wbuf, 4);
        gp_icc_content->m_txhead[4] = 0;
    }
    else if(5 == wlen)
    {
        //case 2
        gp_icc_content->m_lc = 0;
        gp_icc_content->m_le = wbuf[4];
        casetype = 2;
        memcpy(gp_icc_content->m_txhead, wbuf, 5);
    }
    else if(wlen>5)
    {
        //case 3
        memcpy(gp_icc_content->m_txhead, wbuf, 5);
        gp_icc_content->m_lc = wbuf[4];
        memcpy(gp_icc_content->m_txbuf, &wbuf[5], gp_icc_content->m_lc);
        if(wlen == (gp_icc_content->m_lc+5))
        {
            gp_icc_content->m_le = 0;
            casetype = 3;
        }
        else
        {
            //case 4
            gp_icc_content->m_le = wbuf[gp_icc_content->m_lc+5];
            casetype = 4;
        }
    }
IC_T0_W_R_Begin:     
    ret = drv_psam_exchange_t0_block();
    if(ret < 0)
    {
        PSAM_DEBUG("ret=%d\r\n", ret);
        gp_icc_content->m_result = ICC_PROC_ABNORAL;
        //��д����
        goto IC_T0_W_R_END;  
    }

    if(gp_icc_content->m_parityerr)
    {
        //����T0��Ӧ�ò�������������
        gp_icc_content->m_result = ICC_PROC_PARAERR;
        PSAM_DEBUG("PARAERR!\r\n");
        //��д����
        goto IC_T0_W_R_END; 
    }
 
    if(gp_icc_content->m_result>ICC_PROC_OK)
    {
        //��д����
        PSAM_DEBUG("ret=%d\r\n", gp_icc_content->m_result);
        goto IC_T0_W_R_END; 
    }
    if(0 != gp_icc_content->m_rxcnt)
    {
        memcpy(rbuf+*rlen, gp_icc_content->m_rxbuf, gp_icc_content->m_rxcnt);
        *rlen += gp_icc_content->m_rxcnt;
    }
    if(1 == casetype)
    {
//PSAM_DEBUG("casetype==1\r\n");
        goto IC_T0_W_R_END;
    }
//dev_debug_printf("sw=%02X\r\n", gp_icc_content->m_sw[0]);    
    if(0x61 == gp_icc_content->m_sw[0])
    {
//dev_debug_printf("rev=%02X,%02X, %d, %d\r\n", gp_icc_content->m_sw[0], gp_icc_content->m_sw[2],
//    *rlen, (rxmax));        
        if((gp_icc_content->m_sw[1]+*rlen)>(rxmax-2))
        {
            //���ٷ���GET RESPONSE
            goto IC_T0_W_R_END;
        }
        memcpy(gp_icc_content->m_txhead, "\x00\xc0\x00\x00", 4);
        gp_icc_content->m_txhead[4] = gp_icc_content->m_sw[1]; 
        gp_icc_content->m_lc = 0;
        gp_icc_content->m_le = gp_icc_content->m_sw[1];
        casetype = 2;
        goto IC_T0_W_R_Begin;
    }
    else if(0x6c == gp_icc_content->m_sw[0])
    {
        gp_icc_content->m_txhead[4] = gp_icc_content->m_sw[1]; 
        gp_icc_content->m_lc = 0;
        gp_icc_content->m_le = gp_icc_content->m_sw[1];
        casetype = 2;
        goto IC_T0_W_R_Begin;
    }
    else if(((gp_icc_content->m_sw[0]&0xf0)!=0x60)
          &&((gp_icc_content->m_sw[0]&0xf0)!=0x90))
    {
        gp_icc_content->m_result = ICC_PROC_SWERR;  //sw��
        goto IC_T0_W_R_END;
    }
    else if((0x90==gp_icc_content->m_sw[0])
          &&(0x00==gp_icc_content->m_sw[1]))
    {
        goto IC_T0_W_R_END;
    }
    else if((0x62 == gp_icc_content->m_sw[0])
         || (0X63 == gp_icc_content->m_sw[0])
         || (0X9F == gp_icc_content->m_sw[0]))
    {
        if((4==casetype)&&(0==gp_icc_content->m_lc))
        { 
            //����CASE4�����ݷ������ʱ�Ž���
            sw_bak[0] = gp_icc_content->m_sw[0];
            sw_bak[1] = gp_icc_content->m_sw[1];
            memcpy(gp_icc_content->m_txhead, "\x00\xc0\x00\x00\x00", 5);
            gp_icc_content->m_lc = 0;
            gp_icc_content->m_le = 0;
            casetype = 2;
            goto IC_T0_W_R_Begin;
        }
    }
    //----------------------------------------
IC_T0_W_R_END: 
    if(gp_icc_content->m_result>ICC_PROC_OK)
    {
        //�µ�
        drv_psam_inactive();
        PSAM_DEBUG("ERR(%d)\r\n", gp_icc_content->m_result);
        return DEVSTATUS_ERR_FAIL;          //��������
    }
    if(0 != sw_bak[0])
    {
        rbuf[*rlen]=sw_bak[0];
        rbuf[(*rlen)+1]=sw_bak[1];
    }
    else
    {
        rbuf[*rlen]=gp_icc_content->m_sw[0];
        rbuf[(*rlen)+1]=gp_icc_content->m_sw[1];
    }
    *rlen = (*rlen)+2;
    
    return 0;  //ICREADOK;    
}
/**************************************************
���ز������壺
0x00:��ȷ���յ�I��������Ϊ0
��Ҫ����I��
0x01: ��Ҫ�ش���һI��
0x02: ����һI��  
0x03: �ش���һ��
0x0f: ����ظ�������Ҫ�����µ�ʱ��
��Ҫ����R��
0x80: |
      |��ȷ��������Ϊ1��I��,��Ҫ����R��,��R���PCBΪ0x80\0x90  
0x90: |

0x81: |
      |У�����,��Ҫ����R�죬��R���PCBΪ0x81\0x91
0x91: |
110XXXXXB:��ʾ���յ�S�����
111XXXXXB;��ʾ���յ�SӦ���


0xfe:���յ�EDC���У�����
0xff:�ط��Ͽ�����(I��R��S)
**************************************************/
static s32 drv_psam_exchange_t1_phase(u8 *rbuf, u16 *rxlen, u16 rxmax)
{
    u16 i,j;
    u8 n,k;
    u8 errcnt=2;
    u8 flg;
    s32 ret;

    *rxlen = 0;
ic_type1_phase_A:
    n = 0;
    //���ڳ�ʱ��ֱ���µ�
    if((ICC_PROC_CWTOVER == gp_icc_content->m_result)
     ||(ICC_PROC_BWTOVER == gp_icc_content->m_result))
    {
      
        n = 0x0F;     //Ҫ�����µ�ʱ��  
    }
    //else if((ICREADOK<gp_icc_content->m_result)
    //      &&(ICT1PARITYERR!=gp_icc_content->m_result))
    else if((ICC_PROC_OK<gp_icc_content->m_result)
          &&(0 == gp_icc_content->m_parityerr))
    {
        gp_icc_content->m_lasterrtype = IC_T1_LASTEDCERR;
        n = g_psam_info[g_curr_slot].m_nrpcb|0x02;
        if((0x70 == gp_icc_content->m_rxbuf[1+3])
         &&(gp_icc_content->m_rxcnt>(3+2)))
        {
            for(i=3; i<gp_icc_content->m_rxcnt; i++)
            {
                gp_icc_content->m_rxbuf[i] = 0;
            }
        }
    }
    else
    {
        //�����У��
        k = dev_maths_getlrc(0, &gp_icc_content->m_rxbuf[0], gp_icc_content->m_rxcnt);
        if((gp_icc_content->m_parityerr) || (0 != k))
        {   //У���
            n = g_psam_info[g_curr_slot].m_nrpcb|0x01;
            flg = 1;
            gp_icc_content->m_lasterrtype = IC_T1_LASTEDCERR; 
        }
        else if(0 != gp_icc_content->m_rxbuf[0])
        {   //NOD����
            n = g_psam_info[g_curr_slot].m_nrpcb|0x02;
            flg = 1;
            gp_icc_content->m_lasterrtype = IC_T1_LASTNODERR; 
        }
        else
        {
            k=1;
            switch(gp_icc_content->m_rxbuf[1]&0xc0)
            {
            case 0x00:

                k = 0;
            case 0x40:  //I��
                if((g_psam_info[g_curr_slot].m_ipcb&0x20)!=0)
                {
                    n = g_psam_info[g_curr_slot].m_nrpcb|0x02;
                    if(IC_T1_LASTIERR != gp_icc_content->m_lasterrtype)  
                    {   //����ϴβ���I���
                        errcnt = 2;
                        gp_icc_content->m_lasterrtype = IC_T1_LASTIERR;   //I���
                     }
                    break;
                }
                if(0 == (g_psam_info[g_curr_slot].m_nrpcb&0x10))
                {
                    i = 0;
                }
                else
                {
                    i = 0x40;
                }
                i ^= gp_icc_content->m_rxbuf[1];
                i &= 0x40;
                if((gp_icc_content->m_rxbuf[2]== 0xff)
                 ||(i!= 0))                            //�ж�I�����źͳ���
                {
                    n = g_psam_info[g_curr_slot].m_nrpcb|0x02;
                    gp_icc_content->m_lasterrtype = IC_T1_LASTIERR; 
                    flg = 1;
                    break;
                }
                //resyncnt = 0;   //��ͬ����־��������
                //------------------------------------
                //��ȷ����I��
                i = gp_icc_content->m_rxbuf[2]; //ȥ��У��λ��ͷ��
                memcpy(&rbuf[*rxlen], &gp_icc_content->m_rxbuf[3], i);
                *rxlen += i;
                //memcpy(&rbuf[*rxlen], &gp_icc_content->m_rxbuf[3], i);
                if((gp_icc_content->m_rxbuf[1]&0x20)==0)//���ӵ����һ��
                {
                    //memcpy(&rbuf[*rxlen], &gp_icc_content->m_rxbuf[3], i);
                    //*rxlen += i;
                    //��ʱû����sw
                    //gp_icc_content->m_sw[0] = gp_icc_content->m_rxbuf[i+3-2];                    
                    //gp_icc_content->m_sw[1] = gp_icc_content->m_rxbuf[i+3-2];
                    n = 0;
                    if(0 == k)
                    {
                        g_psam_info[g_curr_slot].m_nrpcb = 0x90;
                    }
                    else
                    {
                        g_psam_info[g_curr_slot].m_nrpcb = 0x80;
                    }
                    errcnt = 2;
                    gp_icc_content->m_lasterrtype = IC_T1_LASTOK; //����ȷ
                }
                else  //׼��������һ�����ӿ�
                {
                    if(0 == k)
                    {
                        n = 0x90;
                    }
                    else
                    {
                        n = 0x80;
                    }
                    errcnt = 3;
                    g_psam_info[g_curr_slot].m_nrpcb = n;
                    gp_icc_content->m_lasterrtype = IC_T1_LASTOK; //����ȷ
                }
                break;
            //=========================================
            case 0x80:     //R��
                if((0!=gp_icc_content->m_rxbuf[2])
                 ||(0!=(gp_icc_content->m_rxbuf[1]&0x20))) 
                {
                    n = g_psam_info[g_curr_slot].m_nrpcb|0x02;
                    if(IC_T1_LASTROTHERR!=gp_icc_content->m_lasterrtype)
                    {
                        gp_icc_content->m_lasterrtype = IC_T1_LASTROTHERR;
                        errcnt = 2;
                    }
                    break;
                }
                if(0!=(gp_icc_content->m_rxbuf[1]&0x0f))
                {
                    if(0x80 == (gp_icc_content->m_txbuf[1]&0xef))
                    {
                        n = 0x03;
                        break;
                    }
                }
                if(0 == (g_psam_info[g_curr_slot].m_ipcb&0x40))
                {
                    i = 0x90;
                }
                else
                {
                    i = 0x80;
                }
                if(((gp_icc_content->m_rxbuf[1]&0xf0)==i)
                 && (0 == (g_psam_info[g_curr_slot].m_ipcb&0x20)))
                {   //û�������ˣ���Ҫ��һ��
                    n = g_psam_info[g_curr_slot].m_nrpcb|0x02;
                    flg = 1;
                    if(IC_T1_LASTRNOERR != gp_icc_content->m_lasterrtype)
                    {
                        errcnt = 2;
                        gp_icc_content->m_lasterrtype = IC_T1_LASTRNOERR;
                    }
                    break;
                }
                i = gp_icc_content->m_rxbuf[1];
                k = g_psam_info[g_curr_slot].m_ipcb;
                if(0 != (k&0x40))
                {
                    i ^= 0x10;
                }
                if(0 == (i&0x10))
                {   //˵����һI�鴫������ش�
                    if((0x80 == gp_icc_content->m_rxbuf[1])
                     ||(0x90 == gp_icc_content->m_rxbuf[1]))
                    {
                        n = 0x03;      //�ش���һR�� 
                    }
                    else
                    {
                        n = 0x01;      //�ش���һI�� 
                    }
                    errcnt = 3;
                    gp_icc_content->m_lasterrtype=IC_T1_LASTOK;//˵���յ�û������
                }
                else
                {
                    if(0 != (gp_icc_content->m_rxbuf[1]&0x0f))
                    {
                        n = g_psam_info[g_curr_slot].m_nrpcb|0x02;
                        errcnt = 3;
                        gp_icc_content->m_lasterrtype=IC_T1_LASTTXERR;
                        break;
                    }
                    n = 0x02;    //˵����һI�鴫����ȷ������һ�� 
                    errcnt = 2;
                    gp_icc_content->m_lasterrtype=IC_T1_LASTOK;//˵���յ�û������
                }
                break;
            //=========================================
            case 0xc0:          //S��
                if(gp_icc_content->m_rxbuf[2]>1)
                {
                    n = g_psam_info[g_curr_slot].m_nrpcb|0x02;
                    gp_icc_content->m_lasterrtype=IC_T1_LASTSLENERR;//˵���յ�û������
                    flg = 1;   //�����һ�鷢�͵�S�飬ҲҪ�������������I��
                    break;
                }
                n = gp_icc_content->m_rxbuf[1];    //110XXXXXB  ��ʾ���յ�S�����
                                                    //111XXXXXB  ��ʾ���յ�SӦ���
                if((n&0x20) == 0)
                {
                    switch(n&0x1f)
                    {
                    case 1:
                        if((gp_icc_content->m_rxbuf[3]<0x10)
                         ||(0xff == gp_icc_content->m_rxbuf[3]))
                        {
                            n = g_psam_info[g_curr_slot].m_nrpcb|0x02;
                            gp_icc_content->m_lasterrtype=IC_T1_LASTSINFERR;//˵���յ�û������
                            flg = 1;   //�����һ�鷢�͵�S�飬ҲҪ�������������I��
                            break;
                        }
                        else
                        {
                            j = gp_icc_content->m_rxbuf[2];
                            memcpy(&gp_icc_content->m_txbuf[3], &gp_icc_content->m_rxbuf[3], j);
                            g_psam_info[g_curr_slot].m_ifsi = gp_icc_content->m_rxbuf[3];  //��Ϣ���С
                            errcnt = 3;
                            gp_icc_content->m_lasterrtype=IC_T1_LASTOK;//˵���յ�û������
                        }
                        break;
                    //-------------------------------
                    case 2:     //��������
                        if(0 != gp_icc_content->m_rxbuf[2])
                        {   //NOD��
                            n = g_psam_info[g_curr_slot].m_nrpcb|0x02;
                            gp_icc_content->m_lasterrtype=IC_T1_LASTSINFERR;
                        }
                        else
                        {  //�յ���ȷ�ķ������������µ�
                            n = 0x0f;
                        }
                        break;
                    //-------------------------------
                    case 3:
                        j = gp_icc_content->m_rxbuf[2];
                        memcpy(&gp_icc_content->m_txbuf[3], &gp_icc_content->m_rxbuf[3], j);
                        errcnt = 3;
                        gp_icc_content->m_lasterrtype=IC_T1_LASTOK;//˵���յ�û������
                        break;
                    //-------------------------------
                    default:
                        n=g_psam_info[g_curr_slot].m_nrpcb|0x02;
                        gp_icc_content->m_lasterrtype=IC_T1_LASTSNREQERR;
                        break;
                    }
                }
                else
                {//��֧�ַ�����ͬ��������˲�Ӧ������ͬ��Ӧ����Ϊ��ЧӦ����
                    n=g_psam_info[g_curr_slot].m_nrpcb|0x02;
                    gp_icc_content->m_lasterrtype=IC_T1_LASTSINFERR;
                    flg = 1;       //�����һ�鷢�͵�S�飬ҲҪ�������������I��
                }
                break;
            //=========================================
            }
        }
    }
    j = gp_icc_content->m_txbuf[1];
    if(((0x81==n) || (0x82==n) || (0x91==n) || (0x92==n))
     &&((j&0x80)==0x80)&&((j&0xe0)!=0xe0))
    {
        n = 0x03;
    }
    if(errcnt != 0)
    {
        errcnt--;
        switch(n&0xe0)
        {
        case 0:
            if(0x03 == n)
            {
                //dev_icc_atom_t1_start(p_emvsim_type[nslot], &g_icc_param[nslot]);
                //if(dev_icc_atom_wait_optend(nslot)<0)
                ret = drv_psam_exchange_t1_block();
                if(ret < 0)
                {   //�޿����µ�
                    //return 0x0f;
                    n = 0x0f;
                    break;
                }
                goto ic_type1_phase_A;
            }
            break;
        //-------------------------------------
        case 0x40:
            break;
        //-------------------------------------
        case 0x80:
            gp_icc_content->m_txbuf[0] = 0;
            gp_icc_content->m_txbuf[1]=n;
            gp_icc_content->m_txbuf[2]=0;
            //dev_icc_atom_t1_start(p_emvsim_type[nslot], &g_icc_param[nslot]);
            //if(dev_icc_atom_wait_optend(nslot)<0)
            ret = drv_psam_exchange_t1_block();
            if(ret < 0)
            {   //�޿����µ�
                //return 0x0f;
                n = 0x0f;
                break;
            }
            goto ic_type1_phase_A;
//            break;
        //-------------------------------------
        case 0xc0:
            gp_icc_content->m_txbuf[0] = gp_icc_content->m_rxbuf[0];
            gp_icc_content->m_txbuf[1] = n|0x20;
            gp_icc_content->m_txbuf[2] = gp_icc_content->m_rxbuf[2];
            //dev_icc_atom_t1_start(p_emvsim_type[nslot], &g_icc_param[nslot]);
            //if(dev_icc_atom_wait_optend(nslot)<0)
            ret = drv_psam_exchange_t1_block();
            if(ret < 0)
            {   //�޿����µ�
                //return 0x0f;
                n=0x0f;
                break;
            }
            if((0xe2==gp_icc_content->m_txbuf[1])
            &&((ICC_PROC_CWTOVER==gp_icc_content->m_result)||(ICC_PROC_BWTOVER==gp_icc_content->m_result)))
            {
                gp_icc_content->m_result = ICC_PROC_EXCHGINT;//13;
                n = 0x0f;
                break;
            }
            goto ic_type1_phase_A;
//            break;
        //-------------------------------------
        case 0xE0:
            if(0xfe == n)
            {
                n = 0x0f;
                gp_icc_content->m_result = ICC_PROC_BLOCKERR;
            }
            break;
        //-------------------------------------
        default:
            break;
        }
    }
    else
    {
        n = 0x0f;
        gp_icc_content->m_result = ICC_PROC_BLOCKERR;
    }
    return n;
}
/****************************************************************************
**Description:       PSAM��T1 exchange����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    1:
**Created by:        pengxuebin,20170719
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_psam_exchange_t1(u8 *wbuf, u16 wlen, u8 *rbuf, u16 *rlen, u16 rxmax)
{
    s32 ret;
    u16 n;
    u16 tmpcnt;
    u16 wrcnt;
    u16 cnt;
    u8 errcnt;
//    u8 sw_bak[2];
    u8 casetype;
//    u16 i;
//    u8 m;
    
    if(4 == wlen)
    {
        //case 1
        gp_icc_content->m_lc = 0;
        gp_icc_content->m_le = 0;
        casetype = 1;
        wbuf[4] = 0;
    }
    else if(5 == wlen)
    {
        //case 2
        gp_icc_content->m_lc = 0;
        gp_icc_content->m_le = wbuf[4];
        casetype = 2;
        
    }
    else if(wlen>5)
    {
        //case 3
        gp_icc_content->m_lc = wbuf[4];
       if(wlen == (gp_icc_content->m_lc+5))
        {
            gp_icc_content->m_le = 0;
            casetype = 3;
        }
        else
        {
            //case 4
            gp_icc_content->m_le = wbuf[gp_icc_content->m_lc+5];
            casetype = 4;
        }
    }
    cnt = 0;
    n = 0;
    errcnt = 3;
    *rlen = 0;
    gp_icc_content->m_lasterrtype = IC_T1_LASTOK;
    
    //===================================================
    wrcnt = 0;
IC_T1_W_R_A:
    gp_icc_content->m_txbuf[0] = 0x00;
    if(g_psam_info[g_curr_slot].m_ifsi>=wlen)
    {
        //û�����ӿ�
        g_psam_info[g_curr_slot].m_ipcb &= 0xdf;
        if(1 == casetype)
        {
            gp_icc_content->m_txbuf[2] = 0x05;
        }
        else if(2 == casetype)
        {
            gp_icc_content->m_txbuf[2] = 0x05;
        }
        else if(3 == casetype)
        {
            gp_icc_content->m_txbuf[2] = wlen;
        }
        else
        {
            gp_icc_content->m_txbuf[2] = wlen;
        }
    }
    else
    {
        //�������ӿ�
        g_psam_info[g_curr_slot].m_ipcb |= 0x20;
        gp_icc_content->m_txbuf[2] = g_psam_info[g_curr_slot].m_ifsi;
    }
    cnt = gp_icc_content->m_txbuf[2];
    g_psam_info[g_curr_slot].m_ipcb ^= 0x40;
    gp_icc_content->m_txbuf[1] = g_psam_info[g_curr_slot].m_ipcb;
    memcpy(&gp_icc_content->m_txbuf[3], wbuf, cnt);
    /*m=0;
    for(i=0; i<(cnt+3); i++)
    {
        m ^= gp_icc_content->txbuf[i];
    }
    //LRC�ɷ��ͺ�������
    gp_icc_content->txbuf[cnt+3] = dev_icc_maths_lrc(0, &gp_icc_content->txbuf[0], cnt+3);
    //gp_icc_content->txlen = cnt+4;*/
    //dev_icc_atom_t1_start(p_emvsim_type[nslot], &g_icc_param[nslot]);
    //ret = dev_icc_atom_wait_optend(nslot);
    ret = drv_psam_exchange_t1_block();
    if(ret < 0)
    {
        n = 0x0f;
    }
    else
    {
        n = drv_psam_exchange_t1_phase(rbuf+*rlen, &tmpcnt, rxmax-*rlen);
    }
    switch(n&0xff)
    {
    case 0:
        *rlen += tmpcnt;
        break;
    //----------------------
    case 1:
    case 0xff:
        errcnt--;
        if(errcnt!=0)
        {
            //�ط�I��
            g_psam_info[g_curr_slot].m_ipcb ^= 0x40;
            goto IC_T1_W_R_A;
        }
        else
        {
            n = 0x0f;
        }
        break;
    //----------------------
    case 0x02:
        wrcnt = cnt;
        errcnt = 3;
IC_T1_W_R_C:
        gp_icc_content->m_txbuf[0] = 0x00;    //NAD=0
        if(g_psam_info[g_curr_slot].m_ifsi>=(wlen-wrcnt))
        {
            //û�����ӿ�
            g_psam_info[g_curr_slot].m_ipcb &= 0xdf;
            cnt = wlen-wrcnt;
        }
        else
        {
            //�������ӿ�
            g_psam_info[g_curr_slot].m_ipcb |= 0x20;
            cnt = g_psam_info[g_curr_slot].m_ifsi;
        }
        g_psam_info[g_curr_slot].m_ipcb ^= 0x40;
        gp_icc_content->m_txbuf[1] = g_psam_info[g_curr_slot].m_ipcb;
        memcpy(&gp_icc_content->m_txbuf[3], &wbuf[wrcnt], cnt);
        gp_icc_content->m_txbuf[2] = cnt;
        //LRC�ɷ��ͺ�������
        //gp_icc_content->txbuf[cnt+3] = dev_icc_maths_lrc(0, &gp_icc_content->txbuf[0], cnt+3);
        //gp_icc_content->txlen = cnt+4;
        //dev_icc_atom_t1_start(p_emvsim_type[nslot], &g_icc_param[nslot]);
        //ret = dev_icc_atom_wait_optend(nslot);
        ret = drv_psam_exchange_t1_block();
        if(ret < 0)
        {
            n = 0x0f;
        }
        else
        {
            n = drv_psam_exchange_t1_phase(rbuf+*rlen, &tmpcnt, rxmax-*rlen);
        }
        switch(n&0xff)
        {
        case 0:
            *rlen += tmpcnt;
            break;
        //----------------------------------    
        case 1:
        case 0xff:
            errcnt--;
            if(errcnt!=0)
            {
                //�ط�I��
                g_psam_info[g_curr_slot].m_ipcb ^= 0x40;
                goto IC_T1_W_R_C;
            }
            else
            {
                n = 0x0f;
            }
            break;
        //----------------------------------
        case 0x02:
            wrcnt += cnt;
            errcnt = 3;
            goto IC_T1_W_R_C;
        }
        break;
    }
    //-----------------------------
//IC_T1_W_R_END:    
    if(n == 0x0F)
    {
        gp_icc_content->m_result = ICC_PROC_BLOCKERR;  //sw��
        //�µ�
        drv_psam_inactive();

        return ICC_PROC_BLOCKERR;
    }
    
    return 0;
    
    
}
/****************************************************************************
**Description:       PSAM����ʼ��
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170617
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_psam_init(void)
{
    static u8 flg=0;
    s32 ret;
    str_timer_cfg_t t_cfg;
    s32 i;
    
    if(flg==1)
    {
        return DEVSTATUS_SUCCESS;
    }
    ret = drv_psam_io_init();
    if(ret < 0)
    {
        PSAM_DEBUG("Err(ret=%d)\r\n", ret);
        return DEVSTATUS_ERR_FAIL;
    }
    //��ʼ������
    g_psam_status.m_busying = PSAM_STATUS_FREE;
    g_psam_status.m_bitstep = 0;  
    g_psam_status.m_func = NULL;
    for(i=0; i<PSAM_NUM_MAX; i++)
    {
        g_psam_fd[i] = -1;
        g_psam_info[i].m_resettype = 0;
        g_psam_info[i].m_exist = 1;         //����PSAM���޷��ж��Ƿ��п��������п�����
    }
    //��ʼ����ʱ��
    t_cfg.m_tus = 100;  
    t_cfg.m_reload = 1; 
    ret = dev_timer_request(PSAM_TIMER_ID, t_cfg, drv_psam_timer_handler, NULL);
    if(ret < 0)
    {
        PSAM_DEBUG("Err(ret=%d)\r\n", ret);
        return DEVSTATUS_ERR_FAIL;
    }
    ret = drv_psam_io_irq_request(0, drv_psam_slot0_handler);
    if(ret < 0)
    {
        PSAM_DEBUG("Err(ret=%d)\r\n", ret);
        return DEVSTATUS_ERR_FAIL;
    }
  #if 0  
    ret = drv_psam_io_irq_request(1, drv_psam_slot1_handler);
    if(ret < 0)
    {
        return DEVSTATUS_ERR_FAIL;
    }
  #endif  
    
    flg = 1;
    return DEVSTATUS_SUCCESS;
}
/****************************************************************************
**Description:       PSAM����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170628
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_psam_open(s32 psamno)
{
//    s32 ret;
    if(drv_psam_setslot(psamno) < 0)
    {
        PSAM_DEBUG("nslot=%d\r\n", psamno);
        return DEVSTATUS_ERR_PARAM_ERR;          //��������
    }
        
    if(drv_psam_init() < 0)
    {
        return DEVSTATUS_ERR_FAIL;
    }
    
    if(g_psam_fd[psamno] < 0)
    {
        //�򿪵�Դ
//        drv_psam_power_ctl( psamno, 0);       �ĵ���λ��
        g_psam_info[psamno].m_resettype = 0;
        g_psam_fd[psamno] = 0;
    }
    g_psam_100clk = PSAM_TIMER_CLK/(PSAM_CLK_Frequency/100);    //PSAM��100CLK��Ӧ��ʱ������
    return DEVSTATUS_SUCCESS;
}
/****************************************************************************
**Description:       PSAM���ر�
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170628
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_psam_close(s32 psamno)
{
//    s32 ret;
    
    if(drv_psam_setslot(psamno) < 0)
    {
        PSAM_DEBUG("nslot=%d\r\n", psamno);
        return DEVSTATUS_ERR_PARAM_ERR;          //��������
    }
    if(g_psam_fd[psamno]==0)
    {
        //�µ�
        drv_psam_inactive();
        //g_psam_info[psamno].m_resettype = 0;
        //g_psam_info[psamno].m_exist = 1;
        g_psam_fd[psamno] = -1;
    }
    return DEVSTATUS_SUCCESS;
}
/****************************************************************************
**Description:       PSAM���ر�
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170628
**-------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_psam_poweroff(s32 psamno)
{
    if(drv_psam_setslot(psamno) < 0)
    {
        PSAM_DEBUG("nslot=%d\r\n", psamno);
        return DEVSTATUS_ERR_PARAM_ERR;          //��������
    }
    if(g_psam_fd[psamno]<0)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }
    drv_psam_inactive();
    return DEVSTATUS_SUCCESS;
}
/****************************************************************************
**Description:       PSAM���ϵ縴λ
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170628
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_psam_reset(s32 psamno, u8 *lpAtr)
{
    u8 bit_flg;
    s32 ret;
//    s32 cnt;
    s32 i;
    str_icc_info_t *lp_icc_info;
    u8 tmp[64];
    u16 tmplen;
    
    if(drv_psam_setslot(psamno) < 0)
    {
        PSAM_DEBUG("nslot=%d\r\n", psamno);
        return DEVSTATUS_ERR_PARAM_ERR;          //��������
    }
    if(g_psam_fd[psamno]<0)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }
    lp_icc_info = &g_psam_info[psamno];
    
    if(lp_icc_info->m_exist == 0)
    {
        return DEVSTATUS_ERR_FAIL;//ICNOCARD;        //û�п�
    }

    if(lp_icc_info->m_resettype == 0)
    {
        bit_flg = 0;           //�临λ
    }
    else
    {
        bit_flg = 1;            //�ȸ�λ
    }
    while(1)
    {
        lp_icc_info->m_protocolType = 1;
        lp_icc_info->m_Nbit = 0;
        lp_icc_info->m_icfd = 0;
        lp_icc_info->m_gt   = 0;
        lp_icc_info->m_cwt  = ICC_CWT_DEFAULT;
        lp_icc_info->m_bwt  = ICC_BWT_DEFAULT;
        drv_psam_active(bit_flg);
        
        ret = drv_psam_rec_rst(tmp, &tmplen);
        if((ret == ICC_PROC_T1PARITYERR)||(ret == ICC_PROC_T1CRCERR))
        {
            i = 1;          
        }
        else if(ret != ICC_PROC_OK)//(ret == ICC_PROC_RESETERR) || (ret == ICC_PROC_NORESPOND))
        {
            i = 2;          
        }
        else
        {
            i = drv_psam_rst_phase(bit_flg, tmp, tmplen);
        }
        
        if((bit_flg == 0)&&(i == 2))
        {
            //����������
            bit_flg = 1;
            lp_icc_info->m_icfd = 0;
            drv_psam_waitetu(50);
            continue;
        }
        else if(i != 0)
        {
            drv_psam_inactive();
            ret = DEVSTATUS_ERR_FAIL;
        }
        else
        {
            memcpy(lpAtr, tmp, tmplen);    
            if(lp_icc_info->m_protocolType == 1)
            {
                ret = drv_psam_exchange_t1_IFSC();
                if(ret < 0)
                {
                    break;
                }
            }
            //��λ�ɹ�
            ret = tmplen;//DEVSTATUS_SUCCESS;
            lp_icc_info->m_resettype = 1;
        }
        break;
    }
    return ret;
}
/****************************************************************************
**Description:       PSAM���ϵ縴λ
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170628
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_psam_exchange_apdu(s32 psamno, u8 *wbuf, u32 wlen, u8 *rbuf, u32 *rlen, u32 rxmax)
{
    S32 ret;
    u16 tmplen;

    if(drv_psam_setslot(psamno) < 0)
    {
        PSAM_DEBUG("nslot=%d\r\n", psamno);
        return DEVSTATUS_ERR_PARAM_ERR;          //��������
    }
    if(g_psam_fd[psamno]<0)
    {
        PSAM_DEBUG("NOTOPEN\r\n");
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }
    
    if(g_psam_info[psamno].m_exist == 0)
    {
        PSAM_DEBUG("NO CARD!\r\n");
        return DEVSTATUS_ERR_FAIL;//ICNOCARD;        //û�п�
    }

    if(g_psam_info[psamno].m_resettype == 0)
    {
        PSAM_DEBUG("NOT RESET!\r\n");
        return DEVSTATUS_ERR_FAIL;//ICNOCARD;        //û��û�и�λ
    }
    if((wlen > (256+6)) || (wlen<4))
    {
        //���ݳ��ȴ�������R-APDU����
        PSAM_DEBUG("ICC_PROC_PARAERR!\r\n");
        return DEVSTATUS_ERR_FAIL; 
    }
    if(rxmax < 2)
    {
        //�����ĳ��ȴ�
        PSAM_DEBUG("ICC_PROC_PARAERR!\r\n");
        return DEVSTATUS_ERR_FAIL; 
    }
    //�������
    gp_icc_content = k_malloc(sizeof(icc_content_t));
    if(gp_icc_content == NULL)
    {
        //��̬�������,
        PSAM_DEBUG("kmalloc err!\r\n");
        return -1;
    }
    if(0 == g_psam_info[psamno].m_protocolType)
    {
        ret = drv_psam_exchange_t0(wbuf, wlen, rbuf, &tmplen, rxmax);
    }
    else
    {
        ret = drv_psam_exchange_t1(wbuf, wlen, rbuf, &tmplen, rxmax);
    }
    *rlen = tmplen;
    
    k_free(gp_icc_content);
    gp_icc_content = NULL;
    return ret;
}
#else
s32 drv_psam_init(void)
{
    return 0;
}
s32 drv_psam_open(s32 psamno)
{
    return DEVSTATUS_ERR_FAIL;
}
s32 drv_psam_close(s32 psamno)
{
    return DEVSTATUS_ERR_FAIL;
}
s32 drv_psam_poweroff(s32 psamno)
{
    return DEVSTATUS_ERR_FAIL;
}
s32 drv_psam_reset(s32 psamno, u8 *lpAtr)
{
    return DEVSTATUS_ERR_FAIL;
}
s32 drv_psam_exchange_apdu(s32 psamno, u8 *wbuf, u32 wlen, u8 *rbuf, u32 *rlen, u32 rxmax)
{
    return DEVSTATUS_ERR_FAIL;
}

#endif

