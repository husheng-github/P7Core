

#ifndef _PCILOG_H_
#define _PCILOG_H_



//#define SAVEPCILOG


#define PCIHLLOGFILE "/mtd0/LOG.txt"
#define TMPPCIHLLOGFILE "/mtd0/TMPLOG.txt"





#define PROMPT_DOWNKEY_CERTIFICATEERR "��Կ֤�����!"
#define PROMPT_DOWNKEY_KEYLOCKED      "������Կ�ѱ���!"
#define PROMPT_DOWNKEY_PKNOTFIND      "��Կ֤��δ�ҵ�,��������!"
#define PROMPT_DOWNKEYPOSPKERR        "�ն˹�Կ֤�����,����!"
#define PROMPT_DOWNKEY_SAVEERR        "��Կ����ʧ��!"
#define PROMPT_DOWNKEY_GROUPINDEX_ERR "����Կ�Ŵ���!"
#define PROMPT_DOWNKEY_INDEX_ERR      "��Կ�����Ŵ���!"
#define PROMPT_DOWNKEY_TYPE_ERR       "��Կ���ʹ���!"
#define PROMPT_DOWNKEY_LENGTHERR      "��Կ���ȴ���!"
#define PROMPT_DOWNKEY_AUTHENFAIL     "��֤ʧ��!"
#define PROMPT_DOWNKEY_UNAUTHENTICATE "δ��֤"
#define PROMPT_AUTHENKEYISSUER_ERROR  "��Կ֤�鲻ƥ��"
#define PROMPT_KEYFILENOTUSE          "��Կδ����"
#define PROMPT_DOWNKEY_OTHERERROR     "������Կʧ��!"
#define PROMPT_NOLOGFILE              "û��LOG�ļ�"
#define PROMPT_SENDLOGFILE            "����log�ļ�..."
#define PROMPT_SENDOVERTIME           "���ͳ�ʱ"
#define PROMPT_SENDLOGSUCCSS          "����log�ļ��ɹ�!"



#define KEYTYPE_SK_MACK 0x09
#define KEYTYPE_PIN_MK  0x10
#define KEYTYPE_MAC_MK  0x12
#define KEYTYPE_FIXEDK  0x14
#define KEYTYPE_MAGKEY  0x30
#define KEYTYPE_END     0xff


#define MAXLOGFILELEN   0x2800  //sxl  �����ó�10K


typedef struct _PCILKEYLOGINFO{
	u32 step;
	u8 issuersn[48];
	u8 groupindex;
	u8 keytype;
	u8 keyindex;
	u8 mainkeyindex;
	u8 result;
}PCILKEYLOGINFO;





extern void pcilog_savekeymanagelog(s32 gDownLoadKeyErr);
extern void pcilog_record(u32 command,u8 *data);
extern u32 pcilog_getlogfilelength(u8 *filelendata);
extern u32 pcilog_getlogfiledata(u8 *inputdata,u8 *filelendata);
extern u32 pcilog_logfile_delete(u8 *senddata);

#endif


