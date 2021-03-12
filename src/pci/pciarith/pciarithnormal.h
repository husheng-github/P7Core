

#ifndef _PCIARITHNORMAL_H_
#define _PCIARITHNORMAL_H_



extern int Lib_memcmp(u8 *cmpdata1,u8 *cmpdata2,u32 cmpdatalen);
extern void Lib_memset(u8 *dest,u8 value,u32 length);
extern void pciarithCrc16CCITT(const u8 *pbyDataIn, u32 dwDataLen, u8 *abyCrcOut);
extern unsigned char pciarithgetlrc(unsigned char *data,int datalen);
extern void ulong_to_bit21(u32 longdata,u8 *bitdata);
extern u32 bit21_to_ulong(u8 *bitdata);
extern u32 Long2Byte(u8 *ch);
extern void LongToByte(u32 data,u8 *ch);
extern u32  GetHash(u32 crc, u8 * szSrc, u32 dwSrcLen);
extern void pciarith_BcdSub(u8 *Dest,u8 *Src,u32 Len);
extern void pciarith_BcdAdd(unsigned char *Dest,unsigned char *Src,unsigned char Len);
extern void pciarith_AscToBcd(unsigned char *Dest,unsigned char *Src,unsigned short Len);
extern u32 ascii_2_u32(u8 *pb,u8 asciidatalen);
extern void pciarith_BcdToAsc(char *Dest,char *Src,u16 Len);
extern void pciarith_U32ToHex(u8 *Hex, u32 const Src,u32 Len);
extern void pciarith_HexToU32(u32 *Dest,u8 *Hex,u32 Len);
extern u8 pciarith_StrChr(u8 * str, u8 oldchr,u8 newchr);

extern void ReadBigInt(unsigned char *BigInt, unsigned int sizeBigInt, unsigned char *pBuf, unsigned int Len);


#endif


