/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    ���ļ�����LED�������ӿ�
  
****************************************************************************/
#include "devglobal.h"
#include "ddi/ddi_led.h"



typedef struct _strLedCtrl
{
    u8  m_flg;//ʹ����˸��ʶ, 0 ���� 1 ��
    u8  m_status;   //��¼��ǰ״̬, 0 ��  1 ��
    u32 m_times;    //�صƴ���,��˸�����ƿ�ʼ,�صƽ���
    u32 m_timeid;//��˸��ʼʱ��
    u32 m_ontime;//��ʱ��
    u32 m_offtime;//��ʱ��
//    u32 m_duration;//����ʱ��
}strLedCtrl_t;
typedef struct _strLedAttrib
{
    iomux_pin_name_t m_gpiopin;     //GPIO�ܽ�,
    u8 m_type;                      //�ܽ�����:0:MCU GPIO��1:�ǽ���չ��
    u8 m_pinno;                     //�ܽź�:��Էǽӵ���չ�� 
    u8 m_mod;                       //ģʽ 0:��������˸ 1:������˸
    u8 m_offlevel;                  //LED�رյĵ�ƽ
}strLedAttrib_t;

strLedCtrl_t g_strLedCtrl[LED_NUM_MAX];
static s32 g_dev_led_fd = -1;



//LED_BLUE_NO        0
//LED_YELLOW_NO      1
//LED_GREEN_NO       2
//LED_RED_NO         3
const strLedAttrib_t str_led_attrib_tab[LED_NUM_MAX] = 
{
#if 1
{GPIO_PIN_PTD14,0, 0, 1, 1},     //LED_STATUS_B1        0    
{GPIO_PIN_PTD15,0, 0, 1, 1},     //LED_STATUS_R       1
{GPIO_PIN_PTD13,0, 0, 1, 1},     //LED_SIGNAL_Y       2
{GPIO_PIN_PTD12,0, 0, 1, 1},     //LED_SIGNAL_B2         3

#else
#if(MACHINETYPE==MACHINE_S1)
    {GPIO_PIN_PTC12,0, 0, 1, 1},     //LED_BLUE_NO        0    
    {GPIO_PIN_PTC14,0, 0, 1, 1},     //LED_YELLOW_NO      1
    {GPIO_PIN_PTC15,0, 0, 1, 1},     //LED_GREEN_NO       2
    {GPIO_PIN_PTC13,0, 0, 1, 1},     //LED_RED_NO         3
#elif(MACHINETYPE==MACHINE_M2) 
    {GPIO_PIN_NONE, 1, 4, 0, 0},     //LED_BLUE_NO        0
    {GPIO_PIN_NONE, 1, 3, 0, 0},     //LED_YELLOW_NO      1
    {GPIO_PIN_PTD5, 0, 0, 1, 0},     //LED_GREEN_NO       2
    {GPIO_PIN_PTB0, 0, 0, 1, 0},     //LED_RED_NO         3 
#endif
#endif
};

static s32 g_power_on_status_switch = 1;              //P6��Ӳ��δ�ӻƵƣ�ͨ���������ʵ�֣�������ƵƳ�����˸��Ĭ��Ϊ����ע���󣬹ر�
static s32 g_led_loop_status = 0;//led ѭ����״̬
/****************************************************************************
**Description:       ��LED�豸
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170428
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_led_open(void)
{
    s32 i;
    
    if(g_dev_led_fd < 0)
    {
        //����LED_���
        for(i=0; i<LED_NUM_MAX; i++)
        {
            if(str_led_attrib_tab[i].m_type == 0)
            {
                //GPIO�ܽ�,���ó����0
                #ifdef TRENDIT_BOOT
                dev_gpio_config_mux(str_led_attrib_tab[i].m_gpiopin, MUX_CONFIG_ALT1);   //����ΪGPIO��
                dev_gpio_set_pad(str_led_attrib_tab[i].m_gpiopin, PAD_CTL_PULL_UP);
                dev_gpio_direction_output(str_led_attrib_tab[i].m_gpiopin, str_led_attrib_tab[i].m_offlevel);
                #endif
            }
            else 
            {
                //dev_pcd_open();
                #if(MACHINETYPE==MACHINE_M2)
                #ifdef TRENDIT_CORE
                dev_pcd_export_ctl(str_led_attrib_tab[i].m_pinno, str_led_attrib_tab[i].m_offlevel);
                #endif
                #endif
            }
            g_strLedCtrl[i].m_flg = 0;
            g_strLedCtrl[i].m_status = 0;
        }
        g_dev_led_fd = 0;
    }
    return DEVSTATUS_SUCCESS;
}
/****************************************************************************
**Description:       �ر�LED�豸
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170428
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_led_close(void)
{
    s32 i;

    if(g_dev_led_fd>=0)
    {
        g_dev_led_fd = -1;
        for(i=0; i<LED_NUM_MAX; i++)
        {
            if(str_led_attrib_tab[i].m_type == 0)
            {
                //GPIO�ܽ����
                dev_gpio_set_value(str_led_attrib_tab[i].m_gpiopin, str_led_attrib_tab[i].m_offlevel);
            }
            else 
            {
                #if(MACHINETYPE==MACHINE_M2)
                #ifdef TRENDIT_CORE
                dev_pcd_export_ctl(str_led_attrib_tab[i].m_pinno, str_led_attrib_tab[i].m_offlevel);
                #endif
                #endif
            }
            g_strLedCtrl[i].m_flg = 0;
            g_strLedCtrl[i].m_status = 0;
        }
    } 
    return DEVSTATUS_SUCCESS;
}
/****************************************************************************
**Description:       ����LED
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170428
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 dev_led_ctl(u8 nLed, u8 mode)
{
    s32 i;
    u8 n;


    if(nLed < LED_NUM_MAX)
    {
        if(0 == str_led_attrib_tab[nLed].m_offlevel)
        {
            n = mode;
        }
        else
        {
            n = (~mode)&0x01;
        }
        if(str_led_attrib_tab[nLed].m_type == 0)
        {
            dev_gpio_set_value(str_led_attrib_tab[nLed].m_gpiopin, n);
        }
        else
        {
            #if(MACHINETYPE==MACHINE_M2)
            #ifdef TRENDIT_CORE
            dev_pcd_export_ctl(str_led_attrib_tab[nLed].m_pinno, n);
            #endif
            #endif
        }
        g_strLedCtrl[nLed].m_status = mode;
    }
    else if(nLed == LED_ALL)
    {
        for(i=0; i<LED_NUM_MAX; i++)
        {
            if(0 == str_led_attrib_tab[i].m_offlevel)
            {
                n = mode;
            }
            else
            {
                n = (~mode)&0x01;
            }
            if(str_led_attrib_tab[i].m_type == 0)
            {
                //GPIO�ܽ����
                dev_gpio_set_value(str_led_attrib_tab[i].m_gpiopin, n);
            }
            else 
            {
                #if(MACHINETYPE==MACHINE_M2)
                #ifdef TRENDIT_CORE
                dev_pcd_export_ctl(str_led_attrib_tab[i].m_pinno, n);
                #endif
                #endif
            }
            g_strLedCtrl[i].m_status = mode;
        }
    }
    else
    {   
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    return DEVSTATUS_SUCCESS;
}
/****************************************************************************
**Description:       LED��פ�������ڿ���LED��˸����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170428
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_led_task(void)
{
    u32 timeid;
    u32 tmp;
    u8 led = 0;

    if(g_dev_led_fd < 0)
    {
        return;
    }
    timeid = dev_user_gettimeID();
    for(led=0; led<LED_NUM_MAX; led++)
    {
        if(g_strLedCtrl[led].m_flg == 1)
        {
            tmp = timeid - g_strLedCtrl[led].m_timeid;
            if(g_strLedCtrl[led].m_status == 0)
            {
                if(tmp >= g_strLedCtrl[led].m_offtime)
                {
                    //��Ƴ���ʱ�䵽,��Ҫ����
                    dev_led_ctl(led, 1);
                    g_strLedCtrl[led].m_timeid = timeid;
                }
            }
            else
            {
                if(tmp >= g_strLedCtrl[led].m_ontime)
                {
                    //���Ƴ���ʱ�䵽,��Ҫ���
                    dev_led_ctl(led, 0);
                    g_strLedCtrl[led].m_timeid = timeid;
                    if(g_strLedCtrl[led].m_times > 0)
                    {
                        g_strLedCtrl[led].m_times--;
                    }
                    if(g_strLedCtrl[led].m_times == 0)
                    {
                        g_strLedCtrl[led].m_flg = 0;    //����˸
                    }
                }
            }
        }
    }
}
/****************************************************************************
**Description:       ���õ�
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170428
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_led_sta_set(u32 nLed, u32 nSta)
{
    if(g_dev_led_fd<0)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }
    if(nLed >= LED_NUM_MAX)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    g_strLedCtrl[nLed].m_flg = 0;
    dev_led_ctl(nLed, nSta);
    return DEVSTATUS_SUCCESS;
}
/****************************************************************************
**Description:       ioctl�ӿ�
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170428
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_led_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    s32 ret;
    strLedGleamPara *p_led_gleam;
    u32 i;

    if(g_dev_led_fd < 0)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;        
    }
    switch(nCmd)
    {
  #if 0  
    case DDI_LED_CTL_VER:
        if(wParam == NULL)
        {
            ret = DDI_EINVAL;
        }
        else
        {
            strncpy((u8*)wParam, led_ver, strlen(led_ver));
            ret = DDI_OK;
        }
        break;
  #endif 
    case DDI_LED_CTL_GLEAM:
        p_led_gleam = (strLedGleamPara *)lParam;

        if(p_led_gleam->m_led >= LED_NUM_MAX)
        {
            ret = DEVSTATUS_ERR_PARAM_ERR;
        }
        else if(p_led_gleam->m_ontime == 0 && p_led_gleam->m_offtime == 0)
        {
            //uart_printf("LED ��˸ʱ�䶼Ϊ0\r\n");
            ret = DEVSTATUS_ERR_PARAM_ERR;
        }
        else if(str_led_attrib_tab[p_led_gleam->m_led].m_type != 0)
        {
            //��GPIO��,��������˸
            ret = DEVSTATUS_ERR_FAIL;
        }
        else
        {   
            i = ((p_led_gleam->m_duration+(p_led_gleam->m_ontime+p_led_gleam->m_offtime)-1)/(p_led_gleam->m_ontime+p_led_gleam->m_offtime));
            if(i == 0)
            {
                ret = DEVSTATUS_ERR_PARAM_ERR;
            }
            else
            {
                dev_led_ctl(p_led_gleam->m_led, 1);
                g_strLedCtrl[p_led_gleam->m_led].m_times = i;
                g_strLedCtrl[p_led_gleam->m_led].m_ontime  = p_led_gleam->m_ontime;
                g_strLedCtrl[p_led_gleam->m_led].m_offtime  = p_led_gleam->m_offtime;
                g_strLedCtrl[p_led_gleam->m_led].m_timeid   = dev_user_gettimeID();
                g_strLedCtrl[p_led_gleam->m_led].m_flg  = 1;//��ʼ��˸
                ret = DEVSTATUS_SUCCESS; 
            }
        }
        break; 

    case DDI_LED_CTL_CLOSE_YELLOW:
        g_power_on_status_switch = 0;
        dev_led_sta_set(LED_SIGNAL_Y, 0);
        break;
        
    case DDI_LED_CTL_LOOP:
        g_led_loop_status = lParam;
        ret = DDI_OK;
        break;

    default:
        return DDI_EINVAL;
    }
    
    return ret;
}

/**
 * @brief ���е�ѭ����ʾ1s
 */
void dev_led_loop_show()
{
    s32 loop_time = 100;//1s
    static s32 time_cnt = 0;

    s32 led_cnt = LED_NUM_MAX;
    static s32 i = 0;


    if(time_cnt == loop_time)
    {
        if(i >= LED_NUM_MAX)
        {
            i = 0;
        }
        dev_led_sta_set(LED_STATUS_B, 0);
        dev_led_sta_set(LED_STATUS_R, 0);
        dev_led_sta_set(LED_SIGNAL_Y, 0);
        dev_led_sta_set(LED_SIGNAL_B, 0);
        dev_led_sta_set(i, 1);
        i++;
        time_cnt = 0;
    }
    time_cnt++;
}


/**
 * @brief ͨ������������Ƶ���ʶ�𿪻�״̬
 */
void dev_led_loop(void)
{
    static s32 flag = 0;
    static s32 hold_time = 0;
    s32 hold_max_time = 100;

    if(0 == g_dev_led_fd && g_power_on_status_switch)
    {
        hold_time++;

        if(0 == flag && hold_max_time == hold_time)
        {
            flag = 1;
            hold_time = 0;
            dev_led_sta_set(LED_SIGNAL_Y, 1);
        }
        else if(1 == flag && hold_max_time == hold_time)
        {
            flag = 0;
            hold_time = 0;
            dev_led_sta_set(LED_SIGNAL_Y, 0);
        }
    }
    if((0 == g_dev_led_fd && g_led_loop_status))
    {
        dev_led_loop_show();
        //dev_led_sta_set(LED_STATUS_B, 1);
        //dev_led_sta_set(LED_STATUS_R, 1);
        //dev_led_sta_set(LED_SIGNAL_B, 1);
        //dev_led_sta_set(LED_SIGNAL_Y, 1);
    }
}
