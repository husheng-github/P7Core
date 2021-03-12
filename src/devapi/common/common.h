#ifndef __COMMON_H
#define __COMMON_H

typedef enum{
    LOG_FATAL=1,
    LOG_ERROR=2,
    LOG_INFO=3,
    LOG_DEBUG=4
}PRINT_LEVEL;

typedef union 
{
    u16 num;
    u8  c[2];
}u16_u8_union;

typedef union 
{
    u32 num;
    u8 c[4];
} u32_u8_union;

#define TRACE_DBG(arg...)  debug_Print(LOG_DEBUG,"CORE",__FILE__,__LINE__,__FUNCTION__, ##arg);
#define TRACE_INFO(arg...) debug_Print(LOG_INFO,"CORE",__FILE__,__LINE__,__FUNCTION__, ##arg);
#define TRACE_ERR(arg...)  debug_Print(LOG_ERROR,"CORE",__FILE__,__LINE__,__FUNCTION__, ##arg);
#define TRACE_DBG_HEX(buff, buff_len)  debug_Print_hex(LOG_DEBUG,"CORE",__FILE__,__LINE__,__FUNCTION__, buff, buff_len);
#define TRACE_INFO_HEX(buff, buff_len) debug_Print_hex(LOG_INFO,"CORE",__FILE__,__LINE__,__FUNCTION__, buff, buff_len);
#define TRACE_ERR_HEX(buff, buff_len)  debug_Print_hex(LOG_ERROR,"CORE",__FILE__,__LINE__,__FUNCTION__, buff, buff_len);

void trendit_get_sys_time(s8 *m_time, s32 len);
void debug_Print(PRINT_LEVEL print_level, char *infor, char *file, unsigned short line, const char *func, const char *fmt, ...);
s32 trendit_pack_u8(u8 *buf, u8 u);
s32 trendit_pack_u16(u8 *buf, u16 u);
s32 trendit_pack_u32(u8 *buf, u32 u);
s32 trendit_unpack_u8(u8 *dest, u8 * buf);
s32 trendit_unpack_u16(u16 *dest, u8 * buf);
s32 trendit_unpack_u32(u32 *dest, u8 *buf);
s32 trendit_atoi(const s8 *str);
s32 trendit_get_audio_status(void);
s32 trendit_isdigit(s8 *buff);
s32 trendit_checkip_valid(s8 *ip_addr);
s32 trendit_bcd_to_u16(u16 *pusDest, const u8 *pbcSrc, s32 siSrclen);
s32 trendit_cdc_output_switch(s32 onoff);


#endif

