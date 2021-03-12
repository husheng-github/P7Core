

#ifndef _BOOTDOWNLOAD_PARSEDATA_H_
#define _BOOTDOWNLOAD_PARSEDATA_H_



#define  UARTDOWNLOAD_PORTNUM  PORT_UART0_NO//0

typedef enum _PROTOCOL_TYPE
{
	PROTOCOL_TYPE_55AA = 0,
	PROTOCOL_TYPE_TEI,        //smart pos TEI protocol
	PROTOCOL_TYPE_TRENDIT,   //¥Û«˜¿∂—¿À¢ø®∆˜÷∏¡ÓπÊ∑∂
	
}PROTOCOL_TYPE;


typedef struct _teiframe 
{
    u8 sn;
	u8 res;//œÏ”¶÷°”√
	u16 status;
	u8 type;
    u16 cmd;
    u16 len;
    u8 *data;
}teiframe;



typedef struct _BOOTDOWNLOAD_PARSEDATA
{
    u8 Rx_Valid;
	u8 CommParseCmdStep;
	u8 SynHeadCnt;
	u8 checksum;
	u16 RxCNT;
	u16 RxTotalLen;
	PROTOCOL_TYPE protocoltype;
	teiframe frame;
	u8 *pdownfiledata;
	u32 rectimer;
	u8 crc[2];
}BOOTDOWNLOAD_PARSEDATA;


extern s32 bootdownload_parserecdata(void);

#endif

