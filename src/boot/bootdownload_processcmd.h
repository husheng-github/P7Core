

#ifndef _BOOTDOWNLOAD_PROCESSCMD_H_
#define _BOOTDOWNLOAD_PROCESSCMD_H_




#define LEDSTATUS_OFF        0
#define LEDSTATUS_ON         1



#define COMMAND_BOOTMANAGE_SPSOFTRESET          0x1001
#define COMMAND_BOOTMANAGE_SPBEEP               0x1003
#define COMMAND_BOOTMANAGE_SPLEDCTRL            0x1004
#define COMMAND_BOOTMANAGE_SPVERSIONGET         0x1009



#define COMMAND_BOOT_FIRMWAREMANAGE_GETCERTINFO            0x1301
#define COMMAND_BOOT_FIRMWAREMANAGE_SENDFIRMWAREBINDATA    0x1302
#define COMMAND_BOOT_FIRMWAREMANAGE_UPDATEFLAGSET          0x1303
#define COMMAND_BOOT_FIRMWAREMANAGE_UPDATEFLAGCLR          0x1304
#define COMMAND_BOOT_GETSPSTATUS                           0x1101


#define COMMPROTOCOL_TRENDITSP_FLAG      "TRETEI"
#define COMMPROTOCOL_TRENDITSP_ACKFLAG   "TRERSP"

typedef enum
{
    BOOT_RESPONSECODE_SUCCESS              = 0x00,  //����ɹ�
	BOOT_RESPONSECODE_CMDNOTSUPPORT        = 0x01,  //ָ���벻֧��
	BOOT_RESPONSECODE_PARAMERR             = 0x02,  //��������
	BOOT_RESPONSECODE_VARFIELDLENERR       = 0x03,  //�ɱ������򳤶ȴ���
	BOOT_RESPONSECODE_FRAMEFORMATERR       = 0x04,  //֡��ʽ����
	BOOT_RESPONSECODE_LRCERR               = 0x05,  //LRC
	BOOT_RESPONSECODE_OTHERERR             = 0x06,  //����
	BOOT_RESPONSECODE_OVERTIME             = 0x07,  //��ʱ
	BOOT_RESPONSECODE_FAIL                 = 0x08,   //���ص�ǰ״̬
	BOOT_RESPONSECODE_WAIT                 = 0xff
}BOOT_RESPONSECODE_DEF;



typedef enum
{
    MESSAGE_TYPE_NONE     = 0x00,
    MESSAGE_TYPE_REQUEST  = 0x2F,  //������
    MESSAGE_TYPE_RESPONSE = 0x4F,  //��Ӧ
    MESSAGE_TYPE_ACK      = 0x6F,  //Ӧ����ACK
    
}MESSAGE_TYPE;


#if 0
typedef struct _strLedGleamPara
{
    u32 m_led;          //LED���
    u32 m_ontime;       //������������ʱ�䣬��λ����
    u32 m_offtime;      //�������𱣳�ʱ�䣬��λ����
    u32 m_duration;     //��˸����ʱ�䣬��λ����
} strLedGleamPara;
#endif

extern void bootdownload_processcmd(void);
extern void processcmd_bootload_senddata(teiframe *frame,u8 *respdata,u16 respdatalen);



#endif

