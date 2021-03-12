/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    ���ļ�����������ؽӿ�
  
****************************************************************************/
#include "devglobal.h"
#include "bt_patch.h"
#include "drv_bt.h"
#if(BT_EXIST==1) 
#define BT_COM_PORT     0
#define BT_WAKEUP_PIN   GPIO_PIN_PTA15
#define BT_REST_PIN     GPIO_PIN_PTA3
#define BT_RTS_PIN      GPIO_PIN_PTA13

#define BT_YC_UART_CLOCK        48000000
#define BT_YC_DEFAULT_BAUDRATE  115200
#define BT_YC_WORK_BAUDRATE     750000//1000000//921600

#define BT_YC_CMD           0x01
#define BT_YC_CMD_OGF       0xfc
#define BT_YC_CMD_RESET     0x00
#define BT_YC_CMD_BAUD      0x02
#define BT_YC_CMD_ECHO      0x05

#define BT_RX_BUF_MAX    1024   //BT���ջ���
typedef struct _str_btdrv_buf
{
    str_CircleQueue m_rxqueue;
    u32 m_timeid;
    u8 m_rxbuf[256];        //���չ�������
    u8 m_rxlen;             //���չ��̳���
    u8 m_rxcnt;             //���չ��̼���
    u8 m_rxcmd;             //���յ�������
    volatile u8 m_step;     //����״̬
    volatile u8 m_dealflg;  //��ʱ�����־
    volatile u8 m_connect;  //���ӱ�־,0:δ����, 1:SPP,  2:BLE
}str_btdrv_buf;
static str_btdrv_buf g_bt_buf;
#define BT_EVENT_BYTE_TIMEOUT   100      //���������ַ��䳬ʱ50ms
static volatile u32 g_bt_status_check_timer=0;
#define BT_STATUS_CHECK_TIMEROUT   5000      //����5000ms��������
#define BT_SEND_SPP_PACKET_MAX     127      //����SPPһ�����ݳ��ȣ�����127ʱ���Դﵽ��ѵ�����������
#define BT_SENT_BLE_PACKET_MAX     (254-2)
const u8 bt_ble_attrib[] = {0x2A,0x00}; //Ĭ��ATTRIB
/****************************************************************************
**Description:       ������������
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170610
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_param_set(u8 btmod, u8 *btname, u8 *btmac, u8 *nvram)
{
    s32 ret;
    str_btparam_t l_btparam;

    ret = dev_misc_machineparam_get(MACHINE_PARAM_BT, (u8*)&l_btparam, STR_BTPARAM_LEN);
    if(ret < 0)
    {
        return -1;
    }
    if(btmod&BTPARAM_NAME_BIT)
    {
        l_btparam.m_btmod |= BTPARAM_NAME_BIT;
        memset(l_btparam.m_btname, 0, 32);
        strcpy(l_btparam.m_btname, btname);
    }
    if(btmod&BTPARAM_MAC_BIT)
    {
        l_btparam.m_btmod |= BTPARAM_MAC_BIT;
        memcpy(l_btparam.m_btmac, btmac, 6);
    }
    if(btmod&BTPARAM_NVRAM_BIT)
    {
        l_btparam.m_btmod |= BTPARAM_NVRAM_BIT;
        memcpy(l_btparam.m_nvram, nvram, 120);
    }
//    ret = dev_misc_btparam_write((u8*)&l_btparam, STR_BTPARAM_LEN);
    ret = dev_misc_machineparam_set(MACHINE_PARAM_BT, (u8*)&l_btparam, STR_BTPARAM_LEN);
    if(ret < 0)
    {
        return -1;
    }
    return 0;
}
/****************************************************************************
**Description:       ����������ȡ
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170610
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_param_get(u8 *btmod, u8 *btname, u8 *btmac, u8 *nvram)
{
    s32 ret;
    str_btparam_t l_btparam;

    *btmod = 0;//l_btparam.m_btmod;
    ret = dev_misc_machineparam_get(MACHINE_PARAM_BT, (u8*)&l_btparam, STR_BTPARAM_LEN);
    if(ret < 0)
    {
        return -1;
    }
    if(btname != NULL)
    {
        if(l_btparam.m_btmod&BTPARAM_NAME_BIT)
        {
            *btmod |= BTPARAM_NAME_BIT;
            strcpy(btname, l_btparam.m_btname);
        }
    }
    if(btmac != NULL)
    {
        if(l_btparam.m_btmod&BTPARAM_MAC_BIT)
        {
            *btmod |= BTPARAM_MAC_BIT;
            memcpy(l_btparam.m_btmac, btmac, 6);
        }
    }
    if(nvram != NULL)
    {
        if(l_btparam.m_btmod&BTPARAM_NVRAM_BIT)
        {
            *btmod |= BTPARAM_NVRAM_BIT;
            memcpy(l_btparam.m_nvram, nvram, 120);
        }
    }
    return 0;
}
/****************************************************************************
**Description:       �����˿ڳ�ʼ��
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170606
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_bt_init(void)
{
    static u8 flg=0;

    if(flg==0)
    {
        flg = 1;
        dev_com_open(BT_COM_PORT, 115200, 8, 'n', 0, 0);
        //���ø�λ��,���1
        dev_gpio_config_mux(BT_REST_PIN, MUX_CONFIG_GPIO);
        dev_gpio_set_pad(BT_REST_PIN, PAD_CTL_PULL_UP);
        dev_gpio_direction_output(BT_REST_PIN, 1);
        //���û��ѽ�,���0
        dev_gpio_config_mux(BT_WAKEUP_PIN, MUX_CONFIG_GPIO);
        dev_gpio_set_pad(BT_WAKEUP_PIN, PAD_CTL_PULL_UP);
        dev_gpio_direction_output(BT_WAKEUP_PIN, 0);
        //      
        dev_gpio_config_mux(BT_RTS_PIN, MUX_CONFIG_GPIO);
        dev_gpio_set_pad(BT_RTS_PIN, PAD_CTL_PULL_UP);
        dev_gpio_direction_output(BT_RTS_PIN, 0);
        
        g_bt_buf.m_step = 0;
        g_bt_buf.m_dealflg = 0;
        g_bt_buf.m_connect = 0;
        g_bt_buf.m_timeid = dev_user_gettimeID();
        g_bt_status_check_timer = g_bt_buf.m_timeid;
        if(dev_circlequeue_init(&g_bt_buf.m_rxqueue, BT_RX_BUF_MAX)<0)
        {
            return;
        }
    }
}
/****************************************************************************
**Description:       �������ѽſ���
**Input parameters:    flg:  1 ����
                          0 SLEEP
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170606
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_bt_wakeup_ctl(u8 flg)
{
//BT_DEBUG("flg=%d\r\n", flg);    
    if(flg)
    {
        dev_gpio_set_value(BT_WAKEUP_PIN, 1);
    }
    else
    {
        dev_gpio_set_value(BT_WAKEUP_PIN, 0);
    }
}
/****************************************************************************
**Description:       ����Ӳ����λ
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170606
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_bt_hdrst(void)
{
//BT_DEBUG("\r\n");    
    //dev_gpio_set_value(BT_WAKEUP_PIN, 1);
    drv_bt_wakeup_ctl(1);
    dev_gpio_set_value(BT_REST_PIN, 0);
    dev_user_delay_ms(10);
    dev_gpio_set_value(BT_REST_PIN, 1);
    dev_user_delay_ms(500);
    dev_com_setbaud(BT_COM_PORT, BT_YC_DEFAULT_BAUDRATE, 8, 'n', 0, 0);
    dev_com_flush(BT_COM_PORT);
}
/****************************************************************************
**Description:       BOOT��soft reset
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170609
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_bt_boot_isCmdComplete(void)
{
    u8 event[7];
    s32 timeid;
    s32 i;
    s32 ret;
    
    timeid = dev_user_gettimeID();
    i = 0;
    while(1)
    {
        ret = dev_com_read(BT_COM_PORT, &event[i], 10);
        i += ret;
        if(i>=7)
        {
//BT_DEBUG("rev:i=%d\r\n", i);
//BT_DEBUGHEX(NULL, event, i);
            if((0x04 != event[0])    //packettype == event
             ||(0x0e != event[1])    //event opcde == command complete
             ||(BT_YC_CMD_OGF != event[5])
             ||(0x00 != event[6]))   //status == successful
            {
                BT_DEBUG("ERR(%02X,%02X,%02X,%02X)\r\n", event[0], event[1], event[5], event[6]);
                return -1;
            }
            else
            {
                break;
            }
        }
        if(dev_user_querrytimer(timeid, 100))
        {
            BT_DEBUG("Timeout\r\n");
            return -2;
        }
    } 
    return 0;
}
/****************************************************************************
**Description:       BOOT�������λ
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170609
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_boot_swrst(void)
{
    u8 cmd[4] = {0x01, 0x00, 0xFC, 0x00};
    s32 ret;

    dev_com_flush(BT_COM_PORT);
    dev_com_write(BT_COM_PORT, cmd, 4);
    ret = drv_bt_boot_isCmdComplete();   
    if(ret < 0)
    {
        BT_DEBUG("ERR(%d)\r\n", ret);
    }
    return ret;
}
/****************************************************************************
**Description:       BOOT��soft reset
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170609
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_bt_boot_echo(void)
{
    u8 cmd[4] = {0x01, 0x05, 0xFC, 0x00};
    s32 ret;

    dev_com_flush(BT_COM_PORT);
    dev_com_write(BT_COM_PORT, cmd, 4);
    ret = drv_bt_boot_isCmdComplete();
    if(ret < 0)
    {
        BT_DEBUG("ERR(%d)\r\n", ret);
    }
    return ret;
}/****************************************************************************
**Description:       BOOT��baud set
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170609
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_bt_boot_setbaud(u32 baud)
{
    u8 cmd[6];
    u32 i;
    
    //���㲨����
    i = (BT_YC_UART_CLOCK+(baud>>1))/baud;
    cmd[0] = BT_HCI_PACKET_CMD;
    cmd[1] = BT_YC_CMD_BAUD;        //boot�����ò���������
    cmd[2] = BT_YC_CMD_OGF;         //
    cmd[3] = 0x02;                  //len
    cmd[4] = (i&0xFF);
    cmd[5] = ((i>>8)&0xFF);
//BT_DEBUG("%s(%d):i=%08X\r\n", __FUNCTION__, __LINE__, i);  
//BT_DEBUGHEX(NULL,cmd, 6);  
    dev_com_write(BT_COM_PORT, cmd, 6);
    dev_com_tcdrain(BT_COM_PORT);
    
    dev_user_delay_ms(10);

    dev_com_setbaud(BT_COM_PORT, baud, 8, 'n', 0, 0);

//BT_DEBUG("%s(%d):baud=%d\r\n", __FUNCTION__, __LINE__, baud);    
    return drv_bt_boot_echo();
}

/****************************************************************************
**Description:       ��������Ӧ�ó���
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170606
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_boot_patch(void)
{
    s32 len;
    s32 ret;
    u8 buf[256];
    s32 i;
    s32 total_len;
    s32 offset;
    u32 timeid;
//    u32 time1,time2;

//time1 = dev_user_gettimeID();
    //��ȡpatch����
    total_len = (((u16)yc_bt_patch[1])<<8) + yc_bt_patch[0];
    offset = 2;

//BT_DEBUG("total_len=%d\r\n", total_len);
    ret = drv_bt_boot_setbaud(BT_YC_WORK_BAUDRATE);
    if(ret < 0)
    {
        BT_DEBUG("ERR(%d)!\r\n", ret);
        return ret;
    }
    while(offset < total_len)
    {
        len = yc_bt_patch[offset++];
        if((len+offset) > total_len)
        {
            BT_DEBUG("ERR(%d,%d,%d)\r\n", len, offset, total_len);
            return -1;  //����
        }
        for(i=0; i<len; i++)
        {
            buf[i] = yc_bt_patch[offset++];
        }
//BT_DEBUG("send:len=%d\r\n", len);
//BT_DEBUGHEX(NULL, buf, len);
        dev_com_flush(BT_COM_PORT);
        dev_com_write(BT_COM_PORT, buf, len);
        ret = drv_bt_boot_isCmdComplete();
        if(ret < 0)
        {
            BT_DEBUG("ERR(%d)!\r\n", ret);
            return ret;
        }
    }
    dev_com_setbaud(BT_COM_PORT, BT_YC_DEFAULT_BAUDRATE, 8, 'n', 0, 0);

    //patch����,�ȴ�Ӧ������
    timeid = dev_user_gettimeID();
    i = 0;
    while(1)
    {
        ret = dev_com_read(BT_COM_PORT, &buf[i], 10);
        i += ret;
        if(i>=3)
        {
//BT_DEBUG("rev:i=%d\r\n", i);
//BT_DEBUGHEX(NULL, buf, i);
            if((0x02 != buf[0])    //packettype == event
             ||(0x09 != buf[1])    //event opcde == command complete
             ||(0x00 != buf[2]))   //status == successful
            {
                BT_DEBUG("ERR(%02X,%02X,%02X,%02X)\r\n", buf[0], buf[1], buf[5], buf[6]);
                return -1;
            }
            else
            {
                break;
            }
        }
        if(dev_user_querrytimer(timeid, 1000))
        {
            BT_DEBUG("Timeout\r\n");
            return -2;
        }
    }
 
//time2 = dev_user_gettimeID();
//BT_DEBUG("time=%d(1=%d,2=%d)\r\n", time2-time1, time1, time2);
    return 0;
}
/****************************************************************************
**Description:       ������������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170606
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_bt_write_cmd(u8 opcode, u8 *wbuf, u8 wlen)
{
    u8 head[3];
    //�����������
//    dev_com_flush(BT_COM_PORT);
    //����head
    head[0] = BT_HCI_PACKET_CMD;//packetType;
    head[1] = opcode;
    head[2] = wlen;
BT_DEBUGHEX(NULL, head, 3);
BT_DEBUGHEX(NULL, wbuf, wlen);
    dev_com_write(BT_COM_PORT, head, 3);
    dev_com_write(BT_COM_PORT, wbuf, wlen);
    dev_com_tcdrain(BT_COM_PORT);
    return 0;
}
/****************************************************************************
**Description:       ������������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:û�н��յ���������
                    1:���յ���������
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170606
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_bt_event_receive(void)
{
    static u32 phasetimeid=0;
    u32 i,j;
    u32 currtimeid;
    s32 ret;
    u8 data;

    currtimeid = dev_user_gettimeID();
    if(g_bt_buf.m_step!=0)
    {
        j = currtimeid-phasetimeid;
        i = currtimeid-g_bt_buf.m_timeid;
        if(i > (j+BT_EVENT_BYTE_TIMEOUT))
        {
            //��ʱ
            g_bt_buf.m_step = 0;
            BT_DEBUG("Bytes Timeout!\r\n");
        }
    }
    phasetimeid = currtimeid;
    while(1)
    {
        ret = dev_com_read(BT_COM_PORT, &data, 1);
        if(ret == 1)
        {
dev_debug_printf("%02X ", data);
            g_bt_buf.m_timeid = currtimeid;
            switch(g_bt_buf.m_step)
            {
            case 0:     //����0x02
                if(data == BT_HCI_PACHET_EVENT)
                {
                    g_bt_buf.m_step++;
                }
                break;
            case 1:     //����Opcode
                g_bt_buf.m_rxcmd = data;
                g_bt_buf.m_step++;
                break;
            case 2:     //���ճ���
                if(data == 0)
                {
                    g_bt_buf.m_step = 0;
                    return 1;
                }
                g_bt_buf.m_rxlen = data;
                g_bt_buf.m_rxcnt = 0;
                g_bt_buf.m_step++;
                break;
            case 3:     //��������  
                g_bt_buf.m_rxbuf[g_bt_buf.m_rxcnt++] = data;
                if(g_bt_buf.m_rxcnt >= g_bt_buf.m_rxlen)
                {
                    g_bt_buf.m_step = 0;
                    return 1;
                }
                break;
            }
        }
        else
        {
            break;
        }
    }
    return 0;
}
/****************************************************************************
**Description:       ������������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:û�н��յ���������
                    1:���յ���������
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170606
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_bt_event_phase(u8 mod, u8 opcode, u8 *rbuf, u8 *rlen, u16 le)
{
    s32 ret;
    u8 data;
    
    ret = drv_bt_event_receive();
    if(ret == 1)
    {       
dev_debug_printf("%02X,mod=%d\r\n", g_bt_buf.m_rxcmd, mod); 
        //���յ�����
        switch(g_bt_buf.m_rxcmd)
        {
        case HCI_EVENT_BT_CONNECTED:      //0x00    //BT3.0 ���ӽ���
            g_bt_buf.m_connect = 1;      //SPP������
            g_bt_status_check_timer = dev_user_gettimeID();
            break;
        case HCI_EVENT_BLE_CONNECTED:     //0x02    //BLE ���ӽ���
            g_bt_buf.m_connect = 2;      //BLE������
            g_bt_status_check_timer = dev_user_gettimeID();
            break;
        case HCI_EVENT_BT_DISCONNECTED:   //0x03    //BT3.0 ���ӶϿ�
        case HCI_EVENT_BLE_DISCONNECTED:  //0x05    //BLE ���ӶϿ�
            g_bt_buf.m_connect = 0;      //�Ͽ�����
            g_bt_status_check_timer = dev_user_gettimeID();
            break;
       #if 0     
        case HCI_EVENT_CMD_COMPLETE:      //0x06    //���������
            break;
       #endif     
        case HCI_EVENT_SPP_DATA_RECEIVED: //0x07    //���յ�BT3.0 ���ݣ�SPP��
            //���յ�SPP����
            ret = dev_circlequeue_write(&g_bt_buf.m_rxqueue, g_bt_buf.m_rxbuf, g_bt_buf.m_rxlen);
            if(ret != g_bt_buf.m_rxlen)
            {
                BT_DEBUG("Err(len=%d, %d)!\r\n", ret, g_bt_buf.m_rxlen);
            }
            break;
        case HCI_EVENT_BLE_DATA_RECEIVED: //0x08    //���յ�BLE ����
            //���յ�BLE����
            if(g_bt_buf.m_rxlen>2)
            {
                ret = dev_circlequeue_write(&g_bt_buf.m_rxqueue, &g_bt_buf.m_rxbuf[2], g_bt_buf.m_rxlen-2);
                if(ret != (g_bt_buf.m_rxlen-2))
                {
                    BT_DEBUG("Err(len=%d, %d)!\r\n", ret, g_bt_buf.m_rxlen);
                }
            }
            break;
        case HCI_EVENT_NVRAM_CHANGED:     //0x0D    //�ϴ�NVRAM ����
            //���������
            if(g_bt_buf.m_rxlen == 120)
            {
                drv_bt_param_set(BTPARAM_NVRAM_BIT, NULL, NULL, g_bt_buf.m_rxbuf);
            }
            else
            {
                BT_DEBUG("Err(len=%d)!\r\n", g_bt_buf.m_rxlen);
            }
            break;
        case HCI_EVENT_STAUS_RESPONSE:    //0x0A    //״̬�ظ�
            if(g_bt_buf.m_rxlen == 1)
            {
                data = 0;      //SPP������
                if(g_bt_buf.m_rxbuf[0]&(1<<4))
                {
                    data = 1;      //SPP������
                }
                else if(g_bt_buf.m_rxbuf[0]&(1<<5))
                {
                    data = 2;       //BLE������
                }
                g_bt_buf.m_connect = data;
                g_bt_status_check_timer = dev_user_gettimeID();
            }
            break;
      #if 0      
        case HCI_EVENT_I_AM_READY:        //0x09    //ģ��׼����
            break;
        case HCI_EVENT_UART_EXCEPTION:    //0x0F    //HCI ����ʽ����
            break;
        case HCI_EVENT_GKEY:              //0x0E    //����Numeric Comparison ��Է�ʽ�в�������Կ
            break;
        case HCI_EVENT_GET_PASSKEY:       //0x10    //PASSKEY ��Է�ʽ��֪ͨMCU ������Կ
            break;
      #endif
        default:
            BT_DEBUG("default(opcode=%02X,len=%d)!\r\n", g_bt_buf.m_rxcmd, g_bt_buf.m_rxlen);
            break;
        }
        //�ж��Ƿ���Ҫ�ϴ�����
        if(mod)
        {
            if(opcode == g_bt_buf.m_rxcmd)
            {
                //�յ�ָ����Ӧ
                if(g_bt_buf.m_rxlen > le)
                {
                    *rlen = le;
                    BT_DEBUG("err(rlen=%d,le=%d)\r\n", g_bt_buf.m_rxlen, le);
                }
                else
                {
                    *rlen = g_bt_buf.m_rxlen;
                }
                memcpy(rbuf, g_bt_buf.m_rxbuf, *rlen);
                return 1;
            }
        }
    }
    return 0;
}
/****************************************************************************
**Description:       ������ʱ���������������
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170610
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_bt_task_process(void)
{
    if(g_bt_buf.m_dealflg == 1)
    {
        drv_bt_event_phase(0, 0xFF, NULL, NULL, 0);
    }
}
/****************************************************************************
**Description:       ��������������ʱ�����־
**Input parameters:    
**Output parameters: 
**Returned value:
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_bt_task_ctl_dealflg(u8 flg)
{
  #if 0  
    if(flg == 0)
    {
        g_bt_buf.m_dealflg = 0;
    }
    else
    {
        g_bt_buf.m_dealflg = 1;
    }
  #endif
    g_bt_buf.m_dealflg = flg;
}
/****************************************************************************
**Description:       ��������ָ���¼�
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170606
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_bt_read_event(u8 opcode, u8 *rbuf, u8 *rlen, u16 le, u32 timeout)
{
    u32 timerid;
   
    u32 ret;

    timerid = dev_user_gettimeID();
    while(1)
    {
        ret = drv_bt_event_phase(1, opcode, rbuf, rlen, le);
        if(ret == 1)
        {
            break; 
        }
        if(dev_user_querrytimer(timerid, timeout))
        {
            return DEVSTATUS_ERR_FAIL;
        }
    }
    return DEVSTATUS_SUCCESS;
}
/****************************************************************************
**Description:       HCI_CMD_SET_BT_ADDR ��������BT3.0 �豸��ַ
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_set_bt_addr(u8 *mac)
{
    s32 ret;
    u8 buf[32];
    u8 len;
    
    memcpy(buf, mac, 6);
    ret = drv_bt_write_cmd(HCI_CMD_SET_BT_ADDR, buf, 6);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    
    return ret;
}
/****************************************************************************
**Description:       HCI_CMD_SET_BLE_ADDR ��������BLE �豸��ַ
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_set_ble_addr(u8 *mac)
{
    s32 ret;
    u8 buf[32];
    u8 len;
    
    memcpy(buf, mac, 6);
    ret = drv_bt_write_cmd(HCI_CMD_SET_BLE_ADDR, buf, 6);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    
    return ret;
}
/****************************************************************************
**Description:       HCI_CMD_SET_VISIBILITY �������������Ŀɷ��ֺ͹㲥״̬
**Input parameters:    
                    bt30visi:  bt30�ɷ���(�ɱ�����)
                    bt30connect: bt30������
                    bleADV:    ble�ɷ��ֺ�����
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_set_visibility(u8 bt30visi, u8 bt30connect, u8 bleADV)
{
    s32 ret;
    u8 buf[32];
    u8 len;

    buf[0] = 0;
    if(bt30visi)
    {
        buf[0] |= 0x01;
    }
    if(bt30connect)
    {
        buf[0] |= 0x02;
    }
    if(bleADV)
    {
        buf[0] |= 0x04;
    }
    
    ret = drv_bt_write_cmd(HCI_CMD_SET_VISIBILITY, buf, 1);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    
    return ret;
}
/****************************************************************************
**Description:       HCI_CMD_SET_BT_NAME ��������BT3.0 �������豸����
**Input parameters:    
                    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_set_bt_name(u8 *name)
{
    s32 ret;
    u8 buf[32];
    u8 len;

    memset(buf, 0, 32);
    len = strlen(name);
    if(len >= 32)
    {
        len = 31;
    }
    memcpy(buf, name, len);
        
    ret = drv_bt_write_cmd(HCI_CMD_SET_BT_NAME, buf, len);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    
    return ret;
}
/****************************************************************************
**Description:       HCI_CMD_SET_BLE_NAME ��������BLE �������豸����
**Input parameters:    
                    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_set_ble_name(u8 *name)
{
    s32 ret;
    u8 buf[32];
    u8 len;

    memset(buf, 0, 32);
    len = strlen(name);
    if(len >= 32)
    {
        len = 31;
    }
    memcpy(buf, name, len);
        
    ret = drv_bt_write_cmd(HCI_CMD_SET_BLE_NAME, buf, len);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    
    return ret;
}
/****************************************************************************
**Description:       HCI_CMD_SEND_SPP_DATA ���ڷ���BT3.0 ���ݣ�SPP Э�飩
**Input parameters:    
                    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_bt_hci_cmd_send_spp_data(u8 *wbuf, u8 wlen)
{
    s32 ret;
    u8 buf[256];
    u8 len;

    if(wlen == 0)
    {
        return 0;
    }
        
    ret = drv_bt_write_cmd(HCI_CMD_SEND_SPP_DATA, wbuf, wlen);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 2000);
    if(ret == 0)
    {
        g_bt_status_check_timer = dev_user_gettimeID();
    }
    return ret;
}
/****************************************************************************
**Description:       HCI_CMD_SEND_BLE_DATA ���ڷ���BLE ���ݣ�GATT Э�飩
**Input parameters:    
                   attrib:(Attribute Handle)Ĭ�ϲ���0x2A 00
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 drv_bt_hci_cmd_send_ble_data(u8 *attrib, u8 *wbuf, u8 wlen)
{
    s32 ret;
    u8 buf[256];
    u8 len;

    if(wlen == 0)
    {
        return 0;
    }
    if(wlen>(255-2))
    {
        BT_DEBUG("ERR(wlen=%d)\r\n", wlen);
        return -1;
    }
    memcpy(buf, attrib, 2);
    memcpy(&buf[2], wbuf, wlen);
    ret = drv_bt_write_cmd(HCI_CMD_SEND_BLE_DATA, buf, wlen+2);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    if(ret == 0)
    {
        g_bt_status_check_timer = dev_user_gettimeID();
    }
    return ret;
}
/****************************************************************************
**Description:       HCI_CMD_STATUS_REQUEST ������������״̬
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_status_request(u8 *status)
{
    s32 ret;
    u8 buf[32];
    u8 len;

    ret = drv_bt_write_cmd(HCI_CMD_STATUS_REQUEST, buf, 0);

    ret = drv_bt_read_event(HCI_EVENT_STAUS_RESPONSE, buf, &len, 32, 200);
    if(ret == 0)
    {
        if(len == 1)
        {
            *status = buf[0];
            
        }
        else
        {
            ret = DEVSTATUS_ERR_FAIL;
        }
    }
    return ret;
}
/****************************************************************************
**Description:       HCI_CMD_SET_PAIRING_MODE ��������BT3.0 ����Է�ʽ,Ĭ�����ģʽΪ0x01 Just Work��SSP��
**Input parameters:    
                    mod:0x00:pincode
                        0x01:just work(Ĭ��)
                        0x02: passkey
                        0x03: confirm
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_set_pairing_mode(u8 mod)
{
    s32 ret;
    u8 buf[32];
    u8 len;
    
    buf[0] = mod;
    ret = drv_bt_write_cmd(HCI_CMD_SET_PAIRING_MODE, buf, 1);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    
    return ret;
}
/****************************************************************************
**Description:       HCI_CMD_SET_PINCODE ��������BT3.0 �����PIN ��
**Input parameters:    
                    pincode:  1<=pinlen<=0x10 ģ��Ĭ��pincode: "0000"
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_set_pincode(u8 *pincode, u8 pincodelen)
{
    s32 ret;
    u8 buf[32];
    u8 len;
    
    ret = drv_bt_write_cmd(HCI_CMD_SET_PINCODE, pincode, pincodelen);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    
    return ret;
}
/****************************************************************************
**Description:       HCI_CMD_SET_UART_FLOW ��������UART ����
**Input parameters:    
                    mod:  0x00:�ر�UART����
                          0x00:����UART����
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_set_uart_flow(u8 mod)
{
    s32 ret;
    u8 buf[32];
    u8 len;
    
    buf[0] = mod;
    ret = drv_bt_write_cmd(HCI_CMD_SET_UART_FLOW, buf, 1);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    
    return ret;
}
/****************************************************************************
**Description:        HCI_CMD_SET_UART_BAUD ��������UART ������
**Input parameters:    
                    baud:  ������
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_set_uart_baud(u32 baud)
{
    s32 ret;
    u8 buf[32];
    u8 len;

    if((baud<1200)||(baud > 1000000))
    {
        BT_DEBUG("ERR(baud=%d)\r\n", baud);
    }
    sprintf(buf, "%d", baud);
    len = strlen(buf);
    ret = drv_bt_write_cmd(HCI_CMD_SET_UART_BAUD, buf, len);
    dev_com_setbaud(BT_COM_PORT, baud, 8, 'n', 0, 0);   //??��Ҫ�����Ƿ�������
    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    
    return ret;
}
/****************************************************************************
**Description:        HCI_CMD_VERSION_REQUEST ���ڲ�ѯģ��̼��汾
**Input parameters:    
**Output parameters: 
                ver: 2Byte(1~65535) 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_version_request(u8 *ver)
{
    s32 ret;
    u8 buf[32];
    u8 len;

    ret = drv_bt_write_cmd(HCI_CMD_VERSION_REQUEST, buf, 0);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    if(ret == 0)
    {
        if(len == 2)
        {
            memcpy(ver, buf, 2);
        }
        else
        {
            ret = DEVSTATUS_ERR_FAIL;
        }
    }
    return ret;
}
/****************************************************************************
**Description:        HCI_CMD_BT_DISCONNECT ���ڶϿ�BT3.0��SPP Э�飩����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_bt_disconnect(void)
{
    s32 ret;
    u8 buf[32];
    u8 len;

    ret = drv_bt_write_cmd(HCI_CMD_BT_DISCONNECT, buf, 0);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    
    return ret;
}
/****************************************************************************
**Description:        HCI_CMD_BLE_DISCONNECT ���ڶϿ�BLE ����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_ble_disconnect(void)
{
    s32 ret;
    u8 buf[32];
    u8 len;

    ret = drv_bt_write_cmd(HCI_CMD_BLE_DISCONNECT, buf, 0);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    
    return ret;
}
/****************************************************************************
**Description:        HCI_CMD_SET_COD ��������BT3.0 COD��Class of Device��
**Input parameters:    
                    cod: Ĭ��Ϊ0x040424,���������������ô�ֵ
**Output parameters:
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_set_cod(u8 *cod)
{
    s32 ret;
    u8 buf[32];
    u8 len;

    ret = drv_bt_write_cmd(HCI_CMD_SET_COD, cod, 3);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    
    return ret;
}
/****************************************************************************
**Description:        HCI_CMD_SET_NVRAM ��������NVRAM ����
                  ģ������Ҫ��������ʱ����HCI_EVENT_NVRAM_CHANGED �����ݷ���MCU��MCU ����ģ���ϵ��ʹ��
                  HCI_CMD_SET_NVRAM ������ݷ��͸�ģ��
**Input parameters:    
                    nvram: 
**Output parameters:
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_set_nvram(u8 *nvram)
{
    s32 ret;
    u8 buf[32];
    u8 len;

    ret = drv_bt_write_cmd(HCI_CMD_SET_COD, nvram, 120);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    
    return ret;
}
/****************************************************************************
**Description:        HCI_CMD_ENTER_SLEEP_MODE ����ʹģ�����˯��ģʽ��
                    ����˯��ģʽ��ģ���൱���µ�״̬
**Input parameters:    
                    
**Output parameters:
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_sleep_mode(void)
{
    s32 ret;
    u8 buf[32];
    u8 len;

    drv_bt_wakeup_ctl(0);       
    ret = drv_bt_write_cmd(HCI_CMD_ENTER_SLEEP_MODE, buf, 0);

    //ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);

    return ret;
}
/****************************************************************************
**Description:        HCI_CMD_SET_ADV_DATA ��������BLE ADV DATA����ADV Data ���ȴ���31 �ֽ�ʱ��
                    ģ��Ὣ�������֣�����ADV Data ��ʽ��������Scan Resp Data �С�
**Input parameters:    
                    
**Output parameters:
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_set_adv_data(u8 *advdata, u8 advdatalen)
{
    s32 ret;
    u8 buf[32];
    u8 len;

    if((advdatalen<1)||(advdatalen>0x3E))
    {
        BT_DEBUG("ERR(len=%d)\r\n", advdatalen);
        return -1;
    }
    ret = drv_bt_write_cmd(HCI_CMD_SET_ADV_DATA, advdata, advdatalen);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);

    return ret;
}
/****************************************************************************
**Description:        HCI_CMD_POWER_REQ ���ڲ�ѯģ���Դ��ѹ
                    2 ��byte Ϊ����������һ��byte Ϊ��ѹֵ���������֣�
                    �ڶ���byte Ϊ��ѹֵ��С�����֣�2 ��byte ��Ϊʮ�����ơ�
                    ��03 22 λ3.34V��
**Input parameters:    
                    
**Output parameters:
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_power_req(u8 *vol)
{
    s32 ret;
    u8 buf[32];
    u8 len;

    ret = drv_bt_write_cmd(HCI_CMD_POWER_REQ, buf, 0);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    if(ret == 0)
    {
        if(len == 2)
        {
            memcpy(vol, buf, 2);
        }
        else
        {
            ret = DEVSTATUS_ERR_FAIL;
        }
    }
    return ret;
}
/****************************************************************************
**Description:        HCI_CMD_POWER_SET ��������ģ���ȡ��Դ��ѹ���ܿ���
**Input parameters:    
                    
**Output parameters:
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_power_set(u8 mod)
{
    s32 ret;
    u8 buf[32];
    u8 len;
    
    if(mod)
    {
        buf[0] = 0x01;      //������ѹ���
    }
    else
    {
        buf[0] = 0x00;      //�رյ�ѹ���
    }
    ret = drv_bt_write_cmd(HCI_CMD_POWER_SET, buf, 1);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    
    return ret;
}
/****************************************************************************
**Description:        HCI_CMD_CONFIRM_GKEY ����Numeric Comparison ��Է�ʽ�ж���Կ�ıȽ�
**Input parameters:    
                    
**Output parameters:
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_confirm_gkey(u8 flg)
{
    s32 ret;
    u8 buf[32];
    u8 len;
    
    if(flg)
    {
        buf[0] = 0x01;      //��Կ��ƥ��
    }
    else
    {
        buf[0] = 0x00;      //��Կƥ��
    }
    ret = drv_bt_write_cmd(HCI_CMD_CONFIRM_GKEY, buf, 1);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    
    return ret;
}
/****************************************************************************
**Description:        HCI_CMD_PASSKEY_ENTRY ����PASSKEY ��Է�ʽ�ж���Կ�ıȽ�
**Input parameters:    
                    
**Output parameters:
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_passkey_entry(u8 *passkey, u8 passkeylen)
{
    s32 ret;
    u8 buf[32];
    u8 len;
    u32 i;
    u32 x;

    x = 0;
    for(i=0; i<passkeylen; i++)
    {
        x = (x*10)+(passkey[i]&0x0F);
    }
    buf[0] = (x&0xff);
    buf[1] = ((x>>8)&0xff);
    buf[2] = ((x>>16)&0xff);
    buf[3] = ((x>>24)&0xff);
    ret = drv_bt_write_cmd(HCI_CMD_PASSKEY_ENTRY, buf, 4);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    
    return ret;
}
/****************************************************************************
**Description:        HCI_CMD_SET_GPIO ��������GPIO
**Input parameters:    
                    mod   ��ʾ���\���룬00 Ϊ���� 01 Ϊ�����
                    gpio  ��ʾGPIO �ţ�
                    value ���������Ļ���00 ��������͵�ƽ��01 ��ʾ����ߵ�ƽ��
                          ���������Ļ���00��ʾ������ 01 ��ʾ����
**Output parameters:
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_set_gpio(u8 mod, u8 gpio, u8 value)
{
    s32 ret;
    u8 buf[32];
    u8 len;

    if(mod)
    {
        buf[0] = 0x01;
    }
    else
    {
        buf[0] = 0x00;
    }
    buf[1] = gpio;
    buf[2] = value;
    ret = drv_bt_write_cmd(HCI_CMD_SET_GPIO, buf, 3);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    
    return ret;
}
/****************************************************************************
**Description:        HCI_CMD_READ_GPIO ���ڶ�ȡGPIO ����
**Input parameters:    
**Output parameters:
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_hci_cmd_read_gpio(u8 gpio, u8 *val)
{
    s32 ret;
    u8 buf[32];
    u8 len;

    buf[0] = gpio;
    ret = drv_bt_write_cmd(HCI_CMD_SET_GPIO, buf, 1);

    ret = drv_bt_read_event(HCI_EVENT_CMD_COMPLETE, buf, &len, 32, 200);
    if(ret == 0)
    {
        if(len == 2)
        {
            if(buf[0])
            {
                *val = 1;
            }
            else
            {
                *val = 0;
            }
        }
        else
        {
            ret = DEVSTATUS_ERR_FAIL;
        }
    }
    return ret;
}
/****************************************************************************
**Description:        �ж�BT�Ƿ�����
**Input parameters:    
**Output parameters:
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_get_connect_status(u8 *status)
{
    s32 ret;
    
    if(dev_user_querrytimer(g_bt_status_check_timer, BT_STATUS_CHECK_TIMEROUT))
    {
        drv_bt_task_ctl_dealflg(0);
        ret = drv_bt_hci_cmd_status_request(status);
        drv_bt_task_ctl_dealflg(1);
        if(ret < 0)
        {
            BT_DEBUG("Err(%d)\r\n", ret);
            return 0;   
        }
    }
    *status = g_bt_buf.m_connect;
    return 0;
}
/****************************************************************************
**Description:        BT��������
**Input parameters:    
**Output parameters:
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_send_data(u8 *wbuf, u16 wlen)
{
    s32 i,j;
    s32 ret;
    u8 connect;
    u8 packet_max;

    
    ret = drv_bt_get_connect_status(&connect);
    if(ret < 0)
    {
        BT_DEBUG("Err(ret=%d)\r\n",  ret);
        return -1;
    }
    
    if(connect == 1)
    {
        packet_max = BT_SEND_SPP_PACKET_MAX;
    }
    else if(connect == 2)
    {
        packet_max = BT_SENT_BLE_PACKET_MAX;
    }
    else
    {
        BT_DEBUG("Err(connect=%d)\r\n",  connect);
        return -1;
    }
    i = 0;
    drv_bt_task_ctl_dealflg(0);
BT_DEBUG("sta\r\n");
    while(i<wlen)
    {
        if((i+packet_max)<wlen)
        {
            j = packet_max;
        }
        else
        {
            j = wlen-i;
        }
        if(connect == 1)
        {
            ret = drv_bt_hci_cmd_send_spp_data(&wbuf[i], j);
        }
        else
        {
            ret = drv_bt_hci_cmd_send_ble_data((u8*)bt_ble_attrib, &wbuf[i], j);
        }
        if(ret < 0)
        {
            break;
        }
        i += j;
    }
    drv_bt_task_ctl_dealflg(1);
    return ret;   
}
/****************************************************************************
**Description:        BT��������
**Input parameters:    
**Output parameters:
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_read_data(u8 *rbuf, u16 rlen)
{
    s32 ret;
  #if 0
    u8 connect;

    ret = drv_bt_get_connect_status(&connect);
    if(ret < 0)
    {
        BT_DEBUG("Err(ret=%d)\r\n",  ret);
        return -1;
    }
    if(connect
   #endif
    ret = dev_circlequeue_read(&g_bt_buf.m_rxqueue, rbuf, rlen);
    return ret;
}
/****************************************************************************
**Description:        BT disconnect
**Input parameters:    
**Output parameters:
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
                    DEVSTATUS_ERR_FAIL:  ����
**Created by:        pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_bt_disconnect(void)
{
    s32 ret;
    u8 connect;

    ret = drv_bt_get_connect_status(&connect);
    if(ret < 0)
    {
        BT_DEBUG("Err(ret=%d)\r\n",  ret);
        return -1;
    }
    
    if(connect == 1)
    {
        ret = drv_bt_hci_cmd_bt_disconnect();
    }
    else if(connect == 2)
    {
        ret = drv_bt_hci_cmd_ble_disconnect();
    }
    else
    {
        BT_DEBUG("Err(connect=%d)\r\n",  connect);
        return 0;
    }
    return ret;  
}
#endif

