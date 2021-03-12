/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170422      
** Created Date:     
** Version:        
** Description:    DAC驱动程序  
****************************************************************************/
#include "devglobal.h"
#include "sdk/mhscpu_sdk.h"
#include "drv_dac.h"


#ifdef DEBUG_DAC_EN
#define DAC_DEBUG    dev_debug_printf

#else
#define DAC_DEBUG(...) 
#define DAC_DEBUGHEX(...)
#endif


#define DAC_CHANNEL_MAX     1   //DAC通道个数
#define DAC_CHANNEL_NONE    0xff
#define DAC_DMA_CHANNEL     DMA_Channel_1
#define DAC_SPK_CTRL_PIN    GPIO_PIN_PTA5



static s32 g_drv_dac_fd = -1;
static s32 g_adc_play_flg = 0;


static u8 g_wave_data_count = 0;//pcm数据块
static u8 g_wave_data_index = 0;//第几块pcm数据


static strWaveHeadInfo_t g_strWaveHeadInfo ;//用于解析wave数据head信息
static wave_data_t g_wata_data_t[MAX_WAVE_DATA_LEN];//保存上层传下来的地址和解析的长度

static s32 dac_vol_level = DAC_VOL_5;

static s32 g_dma_block_size = DMA_BLOCK_SIZE;//dma 传输长度
static s32 g_dac_samplerate = 8000;//dac 采样率

static wave_dma_block_t g_wave_dma_block_t;
static uint16_t wavData[DMA_BLOCK_SIZE] = {0};//dma  传输的数据


const str_dac_param_t str_dac_param_tab[DEV_ADC_MAX] = 
{
    {0, GPIO_PIN_PTC1},

};



#if 0
void wave_DataHandle(void)
{
    uint32_t i = 0;

    #if 1
    for (i = 0; i < WAVE_DATA_SIZE/2; i++)
    {
        wavData[i] = (wavData[i] + 0x8000) >> 6;
        //wavData[i] = (wavData[i] >> 6);
        dev_debug_printf("0x%x, ",wavData[i]);
        if(i%15==0)
        {
            dev_debug_printf("\r\n    ");
        }
    }
    #endif
}
#endif

//替换spiflash读取wave数据函数
//s32 dev_flash_read(u32 addrsta, u8* rbuf, u32 rlen)
s32 dev_flash_read_test(u32 addrsta, u8* rbuf, u32 rlen, u8 wavetype)
{
    s32 ret = 0;
    if(wavetype == 1)
    {
        ret = fs_read_file(AUDIO_SOURCE_PATH,rbuf,rlen,addrsta);
    }else
    {
        ret = fs_read_file(TTS_SOURCE_PATH,rbuf,rlen,addrsta);
    }
    
    //memcpy(rbuf, &test_data[addrsta], rlen);
    return ret;
}



/**
 *@brief 清除播放记录状态
 */
static void dac_clear_play_status()
{
        memset(g_wata_data_t, 0, sizeof(g_wata_data_t));//清除保存wave addr和wave len
        g_wave_dma_block_t.dma_blockCount = 0;//清除 dma中断 block
        g_wave_dma_block_t.dma_blockOffset = 0;//清除 dma中断 偏移量
        g_wave_data_index = 0;//清除当前播放的wave index
        g_wave_data_count = 0;//清除播放wave总的个数
}

/*
***16bit 分贝计算公式 :
***        dB = 20 * log(65536) 
***        db = 96
***         level5 = 96 *2
***         level4 = 72 *1.4
***         level3 = 48 0
***         level2 = 24 /16
***         level1 = 0  /251

***     A2为原始声音振幅，A1为调节后的声音振幅大小
***dB = 20 * log(A1 / A2)

*/
static s16 dac_set_volume_level_data(s16 pcm_data,      int level)
{
    s16 pcmval;


    #if 1
    switch (level)
    {
        case DAC_VOL_0:

            break;
        case DAC_VOL_1:
            pcmval = pcm_data/16;
            break;
        case DAC_VOL_2:
            pcmval = pcm_data/8;
            break;
        case DAC_VOL_3:
            pcmval = pcm_data/4;
            break;
        case DAC_VOL_4:
            pcmval = pcm_data/2;
            break;
        case DAC_VOL_5:
            pcmval = pcm_data;
            break;
    }

    #else
    switch (level)
    {
        case DAC_VOL_0:

            break;
        case DAC_VOL_1:
            pcmval = pcm_data/4;
            break;
        case DAC_VOL_2:
            pcmval = pcm_data/2;
            break;
        case DAC_VOL_3:
            pcmval = pcm_data;
            break;
        case DAC_VOL_4:
            pcmval = pcm_data*1.20;
            break;
        case DAC_VOL_5:
            pcmval = pcm_data*1.38;
            break;
    }
    #endif
    if (pcmval < 32767 && pcmval > -32768) {
        //pcmval = pcm_data;
    } else if (pcmval > 32767) {
        pcmval = 32767;
    } else if (pcmval < -32768) {
        pcmval = -32768;
    }

    return pcmval;

}

/****************************************************************************
**Description:       DAC 获取音量等级
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
                    DEVSTATUS_ERR_FAIL:  失败
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 drv_dac_get_volume_level(void)
{
    return dac_vol_level;
}

/****************************************************************************
**Description:       设置 dac音量等级
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 drv_dac_set_volume_level(int level)
{
    if(g_adc_play_flg)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    if((level < DAC_VOL_0) || (level >= DAC_VOL_NUM))
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    DAC_DEBUG("level:%d\r\n",level);
    dac_vol_level = level;
    return 0;
}

/****************************************************************************
**Description:       获取WAVE PCM信息
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
static s32 dac_get_wave_data_info(u32 addrsta, u32 readlen, u8 wavetype)
{
    s32 ret = 0;
    s32 i = 0;
    u8 wavebuf[DMA_BLOCK_SIZE*2] = {0};
    

    ret = dev_flash_read_test(addrsta,wavebuf,readlen, wavetype);
    if(ret < 0)
    {
        return DDI_ENOFILE;
    }

    memset(wavData, 0x00, DMA_BLOCK_SIZE*2);
    for (i = 0; i < readlen; )
    {
        #if 1
        wavData[i/2] = (dac_set_volume_level_data((wavebuf[i+1]<<8 | wavebuf[i]), dac_vol_level) + 0x8000) >> 6;

        #else
        wavData[i/2] = ((wavebuf[i+1]<<8 | wavebuf[i]) +0x8000) >> 6;
        wavData[i/2] = ((wavebuf[i+1]<<8 | wavebuf[i]) +0x8000) >> 6 >>dac_vol_level;
        //wavData[i/2] =(((u8)(wavebuf[i+1]-0x80)<<2)|(wavebuf[i]>>6));
        #endif
        i = i + 2;
        
    }
    //g_wave_dma_block_t.dma_blockOffset = addrsta + readlen;
    g_wave_dma_block_t.dma_blockOffset += readlen;
    //dev_debug_printf("wav_data_offset:%d\r\n",g_wave_dma_block_t.dma_blockOffset);
    return ret;
}

static s32 dac_combine_wave_data(u32 addrsta1, u32 readlen1, u32 addrsta2, u32 readlen2 ,u8 wavetype)
{
    s32 ret = 0;
    s32 i = 0;
    s32 j = 0;
    u8 wavebuf[DMA_BLOCK_SIZE*2] = {0};
    

    ret = dev_flash_read_test(addrsta1,wavebuf,readlen1,wavetype);
    if(ret < 0)
    {
        return ret;
    }

    memset(wavData, 0, DMA_BLOCK_SIZE*2);
    for (i = 0; i < readlen1; )
    {
        wavData[j] = (dac_set_volume_level_data((wavebuf[i+1]<<8 | wavebuf[i]), dac_vol_level) + 0x8000) >> 6;
        j++;
        i = i + 2;
        
    }


    memset(wavebuf, 0, DMA_BLOCK_SIZE*2);
    ret = dev_flash_read_test(addrsta2,wavebuf,readlen2,wavetype);
    if(ret < 0)
    {
        return ret;
    }
    j = 0;
    for (i = 0; i < readlen2; )
    {
        wavData[readlen1/2+j] = (dac_set_volume_level_data((wavebuf[i+1]<<8 | wavebuf[i]), dac_vol_level) + 0x8000) >> 6;
        j++;
        i = i + 2;
        
    }
    //g_wave_dma_block_t.dma_blockOffset = addrsta2 + readlen2;
    //g_wave_dma_block_t.dma_blockOffset = 0;
    g_wave_dma_block_t.dma_blockOffset = readlen2;
    DAC_DEBUG("next wav_data_offset:%d\r\n",g_wave_dma_block_t.dma_blockOffset);
    return ret;
}


/****************************************************************************
**Description:       获取WAVE Head信息
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

static s32 dac_get_wave_head_info(u32 addrsta, u8 wavetype)
{
    s32 ret = 0;
    u8 headbuf[WAVE_HEAD_INFO_LEN] = {0};

    memset(&g_strWaveHeadInfo, 0, sizeof(g_strWaveHeadInfo));
    ret = dev_flash_read_test(addrsta, headbuf, WAVE_HEAD_INFO_LEN, wavetype);

    if(ret < 0)
    {
        return DDI_EDATA;
    }

    g_strWaveHeadInfo.m_riff[0] = headbuf[0];
    g_strWaveHeadInfo.m_riff[1] = headbuf[1];
    g_strWaveHeadInfo.m_riff[2] = headbuf[2];
    g_strWaveHeadInfo.m_riff[3] = headbuf[3];

    g_strWaveHeadInfo.m_fsize = (headbuf[7]<<24) | (headbuf[6]<<16) | (headbuf[5]<<8) | (headbuf[4]);
    
    g_strWaveHeadInfo.m_wavetag[0] = headbuf[8];
    g_strWaveHeadInfo.m_wavetag[1] = headbuf[9];
    g_strWaveHeadInfo.m_wavetag[2] = headbuf[10];
    g_strWaveHeadInfo.m_wavetag[3] = headbuf[11];


    g_strWaveHeadInfo.m_fmttag[0] = headbuf[12];
    g_strWaveHeadInfo.m_fmttag[1] = headbuf[13];
    g_strWaveHeadInfo.m_fmttag[2] = headbuf[14];
    g_strWaveHeadInfo.m_fmttag[3] = headbuf[15];
    
    g_strWaveHeadInfo.m_chunsize = (headbuf[19]<<24) | (headbuf[18]<<16) | (headbuf[17]<<8) | (headbuf[16]);
    g_strWaveHeadInfo.m_formattag = (headbuf[21]<<8) | (headbuf[20]);
    g_strWaveHeadInfo.m_channel = (headbuf[23]<<8) | (headbuf[22]);
    g_strWaveHeadInfo.m_samplerate = (headbuf[27]<<24) | (headbuf[26]<<16) | (headbuf[25]<<8) | (headbuf[24]);
    g_strWaveHeadInfo.m_bytepersec = (headbuf[31]<<24) | (headbuf[30]<<16) | (headbuf[29]<<8) | (headbuf[28]);
    g_strWaveHeadInfo.m_bytesample = (headbuf[33]<<8) | (headbuf[32]);
    g_strWaveHeadInfo.m_bitsample = (headbuf[35]<<8) | (headbuf[34]);
    
    g_strWaveHeadInfo.m_datatag[0] = headbuf[36];
    g_strWaveHeadInfo.m_datatag[1] = headbuf[37];
    g_strWaveHeadInfo.m_datatag[2] = headbuf[38];
    g_strWaveHeadInfo.m_datatag[3] = headbuf[39];
    
    g_strWaveHeadInfo.m_datalen = (headbuf[43]<<24) | (headbuf[42]<<16) | (headbuf[41]<<8) | (headbuf[40]);

    #if 0
    DAC_DEBUG("m_riff:%s\r\n",g_strWaveHeadInfo.m_riff);

    DAC_DEBUG("m_fsize:%d\r\n",g_strWaveHeadInfo.m_fsize);

    DAC_DEBUG("m_wavetag:%s\r\n",g_strWaveHeadInfo.m_wavetag);

    DAC_DEBUG("m_fmttag:%s\r\n",g_strWaveHeadInfo.m_fmttag);

    DAC_DEBUG("m_formattag:%d\r\n",g_strWaveHeadInfo.m_formattag);
    DAC_DEBUG("m_channel:%d\r\n",g_strWaveHeadInfo.m_channel);
    DAC_DEBUG("m_samplerate:%d\r\n",g_strWaveHeadInfo.m_samplerate);
    DAC_DEBUG("m_bitsample:%d\r\n",g_strWaveHeadInfo.m_bitsample);

    DAC_DEBUG("m_datatag:%s\r\n",g_strWaveHeadInfo.m_datatag);

    DAC_DEBUG("m_datalen:%d\r\n",g_strWaveHeadInfo.m_datalen);
    #endif

    //保存flash  wave数据的地址和数据长度
    if(g_wave_data_count >= MAX_WAVE_DATA_LEN)
    {
        return DDI_EDATA;
    }
    g_wata_data_t[g_wave_data_count].m_wave_addr = addrsta+WAVE_HEAD_INFO_LEN;
    g_wata_data_t[g_wave_data_count].m_wave_len = g_strWaveHeadInfo.m_datalen;
    g_wata_data_t[g_wave_data_count].m_wave_samplerate = g_strWaveHeadInfo.m_samplerate;
    g_wata_data_t[g_wave_data_count].m_wave_type = wavetype;

    DAC_DEBUG("index:%d,wave addr:%x,wave_len:%d\r\n",g_wave_data_count,g_wata_data_t[g_wave_data_count].m_wave_addr,g_wata_data_t[g_wave_data_count].m_wave_len);
    g_wave_data_count++;

    return ret;
    
}

/****************************************************************************
**Description:       解析WAVE Head信息合法性
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

static s32 decode_wave_info(u32 addrsta, u8 wavetype)
{
    s32 ret = 0;



    


    ret = dac_get_wave_head_info(addrsta, wavetype);

    if(ret < 0)
    {
        return ret;
    }

    //目前只支持16位 单声道的 PCM格式
    if(strncmp(g_strWaveHeadInfo.m_riff, "RIFF", 4) || strncmp(g_strWaveHeadInfo.m_wavetag, "WAVE", 4))
    {
        return DDI_EDATA;
    }

    if(strncmp(g_strWaveHeadInfo.m_fmttag, "fmt ", 4) || strncmp(g_strWaveHeadInfo.m_datatag, "data", 4))
    {
        return DDI_EDATA;
    }

    if((g_strWaveHeadInfo.m_formattag != 0x01) || (g_strWaveHeadInfo.m_channel != 0x01))//单声道 PCM格式
    {
        return DDI_EDATA;
    }

    if(g_strWaveHeadInfo.m_bitsample != 0x10)//16 bit
    {
        return DDI_EDATA;
    }


    
    return ret;
}

static void DMA_Configuration(uint32_t blockSize)
{
    DMA_InitTypeDef DMA_InitStruct;

    //memory to DAC
    DMA_InitStruct.DMA_Peripheral = (uint32_t)(DAC);
    DMA_InitStruct.DMA_DIR = DMA_DIR_Memory_To_Peripheral;
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&DAC->DAC_DATA;
    DMA_InitStruct.DMA_PeripheralInc = DMA_Inc_Nochange;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_DataSize_HalfWord;
    DMA_InitStruct.DMA_PeripheralBurstSize = DMA_BurstSize_4;
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)&wavData[0];
    DMA_InitStruct.DMA_MemoryInc = DMA_Inc_Increment;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_DataSize_HalfWord;
    DMA_InitStruct.DMA_MemoryBurstSize = DMA_BurstSize_4;
    DMA_InitStruct.DMA_BlockSize = blockSize;
    DMA_InitStruct.DMA_PeripheralHandShake = DMA_PeripheralHandShake_Hardware;
    DMA_Init(DAC_DMA_CHANNEL, &DMA_InitStruct);
}

/****************************************************************************
**Description:       init DAC
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void drv_dac_init(void)
{
    DAC_DEBUG("drv_dac_init\r\n");
    dev_gpio_config_mux(DAC_SPK_CTRL_PIN, MUX_CONFIG_ALT1);   //配置功放GPIO口
    dev_gpio_set_pad(DAC_SPK_CTRL_PIN, PAD_CTL_PULL_UP);
    dev_gpio_direction_output(DAC_SPK_CTRL_PIN, 1);
    drv_adc_open(DEV_DAC_AUDIO);
    //SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_ADC, ENABLE);
    //SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_ADC, ENABLE);
    
    SYSCTRL_AHBPeriphClockCmd(SYSCTRL_AHBPeriph_DMA, ENABLE);
    SYSCTRL_AHBPeriphResetCmd(SYSCTRL_AHBPeriph_DMA, ENABLE);
    
    //配置管脚
    dev_gpio_config_mux(GPIO_PIN_PTC1, MUX_CONFIG_ALT2);
    
    //dev_gpio_set_pad(str_dac_param_tab[dacdev].m_gpiopin, PAD_CTL_PULL_NONE);
    
    
    DMA_Configuration(DMA_BLOCK_SIZE);
    DMA_ClearITPendingBit(DAC_DMA_CHANNEL, DMA_IT_BlockTransferComplete);
    DMA_ITConfig(DAC_DMA_CHANNEL, DMA_IT_BlockTransferComplete, ENABLE);
    
    
    NVIC_ClearPendingIRQ(DMA_IRQn);
    NVIC_EnableIRQ(DMA_IRQn);

}


/****************************************************************************
**Description:       打开DAC
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_dac_open(void)
{
    s32 ret = -1;
    s32 i = 0;

    //wave_DataHandle();
    //解析wave数据格式
    #if 0
    ret = decode_wave_info(waveaddr);

    if(ret < 0)
    {
        return ret;
    }
    #endif

    memset(g_wata_data_t, 0, sizeof(g_wata_data_t));
    g_wave_dma_block_t.dma_blockCount = 0;
    g_wave_dma_block_t.dma_blockOffset = 0;
    
    if(g_drv_dac_fd == -1)
    {
        drv_dac_init();
        g_drv_dac_fd = 0;

    }



    return 0;
}
/****************************************************************************
**Description:       关闭DAC设备
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 drv_dac_close(void)
{

    if(g_drv_dac_fd >= 0)
    {
        dev_gpio_direction_output(DAC_SPK_CTRL_PIN, 0);
        DMA_ChannelCmd(DAC_DMA_CHANNEL, DISABLE);
        DAC_DMACmd(DISABLE);
        DAC_Cmd(DISABLE);//此处喇叭会响


        DMA_ITConfig(DAC_DMA_CHANNEL, DMA_IT_BlockTransferComplete, DISABLE);
        NVIC_DisableIRQ(DMA_IRQn);

        //SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_ADC, DISABLE);
        drv_adc_close(DEV_DAC_AUDIO);
        SYSCTRL_APBPeriphClockCmd(SYSCTRL_AHBPeriph_DMA, DISABLE);
        g_drv_dac_fd = -1;

        //清除dma wave数据的记录
        dac_clear_play_status();
        
        g_adc_play_flg = 0;
        //g_drv_adc_chold = ADC_CHANNEL_NONE;
    }
    //dma_blockCount = 0;
    return 0;
}


/**
 *@brief 设置dma 传输长度
 *@param[in] remain dma长度
 */
static void set_dma_block_size(uint32_t remain)
{
        //SET block size
        if(remain != g_dma_block_size)
        {
            DAC_DEBUG("size:%d\r\n",remain);
            DAC_DMA_CHANNEL->CTL_H &= ~0x0fffU;//DMA_CTL_BLOCK_TS_Mask;
            DAC_DMA_CHANNEL->CTL_H |= remain;//DMA_InitStruct->DMA_BlockSize;
            g_dma_block_size = remain;
        }
}


/**
 *@brief 设置dac 采样率
 *@param[in] samplerate 采样率
 */
static void set_dac_samplerate(s32 samplerate)
{
        DAC_InitTypeDef DAC_InitStruct;

        //SET block size
        if(samplerate != g_dac_samplerate)
        {

            #if 1
                DAC->DAC_TIMER = ((SYSCTRL->PCLK_1MS_VAL * 1000) / samplerate/2) - 1;//22050Hz
            #else
            DAC_DEBUG("samplerate:%d\r\n",samplerate);
            DAC_InitStruct.DAC_CurrSel = DAC_CURR_SEL_2K;
            DAC_InitStruct.DAC_FIFOThr = 10;
            DAC_InitStruct.DAC_TimerExp = ((SYSCTRL->PCLK_1MS_VAL * 1000) / samplerate/2) - 1;//22050Hz
            DAC_Init(&DAC_InitStruct);
            DAC_Cmd(ENABLE);
            //DAC->DAC_TIMER = ((SYSCTRL->PCLK_1MS_VAL * 1000) / samplerate/2) - 1;
            #endif
            g_dac_samplerate = samplerate;
        }
}


void DMA0_IRQHandler(void)
{
    uint32_t remain = 0;
    s32 i = 0;

    if(DMA_GetITStatus(DAC_DMA_CHANNEL, DMA_IT_BlockTransferComplete))
    {
        if(g_wave_data_index < g_wave_data_count)
        {

            if(g_wave_dma_block_t.dma_blockOffset + DMA_BLOCK_SIZE*2 <= g_wata_data_t[g_wave_data_index].m_wave_len)
            {
                set_dac_samplerate(g_wata_data_t[g_wave_data_index].m_wave_samplerate);
                set_dma_block_size(DMA_BLOCK_SIZE);
                dac_get_wave_data_info(g_wata_data_t[g_wave_data_index].m_wave_addr+g_wave_dma_block_t.dma_blockOffset, DMA_BLOCK_SIZE*2, g_wata_data_t[g_wave_data_index].m_wave_type);
            }
            else if(0 < (remain = g_wata_data_t[g_wave_data_index].m_wave_len - g_wave_dma_block_t.dma_blockOffset))
            {
                set_dma_block_size(remain >> 1);
                dac_get_wave_data_info(g_wata_data_t[g_wave_data_index].m_wave_addr+g_wave_dma_block_t.dma_blockOffset, remain, g_wata_data_t[g_wave_data_index].m_wave_type);
                g_wave_data_index++;
                g_wave_dma_block_t.dma_blockOffset = 0;
            }

            DMA_SetSRCAddress(DAC_DMA_CHANNEL, (uint32_t)&wavData[0]);
            DMA_ChannelCmd(DAC_DMA_CHANNEL, ENABLE);
        }
        else//播放完成释放DAC资源
        {
            drv_dac_close();
            DAC_DEBUG("end0\r\n");
        }
        DMA_ClearITPendingBit(DAC_DMA_CHANNEL, DMA_IT_BlockTransferComplete);
    }
    NVIC_ClearPendingIRQ(DMA_IRQn);
}

/****************************************************************************
**Description:       DAC tts播放
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
                    DEVSTATUS_ERR_FAIL:  失败
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 drv_dac_tts_play(strttsData_t *ttsdata, s32 ttslen)
{
    DAC_InitTypeDef DAC_InitStruct;
    s32 ret = 0;
    s32 i = 0;

    //播放停止上一次播放
    dac_clear_play_status();

    if(ttslen <= 0)
    {
        return DDI_EINVAL;
    }

    for(i = 0; i < ttslen; i++)
    {
        //DAC_DEBUG("m_wave_addr:%x,m_wave_type:%d\r\n",ttsdata[i].m_wave_addr,ttsdata[i].m_wave_type);
        ret = decode_wave_info(ttsdata[i].m_wave_addr, ttsdata[i].m_wave_type);
        if(ret < 0)
        {
            return ret;
        }
    }
    
    //DAC_DEBUG("g_adc_play_flg:%d\r\n",g_adc_play_flg);

    if(!g_adc_play_flg)
    {
        ret = dac_get_wave_data_info(ttsdata[0].m_wave_addr+WAVE_HEAD_INFO_LEN, DMA_BLOCK_SIZE*2, ttsdata[0].m_wave_type);//头部数据44字节

        if(ret < 0)
        {
            return ret;
        }

        //drv_dac_close();//cgj ?
        if(g_drv_dac_fd == -1)
        {
            drv_dac_init();
            g_drv_dac_fd = 0;
        
        }
        //DAC_DEBUG("offset:%d,index:%d,count:%d\r\n",g_wave_dma_block_t.dma_blockOffset,g_wave_data_index,g_wave_data_count);
        DAC_InitStruct.DAC_CurrSel = DAC_CURR_SEL_2K;
        DAC_InitStruct.DAC_FIFOThr = 10;
        DAC_InitStruct.DAC_TimerExp = ((SYSCTRL->PCLK_1MS_VAL * 1000) / g_wata_data_t[0].m_wave_samplerate/2) - 1;//22050Hz
        DAC_Init(&DAC_InitStruct);
        g_dac_samplerate = g_wata_data_t[0].m_wave_samplerate;
        

        DAC_Cmd(ENABLE);    
        DAC_DMACmd(ENABLE);
        DMA_ChannelCmd(DAC_DMA_CHANNEL, ENABLE);
        g_adc_play_flg = 1;
    }
    return 0;
}

/****************************************************************************
**Description:       DAC 播放
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
                    DEVSTATUS_ERR_FAIL:  失败
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 drv_dac_play(u32 waveaddr)
{
    DAC_InitTypeDef DAC_InitStruct;
    s32 ret = 0;

    //播放停止上一次播放
    dac_clear_play_status();
    ret = decode_wave_info(waveaddr, 0);
    if(ret < 0)
    {
        return ret;
    }
    
    //DAC_DEBUG("g_adc_play_flg:%d\r\n",g_adc_play_flg);
    if(!g_adc_play_flg)
    {
        ret = dac_get_wave_data_info(waveaddr+WAVE_HEAD_INFO_LEN, DMA_BLOCK_SIZE*2, 0);//头部数据44字节

        if(ret < 0)
        {
            return ret;
        }
        
        if(g_drv_dac_fd == -1)
        {
            drv_dac_init();
            g_drv_dac_fd = 0;
        
        }
        //DAC_DEBUG("offset:%d,index:%d,count:%d\r\n",g_wave_dma_block_t.dma_blockOffset,g_wave_data_index,g_wave_data_count);
        DAC_InitStruct.DAC_CurrSel = DAC_CURR_SEL_2K;
        DAC_InitStruct.DAC_FIFOThr = 10;
        DAC_InitStruct.DAC_TimerExp = ((SYSCTRL->PCLK_1MS_VAL * 1000) / g_strWaveHeadInfo.m_samplerate/2) - 1;//22050Hz
        DAC_Init(&DAC_InitStruct);
        g_dac_samplerate = g_strWaveHeadInfo.m_samplerate;
        
        DAC_Cmd(ENABLE);    
        DAC_DMACmd(ENABLE);
        DMA_ChannelCmd(DAC_DMA_CHANNEL, ENABLE);
        g_adc_play_flg = 1;
    }
    return 0;
}

/****************************************************************************
**Description:       DAC 暂停
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
                    DEVSTATUS_ERR_FAIL:  失败
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 drv_dac_pause(void)
{
    if(g_drv_dac_fd < 0)
    {
        return -1;
    }
    //DMA_ChannelCmd(DAC_DMA_CHANNEL, DISABLE);
    //DAC_DMACmd(DISABLE);
    //DAC_Cmd(DISABLE); 
    NVIC_DisableIRQ(DMA_IRQn);
    return 0;
}

/****************************************************************************
**Description:       DAC 恢复
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
                    DEVSTATUS_ERR_FAIL:  失败
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 drv_dac_replay(void)
{
    if(g_drv_dac_fd < 0)
    {
        return -1;
    }
    //DAC_Cmd(ENABLE);    
    //DAC_DMACmd(ENABLE);
    //DMA_Configuration(DMA_BLOCK_SIZE);
    //DMA_ChannelCmd(DAC_DMA_CHANNEL, ENABLE);
    NVIC_ClearPendingIRQ(DMA_IRQn);
    NVIC_EnableIRQ(DMA_IRQn);
    return 0;
}

/****************************************************************************
**Description:       DAC 获取播放状态
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
                    DEVSTATUS_ERR_FAIL:  失败
                    
**Created by:        pengxuebin,20170422
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/

s32 drv_dac_is_playing(void)
{
    return g_adc_play_flg;
}

void drv_dac_irq_ctl(u8 flg)
{
    if(g_drv_dac_fd < 0)
    {
        return;
    }
    if(flg)
    {
        NVIC_EnableIRQ(DMA_IRQn);
    }
    else
    {
        NVIC_DisableIRQ(DMA_IRQn);
    }
}


