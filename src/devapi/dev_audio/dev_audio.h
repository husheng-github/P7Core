#ifndef __DEV_AUDIO_H
#define __DEV_AUDIO_H

#define AUDIO_WITH_DAC                       FALSE_TRENDIT             //使用mcu上的DAC进行播放
#define AUDIO_ON_WIRELESS                    TRUE_TRENDIT              //使用无线模块上的喇叭进行播放
#define AUDIO_MODEMFILE_MAXNUM               80    //最多支持100个文件


typedef struct _strbeepParam
{
    u32 m_pwmhz;
    u16 m_times;     //发声次数
    u16 m_ontime;    //周期内亮保持时间，单位毫秒
    u16 m_offtime;   //周期内灭保持时间，单位毫秒
}strbeepParam_t;


typedef struct _strttsData
{
    u32 m_wave_addr;//wave地址
    u8  m_wave_index;
    u8  m_wave_text[128];
    u8  m_wave_type; //0-tts 1-audio
}strttsData_t;


//存放音频资源的文件
#define TTS_SOURCE_PATH      "/mtd2/tts.bin"
#define MAX_WAVE_DATA_LEN   10


#define AUDIO_SOURCE_PATH      "/mtd0/audio" //客户定制音源
#define AUDIO_MAX_TEXT_NAME_LEN 128 //客户文件名长度
#define AUDIO_MAX_TEXT_DOWNLOAD_FLG 1//文件下载状态
#define AUDIO_MAX_TEXT_FILE_CNT 50  //50k
#define AUDIO_MAX_TEXT_FILE_LEN AUDIO_MAX_TEXT_FILE_CNT*1024//客户文件内容长度
#define AUDIO_MAX_NUM   10  //客户最大Audio数量


#define audio_filename_addr(n)      (n*(AUDIO_MAX_TEXT_NAME_LEN+AUDIO_MAX_TEXT_DOWNLOAD_FLG+AUDIO_MAX_TEXT_FILE_LEN))
#define audio_filedownflg_addr(n)   (audio_filename_addr(n)+AUDIO_MAX_TEXT_NAME_LEN)
#define audio_filedata_addr(n)      (audio_filedownflg_addr(n)+AUDIO_MAX_TEXT_DOWNLOAD_FLG)





void dev_audio_task(void);
s32 dev_audio_open(void);
s32 dev_audio_close(void);
s32 dev_audio_play(const u8 * lpFile);
s32 dev_audio_playcharacters(s8 audiotype, u8 *tts_text);
s32 dev_audio_stop (void);
s32 dev_audio_pause (void);
s32 dev_audio_replay (void);
s32 dev_audio_ioctl(u32 nCmd, u32 lParam, u32 wParam);
s32 drv_audio_suspend(void);
s32 drv_audio_resume(void);

#endif
