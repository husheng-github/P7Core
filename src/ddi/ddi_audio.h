


#ifndef _DDI_AUDIO_H_
#define _DDI_AUDIO_H_


#include "ddi_common.h"



//����״̬
#define AUDIO_STATUS_IDLE   (0) //����
#define AUDIO_STATUS_PLAYING   (1) //���ڲ���
#define AUDIO_STATUS_PAUSE  (3)  //��ͣ

/*��Ƶ��������*/
#define     DDI_AUDIO_CTL_VER               (0)  //��ȡ��Ƶ�豸�汾
#define     DDI_AUDIO_CTL_VOLUME            (1)  //��������
#define     DDI_AUDIO_CTL_BUZZER            (2)  //���Ʒ���������
#define     DDI_AUDIO_CTL_COMB_PLAY         (3)  //��ϲ���
#define     DDI_AUDIO_CTL_GET_STATUS        (4)  //��ȡ����״̬
#define     DDI_AUDIO_CTL_BUZZER_1          (5)  //���Ʒ���������1,���Կ���Ƶ�ʡ�����������
#define     DDI_AUDIO_GET_VOLUME            (6)  //��ȡ����ֵ
#define     DDI_AUDIO_CTL_VOICE             (7)  //���Ʒ���
#define     DDI_AUDIO_CTL_SPLICE_PLAY       (8)  //ƴ���ַ�������
#define     DDI_AUDIO_CTL_KEY_VOLUME        (9)  //���ð���������
#define     DDI_AUDIO_SYNC_AUDIOFILE            (13)     //ͬ����Ƶ�ļ�
#define     DDI_AUDIO_SET_PLAYMP3FLAG           (14)     //�����Ƿ�Ҫ����MP3������Ĭ�Ͽ���֧��MP3����
#define     DDI_AUDIO_GET_TTSFILECRC            (15)     //��ȡTTS CRC

//=====================================================


#define AUDIO_CTL_VOICE_TYPE_GENERAL  0//ok
#define AUDIO_CTL_VOICE_TYPE_ERROR    1//��������
#define AUDIO_CTL_VOICE_TYPE_CUSTOM   99//�Զ�����Ƶ���ݲ��ţ�wParam����Ϊstr_audio_infoָ��

typedef enum{
    AUDIO_TTS_PRIORITY_NORMAL = 0,//���뵽���һ��
    AUDIO_TTS_PRIORITY_HIGH,//���뵽��һ��
    AUDIO_TTS_PRIORITY_HIGH_WITH_STOP_CUR//���뵽��һ������ֹͣ��ǰ
}AUDIO_TTS_PRIORITY;

typedef enum{
    TTS_ENCODE_UTF8,
    TTS_ENCODE_GB2312
}TTS_ENCODE_TYPE;

typedef struct _str_audio_tts{
    
    u8* m_text;
    AUDIO_TTS_PRIORITY m_priority;
    s8 m_texttype;                 //0:UTF8����   1:Gb2312����
    s8 m_audioid[4];
    s8 m_reserved[7];  // 12-5
    
}str_audio_tts;


//���⺯������
extern s32 ddi_audio_open (void);
extern s32 ddi_audio_close (void);
extern s32 ddi_audio_play(const u8 * lpFile);
extern s32 ddi_audio_playcharacters(str_audio_tts audio_tts);
extern s32 ddi_audio_stop (void);
extern s32 ddi_audio_pause (void);
extern s32 ddi_audio_resumeplay (void);
extern s32 ddi_audio_ioctl(u32 nCmd, u32 lParam, u32 wParam);
//=====================================================
typedef s32 (*core_ddi_audio_open) (void);
typedef s32 (*core_ddi_audio_close) (void);
typedef s32 (*core_ddi_audio_play) (const u8 * lpFile);
typedef s32 (*core_ddi_audio_playcharacters)(str_audio_tts audio_tts);
typedef s32 (*core_ddi_audio_stop) (void);
typedef s32 (*core_ddi_audio_pause) (void);
typedef s32 (*core_ddi_audio_resumeplay) (void);
typedef s32 (*core_ddi_audio_ioctl)(u32 nCmd, u32 lParam, u32 wParam);


#endif


