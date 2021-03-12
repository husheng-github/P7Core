

#include "pciglobal.h"



////20140327
int Lib_memcmp(u8 *cmpdata1,u8 *cmpdata2,u32 cmpdatalen)
{
  #if 0  
    int i = 0;

    i = 0;
    while(cmpdatalen != 0)
    {
        cmpdatalen--;
        i = i | (cmpdata1[cmpdatalen]^cmpdata2[cmpdatalen]);
    }

    return i;
  #endif
    return dev_maths_mem_cmp(cmpdata1, cmpdata2, cmpdatalen);
}


//20140327
void Lib_memset(u8 *dest,u8 value,u32 length)
{
  #if 0  
    u32 i;

    for(i = 0;i < length;i++)
    {
        dest[i] = value;
    }
  #endif
    dev_maths_mem_set(dest, value, length);
    
}


void pciarithCrc16CCITT(const u8 *pbyDataIn, u32 dwDataLen, u8 *abyCrcOut)
{
  #if 0  
    u16 wCrc = 0;
    u8 byTemp;
    u16 mg_awhalfCrc16CCITT[16];
    u8 i;
	
	
    u16 g_awhalfCrc16CCITT[16]={ /* CRC 16bit  *///sxl?
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef
    };

    for(i = 0;i < 16;i++)
    {
        mg_awhalfCrc16CCITT[i] = g_awhalfCrc16CCITT[i];
    }

    while(dwDataLen-- != 0)
    {
        byTemp = ((u8)(wCrc>>8))>>4;
        wCrc <<= 4;
        wCrc ^= mg_awhalfCrc16CCITT[byTemp^(*pbyDataIn/16)];
        byTemp = ((u8)(wCrc>>8))>>4;
        wCrc <<= 4;
        wCrc ^= mg_awhalfCrc16CCITT[byTemp^(*pbyDataIn&0x0f)];
        pbyDataIn++;
    }
    abyCrcOut[0] = wCrc/256;
    abyCrcOut[1] = wCrc%256;
  #endif
    dev_maths_calc_crc16(0, pbyDataIn, dwDataLen, abyCrcOut);
	
} 

unsigned char pciarithgetlrc(unsigned char *data,int datalen)
{
  #if 0  
    unsigned char tmplrc;
    int i;

    tmplrc = 0;
    for(i =0;i<datalen;i++)
    {
        tmplrc ^= data[i];
    }

    return tmplrc;
  #endif
    return dev_maths_getlrc(0, data, datalen);
}

void ulong_to_bit21(u32 longdata,u8 *bitdata)
{
    u16 high,low;

    high = (u16)(longdata>>16);
    low  = (u16)(longdata&0xFFFF);

    bitdata[0] = high&0x1F;
    bitdata[1] = low>>8;
    bitdata[2] = low&0xFF;

}


u32 bit21_to_ulong(u8 *bitdata)
{
    u32 temp = 0;
    s32 i;

    for(i = 0;i<3;i++)
    {
        temp = (temp<<8) + bitdata[i];
    }

    return temp;
}


void LongToByte(u32 data,u8 *ch)
{
    ch[0] = (u8)(data>>24);
    ch[1] = (u8)(data>>16);
    ch[2] = (u8)(data>>8);
    ch[3] = (u8)(data);
}


u32 Long2Byte(u8 *ch)
{
    u32 l = ch[0];  
    l = (l<<8) + ch[1];
    l = (l<<8) + ch[2];
    l = (l<<8) + ch[3];
    return l;
}

#if 0
const u32 tbCRC32[256] = {  //sxl?  要查看是否在flash中
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
#endif

/*******************************************************************
Function Nam         :u32  GetHash(u32 crc, u8 * szSrc, u32 dwSrcLen)
Function Purpose : Get Hash value
Input Paramters  : 1:crc   - initial crc
                          2:szSrc  - source data
                          3:dwSrcLen -source data len
Return Value:crc value
Remark:             0722
********************************************************************/
u32  GetHash(u32 crc, u8 * szSrc, u32 dwSrcLen)
{
  #if 0  
    u32 len = dwSrcLen;
    u8 * buf = (unsigned char *)szSrc;
    u32 mtbCRC32[256];
    u32 i;

    for(i = 0;i<256;i++)
    {
        mtbCRC32[i] = tbCRC32[i];
    }

    while (len--) 
    {
        crc = (crc >> 8) ^ mtbCRC32[(crc & 0xFF) ^ *buf++];
    }
    return crc;
  #endif
    return dev_maths_calc_crc32(crc, szSrc, dwSrcLen);
}


////20140327
void pciarith_BcdSub(u8 *Dest,u8 *Src,u32 Len)
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


void pciarith_BcdAdd(unsigned char *Dest,unsigned char *Src,unsigned char Len)
{
    unsigned char i;
    unsigned char high,low;
    unsigned char carry=0;                     //运算进位标志

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


void pciarith_AscToBcd(unsigned char *Dest,unsigned char *Src,unsigned short Len)
{
  #if 0  
    unsigned short i;
    unsigned char high = 0,low = 0;
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
  #endif
    dev_maths_asc_to_bcd(Dest,Src,Len);
}


void pciarith_BcdToAsc(char *Dest,char *Src,u16 Len)
{
  #if 0  
    u16 i;
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
  #endif
    dev_maths_bcd_to_asc(Dest,Src, Len);
}

/****************************************************************************
**Description:      将ASC字符转换成U32数值
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:            hxj(2014-3-22)
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
u32 ascii_2_u32(u8 *pb,u8 asciidatalen)
{
  #if 0  
    u32 i;
    u32 d;
    u8 tmpvalue;

    d=0;
    for(i=0; i<asciidatalen&&i<8; i++)
    {
        if((*(pb+i))>='a'&&(*(pb+i))<='f')
        {
            tmpvalue = 0x0a + (*(pb+i)) - 'a';
        }
        else if((*(pb+i))>='A'&&(*(pb+i))<='F')
        {
            tmpvalue = 0x0a + (*(pb+i)) - 'A';
        }
        else if((*(pb+i))>='0'&&(*(pb+i))<='9')
        {
            tmpvalue = (*(pb+i));
        }
        else
        {
            return 0;
        }

        d = d<<4;
        d += (tmpvalue & 0x0f);
    }

    return d;
  #endif
    return dev_maths_asc_to_u32(pb, asciidatalen, MODULUS_HEX);
}


void pciarith_U32ToHex(u8 *Hex, u32 const Src,u32 Len)
{
  #if 0
	u32 i;
	u32 num = Src;

	for(i = Len; i > 0; i--)
	{
		*(Hex+i-1) = num % 256;
		num /= 256;
	}
  #endif
    dev_maths_u32_to_hex(Hex, Src, Len);
}
void pciarith_HexToU32(u32 *Dest,u8 *Hex,u32 Len)
{
  #if 0  
	u32 i;
	*Dest = 0;

	for(i = 0; i < Len; i++)
	{
		*Dest *= 256;
		*Dest += Hex[i];
	}
  #endif
    dev_maths_hex_to_u32(Dest, Hex, Len);
}


//字符替换
u8 pciarith_StrChr(u8 * str, u8 oldchr,u8 newchr)
{ 
	u32 i = 0;
	u8 ret = 0;

	while(str[i] != 0 ) 
	{
		if(str[i] == oldchr)
		{
			str[i] = newchr;
			ret = 1;
		}
		else
		{
			i++;
		}
	}
	return ret;
    
}



