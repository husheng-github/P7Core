#ifndef __DEV_MATHS_H
#define __DEV_MATHS_H

#define MODULUS_BIN 2     //二进制
#define MODULUS_OCT 8     //八进制
#define MODULUS_DEC 10    //十进制
#define MODULUS_HEX 16    //十六进制




u8 dev_maths_getlrc(u8 lrcini, const u8 *pdata, u16 len);
u32 dev_maths_str_len(const u8 *s);
u8 *dev_maths_str_cpy(u8 *dest, const u8 *src);
s32 dev_maths_str_cmp(const u8 *cs, const u8 *ct);
u8 *dev_maths_mem_set(u8 *s, u8 c, u32 count);
u8 *dev_maths_mem_cpy(u8 *dest, const u8 *src, u32 count);
s32 dev_maths_mem_cmp(const u8 *cs, const u8 *ct, u32 count);
void dev_maths_u8_to_bcd(u8 *Bcd, u8 const Src,u32 Len);
void dev_maths_bcd_to_u8(u8 *Dest,u8 *Bcd,u32 Len);
void dev_maths_u16_to_bcd(u8 *Bcd, u16 const Src,u32 Len);
void dev_maths_bcd_to_u16(u16 *Dest,u8 *Bcd,u32 Len);
void dev_maths_hex_to_u16(u16 *Dest,u8 *Hex,u32 Len);
void dev_maths_u16_to_hex(u8 *Hex, u16 const Src,u32 Len);
void dev_maths_hex_to_u32(u32 *Dest,u8 *Hex,u32 Len);
void dev_maths_u32_to_hex(u8 *Hex, u32 const Src,u32 Len);
void dev_maths_u32_to_bcd(u8 *Bcd, u32 const Src,u32 Len);
void dev_maths_bcd_to_u32(u32 *Dest,u8 *Bcd,u32 Len);
void dev_maths_bcd_to_asc(u8 *Dest,u8 *Src,u32 Len);
void dev_maths_asc_to_bcd(u8 *Dest,u8 *Src,u32 Len);
u32 dev_maths_asc_to_u32(s8 *src, u32 srclen, u8 base);
void  dev_maths_bcd_add(u8 *Dest,u8 *Src,u32 Len);
void dev_maths_bcd_sub(u8 *Dest,u8 *Src,u32 Len);
void dev_maths_double_to_one(u8 *Dest,u8 *Src,u8 Len);
u8 dev_maths_calc_bcc(u8 *src,u32 num );
u32 dev_maths_truncate_char(u8 *Src,u32 AvailableChar);
s32 dev_maths_str_pos(u8 *str,u8 chr);
void dev_maths_str_trim_left(s8 *str, s8 del_chr);
void dev_maths_str_trim_right(s8 *str, s8 del_chr);
void dev_maths_str_trim(s8 *str);
void dev_maths_str_trim_special(s8 *str, s8 del_chr);
typedef enum{
    SPLIT_NOIGNOR,          //默认值
    SPLIT_IGNOR_LEN0        //split后长度为0，直接忽略
}SPLIT_FLAG;
#define SPLIT_PER_MAX      64         //TLV value数据每个数据最大长度
s32 dev_maths_split(s8 dst[][SPLIT_PER_MAX], s32 dst_num,s8 *str, const s8* spl, SPLIT_FLAG split_flag);
u16 dev_maths_calc_crc16(u16 crcinit, const u8 *pbyDataIn, u32 dwDataLen, u8 *abyCrcOut);
u16 dev_maths_calc_crc16_MODBUS(u16 crcinit, u8 *src, u32 srclen);
u32 dev_maths_calc_crc32(u32 crc, u8 *szSrc, u32 dwSrcLen);
u32 dev_maths_calc_crc32_little(u32 crc, u8* content, u32 numread);
u32 dev_maths_min(u32 t1, u32 t2);
u32 dev_maths_max(u32 t1, u32 t2);
void dev_maths_caleMD5(u8 *dest, u8 *src, s32 len);
s8 *dev_maths_get_basename(s8 *path_name);
s32 dev_maths_atoi(const char *str);

#endif

