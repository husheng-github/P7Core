


#ifndef _DDI_AUDIO_H_
#define _DDI_AUDIO_H_


#include "ddi_common.h"



//播放状态
#define AUDIO_STATUS_IDLE   (0) //空闲
#define AUDIO_STATUS_PLAYING   (1) //正在播放
#define AUDIO_STATUS_PAUSE  (3)  //暂停

/*音频控制命令*/
#define     DDI_AUDIO_CTL_VER               (0)  //获取音频设备版本
#define     DDI_AUDIO_CTL_VOLUME            (1)  //控制音量
#define     DDI_AUDIO_CTL_BUZZER            (2)  //控制蜂鸣器发声
#define     DDI_AUDIO_CTL_COMB_PLAY         (3)  //组合播放
#define     DDI_AUDIO_CTL_GET_STATUS        (4)  //获取播放状态
#define     DDI_AUDIO_CTL_BUZZER_1          (5)  //控制蜂鸣器发声1,可以控制频率、发声次数等
#define     DDI_AUDIO_GET_VOLUME            (6)  //获取音量值
#define     DDI_AUDIO_CTL_VOICE             (7)  //控制发声
#define     DDI_AUDIO_CTL_SPLICE_PLAY       (8)  //拼接字符串播放
#define     DDI_AUDIO_CTL_KEY_VOLUME        (9)  //设置按键音音量
#define     DDI_AUDIO_SYNC_AUDIOFILE            (13)     //同步音频文件
#define     DDI_AUDIO_SET_PLAYMP3FLAG           (14)     //设置是否要采用MP3播报，默认开机支持MP3播报
#define     DDI_AUDIO_GET_TTSFILECRC            (15)     //获取TTS CRC

//=====================================================


#define AUDIO_CTL_VOICE_TYPE_GENERAL  0//ok
#define AUDIO_CTL_VOICE_TYPE_ERROR    1//错误声音
#define AUDIO_CTL_VOICE_TYPE_CUSTOM   99//自定义音频数据播放，wParam参数为str_audio_info指针

typedef enum{
    AUDIO_TTS_PRIORITY_NORMAL = 0,//插入到最后一个
    AUDIO_TTS_PRIORITY_HIGH,//插入到第一个
    AUDIO_TTS_PRIORITY_HIGH_WITH_STOP_CUR//插入到第一个，并停止当前
}AUDIO_TTS_PRIORITY;

typedef enum{
    TTS_ENCODE_UTF8,
    TTS_ENCODE_GB2312
}TTS_ENCODE_TYPE;

typedef struct _str_audio_tts{
    
    u8* m_text;
    AUDIO_TTS_PRIORITY m_priority;
    s8 m_texttype;                 //0:UTF8编码   1:Gb2312编码
    s8 m_audioid[4];
    s8 m_reserved[7];  // 12-5
    
}str_audio_tts;


//对外函数声明
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


