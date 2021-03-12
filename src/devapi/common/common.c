#include "devglobal.h"
#include "ddi_com.h"

#ifndef TRENDIT_BOOT

static s32 g_cdc_output_switch = 0;

/**
 * @brief 获取系统时间到字符串
 * @param[in] m_time 用来填充的字符串
 * @param[in] len 字符串长度
 * @retval  无
 */
void trendit_get_sys_time(s8 *m_time, s32 len)
{
    u8 lpTime[7] = {0};
    
    ddi_misc_get_time(lpTime);
    snprintf(m_time, len, "%02x%02x%02x %02x:%02x:%02x", lpTime[0], lpTime[1], lpTime[2], lpTime[3], lpTime[4], lpTime[5]);
}

/**
 * @brief CDC口日志输出开关
 * @param[in] onoff：TRUE:输出  FALSE:关闭
 * @retval  无
 */
s32 trendit_cdc_output_switch(s32 onoff)
{
    g_cdc_output_switch = onoff;
    return DDI_OK;
}

/**
 * @brief 格式化输出日志到控制台和文件，由TRACE_DBG、TRACE_INF、TRACE_ERR接口调用
 * @param[in] print_level 打印级别
 * @param[in] infor 指定字符串，目前固定为LOG
 * @param[in] file 代码所在的文件名
 * @param[in] line 代码所在的行
 * @param[in] func 代码所在的函数名
 * @param[in] fmt  格式
 * @retval  无
 */
void debug_Print(PRINT_LEVEL print_level, char *infor, char *file, unsigned short line, const char *func, const char *fmt, ...)
{
    va_list ap;
    s8 buffer[2204] = "";
    u16 len = 0;
    s8 time_buff[128] = {0};

    if(NULL == infor || NULL == file || NULL == func || NULL == fmt)
    {
        return;
    }

    memset(time_buff, 0, sizeof(time_buff));
    memset(buffer, 0, sizeof(buffer));
    trendit_get_sys_time(time_buff, sizeof(time_buff));

    switch(print_level)
    {
        case LOG_FATAL:
            snprintf((char *)(buffer+len), sizeof(buffer) - len, "%s-[%s][%s][%4d][%s][%s()][%d ms]:",
                                                  time_buff, "fatal", infor, line, dev_maths_get_basename(file), func, dev_user_gettimeID());
            break;

        case LOG_ERROR:
            snprintf((char *)(buffer+len), sizeof(buffer) - len, "%s-[%s][%s][%4d][%s][%s()][%d ms]:",
                                                  time_buff, "error", infor, line, dev_maths_get_basename(file), func, dev_user_gettimeID());
            break;

        case LOG_INFO:
            snprintf((char *)(buffer+len), sizeof(buffer) - len, "%s-[%s][%s][%4d][%s][%s()][%d ms]:",
                                                  time_buff, "info", infor, line, dev_maths_get_basename(file), func, dev_user_gettimeID());
            break;

        case LOG_DEBUG:
            snprintf((char *)(buffer+len), sizeof(buffer) - len, "%s-[%s][%s][%4d][%s][%s()][%d ms]:",
                                                  time_buff, "debug", infor, line, dev_maths_get_basename(file), func, dev_user_gettimeID());
            break;
    }
    
    len = strlen(buffer);
    va_start(ap, fmt);
    vsnprintf((char *)(buffer+len), sizeof(buffer) - len, fmt, ap);
    va_end(ap);
    
    len = strlen(buffer);
    snprintf((char *)(buffer+len), sizeof(buffer)-len,"\r\n");

#if 0
    if(g_print_level >= print_level)
    {
        write_dat(buffer, ANALYSIS_LOG_PATH);
    }
#else
    if(g_cdc_output_switch)
    {
        ddi_com_write(CDC_PORT, (u8 *)buffer, strlen(buffer));
        ddi_com_tcdrain(CDC_PORT);
    }
#endif
    return;
}

/**
 * @brief 格式化输出日志到控制台和文件，由TRACE_DBG、TRACE_INF、TRACE_ERR接口调用
 * @param[in] print_level 打印级别
 * @param[in] infor 指定字符串，目前固定为LOG
 * @param[in] file 代码所在的文件名
 * @param[in] line 代码所在的行
 * @param[in] func 代码所在的函数名
 * @param[in] fmt  格式
 * @retval  无
 */
void debug_Print_hex(PRINT_LEVEL print_level, char *infor, char *file, unsigned short line, const char *func, const u8 *buff, s32 buff_len)
{
    s8 tmp_buff[128] = {0};
    s32 i = 0;

    if(NULL == infor || NULL == file || NULL == func || NULL == buff)
    {
        return;
    }

    memset(tmp_buff, 0, sizeof(tmp_buff));
    for(i=0; i<buff_len; i++)
    {
        snprintf(tmp_buff+strlen(tmp_buff), sizeof(tmp_buff)-strlen(tmp_buff), "%02x ", buff[i]);
        if(0 == (i+1)%16)
        {
            debug_Print(print_level, infor, file, line, func, "%s", tmp_buff);
            memset(tmp_buff, 0, sizeof(tmp_buff));
        }
    }

    if(strlen(tmp_buff))
    {
        debug_Print(print_level, infor, file, line, func, "%s", tmp_buff);
    }
    
    return;
}

/**
 * @brief 将无符号char变为字符串
 * @param[in] buf 转换后的填充buf
 * @param[in] u  需转换的数据
 * @retval  1
 */
s32 trendit_pack_u8(u8 *buf, u8 u)
{
    buf[0] = u;
    
    return 1;
}

/**
 * @brief 将无符号short数据变为字符串
 * @param[in] buf 转换后的填充buf
 * @param[in] u  需转换的数据
 * @retval  2
 */
s32 trendit_pack_u16(u8 *buf, u16 u)
{
    u16_u8_union tmp;
    s32 i = 0;
    
    tmp.num = u;
    
#ifdef USING_NETWORK_BYTE_ORDER
    for(i = 0; i < 2; i++) 
    {
        buf[i] = tmp.c[1 - i];
    }
#else
    for(i = 0; i < 2; i++) 
    {
        buf[i] = tmp.c[i];
    }
#endif
    
    return 2;
}

/**
 * @brief 将无符号整形数据变为字符串
 * @param[in] buf 转换后的填充buf
 * @param[in] u  需转换的数据
 * @retval  4
 */
s32 trendit_pack_u32(u8 *buf, u32 u)
{
    u32_u8_union tmp;
    s32 i = 0;   
    tmp.num = u;
#ifdef USING_NETWORK_BYTE_ORDER
    for(i = 0; i < 4; i++) 
    {
        buf[i] = tmp.c[3 - i];
    }
#else
    for(i = 0; i < 4; i++) 
    {
        buf[i] = tmp.c[i];
    }
#endif
    
    return 4;
}

/**
 * @brief 将字符串变无符号char型
 * @param[in] dest 转换后的无符号char型
 * @param[in] buf  需转换的数据
 * @retval  1
 */
s32 trendit_unpack_u8(u8 *dest, u8 * buf)
{
    *dest = *buf;
    return 1;
}

/**
 * @brief 将字符串变无符号short型
 * @param[in] dest 转换后的无符号short型
 * @param[in] buf  需转换的数据
 * @retval  2
 */
s32 trendit_unpack_u16(u16 *dest, u8 * buf)
{
    u16_u8_union tmp;
    uint32_t i = 0;
    
#ifdef USING_NETWORK_BYTE_ORDER 
    for(i = 0; i < 2; i++) 
    {
        tmp.c[1 - i] = buf[i];
    }
#else
    for(i = 0; i < 2; i++) 
    {
        tmp.c[i] = buf[i];
    }
#endif

	*dest = tmp.num;
    return 2;
}

/**
 * @brief 将字符串变无符号整形
 * @param[in] dest 转换后的整形数据
 * @param[in] buf  需转换的数据
 * @retval  4
 */
s32 trendit_unpack_u32(u32 *dest, u8 *buf)
{
    u32_u8_union tmp;
    uint32_t i = 0;
    
#ifdef USING_NETWORK_BYTE_ORDER 
    for(i = 0; i < 4; i++) 
    {
        tmp.c[3 - i] = buf[i];
    }
#else
    for(i = 0; i < 4; i++) 
    {
        tmp.c[i] = buf[i];
    }
#endif
    *dest = tmp.num;
    return 4;
}
 
/**
 * @brief 字符串转为整形
 * @param[in] str 需转换的字符串
 * @retval  整形数据
 */
s32 trendit_atoi(const s8 *str)
{
    s32 s = 0; 
    s32 flag = 0;

    while(*str==' ') 
    {  
        str++; 
    }
    
    if(*str=='-'||*str=='+') 
    {  
        if(*str == '-')
        {
            flag = 1;
        }
        str++; 
    }
    
    while(*str >= '0' && *str <= '9')
    {
        s = s*10 + *str - '0';
        str++;
        
        if(s < 0)
        {   
            s=2147483647;
            break;
        }
    }
    
    return s*(flag?-1:1);
}

/**
 * @brief 判断字符串是否为整形
 * @param[in] buff 判断的字符串
 * @retval  DDI_OK 整形
 * @retval  DDI_ERR 不是整形
 */
s32 trendit_isdigit(s8 *buff)
{
    s32 ret = DDI_OK;
    s8 *pstr = buff;

    if(NULL == buff)
    {
        return ret;
    }
    
    while(pstr)
    {
        if(0 == *pstr)
        {
            break;
        }
        
        if(*pstr < '0' || *pstr > '9')
        {
            ret = DDI_ERR;
            break;
        }

        pstr++;
    }

    return ret;
}

/**
 * @brief 判断ip地址是否合法
 * @param[in] buff 判断的字符串
 * @retval  DDI_OK 整形
 * @retval  DDI_ERR 不是整形
 */
s32 trendit_checkip_valid(s8 *ip_addr)
{
    s8 dst[5][SPLIT_PER_MAX] = {0};
    s32 ret = DDI_ERR;
    s32 i = 0;

    do{
        memset(dst, 0, sizeof(dst));
        ret = dev_maths_split(dst, 5, ip_addr, ".", SPLIT_NOIGNOR);
        if(4 != ret)
        {
            TRACE_ERR("invalid ip:%s", ip_addr);
            ret = DDI_ERR;
            break;
        }

        ret = DDI_OK;
        for(i=0; i<4; i++)
        {
            if(DDI_OK != trendit_isdigit(dst[i]))
            {
                TRACE_ERR("invalid ip:%s,%s", ip_addr, dst[i]);
                ret = DDI_ERR;
                break;
            }

            if(trendit_atoi(dst[i]) < 0 || trendit_atoi(dst[i]) > 255)
            {
                TRACE_ERR("invalid range:%s,%s", ip_addr, dst[i]);
                ret = DDI_ERR;
                break;
            }
        }
    }while(0);

    return ret;
}

/**
 * @brief 将ascii码转换为BCD码，长度为奇数个末尾补0x00
 * @param[in] buff 判断的字符串
 * @retval  正确则返回BCD长度。
 * @retval  DDI_EINVAL 
 */
s32 trendit_asc_to_bcd(u8 *dest_buff, const u8 *src_buff, s32 src_len)
{
    s32 i = 0;
    u8 high = 0, low = 0;
    s32 ret = DDI_ERR;

    do{
        if((NULL == src_buff) || (NULL == dest_buff) || (src_len < 0))
        {
            ret = DDI_EINVAL;
            break;
        }

        for(i = 0; i < src_len; i++)
        {
            if((src_buff[i] < '0') || ((src_buff[i] > '9') && (src_buff[i] < 'A')) || ((src_buff[i] > 'F') && (src_buff[i] < 'a')) || (src_buff[i] > 'f'))
            {
                ret = DDI_EINVAL;
                break;
            }
        }

        if(DDI_EINVAL == ret)
        {
            break;
        }

        for(i = 0; i < src_len; i++)
        {
            //待转bcd码高Nibble
            if((*(src_buff + i) >= 0x61) && (*(src_buff + i) <= 0x66))          //range a~f
            {
                high = *(src_buff + i) - 0x57;
            }
            else if((*(src_buff + i) >= 0x41) && (*(src_buff + i) <= 0x46))      //range A~F
            {
                high = *(src_buff + i) - 0x37;
            }
            else if((*(src_buff + i) >= 0x30) && (*(src_buff + i) <= 0x39))      //range 0~9
            {
                high = *(src_buff + i) - 0x30;
            }
            else if(*(src_buff + i) == '=')  
            {
                high = 0x0d;
            }
            else
            {
                high = 0x00;                                        //其他
            }
            //待转bcd码低Nibble
            i++;

            if(i < src_len)
            {
                if((*(src_buff + i) >= 0x61) && (*(src_buff + i) <= 0x66))        //range a~f
                {
                    low = *(src_buff + i) - 0x57;
                }
                else if((*(src_buff + i) >= 0x41) && (*(src_buff + i) <= 0x46)) //range A~F
                {
                    low = *(src_buff + i) - 0x37;
                }
                else if((*(src_buff + i) >= 0x30) && (*(src_buff + i) <= 0x39))      //range 0~9
                {
                    low = *(src_buff + i) - 0x30;
                }
                else if(*(src_buff + i) == '=')  
                {
                    low = 0x0d;
                }
                else
                {
                    low = 0x00;                                            //其他
                }
            }
            else
            {
                i--;                                                    //预防255个时溢出出错
                low = 0x0F;                                            //如果是奇数个末尾补0x00 0x0F
            }
            *(dest_buff + i / 2) = (high << 4) | low;                      //合并BCD码
        }

        ret = (src_len + 1) / 2;
    }while(0);

    return ret;
}

/**
 * @brief 将BCD码转换为ascii码, dest_buff长度至少是src_buff2倍
 * @param[in] dest_buff 转换后的字符串
 * @param[in] src_buff 原16进制字符串
 * @param[in] src_len 原16进制字符串长度
 * @retval  正确则返回BCD长度。
 * @retval  DDI_EINVAL 
 */
s32 trendit_bcd_to_asc(u8 *dest_buff, const u8 *src_buff, s32 src_len)
{
    s32 i = 0;
    s32 ret = DDI_ERR;

    do{
        if((NULL == src_buff) || (NULL == dest_buff) || (src_len < 0))
        {
            ret = DDI_EINVAL;
            break;
        }

        for(i = 0; i < src_len; i++)
        {
            //高Nibble转换
            if(((*(src_buff + i) & 0xF0) >> 4) <= 9)
            {
                *(dest_buff + 2 * i) = ((*(src_buff + i) >> 4) & 0X0F) + 0x30;
            }
            else
            {
                *(dest_buff + 2 * i)  = ((*(src_buff + i) >> 4) & 0x0F) + 0x37;   //大写A~F
            }

            //低Nibble转换
            if((*(src_buff + i) & 0x0F) <= 9)
            {
                *(dest_buff + 2 * i + 1) = (*(src_buff + i) & 0x0F) + 0x30;
            }
            else
            {
                *(dest_buff + 2 * i + 1) = (*(src_buff + i) & 0x0F) + 0x37;   //大写A~F
            }
        }

        ret = src_len * 2;
    }while(0);

    return ret;
}

s32 trendit_bcd_to_u8(u8 *pucDest, const u8 *pbcSrc, s32 siSrclen)
{
    s32 i = 0;
    u32 sum = 0;

    if((NULL == pbcSrc) || (NULL == pucDest) || (siSrclen < 0))
    {
        return DDI_EINVAL;
    }

    for(i = 0; i < siSrclen; i++)
    {
        sum *= 100;
        sum += 10 * ((*(pbcSrc + i) & 0xF0) >> 4) + (*(pbcSrc + i) & 0x0F);
    }

    *pucDest = (u8)sum;

    if(sum > 255)
    {
        //  Assert(0);// 2013.01.05 14:36
        return DDI_EINVAL;
    }
    return DDI_OK;
}

s32 tolower(s32 c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c + 'a' - 'A';
    }
    else
    {
        return c;
    }
}

s32 trendit_bcd_to_u16(u16 *dest, const u8 *src, s32 src_len)
{
    s32 i = 0;
    s32 tmp = 0;
    s32 ret = DDI_ERR;

    do{
        if(NULL == dest || NULL == src || 0 == src_len)
        {
            ret = DDI_EINVAL;
            break;
        }

        ret = DDI_OK;
        while(i < src_len)
        {
            tmp = tolower(src[i++]);
            if((tmp >= '0' && tmp <= '9') || (tmp >= 'a' && tmp <= 'z'))
            {
                if(tmp > '9')
                {
                    *dest = 16 * (*dest) + (10 + tmp - 'a');
                }
                else
                {
                    *dest = 16 * (*dest) + (tmp - '0');
                }
            }
            else
            {
                ret = DDI_EINVAL;
                break;
            }
        }
    }while(0);
    
    return ret;
}

/**
 * @brief 判断文件是否存在
 * @param[in] filename 文件名
 * @retval  DDI_OK 存在
 * @retval  DDI_ERR 不存在
 */
s32 trendit_file_access(s8 *filename)
{
    s32 fd = vfs_open(filename, "r");
    s32 ret = DDI_ERR;

    if(fd > 0)
    {
        vfs_close(fd);
        ret = DDI_OK;
    }

    return ret;
}

#else
void debug_Print(PRINT_LEVEL print_level, char *infor, char *file, unsigned short line, const char *func, const char *fmt, ...)
{
}

s32 trendit_atoi(const s8 *str)
{
}

#endif


