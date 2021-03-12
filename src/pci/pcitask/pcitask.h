

#ifndef _PCITASK_H_
#define _PCITASK_H_



////////////communicate protocol   from TRSM or PC to PCI module //////
#define AUTHENTICATE_REQUEST        0xb100
#define AUTHENTICATE_STEP3          0xb102


#define LOAD_POSAUTHENK_REQUEST     0xB320
#define LOAD_POSAUTHENK_RESP        0xB321



#define LOAD_APPKEY_REQUEST         0xb322

#define COMMAND_LOADMAGKEY          0xb324
#define COMMAND_LOADMAGKEY_RESP     0xb325
#define COMMAND_SYNCMAGKEY          0xb326
#define COMMAND_SYNCMAGKEY_RESP     0xb327

#define LOAD_APPDUKPTKEY_REQUEST    0xb330

#define COMMAND_DELETEALLKEY        0xb400
#define COMMAND_DELETEALLKEY_RESP   0xb401
#define COMMAND_DELETEGROUPALLKEY        0xb402
#define COMMAND_DELETEGROUPALLKEY_RESP   0xb403
#define COMAMND_SETGROUPKEYAPPNAME       0xb404
#define COMAMND_SETGROUPKEYAPPNAME_RESP  0xb405
#define COMMAND_SETINSTALLTOUCHSCREEN       0xb406
#define COMMAND_SETINSTALLTOUCHSCREEN_RESP  0xb407      //add by 2011-07-30





////////////communicate protocol   from PCI module to TRSM or PC //////
#define AUTHENTICATE_STEP2          0xb101
#define AUTHENTICATE_RESPOND        0xb103
#define LOAD_APPKEY_RESPOND         0xb323
#define APPKEY_SETPINK              0x9400
#define APPKEY_SETPINK_RESPONSE     0x9401
#define APPKEY_SETMACK              0x9500
#define APPKEY_SETMACK_RESPONSE     0x9501
#define APPKEY_SETTDK               0x9600
#define APPKEY_SETTDK_RESPONSE      0x9601




#define COMMAND_GETRANDOMNUMBER      0x9100
#define COMMAND_GETRANDOMNUMBER_RESP 0x9101



#define COMMAND_GETPIN                      0x9A00
#define COMMAND_GETPIN_RESP                 0x9A01
#define COMMAND_CHECKPINKEYSTATUS           0x9A10
#define COMMAND_CHECKPINKEYSTATUS_RESP      0x9A11


#define COMMAND_GETMAC                      0x9B00
#define COMMAND_GETMAC_RESP                 0x9B01
#define COMMAND_GETMAGENCRYPTEDDATA         0x9C00
#define COMMAND_GETMAGENCRYPTEDDATA_RESP    0x9C01


#define COMMAND_DPATEST                     0x9D00
#define COMMAND_DPATEST_RESP                0x9D01


#define COMMAND_GETDUKPTKPIN                0xA000
#define COMMAND_GETDUKPTKPIN_RESP           0xA001
#define COMMAND_GETDUKPTKMAC                0xA100
#define COMMAND_GETDUKPTKMAC_RESP           0xA101
#define COMMAND_GETFIXEDPIN                 0xB000
#define COMMAND_GETFIXEDPIN_RESP            0xB001
#define COMMAND_GETOFFLINEPLAINPIN          0xF001
#define COMMAND_GETOFFLINEENCPIN            0xF002


#define COMMAND_LOGGETLENGTH                0x7A04
#define COMMAND_LOGGETDATA                  0x7A05
#define COMMAND_LOGDELETE                   0x7A06



extern void pcitask_commu_SendPacket(s32 protocolcmd,u8 *senddata,s32 senddatalen,u32 arg);
extern void pcitask_dealdpatest(u32 OperateInstruction,u8 *rxbuf,s32 rxbuflen,u32 arg);
extern s32 pcitask_dealsecurityservice(u32 OperateInstruction,u8 *rxbuf,s32 rxbuflen,u32 arg);
extern s32 pcitask_process_downloadkeycmd(u32 OperateInstruction,u8 *rxbuf,s32 rxbuflen,u32 arg);
extern void pcitask_dealcommanddata(unsigned int command,u8 *data,unsigned int datalen);


#endif


