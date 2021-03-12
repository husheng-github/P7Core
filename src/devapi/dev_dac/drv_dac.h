#ifndef __DRV_DAC_H
#define __DRV_DAC_H

typedef struct _str_DAC_PARAM
{
    s32 m_ch;               //DAC通道号
    iomux_pin_name_t m_gpiopin;     //GPIO管脚做片选
}str_dac_param_t;

typedef struct WAVE_DATA
{
    u32 m_wave_addr;//wave地址
    s32 m_wave_len;//wave数据长度
    s32 m_wave_samplerate;//采样率
    u8  m_wave_type;//tts或audio
}wave_data_t;

typedef struct WAVE_DMA_BLOCK
{
    u32 dma_blockCount;//dma传输的数量
    u32 dma_blockOffset;//当前的wava数据的offset
}wave_dma_block_t;

//#define DEBUG_DAC_EN
#define WAVE_HEAD_INFO_LEN  44
#define SAMPLE_RATE = 22050 //sample rate：22050Hz
#define DMA_BLOCK_SIZE      1024

typedef struct _strWaveHeadInfo
{
    s8  m_riff[4];          //4个字节 RIFF
    s32 m_fsize;            //4个字节 文件大小
    s8  m_wavetag[4];       //4个字节 WAVE
    s8  m_fmttag[4];        //4个字节 FMT 
    s32 m_chunsize;         //4个字节 字块总数
    s16 m_formattag;        //2个字节 01H代表PCM格式
    s16 m_channel;          //2个字节 1-单声道 2-双声道
    s32 m_samplerate;       //4个字节 采样率
    s32 m_bytepersec;       //4个字节 每秒播放字节数
    s16 m_bytesample;       //2个字节 每个样点的字节数
    s16 m_bitsample;        //2个字节 每个样点的数据位数
    s8  m_datatag[4];       //4个字节 DATA
    s32 m_datalen;          //4个字节 数据长度

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
