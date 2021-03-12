/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    ���ڵײ���������
  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_uart.h"

#define UART0_TXBUF_SIZE    (1024+512)
#define UART0_RXBUF_SIZE    1024
#define UART1_TXBUF_SIZE    1024
#define UART1_RXBUF_SIZE    4096
#define UART2_TXBUF_SIZE    256
#define UART2_RXBUF_SIZE    256
#define UART3_TXBUF_SIZE    2048
#define UART3_RXBUF_SIZE    4096

static UART_TypeDef *const uart_type_tab[DRVUARTNUM_MAX] = {UART0, UART1, UART2, UART3};
static volatile u8 g_uart_sending_flg[DRVUARTNUM_MAX]={0};
typedef struct _str_uartdrv_buf
{
    str_CircleQueue m_txqueue;
    str_CircleQueue m_rxqueue;
}str_uartdrv_buf;
static str_uartdrv_buf g_uartdrv_buf[DRVUARTNUM_MAX];
static volatile u8 g_uart_flowctl_flg[DRVUARTNUM_MAX];
static u32 g_uart_bpsbak[DRVUARTNUM_MAX];   //���Ӳ����ʱ��ݣ����ڻ�ȡ������

const str_uart_param_t g_uart_param_tab[DRVUARTNUM_MAX] = 
{
    //TXPIN        RXPIN          RTSPIN         CTSPIN          baud        
    {GPIO_PIN_PTA1, GPIO_PIN_PTA0, GPIO_PIN_PTA3, GPIO_PIN_PTA2,  115200   , 0, 0, 0, 0},
    {GPIO_PIN_PTB13,GPIO_PIN_PTB12,GPIO_PIN_PTC3,GPIO_PIN_PTC2, 115200   , 3, 3, 3, 3},
    {GPIO_PIN_PTB3, GPIO_PIN_PTB2, GPIO_PIN_PTB5, GPIO_PIN_PTB4,  115200   , 3, 3, 3, 3},
    {GPIO_PIN_PTE9, GPIO_PIN_PTE8, GPIO_PIN_PTE7,GPIO_PIN_NONE, 115200   , 2, 2, 0, 2},
};
/****************************************************************************
**Description:        ��ʼ����������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
**Created by:        pengxuebin 
**--------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static u8 comflag = 1;
void drv_com_init(void)
{
    //static u8 flg = 1;
    u32 i;
    
    if(comflag==1)
    {
        comflag = 0;
        //�����д����ж�
        NVIC_DisableIRQ(UART0_IRQn);
        NVIC_DisableIRQ(UART1_IRQn);
        NVIC_DisableIRQ(UART2_IRQn);
        NVIC_DisableIRQ(UART3_IRQn);
        //��������ⲿ�����ж�
        NVIC_ClearPendingIRQ(UART0_IRQn);
        NVIC_ClearPendingIRQ(UART1_IRQn);
        NVIC_ClearPendingIRQ(UART2_IRQn);
        NVIC_ClearPendingIRQ(UART3_IRQn);
        //��ʼ������
        for(i=0; i<DRVUARTNUM_MAX; i++)
        {
            g_uartdrv_buf[i].m_txqueue.m_buf = NULL;
            g_uartdrv_buf[i].m_txqueue.m_size = 0;
            g_uartdrv_buf[i].m_txqueue.m_in = 0;
            g_uartdrv_buf[i].m_txqueue.m_out = 0;
            g_uartdrv_buf[i].m_rxqueue.m_buf = NULL;
            g_uartdrv_buf[i].m_rxqueue.m_size = 0;
            g_uartdrv_buf[i].m_rxqueue.m_in = 0;
            g_uartdrv_buf[i].m_rxqueue.m_out = 0;
            g_uart_sending_flg[i] = 0;
            g_uart_bpsbak[i] = 115200;
        }
    }
}





static void drv_uart_irqdeal(s32 nCom)
{
    UART_TypeDef *lp_uart; 
    u32 i,j;
    u8 fifotmp[16];
    
    lp_uart = uart_type_tab[nCom];

    switch(lp_uart->OFFSET_8.IIR & 0x0f)
    {
    case UART_IT_ID_RX_RECVD:
        {
            i = lp_uart->RFL&0x0f;
            if((UART_IsRXFIFONotEmpty(lp_uart))&&(i==0))
            {
                i = 16;
            }
            for(j=0; j<i; j++)
            {
                fifotmp[j] = UART_ReceiveData(lp_uart);
            }
            dev_circlequeue_write(&g_uartdrv_buf[nCom].m_rxqueue, fifotmp, i);
            if(dev_circlequeue_getspace(&g_uartdrv_buf[nCom].m_rxqueue) < 16)
            {
                if(g_uart_flowctl_flg[nCom]==1)
                {
                    UART_ResetRTS(lp_uart);
                }
            }
            
        }
        break;
    case UART_IT_ID_TX_EMPTY:
        {
            i = 15 - (lp_uart->TFL&0x0f);
            j = dev_circlequeue_read(&g_uartdrv_buf[nCom].m_txqueue, fifotmp, i);
            if(j<i)
            {
                UART_ITConfig(lp_uart, UART_IT_TX_EMPTY, DISABLE);
                //lp_uart->OFFSET_4.IER |= UART_IT_TX_EMPTY;
                g_uart_sending_flg[nCom]=0;
            }
            for(i=0; i<j; i++)
            {
                UART_SendData(lp_uart, fifotmp[i]);
                //lp_uart->OFFSET_0.THR = fifotmp[i];
            }
        }
        break;
    case UART_IT_ID_MODEM_STATUS:
        {
            u32 msr = lp_uart->MSR;            
        }
        break;
    case UART_IT_ID_LINE_STATUS:
        {
            u32 lsr = lp_uart->LSR;            
        }    
        break;
    case UART_IT_ID_BUSY_DETECT:
        {
            u32 usr = lp_uart->USR;            
        }    
        break;
    case UART_IT_ID_CHAR_TIMEOUT:
        break;
    default:
        break;

    }
    
}

void UART0_IRQHandler(void)
{
    drv_uart_irqdeal(0);
}

void UART1_IRQHandler(void)
{
    drv_uart_irqdeal(1);
}
void UART2_IRQHandler(void)
{
    drv_uart_irqdeal(2);
}

void UART3_IRQHandler(void)
{
    drv_uart_irqdeal(3);
}

s32 drv_com_setbaud(s32 nCom,  s32 baud, s32 databits, u32 parity, s32 stopbits, s32 flowctl)
{
    UART_TypeDef *lp_uart;
    UART_InitTypeDef UART_InitStructure;
    s32 i;
    
    if(nCom>=DRVUARTNUM_MAX)
    {
        return -1;
    }
    lp_uart = uart_type_tab[nCom];
    //������
    UART_InitStructure.UART_BaudRate = baud;
    g_uart_bpsbak[nCom] = baud;
    //����λ��
    switch(databits)
    {
    case 5:    
        i = UART_WordLength_5b;
        break;
    case 6:
        i = UART_WordLength_6b;
        break;
    case 7:
        i = UART_WordLength_7b;
        break;
    case 8:
        i = UART_WordLength_8b;
        break;
    default:
        return -1;
    }
    UART_InitStructure.UART_WordLength = i;
    //ֹͣλ
    switch(stopbits)
    {
    case 0:
        i = UART_StopBits_1;
        break;
    case 1:
        i = UART_StopBits_1_5;
        break;
    case 2:
        i = UART_StopBits_2;
        break;
    default:
        return -1;
    }
    UART_InitStructure.UART_StopBits = i;
    //��żУ��
    switch(parity)
    {
    case 'o':
    case 'O':   
        i = UART_Parity_Even;   //żУ��
        break;
    case 'e':
    case 'E':   
        i = UART_Parity_Odd;   //��У��
        break;
    case 'n': 
    case 'N':
    default:
        i = UART_Parity_No;
        break;
    }
    UART_InitStructure.UART_Parity = i;
    //��������
    switch(flowctl)
    {
    case 1:     //Ӳ������
        i = ENABLE;
        //Ӳ������
        dev_gpio_config_mux(g_uart_param_tab[nCom].m_rtspin, g_uart_param_tab[nCom].m_rtspinalt);
        dev_gpio_set_pad(g_uart_param_tab[nCom].m_rtspin, PAD_CTL_PULL_NONE);
        dev_gpio_config_mux(g_uart_param_tab[nCom].m_ctspin, g_uart_param_tab[nCom].m_ctspinalt);
        dev_gpio_set_pad(g_uart_param_tab[nCom].m_ctspin, PAD_CTL_PULL_NONE);
        break;
    case 2:     //�������,�ݲ�֧��
    default:    //������
        i = DISABLE;
        break;
    }
    //���ж�
    UART_ITConfig(lp_uart, UART_IT_RX_RECVD, DISABLE);
    UART_AutoFlowCtrlCmd(lp_uart, (FunctionalState)i);
    UART_Init(lp_uart, &UART_InitStructure);
    g_uartdrv_buf[nCom].m_txqueue.m_in = 0;
    g_uartdrv_buf[nCom].m_txqueue.m_out = 0;
    g_uartdrv_buf[nCom].m_rxqueue.m_in = 0;
    g_uartdrv_buf[nCom].m_rxqueue.m_out = 0;
    //��FIFO
    UART_FIFOReset(lp_uart, UART_FIFO_TX|UART_FIFO_RX);
    g_uart_sending_flg[nCom] = 0;
    g_uart_flowctl_flg[nCom] = flowctl;
    if(g_uart_flowctl_flg[nCom]==1)
    {
        UART_SetRTS(lp_uart);
    }
    UART_ITConfig(lp_uart, UART_IT_RX_RECVD, ENABLE);
    return 0;
}

s32 drv_com_open(s32 nCom, s32 baud, s32 databits, u32 parity, s32 stopbits, s32 flowctl)
{
    u16 tx_size;
    u16 rx_size;
    UART_TypeDef *lp_uart;
    UART_FIFOInitTypeDef UART_FIFOInitStruct;
    IRQn_Type irqn;
    
    if(nCom >= DRVUARTNUM_MAX)
    {
        return -1;
    }
    if(g_uartdrv_buf[nCom].m_txqueue.m_buf == NULL)
    {
        drv_com_init();
        switch(nCom)
        {
        case 0:
            lp_uart = UART0;
            tx_size = UART0_TXBUF_SIZE;
            rx_size = UART0_RXBUF_SIZE;
            //�򿪴���ʱ��
            SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_UART0,ENABLE);
            SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_UART0,ENABLE);
            //���ùܽ�
            dev_gpio_config_mux(GPIO_PIN_PTA0, MUX_CONFIG_ALT0);
            dev_gpio_config_mux(GPIO_PIN_PTA1, MUX_CONFIG_ALT0);
            irqn = UART0_IRQn;
            break;
        case 1:
            lp_uart = UART1;
            tx_size = UART1_TXBUF_SIZE;
            rx_size = UART1_RXBUF_SIZE;
            //�򿪴���ʱ��
            SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_UART1,ENABLE);
            SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_UART1,ENABLE);
            irqn = UART1_IRQn;
            break;
        case 2:
            lp_uart = UART2;
            tx_size = UART2_TXBUF_SIZE;
            rx_size = UART2_RXBUF_SIZE;
            //�򿪴���ʱ��
            SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_UART2,ENABLE);
            SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_UART2,ENABLE);
            irqn = UART2_IRQn;
            break;
        case 3:
            lp_uart = UART3;
            tx_size = UART3_TXBUF_SIZE;
            rx_size = UART3_RXBUF_SIZE;
            //�򿪴���ʱ��
            SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_UART3,ENABLE);
            SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_UART3,ENABLE);
            irqn = UART3_IRQn;
            break;
        default:
            return -1;
        }
         //���ùܽ� 
        dev_gpio_config_mux(g_uart_param_tab[nCom].m_txpin, g_uart_param_tab[nCom].m_txpinalt);
        dev_gpio_config_mux(g_uart_param_tab[nCom].m_rxpin, g_uart_param_tab[nCom].m_rxpinalt);
        //����FIFO
        UART_FIFOStructInit(&UART_FIFOInitStruct);
        UART_FIFOInitStruct.FIFO_Enable = ENABLE;
        UART_FIFOInitStruct.FIFO_DMA_Mode = UART_FIFO_DMA_Mode_0;
        UART_FIFOInitStruct.FIFO_RX_Trigger = UART_FIFO_RX_Trigger_1_Char;//UART_FIFO_RX_Trigger_1_4_Full;//
        UART_FIFOInitStruct.FIFO_TX_Trigger = UART_FIFO_TX_Trigger_1_4_Full;
        UART_FIFOInitStruct.FIFO_TX_TriggerIntEnable = ENABLE;
        UART_FIFOInit(lp_uart, &UART_FIFOInitStruct);
        
        drv_com_setbaud(nCom, baud, databits, parity, stopbits, flowctl);
        
        //UART_ITConfig(lp_uart, UART_IT_RX_RECVD, ENABLE);
        //�������ж�
        NVIC_EnableIRQ(irqn);    
        if(dev_circlequeue_init(&g_uartdrv_buf[nCom].m_txqueue, tx_size)<0)
        {
            goto err;
        }
        if(dev_circlequeue_init(&g_uartdrv_buf[nCom].m_rxqueue, rx_size)<0)
        {
            goto err;
        }
    }
    //����DMA
    return 0;
err:
    //�ش����ж�
    NVIC_DisableIRQ(irqn);    
    UART_ITConfig(lp_uart, UART_IT_RX_RECVD, DISABLE);
    dev_circlequeue_free(&g_uartdrv_buf[nCom].m_txqueue);
    dev_circlequeue_free(&g_uartdrv_buf[nCom].m_rxqueue);
    return -1;
}
s32 drv_com_close(s32 nCom)
{
    UART_TypeDef *lp_uart;
    IRQn_Type irqn;

    if(nCom >= DRVUARTNUM_MAX)
    {
        return -1;
    }
    lp_uart = uart_type_tab[nCom];
    switch(nCom)
    {
    case 0:
        irqn = UART0_IRQn;
        break;
    case 1:
        irqn = UART1_IRQn;
        break;
    case 2:
        irqn = UART2_IRQn;
        break;
    case 3:
        irqn = UART3_IRQn;
        break;
    }
    
    //�ش����ж�
    NVIC_DisableIRQ(irqn); 
    UART_ITConfig(lp_uart, UART_IT_RX_RECVD, DISABLE);
    dev_circlequeue_free(&g_uartdrv_buf[nCom].m_txqueue);
    dev_circlequeue_free(&g_uartdrv_buf[nCom].m_rxqueue);
    return 0;
}
static void drv_com_sta(s32 nCom)
{
    s32 i,j,k;
    UART_TypeDef *lp_uart;
    u8 fifotmp[16];
    
    lp_uart = uart_type_tab[nCom];
    i = 15 - (lp_uart->TFL&0x0f);
    j = dev_circlequeue_read(&g_uartdrv_buf[nCom].m_txqueue, fifotmp, i);
    for(k=0; k<j; k++)
    {
        UART_SendData(lp_uart, fifotmp[k]);
    }
    if(j>=i) 
    {
        g_uart_sending_flg[nCom] = 1;
        UART_ITConfig(lp_uart, UART_IT_TX_EMPTY, ENABLE);
    }
}

s32 drv_com_write(s32 nCom, u8 *wbuf, u16 wlen)
{
    s32 cnt;
    
    if(nCom >= DRVUARTNUM_MAX)
    {
        return -1;
    }
    
    cnt = dev_circlequeue_write(&g_uartdrv_buf[nCom].m_txqueue, wbuf, wlen);
    //��������
    if(g_uart_sending_flg[nCom]==0)
    {
        drv_com_sta(nCom);
    }
    return cnt;
}
s32 drv_com_read(s32 nCom, u8 *rbuf, u16 rlen)
{
    s32 ret;
    
    if(nCom >= DRVUARTNUM_MAX)
    {
        return -1;
    }
    ret = dev_circlequeue_read(&g_uartdrv_buf[nCom].m_rxqueue, rbuf, rlen);
    
    if(g_uart_flowctl_flg[nCom]==1)
    {
        if(dev_circlequeue_getspace(&g_uartdrv_buf[nCom].m_rxqueue)>=16)
        {
            UART_SetRTS(uart_type_tab[nCom]);
        }
    }
    return ret;
}
s32 drv_com_tcdrain(s32 nCom)
{
    if(nCom >= DRVUARTNUM_MAX)
    {
        return -1;
    }
    while(0 == dev_circlequeue_isempty(&g_uartdrv_buf[nCom].m_txqueue))
    {
        
    }
    return 0;
}
s32 drv_com_flush(s32 nCom)
{
    if(nCom >= DRVUARTNUM_MAX)
    {
        return -1;
    }
    dev_circlequeue_clear(&g_uartdrv_buf[nCom].m_rxqueue);
    if(g_uart_flowctl_flg[nCom]==1)
    {
       UART_SetRTS(uart_type_tab[nCom]);
    }
    //���FIFO����ʱ������
    return 0;
}


void drv_com_writedirect(u8 data)
{
    while((UART0->LSR&(1<<6)) == 0);
    UART0->OFFSET_0.THR = (data & 0xFF); //TEMT
    while((UART0->LSR&(1<<6)) == 0);
    
    
}
/**
 * @brief ��ȡ������ 
 * @param [in] nCom: ���ں� 
 * @param [out] *bps:���ص�ǰ������
 * @retval =0 ��ȡ�ɹ���
           <0 ��ȡʧ��
 * @since pengxuebin,20190813 ����1902 96MhzӰ������ͨѶ����72Mhz��ƵͨѶ
 */
s32 drv_com_get_baud(u8 nCom, u32 *bps)
{
    if(nCom >= DRVUARTNUM_MAX)
    {
        return -1;
    }
    if(g_uartdrv_buf[nCom].m_txqueue.m_buf == NULL)
    {
        //�豸δ��
        return -1;
    }
    *bps = g_uart_bpsbak[nCom];
    return 0;
}



