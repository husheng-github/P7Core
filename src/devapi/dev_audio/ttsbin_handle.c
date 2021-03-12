#include "devglobal.h"

static tts_bin_instance_t *g_tts_bin_instance = NULL;

static const unsigned char crc_high_tatle[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40
};

static const unsigned char crc_low_tatle[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
    0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
    0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
    0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
    0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
    0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
    0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 
    0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
    0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
    0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 
    0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
    0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
    0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
    0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
    0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
    0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
    0x41, 0x81, 0x80, 0x40
};

/*---------------------------------- Function declare ------------------------*/
static unsigned short _cal_crc16(unsigned short oldcrc, unsigned char *buff, unsigned int len)
{
    unsigned char crc_h = oldcrc >> 8;
    unsigned char crc_l = oldcrc & 0xFF;
    unsigned int i;
    
    while( len-- ){
        i = crc_l ^ *buff++;
        crc_l = (unsigned char)(crc_h ^ crc_high_tatle[i]);
        crc_h = crc_low_tatle[i];
    }
    
    return (unsigned short)(crc_h << 8 | crc_l);
}

/**
 * @brief 检查tts.bin文件的合法性
 * @retval  TRUE  合法
 * @retval  FALSE_TRENDIT 非法
 */
static s32 _ttsbin_crc_check()
{
    s32 ret = FALSE_TRENDIT;
    u32 offset = 0;
    u32 total_data_len = 0;
    u32 read_len = 0;
    u16 tmp_crc = 0xFFFF;
    u16 file_crc = 0;
    u8 buf_data_len[4] = {0};
    u8 buf_file_crc[2] = {0};
    u8 buf[READ_ONCE_BYTES] = {0};

    do{
        read_len = fs_read_file(TTS_SOURCE_PATH, buf_data_len, sizeof(buf_data_len), offset);
        offset += sizeof(buf_data_len);
        if(read_len != sizeof(buf_data_len))
        {
            TRACE_ERR("open fail fail:%s", TTS_SOURCE_PATH);
            ret = FALSE_TRENDIT;
            break;
        }

        //检查长度
        total_data_len = (buf_data_len[0]<<24) + (buf_data_len[1]<<16) + (buf_data_len[2]<<8) + buf_data_len[3];
        if(total_data_len > fs_access_file(TTS_SOURCE_PATH))
        {
            TRACE_ERR("file len is invalid");
            ret = FALSE_TRENDIT;
            break;
        }

        //计算crc，从第四个字节开始计算，长度为total_data_len
        while(1)
        {
            if(total_data_len < READ_ONCE_BYTES)
            {
                read_len = fs_read_file(TTS_SOURCE_PATH, buf, total_data_len, offset);
                tmp_crc = _cal_crc16(tmp_crc, buf, read_len);
                offset += read_len;
                break;
            }
            else
            {
                read_len = fs_read_file(TTS_SOURCE_PATH, buf, READ_ONCE_BYTES, offset);
                offset += read_len;
                total_data_len -= read_len;
                tmp_crc = _cal_crc16(tmp_crc, buf, read_len);
            }
            
            if(total_data_len == 0)
            {
                break;
            }
        }

        fs_read_file(TTS_SOURCE_PATH, buf_file_crc, sizeof(buf_file_crc), offset);
        file_crc = (buf_file_crc[0] << 8) + buf_file_crc[1];
        ret = tmp_crc == file_crc;

        if(TRUE_TRENDIT == ret)
        {
            g_tts_bin_instance->m_crc = file_crc;
        }
    }while(0);
    
    return ret;
}

/**
 * @brief 获取tts.bin里对应索引文件的长度
 * @param[in] index:文件索引
 * @retval  >0长度
 * @retval  <0失败
 */
static s32 _get_audio_filelen(s32 index)
{
    s32 ret = DDI_ERR;
    
    do{
        if(index >= g_tts_bin_instance->m_num || index < 0)
        {
            TRACE_ERR("index is invalid");
            ret = DDI_EINVAL;
            break;
        }

        ret = g_tts_bin_instance->m_audio_file_node[index].m_audio_data_len;
    }while(0);

    return ret;
}

/**
 * @brief 将ttsbin解析的内容打印
 * @param[in] 无
 * @retval  无
 */
static void ttsbin_info_print(void)
{
    s32 i = 0;
    
    for(i=0; i<g_tts_bin_instance->m_num; i++)
    {
        TRACE_INFO("%d:text:%s, len:%d, startaddr:%d", i, 
                                                       g_tts_bin_instance->m_audio_file_node[i].m_text,
                                                       g_tts_bin_instance->m_audio_file_node[i].m_audio_data_len,
                                                       g_tts_bin_instance->m_audio_file_node[i].m_file_start_addr);
    }
}

/**
 * @brief 加载时的操作，分配ttsbin占用的资源，并将文件内容解析
 * @param[in] 无
 * @retval  DDI_OK  成功
 * @retval  <0 失败
 */
static s32 ttsbin_init(void)
{
    s32 ret = DDI_ERR;
    u32 total_data_len = 0;
    u32 total_tts_num = 0;
    u32 cur_tts_num = 0;
    u32 tmp_text_len = 0;
    u32 tmp_audio_len = 0;
    u8  tmp_buf[8] = {0};
    u32 file_offset = 0;

    do{
        //校验tts.source文件crc
        if(!_ttsbin_crc_check())
        {
            TRACE_ERR("check_tts_source_crc err");
            ret = DDI_ERR;
            break;
        }

        //数据总长度4个字节+总音频文件个数4字节
        fs_read_file(TTS_SOURCE_PATH, tmp_buf, sizeof(tmp_buf), 0);
        total_data_len = (tmp_buf[0] << 24) + (tmp_buf[1] << 16) + (tmp_buf[2] << 8) + tmp_buf[3];
        total_tts_num = (tmp_buf[4] << 24) + (tmp_buf[5] << 16) + (tmp_buf[6] << 8) + tmp_buf[7];
        file_offset += sizeof(tmp_buf);
        TRACE_INFO("total_data_len = %d total_tts_size = %d", total_data_len, total_tts_num);

        g_tts_bin_instance->m_audio_file_node = k_malloc(sizeof(str_audio_file_node) * total_tts_num);
        if(!g_tts_bin_instance->m_audio_file_node)
        {
            ret = DDI_EOVERFLOW;
            TRACE_ERR("malloc fail");
            break;
        }

        g_tts_bin_instance->m_num = total_tts_num;
        while(cur_tts_num < total_tts_num)
        {
            //4个字节表示字符长度+4个字节表示音频数据长度
            fs_read_file(TTS_SOURCE_PATH, tmp_buf, sizeof(tmp_buf), file_offset);
            tmp_text_len = (tmp_buf[0] << 24) + (tmp_buf[1] << 16) + (tmp_buf[2] << 8) + tmp_buf[3];
            tmp_audio_len = (tmp_buf[4] << 24) + (tmp_buf[5] << 16) + (tmp_buf[6] << 8) + tmp_buf[7];
            file_offset +=  sizeof(tmp_buf);
            
            //读取字符串拷到表中
            g_tts_bin_instance->m_audio_file_node[cur_tts_num].m_text = k_malloc(tmp_text_len+1);
            memset(g_tts_bin_instance->m_audio_file_node[cur_tts_num].m_text, 0x00, tmp_text_len+1);
            fs_read_file(TTS_SOURCE_PATH, g_tts_bin_instance->m_audio_file_node[cur_tts_num].m_text, tmp_text_len, file_offset);
            
            //字符串长度
            file_offset += tmp_text_len;
            g_tts_bin_instance->m_audio_file_node[cur_tts_num].m_audio_data_len = tmp_audio_len;
            g_tts_bin_instance->m_audio_file_node[cur_tts_num].m_file_start_addr = file_offset;
            //音频数据长度
            file_offset += tmp_audio_len;
            //sort_audio_type(g_audio_file_nodes[cur_tts_num], cur_tts_num);
            cur_tts_num ++;
        }
        ret = DDI_OK;
    }while(0);

    return ret;
}

/**
 * @brief 退出时的操作，释放ttsbin占用的资源
 * @param[in] 无
 * @retval  DDI_OK  成功
 * @retval  <0 失败
 */
static void ttsbin_exit(void)
{
    s32 i = 0;
    
    for(i=0; i<g_tts_bin_instance->m_num; i++)
    {
        if(g_tts_bin_instance->m_audio_file_node[i].m_text)
        {
            k_free(g_tts_bin_instance->m_audio_file_node[i].m_text);
            g_tts_bin_instance->m_audio_file_node[i].m_text = NULL;
        }
    }

    if(g_tts_bin_instance->m_audio_file_node)
    {
        k_free(g_tts_bin_instance->m_audio_file_node);
        g_tts_bin_instance->m_audio_file_node = NULL;
    }

    if(g_tts_bin_instance)
    {
        k_free(g_tts_bin_instance);
        g_tts_bin_instance = NULL;
    }
}

/**
 * @brief 获取tts.bin里对应索引文件的内容
 * @param[in] index:文件索引
 * @param[in] data_buff:用来填充文件内容的缓存
 * @param[in] data_buff_len:用来填充文件内容的缓存长度
 * @param[in] file_offset:文件偏移
 * @retval  >0文件内容长度
 * @retval  <0失败
 */
static s32 ttsbin_get_audio_filedata(s32 index, u8 *data_buff, s32 data_buff_len, s32 file_offset)
{
    s32 ret = DDI_ERR;
    
    do{
        if(data_buff_len < 0 || NULL == data_buff)
        {
            TRACE_ERR("index is invalid");
            ret = DDI_EINVAL;
            break;
        }

        ret = fs_read_file(TTS_SOURCE_PATH, data_buff, data_buff_len, g_tts_bin_instance->m_audio_file_node[index].m_file_start_addr+file_offset);
    }while(0);

    return ret;
}

/**
 * @brief 获取tts.bin里对应索引文件的内容
 * @note 外部传入的file_data是空指针，由接口内部分配内存，外部使用完需释放此内存
 * @param[in] index:文件索引
 * @param[in] file_data:用来填充文件内容的二级指针
 * @retval  >0文件内容长度
 * @retval  <0失败
 */
static s32 ttsbin_get_audio_filelen(s32 index)
{
    return _get_audio_filelen(index);
}

/**
 * @brief 获取tts.bin里对应索引文件的text
 * @param[in] index:文件索引
 * @retval  文件text
 */
static s8* ttsbin_get_audio_filetext(s32 index)
{
    s8 *pstr = NULL;
    
    do{
        if(index >= g_tts_bin_instance->m_num || index < 0)
        {
            TRACE_ERR("index is invalid");
            break;
        }

        pstr = g_tts_bin_instance->m_audio_file_node[index].m_text;
    }while(0);

    return pstr;
}

/**
 * @brief 解析播放的语音，从mp3文件里找到播放的索引
 * @param[in] audio_string:要播放的语音字符串
 * @param[in] index_tab:外部传入的存放索引的tab
 * @param[in out] index_tab_num:in----index_tab的个数
                                out----填入的索引个数
 * @retval  DDI_OK  要播放的语音都在mp3音源列表里
 * @retval  DDI_EINVAL  参数错误
 * @retval  DDI_ERR  要播放的语音未完全在mp3音源列表里
 */
static s32 ttsbin_get_audiostring_index(s8 *audio_string, u16 *index_tab, s32 *index_tab_num)
{
    s32 ret = DDI_ERR;
    s32 i = 0, j = 0;
    s32 len = 0;
    
    do{
        if(!audio_string || !index_tab || 0 == *index_tab_num)
        {
            ret = DDI_EINVAL;
            TRACE_ERR("index is invalid");
            break;
        }

        for(i=0; i<g_tts_bin_instance->m_num; )
        {
            //TRACE_DBG("%s,%d", g_tts_bin_instance->m_audio_file_node[i].m_text, strlen(g_tts_bin_instance->m_audio_file_node[i].m_text));
            if(0 == memcmp(g_tts_bin_instance->m_audio_file_node[i].m_text, audio_string+len, strlen(g_tts_bin_instance->m_audio_file_node[i].m_text)))
            {
                index_tab[j++] = i;
                len += strlen(g_tts_bin_instance->m_audio_file_node[i].m_text);
                TRACE_DBG("find %s", g_tts_bin_instance->m_audio_file_node[i].m_text);
                if(len == strlen(audio_string))
                {
                    break;
                }

                if(j >= *index_tab_num)
                {
                    TRACE_INFO("reach the tab end");
                    break;
                }

                i = 0;  //从头开始找下一个
            }
            else
            {
                i++;
            }
        }

        if(len == strlen(audio_string))
        {
            *index_tab_num = j;
            ret = DDI_OK;
        }
        else
        {
            TRACE_INFO("can't match all play string:%s", audio_string+len);
            ret = DDI_ERR;
            break;
        }
    }while(0);

    return ret;
}

/**
 * @brief 获取ttsbin文件crc
 * @param[in] 无
 * @retval  crc
 */
static s32 ttsbin_get_crc(void)
{
    if(-1 == g_tts_bin_instance->m_crc)
    {
        _ttsbin_crc_check();
    }
    
    return g_tts_bin_instance->m_crc;
}

tts_bin_instance_t *ttsbin_instance(void)
{
    do{
        if(g_tts_bin_instance)
        {
            break;
        }

        g_tts_bin_instance = (tts_bin_instance_t *)k_malloc(sizeof(tts_bin_instance_t));
        if(!g_tts_bin_instance)
        {
            TRACE_ERR("malloc fail");
            break;
        }

        memset(g_tts_bin_instance, 0, sizeof(tts_bin_instance_t));
        g_tts_bin_instance->m_crc = -1;
        g_tts_bin_instance->init = ttsbin_init;
        g_tts_bin_instance->exit = ttsbin_exit;
        g_tts_bin_instance->info_print = ttsbin_info_print;
        g_tts_bin_instance->get_audio_filedata = ttsbin_get_audio_filedata;
        g_tts_bin_instance->get_audio_filelen = ttsbin_get_audio_filelen;
        g_tts_bin_instance->get_audio_filetext = ttsbin_get_audio_filetext;
        g_tts_bin_instance->get_audiostring_index = ttsbin_get_audiostring_index;
        g_tts_bin_instance->get_crc = ttsbin_get_crc;
    }while(0);
        
    return g_tts_bin_instance;
}

