/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:        pengxuebin     
** Created Date:    20170316 
** Version:        
** Description:    该文件处理循环队列操作
****************************************************************************/
#include "devglobal.h"
//#include "md5.h"

/**********************************************
**********************************************/
u8 dev_maths_getlrc(u8 lrcini, const u8 *pdata, u16 len)
{
    u16 i;
    u8 lrc;

    lrc = lrcini;
    for(i=0; i<len; i++)
    {
        lrc ^= pdata[i];
    }
    return lrc;
}
/**********************************************
**********************************************/
static const u16 CRC16TAB[16] = 
{ 
    //CRC-16 x16+x12+x5+1 (ITU) 
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef
};

static const u32 CRC32TAB[256] = 
{
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};


/*******************************************************************
函数名称: u32 str_len(const u8 *s)
函数功能: 字符串长度
入口参数: 
返 回 值: 无
相关调用:
修改信息:         
********************************************************************/
u32 dev_maths_str_len(const u8 *s)
{
    const u8 *sc;

    for (sc = s; *sc != '\0'; ++sc)
        /* nothing */;
    return sc - s;
}

/*******************************************************************
函数名称: u8 *str_cpy(u8 *dest, const u8 *src)
函数功能: 字符串拷贝
入口参数: 
返 回 值: 无
相关调用:
修改信息:         
********************************************************************/
u8 *dev_maths_str_cpy(u8 *dest, const u8 *src)
{
    u8 *tmp = dest;

    while ((*dest++ = *src++) != '\0')
        /* nothing */;
    return tmp;
}

/*******************************************************************
函数名称: u8 *str_copy(u8 *dest, const u8 *src)
函数功能: 字符串比较
入口参数: 
返 回 值: 无
相关调用:
修改信息:         
********************************************************************/
s32 dev_maths_str_cmp(const u8 *cs, const u8 *ct)
{
    u8 c1, c2;

    while (1) {
        c1 = *cs++;
        c2 = *ct++;
        if (c1 != c2)
            return c1 < c2 ? -1 : 1;
        if (!c1)
            break;
    }
    return 0;
}
/*******************************************************************
函数名称: u8 *mem_set(u8 *s, u8 c, u32 count)
函数功能: 内存设置函数
入口参数: 
返 回 值: 无
相关调用:
修改信息:         
********************************************************************/
u8 *dev_maths_mem_set(u8 *s, u8 c, u32 count)
{
    u8 *xs = s;

    while (count--)
        *xs++ = c;
    return s;
}
/*******************************************************************
函数名称: u8 *mem_cpy(u8 *dest, const u8 *src, u32 count)
函数功能: 内存拷贝函数
入口参数: 
返 回 值: 无
相关调用:
修改信息:         
********************************************************************/
u8 *dev_maths_mem_cpy(u8 *dest, const u8 *src, u32 count)
{
    u8 *tmp = dest;
    const u8 *s = src;

    while (count--)
        *tmp++ = *s++;
    return dest;
}
/****************************************************************************
**Description:        内存比较, 避开使用标准库
**Input parameters:   cs: 内存比较地址1
                      ct: 内存比较地址2
                      count: 比较字节数
**Output parameters:  N/A
**Returned value:     内存比较差值
**Modified by:  
****************************************************************************/
s32 dev_maths_mem_cmp(const u8 *cs, const u8 *ct, u32 count)
{
    s32 i = 0;
    const u8 *p1 = cs;
    const u8 *p2 = ct;
    
    while(count != 0)
    {
        count--;
        i = i | (p1[count] ^ p2[count]);
    }
    
    return i;
}

/*******************************************************************
函数名称: void U16ToBcd(u8 *Bcd, u16 const Src,u32 Len)
函数功能: 将一个8位无符号数转换成固定字节右对齐压缩BCD码
入口参数: 1.压缩BCD数组地址;2.原u8数据;3.BCD字节个数
返 回 值: 无
相关调用:
备    注: uClinux 不支持有符号的字符型,却支持有符号整型  
修改信息:         
********************************************************************/
void dev_maths_u8_to_bcd(u8 *Bcd, u8 const Src,u32 Len)
{
    u32 i;
    u8 num = Src;
    for(i = Len; i > 0; i--)
    {
        *(Bcd+i-1) = (((num%100) / 10) << 4) | ((num%100) % 10);
        num /= 100;
    }
}
/*******************************************************************
函数名称: void BcdToU16(u16 *Dest,u8 *Bcd,u32 Len)
函数功能: 将压缩BCD码转换成一个8位无符号数
入口参数: 1.u8数据地址; 2压缩BCD数组地址; 3.BCD字节个数
返 回 值: 无
相关调用:
备    注:   
修改信息:         
********************************************************************/
void dev_maths_bcd_to_u8(u8 *Dest,u8 *Bcd,u32 Len)
{
    u32 i;
    *Dest = 0;
    for(i = 0; i < Len; i++)
    {
        *Dest *= 100;
        *Dest += 10*((*(Bcd+i) & 0xF0) >> 4) + (*(Bcd+i) & 0x0F);
    }
}
/*******************************************************************
函数名称: void U16ToBcd(u8 *Bcd, u16 const Src,u32 Len)
函数功能: 将一个16位无符号数转换成固定字节右对齐压缩BCD码
入口参数: 1.压缩BCD数组地址;2.原u16数据;3.BCD字节个数
返 回 值: 无
相关调用:
备    注: uClinux 不支持有符号的字符型,却支持有符号整型  
修改信息:         
********************************************************************/
void dev_maths_u16_to_bcd(u8 *Bcd, u16 const Src,u32 Len)
{
    u32 i;
    u16 num = Src;
    for(i = Len; i > 0; i--)
    {
        *(Bcd+i-1) = (((num%100) / 10) << 4) | ((num%100) % 10);
        num /= 100;
    }
}
/*******************************************************************
函数名称: void BcdToU16(u16 *Dest,u8 *Bcd,u32 Len)
函数功能: 将压缩BCD码转换成一个16位无符号数
入口参数: 1.u16数据地址; 2压缩BCD数组地址; 3.BCD字节个数
返 回 值: 无
相关调用:
备    注:   
修改信息:         
********************************************************************/
void dev_maths_bcd_to_u16(u16 *Dest,u8 *Bcd,u32 Len)
{
       u32 i;
     *Dest = 0;
     for(i = 0; i < Len; i++)
     {
        *Dest *= 100;
        *Dest += 10*((*(Bcd+i) & 0xF0) >> 4) + (*(Bcd+i) & 0x0F);
     }
}
/*******************************************************************
函数名称: void HexToU16(u16 *Dest,u8 *Bcd,u32 Len)
函数功能: 将HEX数据转换成一个16位无符号数
入口参数: 1.u16数据地址; 2HEX数组地址; 3.Hex字节个数
返 回 值: 
相关调用:
备    注: Hex为高位在前,低位在后的存储顺序  
修改信息:         
********************************************************************/
void dev_maths_hex_to_u16(u16 *Dest,u8 *Hex,u32 Len)
{
       u32 i;
     *Dest = 0;
     
     for(i = 0; i < Len; i++)
     {
        *Dest *= 256;
        *Dest += Hex[i];
     }
}
/*******************************************************************
函数名称: void U16ToHex(u8 *Hex, u16 const Src,u32 Len)
函数功能: 将一个16位无符号数转换成固定字节右对齐HEX
入口参数: 1.HEX数组地址;2.原u16数据;3.HEX字节个数
返 回 值: 无
相关调用:
备    注: Hex为高位在前,低位在后的存储顺序 
修改信息:         
********************************************************************/
void dev_maths_u16_to_hex(u8 *Hex, u16 const Src,u32 Len)
{
    u32 i;
    u16 num = Src;
    
    for(i = Len; i > 0; i--)
    {
        *(Hex+i-1) = num % 256;
        num /= 256;
    }
}
/*******************************************************************
函数名称: void HexToU32(u32 *Dest,u8 *Hex,u32 Len)
函数功能: 将HEX数据转换成一个32位无符号数
入口参数: 1.u32数据地址; 2HEX数组地址; 3.Hex字节个数
返 回 值: 
相关调用:
备    注: Hex为高位在前,低位在后的存储顺序  
修改信息:         
********************************************************************/
void dev_maths_hex_to_u32(u32 *Dest,u8 *Hex,u32 Len)
{
       u32 i;
     *Dest = 0;
     
     for(i = 0; i < Len; i++)
     {
        *Dest *= 256;
        *Dest += Hex[i];
     }
}
/*******************************************************************
函数名称: void U32ToHex(u8 *Hex, u32 const Src,u32 Len)
函数功能: 将一个32位无符号数转换成固定字节右对齐HEX
入口参数: 1.HEX数组地址;2.原u32数据;3.HEX字节个数
返 回 值: 无
相关调用:
备    注: Hex为高位在前,低位在后的存储顺序 
修改信息:         
********************************************************************/
void dev_maths_u32_to_hex(u8 *Hex, u32 const Src,u32 Len)
{
    u32 i;
    u32 num = Src;
    
    for(i = Len; i > 0; i--)
    {
        *(Hex+i-1) = num % 256;
        num /= 256;
    }
}

/*******************************************************************
函数名称: void U32ToBcd(u8 *Bcd, u32 const Src,u32 Len)
函数功能: 将一个32位无符号数转换成右对齐压缩BCD码
入口参数: 1.压缩BCD数组地址;2.原u32数据;3.BCD字节个数
返 回 值: 无
相关调用:
备    注: uClinux 不支持有符号的字符型,却支持有符号整型  
修改信息:         
********************************************************************/
void dev_maths_u32_to_bcd(u8 *Bcd, u32 const Src,u32 Len)
{
    u32  i;
    u32 num = Src;
    for(i = Len; i > 0; i--)
    {
        *(Bcd+i-1) = (((num%100) / 10) << 4) | ((num%100) % 10);
        num /= 100;
    }
}
/*******************************************************************
函数名称: void BcdToU32(u32 *Dest,u8 *Bcd,u32 Len)
函数功能: 将压缩BCD码转换成一个32位无符号数
入口参数: 1.u32数据地址; 2压缩BCD数组地址; 3.BCD字节个数
返 回 值: 无
相关调用:
备    注:   
修改信息:         
********************************************************************/
void dev_maths_bcd_to_u32(u32 *Dest,u8 *Bcd,u32 Len)
{
       u32 i;
     *Dest = 0;
     for(i = 0; i < Len; i++)
     {
        *Dest *= 100;
        *Dest += 10*((*(Bcd+i) & 0xF0) >> 4) + (*(Bcd+i) & 0x0F);
     }
}
/*******************************************************************
函数名称: void BcdToAsc(u8 *Dest,u8 *Src,u32 Len)
函数功能: 将压缩BCD码转换为ascii码
入口参数: 1.ascii码地址; 2.压缩BCD数组地址; 3.Bcd码字节个数
返 回 值: 无
相关调用:
备    注: Dest地址为Len的两倍  
修改信息:         
********************************************************************/
void dev_maths_bcd_to_asc(u8 *Dest,u8 *Src,u32 Len)
{
    u32 i;
    for(i=0;i<Len;i++)
    {
        //高Nibble转换
        if(((*(Src + i) & 0xF0) >> 4) <= 9)
        {
            *(Dest + 2*i) = ((*(Src + i) & 0xF0) >> 4) + 0x30;
        }
        else
        {
            *(Dest + 2*i)  = ((*(Src + i) & 0xF0) >> 4) + 0x37;   //大写A~F
        }    
        //低Nibble转换
        if((*(Src + i) & 0x0F) <= 9)
        {
            *(Dest + 2*i + 1) = (*(Src + i) & 0x0F) + 0x30;
        }
        else
        {
            *(Dest + 2*i + 1) = (*(Src + i) & 0x0F) + 0x37;   //大写A~F
        }    
    }
}
/*******************************************************************
函数名称: void AscToBcd(u8 *Dest,u8 *Src,u32 Len)
函数功能: 将ascii码转换为压缩BCD码
入口参数: 1.压缩bcd数组地址; 2.ascii码地址; 3.ascii字节个数
返 回 值: 无
相关调用:
备    注: 末尾不够补0x00,非ascii码填0x00  
修改信息:       
********************************************************************/
void dev_maths_asc_to_bcd(u8 *Dest,u8 *Src,u32 Len)
{
    u32 i;
    u8 high = 0,low = 0;
    for(i = 0; i < Len; i++) 
    {
        //待转bcd码高Nibble
        if((*(Src + i) >= 0x61) && (*(Src + i) <= 0x66))      //range a~f
        {
            high = *(Src + i) - 0x57;
        }
        else if((*(Src + i) >= 0x41) && (*(Src + i) <= 0x46))  //range A~F
        {
            high = *(Src + i) - 0x37;
        }
        else if((*(Src + i) >= 0x30) && (*(Src + i) <= 0x39))  //range 0~9
        {
            high = *(Src + i) - 0x30;   
        }
        else
        {
            high = 0x00 ;                                       //其他
        }
        
        //待转bcd码低Nibble
        i++;
        if(i < Len)
        {
            if((*(Src + i) >= 0x61) && (*(Src + i) <= 0x66))    //range a~f
            {
                low = *(Src + i) - 0x57;
            }
            else if((*(Src + i) >= 0x41) && (*(Src + i) <= 0x46)) //range A~F
            {   
                low = *(Src + i) - 0x37;
            }
            else if((*(Src + i) >= 0x30) && (*(Src + i) <= 0x39))  //range 0~9
            {
                low = *(Src + i) - 0x30;
            }
            else
            {
                low = 0x00 ;                                       //其他
            }
        }
        else
        {
            i--;                                                //预防255个时溢出出错
            low = 0x00 ;                                       //如果是奇数个末尾补0x00 
        }
        *(Dest + i/2) = (high << 4) | low;                      //合并BCD码
    }
}
/*******************************************************************
函数名称: 
入口参数: 
返 回 值: 无
相关调用:
备    注: 
修改信息:         
********************************************************************/
#if 0
void tolower(s8 *ch)
{   
    if((*ch >= 'A')&&(*ch <= 'Z'))
        *ch += 0x20;
}
#endif
u32 dev_maths_asc_to_u32(s8 *src, u32 srclen, u8 base)
{
    u32 i, j;
    s8 *p;
    u32 sum=0;

    p = src;
    for(i=0; i<srclen; i++)
    {
        //大小写转换
 //       tolower(p);
        if(base == MODULUS_HEX)
        {
            if(((*p>='0') && (*p<='9'))
             ||((*p>='A') && (*p<='F'))
             ||((*p>='a') && (*p<='f'))) 
            {
                
            }
            else
            {
                break;
            }
        }
        else
        {
            if((*p<'0') || (*p>('0'+base-1)))
            {
                break;
            }
        }
        p++;
    }
    //------------------------------
    p = src;  
    for(j=0; j<i; j++)
    {
        if(*p <= '9')
        {
            sum = (sum*base + (*p-'0'));
        }
        else if(*p <= 'F')
        {
            sum = (sum*base + (*p-'A'+10));
        }
        else
        {
            sum = (sum*base + (*p-'a'+10));
        }
        p++;        
    }
    return sum;
}
/*******************************************************************
函数名称: void BcdAdd(u8 *Dest,u8 *Src,u32 Len)
函数功能: BCD码加法运算
入口参数: 1.目源地址; 2.源地址; 3.字节个数
返 回 值: 无
相关调用:
备    注: 
修改信息:         
********************************************************************/
void  dev_maths_bcd_add(u8 *Dest,u8 *Src,u32 Len)
{
    u32 i;
    u8 high,low;
    u8 carry=0;                     //运算进位标志

    for(i=Len;i>0;i--)
    {
         //低nibble的运算
         low = (*(Dest+i-1) & 0x0F) + (*(Src+i-1) & 0x0F) + carry;
         carry=0;
         if(low > 0x09)
         {
            low = low-0x0A; 
            carry = 1;  
         }
         //高nibble的运算 
         high  = ( (*(Dest+i-1) & 0xF0) >> 4) + ((*(Src+i-1) & 0xF0) >> 4) + carry;  
         carry=0;
         if(high > 0x09)
         {
            high= high-0x0A; 
            carry = 1;  
         }  
         *(Dest+i-1) = (high << 4) | low;
    }   
}
/*******************************************************************
函数名称: void BcdSub(u8 *Dest,u8 *Src,u32 Len)
函数功能: BCD码减法运算
入口参数: 1.目源地址; 2.源地址; 3.字节个数
返 回 值: 无
相关调用:
备    注: Len为u8,指名Src长度最大可以到255,当运算溢出时不处理,
          源目的操作数必须位数一样
修改信息:         
********************************************************************/
void dev_maths_bcd_sub(u8 *Dest,u8 *Src,u32 Len)
{
    u32 i;
    u8 high,low;
    u8 carry=0;                     //运算借位标志

    for(i=Len;i>0;i--)
    {
         //低nibble的运算
         if( (*(Dest+i-1) & 0x0F) < ( (*(Src+i-1) & 0x0F) + carry) )
         {
            low = ((*(Dest+i-1) & 0x0F)+0x0A) - (*(Src+i-1) & 0x0F) - carry;
            carry=1;
         }
         else
         {
            low = (*(Dest+i-1) & 0x0F) - (*(Src+i-1) & 0x0F) - carry;
            carry=0;
         }
         
         //高nibble的运算 
         if( ((*(Dest+i-1) & 0xF0) >> 4) < (((*(Src+i-1) & 0xF0) >> 4) + carry) )
         {
            high = (((*(Dest+i-1) & 0xF0) >> 4)+0x0A) - ((*(Src+i-1) & 0xF0) >> 4)  - carry;
            carry=1;
         }
         else 
         {
            high = ((*(Dest+i-1) & 0xF0) >> 4) - ((*(Src+i-1) & 0xF0) >> 4)  - carry;
            carry=0;         
         }   
         *(Dest+i-1) = (high << 4) | low;
    }   
}     
/*******************************************************************
函数名称: void DoubleToOne(u8 *Dest,u8 *Src,u8 Len)
函数功能: 将ascii码转换为右对齐固定长度为Len个字节的BCD码
入口参数: 1.压缩bcd数组地址; 2.ascii码地址; 3.固定长度
返 回 值: 无
相关调用:
备    注: Len的长度为Dest的固定长度.必须小于127.
修改信息:         
********************************************************************/
#if 0
void dev_maths_double_to_one(u8 *Dest,u8 *Src,u8 Len)
{
    u8 i,j,k;
    u8 temp[255];

    mem_set(temp,0,sizeof(temp));        //清缓冲
    Len *=2;
    i=str_len(Src);                      //判断字符个数
    if(i > Len ) i=Len;
    
    if(i == Len)
    {
          asc_to_bcd(Dest,Src,Len);
    }
    else 
    {
       for(j=0;j<(Len-i);j++) temp[j]=0x30;
       for(k=0;k<i;k++,j++)
       {
           temp[j]=*Src;
           Src++;
       }
       asc_to_bcd(Dest,temp,Len);
    }
}  
#endif
/*******************************************************************
函数名称: u8 CalcBcc(u8 *Src,u32 num )
函数功能: 计算数据的累加和
入口参数: 1.原数据; 2.待计算个数
返 回 值: 计算Bcc后的结果值
相关调用:
备    注: 
修改信息:         
********************************************************************/
u8 dev_maths_calc_bcc(u8 *src,u32 num )
{
    u32 i;                                            //临时个数
    u8 bcc = 0;                                        //累加和

    for(i=0; i<num; i++)    
    {
        bcc += src[i];                                //计算校验和
    }
    return bcc;                                        //返回校验和
}
/*******************************************************************
函数名称: u32 TruncateChar(u8 *Src,u32 AvailableChar)
函数功能: 获得指定范围可用字符个数，最大不超过字符串长度
入口参数: Src:源串;AvailableChar:可用字符数
返 回 值: 实际字符串的个数
相关调用: 
备    注: 
修改信息: 
********************************************************************/
#if 0
u32 dev_maths_truncate_char(u8 *Src,u32 AvailableChar)
{
    u32 Len,i;

    Len = str_len(Src);                                        //取原串的长度
    for(i = 0; (i < AvailableChar) && (i < Len); i++)
    {
        if(Src[i] > 0x80)                                    
        {
            i++;                                            //汉字推进
            if((i >= AvailableChar) || (i >= Len))
            {
                if(i >= Len)
                {
                    Src[i-1] = 0;                            //修复源串非法字符
                }
                i --;
                break;
            }
        }
    }
    return i;
}
#endif
/*******************************************************************
函数名称: s32 strpos(u8 *str,u8 chr)
函数功能: 查找指定字符在字符串中的位置
入口参数: 1: 字符串, 2: 待查字符
返 回 值: 返回待查字符下标,如果未找到返回-1.
备    注: 
修改信息:         
********************************************************************/
s32 dev_maths_str_pos(u8 *str,u8 chr)
{
    u32 i = 0;

    while(str[i] != 0 ) 
    {
        if(str[i] == chr)
        {
            return i;
        }
        else
        {
            i++;
        }
    }
    return -1;
}
/*******************************************************************
函数功能: 删除字符串靠左空格
入口参数: 1: 字符串
返 回 值: 
备    注: 
修改信息:         
********************************************************************/
void dev_maths_str_trim_left(s8 *str, s8 del_chr)
{
    u8 flag;
    u32 len,i,j;

    len = strlen(str);
    for(i = 0,j = 0,flag = 0; i < len; i++)
    {
        if(str[i] != del_chr) flag = 1;
        if(flag) str[j++] = str[i];
    }
    str[j] = 0;
}
/*******************************************************************
函数功能: 删除字符串靠右空格
入口参数: 1: 字符串
返 回 值: 
备    注: 
修改信息:         
********************************************************************/
void dev_maths_str_trim_right(s8 *str, s8 del_chr)
{
    u32 len,i;

    len = strlen(str);
    for(i = len; i > 0; i--)
    {
        if(str[i-1] != del_chr) 
        {
            break;
        }
    }
    str[i] = 0;
}
/*******************************************************************
函数功能: 删除字符串两边空格
入口参数: 1: 字符串
返 回 值: 
备    注: 
修改信息:         
********************************************************************/
void dev_maths_str_trim(s8 *str)
{
    dev_maths_str_trim_left(str, ' ');
    dev_maths_str_trim_right(str, ' ');
}

/*******************************************************************
函数功能: 删除字符串两边空格
入口参数: 1: 字符串
返 回 值: 
备    注: 
修改信息:         
********************************************************************/
void dev_maths_str_trim_special(s8 *str, s8 del_chr)
{
    dev_maths_str_trim_left(str, del_chr);
    dev_maths_str_trim_right(str, del_chr);
}
/**
 * @brief 将str字符以spl分割,存于dst中，并返回子字符串数量
 * @param[in] dst 目标存储字符串
 * @param[in] dst_num 缓存个数
 * @param[in] str 需要分离的字符串
 * @param[in] spl 分隔符
 * @param[in] split_flag 长度为0的是否需忽略
 * @retval  
 * @retval  
 */
s32 dev_maths_split(s8 dst[][SPLIT_PER_MAX], s32 dst_num,s8 *str, const s8* spl, SPLIT_FLAG split_flag)
{
    s32 n = 0;
    s8 *result = NULL;
    s32 len = 0;
    s8 *ptmp = str;

    while(1)
    {
        if(n == dst_num)
        {
            break;
        }
        
        result = strstr(ptmp, spl);
        //TRACE_DBG("result:%s, %s", result, ptmp);
        if(NULL == result)
        {   
            if(!(0 == strlen(ptmp) && split_flag == SPLIT_IGNOR_LEN0))
            {
                snprintf(dst[n++], SPLIT_PER_MAX, "%s", ptmp);
            }
            break;
        }

        memset(dst[n], 0, SPLIT_PER_MAX);
        len = result-ptmp;
        len = len > (SPLIT_PER_MAX-1) ? (SPLIT_PER_MAX-1):len;

        if(!(0 == len && split_flag == SPLIT_IGNOR_LEN0))
        {
            memcpy(dst[n++], ptmp, len);
        }
        ptmp = result + strlen(spl);
    }

    return n;
}

/****************************************************************************
**Description:        计算CRC-16值(采用ITU标准，CITT)
**Input parameters:   pbyDataIn: 需计算的数据
**                    dwDataLen: 数据长度
**Output parameters:  abyCrcOut：计算得到的CRC-16值
**Returned value:     无
**Modified by:  
****************************************************************************/
u16 dev_maths_calc_crc16(u16 crcinit, const u8 *pbyDataIn, u32 dwDataLen, u8 *abyCrcOut)
{
    u16 wCrc = crcinit;
    u8 byTemp;

    while(dwDataLen-- != 0)
    {
        byTemp = ((u8)(wCrc>>8))>>4;
        wCrc <<= 4;
        wCrc ^= CRC16TAB[byTemp^(*pbyDataIn/16)];
        byTemp = ((u8)(wCrc>>8))>>4;
        wCrc <<= 4;
        wCrc ^= CRC16TAB[byTemp^(*pbyDataIn&0x0f)];
        pbyDataIn++;
    }

    if(abyCrcOut != NULL)
    {
        abyCrcOut[0] = wCrc/256;
        abyCrcOut[1] = wCrc%256;
    }
    return wCrc;
} 
/****************************************************************************
**Description:        计算CRC-16值(采用ITU标准，CITT,MODBUS(X16+X15+X2+1))
**Input parameters:   pbyDataIn: 需计算的数据
**                    dwDataLen: 数据长度
**Output parameters:  abyCrcOut：计算得到的CRC-16值
**Returned value:     无
**Modified by:  
****************************************************************************/
const u16 poly_tab[2] = {0, 0xa001};
u16 dev_maths_calc_crc16_MODBUS(u16 crcinit, u8 *src, u32 srclen)
{
    u16 crc = crcinit;
    u32 i,j;
    u8 ds;

    for(j=srclen; j>0; j--)
    {
        ds = *src++;
        for(i=0; i<8; i++)
        {
            crc = (crc >> 1) ^ poly_tab[(crc ^ ds ) & 1];
            ds = ds >> 1;
        }
    }
    return crc;
    
}
u16 dev_maths_calcCRC16_buypass(u16 crcinit, const u8 *data, u32 length, u8 *abyCrcOut) 
{ 
    #define POLYNOMIAL 0x8005     
     
    u16 crc = crcinit;//0x0000; 
    u32 j; 
    u8 i; 
    for (j=length; j>0; j--) { 
    crc ^= (u16)(*data++) << 8; 
    for (i=0; i<8; i++) { 
        if (crc & 0x8000) crc = (crc<<1) ^ POLYNOMIAL; 
        else crc <<= 1; 
        } 
    } 
    if(abyCrcOut != NULL)
    {
        abyCrcOut[0] = crc/256;
        abyCrcOut[1] = crc%256;
    }
    return (crc); 
} 
#if 0
//------------------------------------------------------
u16 crc16tab1[] = { 0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0,
    0x0280, 0xC241, 0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1,
    0xC481, 0x0440, 0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1,
    0xCE81, 0x0E40, 0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0,
    0x0880, 0xC841, 0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1,
    0xDA81, 0x1A40, 0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0,
    0x1C80, 0xDC41, 0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0,
    0x1680, 0xD641, 0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1,
    0xD081, 0x1040, 0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1,
    0xF281, 0x3240, 0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0,
    0x3480, 0xF441, 0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0,
    0x3E80, 0xFE41, 0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1,
    0xF881, 0x3840, 0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0,
    0x2A80, 0xEA41, 0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1,
    0xEC81, 0x2C40, 0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1,
    0xE681, 0x2640, 0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0,
    0x2080, 0xE041, 0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1,
    0xA281, 0x6240, 0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0,
    0x6480, 0xA441, 0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0,
    0x6E80, 0xAE41, 0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1,
    0xA881, 0x6840, 0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0,
    0x7A80, 0xBA41, 0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1,
    0xBC81, 0x7C40, 0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1,
    0xB681, 0x7640, 0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0,
    0x7080, 0xB041, 0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0,
    0x5280, 0x9241, 0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1,
    0x9481, 0x5440, 0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1,
    0x9E81, 0x5E40, 0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0,
    0x5880, 0x9841, 0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1,
    0x8A81, 0x4A40, 0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0,
    0x4C80, 0x8C41, 0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0,
    0x4680, 0x8641, 0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1,
    0x8081, 0x4040 };

 static char flipAByte(char dat) 
 {
    char i;
    char v;
    
    v = 0;
    
    for (i = 0; i < 8; ++i) {
        v |= ((dat >> (7 - i)) & 0x01) << i;
    }
    
    return v;
}

u16 dev_maths_Crc16CCITT(u8* pbyDataIn, u32 len, u8 *abyCrcOut)
{
    
    u8 result[2];
//    u8 abyCrcOut[2];
    int i; // sxl20110225
    int val;
    
    val = 0;
    for (i = 0; i < len; i++) {
        val = (val >> 8) ^ crc16tab1[(val ^ flipAByte(pbyDataIn[i])) & 0xFF];
    }
    result[0] = (char) (val >> 8);
    result[1] = (char) val;
    
    for (i = 0; i < 2; i++) {
        result[i] = flipAByte(result[i]);
    }
    
    if(abyCrcOut != NULL)
    {
        abyCrcOut[0] = result[1];
        abyCrcOut[1] = result[0];
    }
    
    return (result[1]<<8)+result[0];
}
#endif
/****************************************************************************
**Description:        计算CRC-32值
**Input parameters:   crc：初始值
**                      szSrc: 需计算的数据
**                    dwSrcLen: 数据长度
**Output parameters:  无
**Returned value:     CRC-32值
**Created by:         
**Modified by:  
****************************************************************************/
u32 dev_maths_calc_crc32(u32 crc, u8 *szSrc, u32 dwSrcLen)
{
    u32 len = dwSrcLen;
    u8 * buf = (u8 *)szSrc;

    while(len--) 
    {
        crc = (crc >> 8) ^ CRC32TAB[(crc & 0xFF) ^ *buf++];
    }
	
	return crc;
}
/****************************************************************************
**Description:        计算CRC-32值(小端模式,BK3231固件下载)
**Input parameters:   crc：初始值
**				      szSrc: 需计算的数据
**                    dwSrcLen: 数据长度
**Output parameters:  无
**Returned value:     CRC-32值
**Created by:         
**Modified by:  
****************************************************************************/
typedef struct DWORD_BITS_Struct
{
	char b0:1;
	char b1:1;
	char b2:1;
	char b3:1;
	char b4:1;
	char b5:1;
	char b6:1;
	char b7:1;
	char b8:1;
	char b9:1;
	char b10:1;
	char b11:1;
	char b12:1;
	char b13:1;
	char b14:1;
	char b15:1;
	char b16:1;
	char b17:1;
	char b18:1;
	char b19:1;
	char b20:1;
	char b21:1;
	char b22:1;
	char b23:1;
	char b24:1;
	char b25:1;
	char b26:1;
	char b27:1;
	char b28:1;
	char b29:1;
	char b30:1;
	char b31:1;
	
} DWORD_BITS;

typedef struct BYTES_BITS_Struct
{
	char b0:1;
	char b1:1;
	char b2:1;
	char b3:1;
	char b4:1;
	char b5:1;
	char b6:1;
	char b7:1;
} BYTES_BITS;


typedef union _BYTE_UNION
{
  BYTES_BITS bits_v;
  unsigned char byte_v;
}BYTE_UNION;


typedef union _DWORD_UNION
{
  DWORD_BITS bits_v;
  unsigned int int_v;
}DWORD_UNION;
u32 dev_maths_calc_crc32_little(u32 crc, u8* content, u32 numread)
{
	DWORD_UNION u_crc;
	DWORD_UNION u_new_crc;
	BYTE_UNION u_d;
	unsigned int i;
	
	u_crc.int_v=crc;

	for(i=0;i<numread;i++)
	{

		u_d.byte_v=content[i];

		u_new_crc.bits_v.b0 =u_d.bits_v.b6 ^ u_d.bits_v.b0 ^u_crc.bits_v.b24 ^ u_crc.bits_v.b30;
		u_new_crc.bits_v.b1 = u_d.bits_v.b7 ^ u_d.bits_v.b6 ^ u_d.bits_v.b1 ^ u_d.bits_v.b0 ^ u_crc.bits_v.b24 ^ u_crc.bits_v.b25 ^ u_crc.bits_v.b30 ^ u_crc.bits_v.b31;
		u_new_crc.bits_v.b2 = u_d.bits_v.b7 ^ u_d.bits_v.b6 ^ u_d.bits_v.b2 ^ u_d.bits_v.b1 ^ u_d.bits_v.b0 ^ u_crc.bits_v.b24 ^ u_crc.bits_v.b25 ^ u_crc.bits_v.b26 ^ u_crc.bits_v.b30 ^ u_crc.bits_v.b31;
		u_new_crc.bits_v.b3 = u_d.bits_v.b7 ^ u_d.bits_v.b3 ^ u_d.bits_v.b2 ^ u_d.bits_v.b1 ^ u_crc.bits_v.b25 ^ u_crc.bits_v.b26 ^ u_crc.bits_v.b27 ^ u_crc.bits_v.b31;
		u_new_crc.bits_v.b4 = u_d.bits_v.b6 ^ u_d.bits_v.b4 ^ u_d.bits_v.b3 ^ u_d.bits_v.b2 ^ u_d.bits_v.b0 ^ u_crc.bits_v.b24 ^ u_crc.bits_v.b26 ^ u_crc.bits_v.b27 ^ u_crc.bits_v.b28 ^ u_crc.bits_v.b30;
		u_new_crc.bits_v.b5 = u_d.bits_v.b7 ^ u_d.bits_v.b6 ^ u_d.bits_v.b5 ^ u_d.bits_v.b4 ^ u_d.bits_v.b3 ^ u_d.bits_v.b1 ^ u_d.bits_v.b0 ^ u_crc.bits_v.b24 ^ u_crc.bits_v.b25 ^ u_crc.bits_v.b27 ^ u_crc.bits_v.b28 ^ u_crc.bits_v.b29 ^ u_crc.bits_v.b30 ^ u_crc.bits_v.b31;
		u_new_crc.bits_v.b6 = u_d.bits_v.b7 ^ u_d.bits_v.b6 ^ u_d.bits_v.b5 ^ u_d.bits_v.b4 ^ u_d.bits_v.b2 ^ u_d.bits_v.b1 ^ u_crc.bits_v.b25 ^ u_crc.bits_v.b26 ^ u_crc.bits_v.b28 ^ u_crc.bits_v.b29 ^ u_crc.bits_v.b30 ^ u_crc.bits_v.b31;
		u_new_crc.bits_v.b7 = u_d.bits_v.b7 ^ u_d.bits_v.b5 ^ u_d.bits_v.b3 ^ u_d.bits_v.b2 ^ u_d.bits_v.b0 ^ u_crc.bits_v.b24 ^ u_crc.bits_v.b26 ^ u_crc.bits_v.b27 ^ u_crc.bits_v.b29 ^ u_crc.bits_v.b31;
		u_new_crc.bits_v.b8 = u_d.bits_v.b4 ^ u_d.bits_v.b3 ^ u_d.bits_v.b1 ^ u_d.bits_v.b0 ^ u_crc.bits_v.b0 ^ u_crc.bits_v.b24 ^ u_crc.bits_v.b25 ^ u_crc.bits_v.b27 ^ u_crc.bits_v.b28;
		u_new_crc.bits_v.b9 = u_d.bits_v.b5 ^ u_d.bits_v.b4 ^ u_d.bits_v.b2 ^ u_d.bits_v.b1 ^ u_crc.bits_v.b1 ^ u_crc.bits_v.b25 ^ u_crc.bits_v.b26 ^ u_crc.bits_v.b28 ^ u_crc.bits_v.b29;
		u_new_crc.bits_v.b10 = u_d.bits_v.b5 ^ u_d.bits_v.b3 ^ u_d.bits_v.b2 ^ u_d.bits_v.b0 ^ u_crc.bits_v.b2 ^ u_crc.bits_v.b24 ^ u_crc.bits_v.b26 ^ u_crc.bits_v.b27 ^ u_crc.bits_v.b29;
		u_new_crc.bits_v.b11 = u_d.bits_v.b4 ^ u_d.bits_v.b3 ^ u_d.bits_v.b1 ^ u_d.bits_v.b0 ^ u_crc.bits_v.b3 ^ u_crc.bits_v.b24 ^ u_crc.bits_v.b25 ^ u_crc.bits_v.b27 ^ u_crc.bits_v.b28;
		u_new_crc.bits_v.b12 = u_d.bits_v.b6 ^ u_d.bits_v.b5 ^ u_d.bits_v.b4 ^ u_d.bits_v.b2 ^ u_d.bits_v.b1 ^ u_d.bits_v.b0 ^ u_crc.bits_v.b4 ^ u_crc.bits_v.b24 ^ u_crc.bits_v.b25 ^ u_crc.bits_v.b26 ^ u_crc.bits_v.b28 ^ u_crc.bits_v.b29 ^ u_crc.bits_v.b30;
		u_new_crc.bits_v.b13 = u_d.bits_v.b7 ^ u_d.bits_v.b6 ^ u_d.bits_v.b5 ^ u_d.bits_v.b3 ^ u_d.bits_v.b2 ^ u_d.bits_v.b1 ^ u_crc.bits_v.b5 ^ u_crc.bits_v.b25 ^ u_crc.bits_v.b26 ^ u_crc.bits_v.b27 ^ u_crc.bits_v.b29 ^ u_crc.bits_v.b30 ^ u_crc.bits_v.b31;
		u_new_crc.bits_v.b14 = u_d.bits_v.b7 ^ u_d.bits_v.b6 ^ u_d.bits_v.b4 ^ u_d.bits_v.b3 ^ u_d.bits_v.b2 ^ u_crc.bits_v.b6 ^ u_crc.bits_v.b26 ^ u_crc.bits_v.b27 ^ u_crc.bits_v.b28 ^ u_crc.bits_v.b30 ^ u_crc.bits_v.b31;
		u_new_crc.bits_v.b15 = u_d.bits_v.b7 ^ u_d.bits_v.b5 ^ u_d.bits_v.b4 ^ u_d.bits_v.b3 ^ u_crc.bits_v.b7 ^ u_crc.bits_v.b27 ^ u_crc.bits_v.b28 ^ u_crc.bits_v.b29 ^ u_crc.bits_v.b31;
		u_new_crc.bits_v.b16 = u_d.bits_v.b5 ^ u_d.bits_v.b4 ^ u_d.bits_v.b0 ^ u_crc.bits_v.b8 ^ u_crc.bits_v.b24 ^ u_crc.bits_v.b28 ^ u_crc.bits_v.b29;
		u_new_crc.bits_v.b17 = u_d.bits_v.b6 ^ u_d.bits_v.b5 ^ u_d.bits_v.b1 ^ u_crc.bits_v.b9 ^ u_crc.bits_v.b25 ^ u_crc.bits_v.b29 ^ u_crc.bits_v.b30;
		u_new_crc.bits_v.b18 = u_d.bits_v.b7 ^ u_d.bits_v.b6 ^ u_d.bits_v.b2 ^ u_crc.bits_v.b10 ^ u_crc.bits_v.b26 ^ u_crc.bits_v.b30 ^ u_crc.bits_v.b31;
		u_new_crc.bits_v.b19 = u_d.bits_v.b7 ^ u_d.bits_v.b3 ^ u_crc.bits_v.b11 ^ u_crc.bits_v.b27 ^ u_crc.bits_v.b31;
		u_new_crc.bits_v.b20 = u_d.bits_v.b4 ^ u_crc.bits_v.b12 ^ u_crc.bits_v.b28;
		u_new_crc.bits_v.b21 = u_d.bits_v.b5 ^ u_crc.bits_v.b13 ^ u_crc.bits_v.b29;
		u_new_crc.bits_v.b22 = u_d.bits_v.b0 ^ u_crc.bits_v.b14 ^ u_crc.bits_v.b24;
		u_new_crc.bits_v.b23 = u_d.bits_v.b6 ^ u_d.bits_v.b1 ^ u_d.bits_v.b0 ^ u_crc.bits_v.b15 ^ u_crc.bits_v.b24 ^ u_crc.bits_v.b25 ^ u_crc.bits_v.b30;
		u_new_crc.bits_v.b24 = u_d.bits_v.b7 ^ u_d.bits_v.b2 ^ u_d.bits_v.b1 ^ u_crc.bits_v.b16 ^ u_crc.bits_v.b25 ^ u_crc.bits_v.b26 ^ u_crc.bits_v.b31;
		u_new_crc.bits_v.b25 = u_d.bits_v.b3 ^ u_d.bits_v.b2 ^ u_crc.bits_v.b17 ^ u_crc.bits_v.b26 ^ u_crc.bits_v.b27;
		u_new_crc.bits_v.b26 = u_d.bits_v.b6 ^ u_d.bits_v.b4 ^ u_d.bits_v.b3 ^ u_d.bits_v.b0 ^ u_crc.bits_v.b18 ^ u_crc.bits_v.b24 ^ u_crc.bits_v.b27 ^ u_crc.bits_v.b28 ^ u_crc.bits_v.b30;
		u_new_crc.bits_v.b27 = u_d.bits_v.b7 ^ u_d.bits_v.b5 ^ u_d.bits_v.b4 ^ u_d.bits_v.b1 ^ u_crc.bits_v.b19 ^ u_crc.bits_v.b25 ^ u_crc.bits_v.b28 ^ u_crc.bits_v.b29 ^ u_crc.bits_v.b31;
		u_new_crc.bits_v.b28 = u_d.bits_v.b6 ^ u_d.bits_v.b5 ^ u_d.bits_v.b2 ^ u_crc.bits_v.b20 ^ u_crc.bits_v.b26 ^ u_crc.bits_v.b29 ^ u_crc.bits_v.b30;
		u_new_crc.bits_v.b29 = u_d.bits_v.b7 ^ u_d.bits_v.b6 ^ u_d.bits_v.b3 ^ u_crc.bits_v.b21 ^ u_crc.bits_v.b27 ^ u_crc.bits_v.b30 ^ u_crc.bits_v.b31;
		u_new_crc.bits_v.b30 = u_d.bits_v.b7 ^ u_d.bits_v.b4 ^ u_crc.bits_v.b22 ^ u_crc.bits_v.b28 ^ u_crc.bits_v.b31;
		u_new_crc.bits_v.b31 = u_d.bits_v.b5 ^ u_crc.bits_v.b23 ^ u_crc.bits_v.b29;
		 
		u_crc.int_v=u_new_crc.int_v;
	}

	return u_crc.int_v;

}
/****************************************************************************
**Description:        返回最小值
**Input parameters:   

**Output parameters:  无
**Returned value:     
**Created by:         pengxuebin, 20170316
**Modified by:  
****************************************************************************/
u32 dev_maths_min(u32 t1, u32 t2)
{
    if(t1<=t2)
    {
        return t1;
    }
    else
    {
        return t2;
    }
}
/****************************************************************************
**Description:        返回最大值
**Input parameters:   

**Output parameters:  无
**Returned value:     
**Created by:         pengxuebin, 20170316
**Modified by:  
****************************************************************************/
u32 dev_maths_max(u32 t1, u32 t2)
{
    if(t1>=t2)
    {
        return t1;
    }
    else
    {
        return t2;
    }
}

#if 0
void dev_maths_test(void)
{
    u8 tmp[16];
    u16 crc;
    u8 tmp1[8];
    u32 crc32;

    memset(tmp, 0x38, 8);
    crc = dev_maths_calc_crc16_MODBUS(0xffff, tmp, 8);
dev_debug_printf("dev_maths_calc_crc16_MODBUS:crc=%08X\r\n", crc);
    dev_maths_calc_crc16(tmp, 8, tmp1);
dev_debug_printf("dev_maths_calc_crc16_xx:crc=%02X%02X\r\n", tmp1[0], tmp1[1]);
    
    crc32 = dev_maths_calc_crc32(0xFFFFFFFF, tmp, 8);
dev_debug_printf("dev_maths_calc_crc32:crc=%08X\r\n", crc32);
}
#endif

/**
 * @brief 获取地址对应的文件名
 * @param[in] path_name 地址
 * @retval  根文件名
 */
s8 *dev_maths_get_basename(s8 *path_name)
{
    s8 *pstr = NULL;
    
    do{
        if(NULL == path_name)
        {
            break;
        }
        
        pstr = strrchr(path_name, '\\');
        if(NULL == pstr)
        {
            break;
        }

        pstr++;
    }while(0);

    return pstr;
}

s32 dev_maths_atoi(const char *str)
{
    int s = 0; 
    char flag = 0;  

    while(*str==' ') 
    {  
        str++; 
    }  
    if(*str=='-'||*str=='+') 
    {  
        if(*str=='-')  
            flag = 1;
        
        str++; 
    }  
    while(*str>='0'&&*str<='9') 
    {  
        s=s*10+*str-'0';  
        str++;  
        if(s<0)  
        {   
            s=2147483647;   
            break;  
        } 
    } 
    return s*(flag?-1:1);
}