#ifndef __DRV_DAC_H
#define __DRV_DAC_H

typedef struct _str_DAC_PARAM
{
    s32 m_ch;               //DACͨ����
    iomux_pin_name_t m_gpiopin;     //GPIO�ܽ���Ƭѡ
}str_dac_param_t;

typedef struct WAVE_DATA
{
    u32 m_wave_addr;//wave��ַ
    s32 m_wave_len;//wave���ݳ���
    s32 m_wave_samplerate;//������
    u8  m_wave_type;//tts��audio
}wave_data_t;

typedef struct WAVE_DMA_BLOCK
{
    u32 dma_blockCount;//dma���������
    u32 dma_blockOffset;//��ǰ��wava���ݵ�offset
}wave_dma_block_t;

//#define DEBUG_DAC_EN
#define WAVE_HEAD_INFO_LEN  44
#define SAMPLE_RATE = 22050 //sample rate��22050Hz
#define DMA_BLOCK_SIZE      1024

typedef struct _strWaveHeadInfo
{
    s8  m_riff[4];          //4���ֽ� RIFF
    s32 m_fsize;            //4���ֽ� �ļ���С
    s8  m_wavetag[4];       //4���ֽ� WAVE
    s8  m_fmttag[4];        //4���ֽ� FMT 
    s32 m_chunsize;         //4���ֽ� �ֿ�����
    s16 m_formattag;        //2���ֽ� 01H����PCM��ʽ
    s16 m_channel;          //2���ֽ� 1-������ 2-˫����
    s32 m_samplerate;       //4���ֽ� ������
    s32 m_bytepersec;       //4���ֽ� ÿ�벥���ֽ���
    s16 m_bytesample;       //2���ֽ� ÿ��������ֽ���
    s16 m_bitsample;        //2���ֽ� ÿ�����������λ��
    s8  m_datatag[4];       //4���ֽ� DATA
    s32 m_datalen;          //4���ֽ� ���ݳ���

}strWaveHeadInfo_t;


typedef enum
{
    DAC_VOL_0,      /* Volume level 0 (Not mute) */
    DAC_VOL_1,      /* Volume level 1 */
    DAC_VOL_2,      /* Volume level 2 */
    DAC_VOL_3,      /* Volume level 3 */
    DAC_VOL_4,      /* Volume level 4 */
    DAC_VOL_5,      /* Volume level 5 */
    DAC_VOL_NUM     /* Total volume level */
} DAC_VOL_ENUM;



s32 drv_dac_open(void);
s32 drv_dac_close(void);
s32 drv_dac_tts_play(strttsData_t *ttsdata, s32 ttslen);
s32 drv_dac_play(u32 waveaddr);
s32 drv_dac_pause(void);
s32 drv_dac_replay(void);
s32 drv_dac_set_volume_level(int level);
s32 drv_dac_get_volume_level(void);
s32 drv_dac_is_playing(void);

#endif
