

#ifndef _DDI_LED_H_
#define _DDI_LED_H_


#include "ddi_common.h"

//����LED���
#define   LED_NUM_MAX         4   //LED����

#if 1
#define   LED_STATUS_B        0
#define   LED_STATUS_R         1

#define   LED_SIGNAL_Y      2
#define   LED_SIGNAL_B       3


#else
#define   LED_BLUE_NO        0
#define   LED_YELLOW_NO      1
#define   LED_GREEN_NO       2
#define   LED_RED_NO         3
#endif
#define   LED_ALL           0xff


#define   LED_OFF          0
#define   LED_ON           1

typedef struct _strLedGleamPara
{
    u32 m_led;          //LED���
    u32 m_ontime;       //������������ʱ�䣬��λ����
    u32 m_offtime;      //�������𱣳�ʱ�䣬��λ����
    u32 m_duration;     //��˸����ʱ�䣬��λ����
}strLedGleamPara;


/********************************��Ķ���***********************************/
/*Led�ȿ���ָ��*/
#define     DDI_LED_CTL_VER                     (0) //��ȡledģ��汾
#define     DDI_LED_CTL_GLEAM                   (1) //����led����˸
#define     DDI_LED_CTL_CLOSE_YELLOW            (2) //Ӳ��δ�ӻƵƣ�����ǽ��źŵƵĻƵ���˸��Ϊ����״̬��ע���󣬾ͽ���״̬�ر�
#define     DDI_LED_CTL_LOOP                    (3) //��ѭ����,1sһ��


//=====================================================
//���⺯������
extern s32 ddi_led_open (void);
extern s32 ddi_led_close (void);
extern s32 ddi_led_sta_set(u32 nLed, u32 nSta);
extern s32 ddi_led_ioctl(u32 nCmd, u32 lParam, u32 wParam);

typedef s32 (*core_ddi_led_open) (void); 
typedef s32 (*core_ddi_led_close) (void); 
typedef s32 (*core_ddi_led_sta_set) (u32 nLed, u32 nSta); 
typedef s32 (*core_ddi_led_ioctl)(u32 nCmd, u32 lParam, u32 wParam);

#endif

