/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:     pengxuebin,20170428      
** Created Date:     
** Version:        
** Description:    AUDIO��������  
****************************************************************************/
#include "devglobal.h"
#include "drv_audio.h"
#include "ddi_audio.h"
#include "ddi_misc.h"
#if(AUDIO_EXIST==1)
static s32 g_dev_audio_fd = -1;


static u8 tts_source_init = 0;
static u32  g_audio_file_node_num = 0;


static strttsData_t g_ttsData[MAX_WAVE_DATA_LEN];//����TTS wave������
static u32  g_tts_num= 0;//����TTS������

strttsData_t g_audioData[MAX_WAVE_DATA_LEN];//����audio wave������
static u32  g_audio_num= 0;//����TTS������
static s32 g_mp3_play_flag = TRUE_TRENDIT;         //TRUE_TRENDIT 

#if(AUDIO_WITH_DAC == TRUE_TRENDIT)
//�����ʾ����Ӧ��index
static s32 g_index_table[MAX_INTERNAL_AUDIO_DATA_SIZE] = {-1};
/****************************************************************************
**Description:        audio�豸��ʱ��������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
**Created by:        pengxuebin,20170429
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
void dev_audio_task(void)
{
    drv_audio_task();
}


/****************************************************************************
**Description:        ��audio�豸
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170429
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_audio_open(void)
{
    s32 ret ;
    s32 i;
    
    if(g_dev_audio_fd < 0)
    {
        #if 0
        ret = drv_audio_open();
        if(ret < 0)
        {
            return DEVSTATUS_ERR_FAIL;
        }
        #endif
        
        //��ʼ��
        if(!tts_source_init)
        {

            for(i = 0; i < MAX_INTERNAL_AUDIO_DATA_SIZE; i++)
            {
                g_index_table[i] = -1;
            }
            //dev_debug_printf("init_source_table000\r\n");
            init_source_table();
            //dev_debug_printf("init_source_table111\r\n");
            tts_source_init = 1;
        }
        /*
        ret = dev_dac_open();
        if(ret < 0)
        {
            return DEVSTATUS_ERR_FAIL;
        }
        */
        g_dev_audio_fd = 0;
    }
    return DEVSTATUS_SUCCESS;
}
/****************************************************************************
**Description:        �ر�audio�豸
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170429
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_audio_close(void)
{
    if(g_dev_audio_fd >= 0)
    {
        //drv_audio_close();
        dev_dac_close();
        g_dev_audio_fd = -1;
    }
    return DEVSTATUS_SUCCESS;
}
/**
 *@brief �ҳ��ַ�����Ӧ������
 *@param[in]text �ַ���
 *@retval:����-1 ���� -1������
 */
s32 index_of_table(u8     *text)
{
    s32 ret = 0;
    s32 i = 0;

    for(i = 0; i < g_audio_file_node_num; i++)
    {
        if(strcmp(g_audio_file_nodes[i]->m_text,text) == 0)
        {
            return i;
        }
    }
    return -1;
}

/**
 *@brief �ҳ��ַ�����Ӧ��λ��
 *@param[in]strsrc src�ַ���
 *@param[in]strdst dst�ַ���
 *@retval:���ص�ǰ�����ַ�����λ��
 */

s32 find_str_position(u8 *strsrc, u8 *strdst)
{
    int i = 0;
    int flg = 0;

    if(strlen(strsrc) < strlen(strdst))
    {
        return -1;
    }
    for(i = 0; i < strlen(strdst); i++)
    {
        //dev_debug_printf("src:%x,dst:%x\r\n",strsrc[i],strdst[i]);
        if(strsrc[i] == strdst[i])
        {
            flg = 1;
        }
        else
        {
            flg = 0;
            break;
        }
    }

    if(flg)
    {
        return strlen(strdst);
    }
    
    return -1;
}


/**
 *@brief �ҳ��ַ�����Ӧ��λ��
 *@param[in]strsrc src�ַ���
 *@param[in]strdst dst�ַ���
 *@retval:���ص�ǰ�����ַ�����λ��
 */

static s32 is_strsubstring(u8 *strsrc, u8 *strdst)
{
    s32 i=0,j=0,index=-1;
    if((strsrc == NULL) || (strdst == NULL))
    {
        return -1;
    }
    while(i<strlen(strsrc) && j<strlen(strdst)){
        if(strsrc[i]==strdst[j]){//����ַ���ͬ�������ַ�������
            i++;
            j++;
        }else{
            i=i-j+1; //�����ַ��ص��Ƚ��ʼ�Ƚϵĺ�һ���ַ�
            j=0;     //�ִ��ַ����¿�ʼ
        }
        if(j==strlen(strdst)){ //���ƥ��ɹ�
            //flag=0;  //�ִ�����
            index = i-strlen(strdst);
            break;
        }
    }
    return index;
}

/**
 *@brief ��/mtd0/audioȥ���ļ����ظ���
 *@param[in]filename �ļ���
 *@retval:0  -1�ļ����а�������
 */
static s32 check_audio_invalid(u8 *filename)
{
    s32 i = 0;
    s32 ret = DEVSTATUS_SUCCESS;
    if(filename == NULL)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    for(i = 0; i < g_audio_num; i++)
    {
        //���������ַ�����������
        if(strcmp(g_audioData[i].m_wave_text, filename) == 0)
        {
            //return DEVSTATUS_ERR_FAIL;
            ret = DEVSTATUS_ERR_FAIL;
        }
        
        if(is_strsubstring(g_audioData[i].m_wave_text,filename) >= 0)
        {
            ret = DEVSTATUS_ERR_FAIL;
        }


        if((is_strsubstring(filename, g_audioData[i].m_wave_text) >= 0))
        {
            strcpy(g_audioData[i].m_wave_text, filename);
            ret = DEVSTATUS_ERR_FAIL;
        }
    }

    return ret;
}
/**
 *@brief ��/mtd0/audio���ҿͻ�������Դ
 *@param[in]text �ַ���
 *@retval:����-1 ���� -1������
 */
s32 find_text_from_audio_path(u8 *text)
{
    s32 i = 0;
    s32 ret = DEVSTATUS_ERR_FAIL;
    s32 index = -1;
    u8 filename[128];

    g_audio_num = 0;
    memset(g_audioData, 0, sizeof(strttsData_t)*MAX_WAVE_DATA_LEN);
    if(text == NULL)
    {
        return ret;
    }
    for(i = 0; i < AUDIO_MAX_NUM; i++)
    {
        memset(filename, 0, sizeof(filename));
        fs_read_file(AUDIO_SOURCE_PATH,filename,sizeof(filename),audio_filename_addr(i));
        index = is_strsubstring(text, filename);
        //dev_debug_printf("find audio name :%s,index:%d\r\n",filename,index);
        if(index >= 0)
        {
            ret = DEVSTATUS_SUCCESS;
            if(check_audio_invalid(filename) == DEVSTATUS_SUCCESS)
            {
                g_audioData[g_audio_num].m_wave_addr = audio_filedata_addr(i);
                g_audioData[g_audio_num].m_wave_index = index;
                memcpy(g_audioData[g_audio_num].m_wave_text, filename, strlen(filename));
                g_audioData[g_audio_num].m_wave_type = 1;
                g_audio_num++;
                if(g_audio_num >= MAX_WAVE_DATA_LEN)
                {
                    return DEVSTATUS_ERR_FAIL;
                }
            }

        }
    }
    return ret;
}



/**
 *@brief ��/mtd2/tts.binȥ���ļ����ظ���
 *@param[in]filename �ļ���
 *@retval:0  -1�ļ����а�������
 */
static s32 check_tts_invalid(u8 *filename)
{
    s32 i = 0;
    s32 ret = DEVSTATUS_SUCCESS;
    if(filename == NULL)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }

    //
    #if 1//������audio��Դcontain����
    for(i = 0; i < g_audio_num; i++)
    {
        if(is_strsubstring(g_audioData[i].m_wave_text,filename) >= 0)
        {
            ret = DEVSTATUS_ERR_FAIL;
        }
        if(is_strsubstring(filename, g_audioData[i].m_wave_text) >= 0)
        {
            ret = DEVSTATUS_ERR_FAIL;
        }
    }
    #endif
    for(i = 0; i < g_tts_num; i++)
    {
        //���������ַ�����������
        if(strcmp(g_ttsData[i].m_wave_text, filename) == 0)
        {
            //return DEVSTATUS_ERR_FAIL;
            ret = DEVSTATUS_ERR_FAIL;
        }
        
        if(is_strsubstring(g_ttsData[i].m_wave_text,filename) >= 0)
        {
            ret = DEVSTATUS_ERR_FAIL;
        }


        if((is_strsubstring(filename, g_ttsData[i].m_wave_text) >= 0))
        {
            strcpy(g_ttsData[i].m_wave_text, filename);
            ret = DEVSTATUS_ERR_FAIL;
        }
    }

    return ret;
}

/**
 *@brief ��/mtd2/tts.bin���ҿͻ�������Դ
 *@param[in]text �ַ���
 *@retval:����-1 ���� -1������
 */
s32 find_text_from_tts_path(u8 *text)
{
    s32 i = 0;
    s32 index = -1;
    s32 ret = DEVSTATUS_ERR_FAIL;
    u8 filename[128];


    g_tts_num = 0;
    memset(g_ttsData, 0, sizeof(strttsData_t)*MAX_WAVE_DATA_LEN);

    if(text == NULL)
    {
        return ret;
    }

    for(i = 0; i < g_audio_file_node_num; i++)
    {

        index = is_strsubstring(text, g_audio_file_nodes[i]->m_text);
        //dev_debug_printf("find audio name :%s,index:%d\r\n",g_audio_file_nodes[i]->m_text,index);
        if(index >= 0)
        {
            ret = DEVSTATUS_SUCCESS;
            if(check_tts_invalid(g_audio_file_nodes[i]->m_text)  == DEVSTATUS_SUCCESS)
            {
                g_ttsData[g_tts_num].m_wave_addr = g_audio_file_nodes[i]->m_file_start_addr;
                g_ttsData[g_tts_num].m_wave_index = index;
                memcpy(g_ttsData[g_tts_num].m_wave_text, g_audio_file_nodes[i]->m_text, strlen(g_audio_file_nodes[i]->m_text));
                g_ttsData[g_tts_num].m_wave_type = 0;
                g_tts_num++;
                if(g_tts_num >= MAX_WAVE_DATA_LEN - g_audio_num)
                {
                    return DEVSTATUS_ERR_FAIL;
                }
            }

        }
    }
    return ret;

}


s32 find_text_from_all_audio_path(u8 *text)
{
    s32 i,j;
    s32 audio_ret = DEVSTATUS_ERR_FAIL;
    s32 tts_ret = DEVSTATUS_ERR_FAIL;
    s32 ret = DEVSTATUS_SUCCESS;
    strttsData_t tmpttsdata;


    audio_ret = find_text_from_audio_path(text);

    tts_ret = find_text_from_tts_path(text);


    if((tts_ret < 0) && (audio_ret < 0))
    {
        return DEVSTATUS_ERR_FAIL;
    }
    #if 0
    for(i = 0; i < g_audio_num; i++)
    {
                dev_debug_printf("audio addr:%x,index:%d,text:%s,type:%d\r\n",
                    g_audioData[i].m_wave_addr,g_audioData[i].m_wave_index,g_audioData[i].m_wave_text,g_audioData[i].m_wave_type);
    }
    for(i = 0; i < g_tts_num; i++)
    {
                dev_debug_printf("tts addr:%x,index:%d,text:%s,type:%d\r\n",
                    g_ttsData[i].m_wave_addr,g_ttsData[i].m_wave_index,g_ttsData[i].m_wave_text,g_ttsData[i].m_wave_type);
    }
    #endif


    //���TTS
    for(i = 0; i < g_audio_num; i++)
    {
        g_ttsData[g_tts_num] = g_audioData[i]; 
        g_tts_num++;
        if(g_tts_num > MAX_WAVE_DATA_LEN)
        {
            return DEVSTATUS_ERR_OVERFLOW;
        }
    }



    //������Դ����index��������
    for(i = 0; i < (g_tts_num-1); i++)
    {
        for(j = i+1; j < g_tts_num; j++)
        {
            if(g_ttsData[i].m_wave_index > g_ttsData[j].m_wave_index)
            {
                tmpttsdata = g_ttsData[i];

                g_ttsData[i] = g_ttsData[j];

                g_ttsData[j] = tmpttsdata;

            }
        }
    }

    for(i = 0; i < g_tts_num; i++)
    {
        dev_debug_printf("sort addr:%x,index:%d,text:%s,type:%d\r\n",
                    g_ttsData[i].m_wave_addr,g_ttsData[i].m_wave_index,g_ttsData[i].m_wave_text,g_ttsData[i].m_wave_type);
    }

    return ret;
}

/**
 *@brief �ָ��ַ�������wave����addr
 *@param[in]text �ַ���
 *@retval:����-1 ���� -1������
 */
s32 split_str_set_wave(u8     *text)
{
    s32 ret = -1;
    s32 currpos = 0;
    s32 i = 0;
    s32 j = 0;
    
    g_tts_num = 0;
    memset(g_ttsData, 0, sizeof(g_ttsData));
    for(j = 0; j < strlen(text);)
    {
        
        for(i = 0; i < g_audio_file_node_num; i++)
        {
            currpos = find_str_position(text+j, g_audio_file_nodes[i]->m_text);
            if(currpos > 0)
            {
                AUDIO_DEBUG("currpos:%d,text:%s,addr:%x\r\n",currpos, g_audio_file_nodes[i]->m_text, g_audio_file_nodes[i]->m_file_start_addr);
                g_ttsData[g_tts_num++].m_wave_addr = g_audio_file_nodes[i]->m_file_start_addr;//�����ϲ�TTS�����ݲ������ַ
                if(g_tts_num >= MAX_WAVE_DATA_LEN)
                {
                    return -1;
                }
                ret = 0;
                break;
            }
        }
        if(currpos > 0)
        {
            j = currpos+j;
        }
        else
        {
            j++;
        }
    }


    return ret;
}


/**
 *@brief ����tts
 *@param[in] audio_file_node ���tts��Ӧ�Ľڵ�
 *@retval 0�ɹ� ����ʧ��
 */
static s32 dev_audio_play_audio_file_node(str_audio_file_node *audio_file_node)
{
    s32 ret = 0;
    u8 volume = 0;


    #if 1
    ret = dev_dac_play(audio_file_node->m_file_start_addr);
    #else
    if(audio_file_node->m_audio_data == NULL)
    {
        audio_file_node->m_audio_data = k_malloc(audio_file_node->m_audio_data_len);
        if(audio_file_node->m_audio_data == NULL)
        {
            return DEVSTATUS_ERR_OVERFLOW;
        }
        ret = fs_read_file(TTS_SOURCE_PATH, audio_file_node->m_audio_data, audio_file_node->m_audio_data_len, audio_file_node->m_file_start_addr);
        if(ret != audio_file_node->m_audio_data_len)
        {
            k_free(audio_file_node->m_audio_data);
            audio_file_node->m_audio_data = NULL;
            return DEVSTATUS_ERR_FAIL;
        }
    }
    if(audio_file_node->m_audio_tone == AUDIO_TONE_KEY)
    {
        volume = g_key_volume;
    }
    else
    {
        volume = dev_audio_get_volume();
    }
    ret = dev_audio_play_data(audio_file_node->m_audio_data, audio_file_node->m_audio_data_len, volume);
    //������ʾ�������ͷ��ڴ档
    if(audio_file_node->m_audio_tone != AUDIO_TONE_KEY)
    {
        k_free(audio_file_node->m_audio_data);
        audio_file_node->m_audio_data = NULL;
    }
    return ret;
    #endif
}
/****************************************************************************
**Description:        ��������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170429
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_audio_set_volume(u8 volume)
{

    return dev_dac_set_volume_level(volume);
}


/****************************************************************************
**Description:        ��ȡ����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170429
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/


u8 dev_audio_get_volume(void)
{

    return dev_dac_get_volume_level();
}

/**
 * @brief ���Ÿ�ʽΪgb2312��ʽ�ַ���
 * @param [in] str gb2312��ʽ���ַ���
 * @retval DEVSTATUS_SUCCESS�ɹ�
 * @retval other ����
 */
s32 dev_audio_playcharacters (s8 audiotype, u8 *tts_text)
{
    s32 index;
    u8 volume;
    s32 ret;

    //��ƥ�䣬����tts����
    ret = dev_wireless_ttsaudio_play(audiotype, tts_text, strlen(tts_text));

    return ret;
}

/**
 * @brief �����ļ�
 * @param [in] lpFile �ļ���
 * @retval DEVSTATUS_SUCCESS�ɹ�
 * @retval other ����
 */
s32 dev_audio_play_file(const u8 *lpFile)
{
    s32 file_len = 0;
    s32 i = 0;
    s32 play_result = DEVSTATUS_ERR_FAIL;
    //u8* buf = NULL;
    
    u8 headbuf[44] = {0};

    if(g_dev_audio_fd < 0)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }
    if(NULL == lpFile)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }

    #if 1
    file_len = fs_access_file(lpFile);

    dev_user_delay_ms(20);
    if(file_len <= 0)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    #endif
    #if 1

    #if 0
    
    fs_read_file(lpFile, headbuf, sizeof(headbuf), 0);

    //drv_dac_play(0,);
    for(i = 0; i < 44; i++)
    {
        AUDIO_DEBUG("headbuf[%d] = %x\r\n", i, headbuf[i]);
    }
    dev_dac_play(sizeof(headbuf));
    #endif
    #else
    buf = k_malloc(file_len);
    if(NULL == buf)
    {
        AUDIO_DEBUG("no memory = %d", file_len);
        return DEVSTATUS_ERR_OVERFLOW;
    }
    memset(buf, 0, file_len);
    fs_read_file(lpFile, buf, file_len, 0);
    play_result = dev_audio_play_data(buf, file_len, dev_audio_get_volume());
    k_free(buf);
    buf  = NULL;
    #endif
    return play_result;
}

/****************************************************************************
**Description:        �����ļ�
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170429
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_audio_play(const u8 * lpFile)
{

    s32 ret;

    if(g_dev_audio_fd < 0)
    {
        return DEVSTATUS_ERR_DEVICE_NOTOPEN;
    }
    if(lpFile == NULL || strlen(lpFile) <= 0)
    {
        return DEVSTATUS_ERR_PARAM_ERR;
    }
    ret = dev_audio_play_file(lpFile);
    return ret;

    return DDI_OK;
}
/**
 *@brief ������������ʾ��ʵ��
 *@param[in] voice_type ��������AUDIO_CTL_VOICE_TYPE_...
 */
static s32 dev_audio_buzzer_impl(u32 voice_type)
{
    strbeepParam_t beepparam;

    beepparam.m_times = 1;
    beepparam.m_ontime = 50;
    beepparam.m_offtime = 20;
    switch(voice_type)
    {
        case AUDIO_CTL_VOICE_TYPE_GENERAL:
            drv_audio_beep(beepparam);
            return DEVSTATUS_SUCCESS;
        case AUDIO_CTL_VOICE_TYPE_ERROR:
            drv_audio_beep(beepparam);
            //ddi_misc_msleep(100);
            dev_user_delay_ms(100);
            drv_audio_beep(beepparam);
            return DEVSTATUS_SUCCESS;
        default:
            return DEVSTATUS_ERR_PARAM_ERR;
    }
}

/**
 *@brief ���Ȳ�������ʵ��
 *@param[in] voice_type ��������AUDIO_CTL_VOICE_TYPE_...
 */
static s32 dev_audio_horn_impl(u32 voice_type, u8 *data)
{
    s32 ret = DEVSTATUS_ERR_FAIL;
    s32 index;
    u16 data_len = 0;

    switch(voice_type)
    {
        case AUDIO_CTL_VOICE_TYPE_GENERAL:
        case AUDIO_CTL_VOICE_TYPE_ERROR:
            #if 1
            index = g_index_table[voice_type];
            if(index < 0)
            {
                return DEVSTATUS_ERR_FAIL;
            }
            ret = dev_audio_play_audio_file_node(g_audio_file_nodes[index]);
            return ret;
            #endif
        case AUDIO_CTL_VOICE_TYPE_CUSTOM:
            #if 0
            if(data == NULL)
            {
                return DEVSTATUS_ERR_PARAM_ERR;
            }
            data_len = (data[0] << 8) + data[1];
            ret = dev_audio_play_data(&data[2], data_len, g_key_volume);
            return ret;
            #endif
        default:
            return DEVSTATUS_ERR_PARAM_ERR;
    }
    return ret;
}
static s32 dev_audio_play_voice(u32 voice_type, u8 *data)
{

    #if 1
        return dev_audio_horn_impl(voice_type,data);
    #else
    if(ddi_misc_probe_dev(DEV_BEEP) == DDI_OK)
    {
        return dev_audio_buzzer_impl(voice_type);
    }
    else if(ddi_misc_probe_dev(DEV_AUDIO) == DDI_OK)
    {
        return dev_audio_horn_impl(voice_type,data);
    }
    return DEVSTATUS_ERR_FAIL;
    #endif
}



/****************************************************************************
**Description:        ֹͣ����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170429
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_audio_stop (void)
{
    return dev_dac_close();
    //return DDI_OK;
}
/****************************************************************************
**Description:        ��ͣ����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170429
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_audio_pause (void)
{
    return dev_dac_pause();
    //return DDI_OK;
}
/****************************************************************************
**Description:        ���²���
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170429
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_audio_replay (void)
{
    return dev_dac_replay();
    //return DDI_OK;
}


/****************************************************************************
**Description:        IOCTL
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170429
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_audio_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    strbeepParam_t beepparam;
    strbeepParam_t *lp_beepparam;
    s32 ret = DDI_ERR;
    
    if(g_dev_audio_fd < 0)
    {
        return DDI_EIO;
    }
    switch(nCmd)
    {
        case DDI_AUDIO_CTL_VER:               //(0)  //��ȡ��Ƶ�豸�汾
            break;
        //------------------------------------
        case DDI_AUDIO_CTL_VOLUME:            //(1)  //��������
            ret = dev_audio_set_volume(lParam);
            break;
        //------------------------------------
        case DDI_AUDIO_CTL_BUZZER:            //(2)  //���Ʒ���������
            beepparam.m_pwmhz = drv_audio_get_pwm();    //����Ĭ��Ƶ��
            if(wParam == 0)
            {
                beepparam.m_times = 0;
            }
            else
            {
                beepparam.m_times = 1;
                beepparam.m_ontime = wParam;
                beepparam.m_offtime = 20;
            }      
            ret = drv_audio_beep(beepparam);
            break;
        //------------------------------------
        case DDI_AUDIO_CTL_COMB_PLAY:         //(3)  //��ϲ���
            break;
        //------------------------------------
        case DDI_AUDIO_CTL_GET_STATUS:        //(4)  //��ȡ����״̬
            if(lParam == NULL)
            {
                return DDI_EINVAL;
            }
            (*(u8*)lParam) = dev_dac_is_playing();
            ret = DDI_OK;
            break;

        //------------------------------------
        case DDI_AUDIO_CTL_BUZZER_1:         //���Ʒ���������1,���Է���������
            //if(lParam == 0)   //��ʱʹ��Ĭ��Ƶ��
            {
                beepparam.m_pwmhz = drv_audio_get_pwm();    //����Ĭ��Ƶ��
            }
            lp_beepparam = (strbeepParam_t*)wParam;
            if(lp_beepparam->m_times == 0)
            {
                beepparam.m_times = 0;
            }
            else
            {
                beepparam.m_times = lp_beepparam->m_times;
                beepparam.m_ontime = lp_beepparam->m_ontime;
                beepparam.m_offtime = lp_beepparam->m_offtime;
            }
            ret =  drv_audio_beep(beepparam);
            break;

        case DDI_AUDIO_GET_VOLUME:          //(6)��ȡ����
            if(lParam == NULL)
            {
                return DDI_ERR;
            }
            (*(u32*)lParam) = dev_audio_get_volume();
            ret = DDI_OK;
            break;
        case DDI_AUDIO_CTL_VOICE:           //(7)������Ƶ
            ret = dev_audio_play_voice(lParam, (u8*)wParam);
            break;
        default:
            break;
    }
    return ret;
}
/****************************************************************************
**Description:        ��ͣ
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170808
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_audio_suspend(void)
{
    
    if(g_dev_audio_fd < 0)
    {
        return DDI_EIO;
    }
    return drv_audio_suspend();
    
}
/****************************************************************************
**Description:        ����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170808
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_audio_resume(void)
{
    if(g_dev_audio_fd < 0)
    {
        return DDI_EIO;
    }
    return drv_audio_resume();
}

#elif(AUDIO_ON_WIRELESS == TRUE_TRENDIT)
void dev_audio_task(void)
{
}
s32 dev_audio_open(void)
{
    s32 ret = DDI_ERR;

#ifdef TRENDIT_CORE
    do{
        if(g_dev_audio_fd < 0)
        {
            ret = ttsbin_instance()->init();
            if(DDI_OK == ret)
            {
                g_dev_audio_fd = 1;
            }
        }
        else
        {
            ret = DDI_OK;
        }
    }while(0);
#endif
        
    return ret;
}
s32 dev_audio_close(void)
{
    s32 ret = DDI_OK;

#ifdef TRENDIT_CORE
    if(g_dev_audio_fd > 0)
    {
        ttsbin_instance()->exit();
        g_dev_audio_fd = -1;
    }
#endif

    return ret;
}
s32 dev_audio_play(const u8 * lpFile)
{
    return DDI_EUN_SUPPORT;
}
s32 dev_audio_stop (void)
{
    return DDI_EUN_SUPPORT;
}
s32 dev_audio_pause (void)
{
    return DDI_EUN_SUPPORT;
}
s32 dev_audio_replay (void)
{
    return DDI_EUN_SUPPORT;
}

/**
 * @brief �����ص�tts����ͬ��������ģ����
 * @param[in] ��
 * @retval  DDI_OK
 */
s32 dev_audio_syncfilewithmodem(void)
{
    s32 ret = DDI_ERR;
    s32 i = 0, j = 0;
    s8 (*filelist)[WIRELESS_AUDIO_FILE_NAME_MAX] = NULL;
    s32 filelist_num = 0;

    do{
        TRACE_DBG("enter");
        filelist = (s8 (*)[WIRELESS_AUDIO_FILE_NAME_MAX])k_malloc(AUDIO_MODEMFILE_MAXNUM*WIRELESS_AUDIO_FILE_NAME_MAX);
        if(NULL == filelist)
        {
            TRACE_ERR("malloc failed");
            ret = DDI_EOVERFLOW;
            break;
        }

        filelist_num = AUDIO_MODEMFILE_MAXNUM;
        ret = dev_wireless_get_audio_file_list(filelist, filelist_num);
        TRACE_DBG("get audio num:%d", ret);
        if(ret < 0)
        {
            TRACE_ERR("get audio num failed");
            ret = DDI_ERR;
            break;
        }
        else
        {
            filelist_num = ret;
        }
        
        //ɾ������ģ����������Դ�ļ�
        for(i=0; i<filelist_num; i++)
        {
            //ɾ�������3��
            for(j=0; j<3; j++)
            {
                dev_watchdog_feed();
                ret = dev_wireless_delete_audio_file(filelist[i]);
                if(DDI_OK == ret)
                {
                    break;
                }
                else
                {
                    TRACE_ERR("delete audio file %s fail, try times:%d", filelist[i], j+1);
                    ddi_misc_msleep(500);
                }
            }

            if(3 == j)
            {
                TRACE_ERR("delete module audio file fail");
                ret = DDI_ERR;
                break;
            }
        }

        if(DDI_OK != ret && 3 == j)
        {
            break;
        }

        //���¼���tts.bin
        dev_audio_close();
        ret = dev_audio_open();
        if(DDI_OK != ret)
        {
            ret = DDI_ERR;
            break;
        }

        TRACE_DBG("m_num:%d", ttsbin_instance()->m_num);
        //������Դ�ļ���ģ��
        for(i=0; i<ttsbin_instance()->m_num; i++)
        {
            dev_watchdog_feed();
            //�����3��
            for(j=0; j<3; j++)
            {
                ret = dev_wireless_add_audio_file(i);
                if(DDI_OK == ret)
                {
                    break;
                }
                else
                {
                    TRACE_ERR("add audio file %d fail, try times:%d", i, j+1);
                    ddi_misc_msleep(500);
                }
            }

            if(3 == j)
            {
                TRACE_ERR("add module audio file fail");
                ret = DDI_ERR;
                break;
            }
        }
    }while(0);

    if(filelist)
    {
        k_free(filelist);
        filelist = NULL;
    }
    TRACE_DBG("ret:%d", ret);
    return ret;
}

s32 dev_audio_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    s32 ret = DDI_EINVAL;
#ifdef TRENDIT_CORE
    switch(nCmd)
    {
        case DDI_AUDIO_CTL_VER:               //(0)  //��ȡ��Ƶ�豸�汾
            break;

        case DDI_AUDIO_CTL_VOLUME:            //(1)  //��������
            ret = dev_wireless_setaudiovolume(lParam);
            break;
        
        case DDI_AUDIO_CTL_GET_STATUS:        //(4)  //��ȡ����״̬
            if(lParam == NULL)
            {
                return DDI_EINVAL;
            }
            (*(u8*)lParam) = dev_wireless_audio_isplaying();
            ret = DDI_OK;
            break;

        case DDI_AUDIO_GET_VOLUME:          //(6)��ȡ����
            TRACE_DBG("enter");
            if(lParam == NULL)
            {
                return DDI_ERR;
            }
            TRACE_DBG("enter");
            ret = dev_wireless_getaudiovolume((u32*)lParam);
            break;

        case DDI_AUDIO_SET_PLAYMP3FLAG:
            g_mp3_play_flag = lParam;
            break;
            
        case DDI_AUDIO_SYNC_AUDIOFILE:
            dev_wireless_strategy_run_switch(FALSE_TRENDIT);
            ret = dev_audio_syncfilewithmodem();
            dev_wireless_strategy_run_switch(TRUE_TRENDIT);
            break;

        case DDI_AUDIO_GET_TTSFILECRC:
            ret = ttsbin_instance()->get_crc();
            break;
        
        default:
            break;
    }
#endif
    return ret;
}

s32 dev_audio_suspend(void)
{
    return DDI_OK;
}
s32 dev_audio_resume(void)
{
    return DDI_OK;
}

/**
 * @brief ���Ÿ�ʽΪgb2312��ʽ�ַ���
 * @param [in] str gb2312��ʽ���ַ���
 * @retval DEVSTATUS_SUCCESS�ɹ�
 * @retval other ����
 */
s32 dev_audio_playcharacters(s8 audiotype, u8 *tts_text)
{
    s32 ret = DDI_ERR;
    u16 index_tab[32];
    s32 index_tab_num = sizeof(index_tab)/sizeof(index_tab[0]);
    s8 buff[512] = {0};
    s32 i = 0;

#ifdef TRENDIT_CORE
    do{
        if(g_dev_audio_fd > 0)
        {
            if(g_mp3_play_flag)
            {
                ret = ttsbin_instance()->get_audiostring_index(tts_text, index_tab, &index_tab_num);
                if(DDI_OK == ret)    //ƥ��mp3��ʹ��mp3��ʽ����
                {
                    memset(buff, 0, sizeof(buff));
                    snprintf(buff, sizeof(buff), "\"%s.mp3\"", ttsbin_instance()->get_audio_filetext(index_tab[0]));

                    if(index_tab_num > 1)
                    {
                        for(i=1; i<index_tab_num; i++)
                        {
                            snprintf(buff+strlen(buff), sizeof(buff)-strlen(buff), ",\"%s.mp3\"", ttsbin_instance()->get_audio_filetext(index_tab[i]));
                        }
                    }
                    
                    ret = dev_wireless_audiofile_play(buff);
                    break;
                }
            }
        }

        ret = dev_wireless_ttsaudio_play(audiotype, tts_text, strlen(tts_text));
    }while(0);
#endif

    return ret;
}

/****************************************************************************
**Description:        ��������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170429
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_audio_set_volume(u8 volume)
{
#ifdef TRENDIT_CORE
    return dev_wireless_setaudiovolume(volume);
#endif
}


/****************************************************************************
**Description:        ��ȡ����
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
**Created by:        pengxuebin,20170429
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/


u8 dev_audio_get_volume(void)
{
    u32 volume = 0;

#ifdef TRENDIT_CORE
    dev_wireless_getaudiovolume(&volume);
#endif

    return volume;
}
#endif

#else
void dev_audio_task(void)
{
}
s32 dev_audio_open(void)
{
    return DDI_ENODEV;
}
s32 dev_audio_close(void)
{
    return DDI_ENODEV;
}
s32 dev_audio_play(const u8 * lpFile)
{
    return DDI_ENODEV;
}
s32 dev_audio_stop (void)
{
    return DDI_ENODEV;
}
s32 dev_audio_pause (void)
{
    return DDI_ENODEV;
}
s32 dev_audio_replay (void)
{
    return DDI_ENODEV;
}

s32 dev_audio_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    return DDI_ENODEV;
}

s32 dev_audio_suspend(void)
{
    return DDI_ENODEV;
}
s32 dev_audio_resume(void)
{
    return DDI_ENODEV;
}

s32 dev_audio_playcharacters (s8 audiotype, u8 *tts_text)
{
    return DDI_ENODEV;
}

s32 dev_audio_set_volume(u8 volume)
{
    return DDI_ENODEV;
}

u8 dev_audio_get_volume(void)
{
    return DDI_ENODEV;
}
#endif
