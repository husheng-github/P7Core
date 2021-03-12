#ifndef __DEV_AUDIO_H
#define __DEV_AUDIO_H

#define AUDIO_WITH_DAC                       FALSE_TRENDIT             //ʹ��mcu�ϵ�DAC���в���
#define AUDIO_ON_WIRELESS                    TRUE_TRENDIT              //ʹ������ģ���ϵ����Ƚ��в���
#define AUDIO_MODEMFILE_MAXNUM               80    //���֧��100���ļ�


typedef struct _strbeepParam
{
    u32 m_pwmhz;
    u16 m_times;     //��������
    u16 m_ontime;    //������������ʱ�䣬��λ����
    u16 m_offtime;   //�������𱣳�ʱ�䣬��λ����
}strbeepParam_t;


typedef struct _strttsData
{
    u32 m_wave_addr;//wave��ַ
    u8  m_wave_index;
    u8  m_wave_text[128];
    u8  m_wave_type; //0-tts 1-audio
}strttsData_t;


//�����Ƶ��Դ���ļ�
#define TTS_SOURCE_PATH      "/mtd2/tts.bin"
#define MAX_WAVE_DATA_LEN   10


#define AUDIO_SOURCE_PATH      "/mtd0/audio" //�ͻ�������Դ
#define AUDIO_MAX_TEXT_NAME_LEN 128 //�ͻ��ļ�������
#define AUDIO_MAX_TEXT_DOWNLOAD_FLG 1//�ļ�����״̬
#define AUDIO_MAX_TEXT_FILE_CNT 50  //50k
#define AUDIO_MAX_TEXT_FILE_LEN AUDIO_MAX_TEXT_FILE_CNT*1024//�ͻ��ļ����ݳ���
#define AUDIO_MAX_NUM   10  //�ͻ����Audio����


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
