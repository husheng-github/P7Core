

#ifndef _PCILOG_H_
#define _PCILOG_H_



//#define SAVEPCILOG


#define PCIHLLOGFILE "/mtd0/LOG.txt"
#define TMPPCIHLLOGFILE "/mtd0/TMPLOG.txt"





#define PROMPT_DOWNKEY_CERTIFICATEERR "公钥证书错误!"
#define PROMPT_DOWNKEY_KEYLOCKED      "该组密钥已被锁!"
#define PROMPT_DOWNKEY_PKNOTFIND      "公钥证书未找到,请先下载!"
#define PROMPT_DOWNKEYPOSPKERR        "终端公钥证书错误,请检查!"
#define PROMPT_DOWNKEY_SAVEERR        "密钥下载失败!"
#define PROMPT_DOWNKEY_GROUPINDEX_ERR "组密钥号错误!"
#define PROMPT_DOWNKEY_INDEX_ERR      "密钥索引号错误!"
#define PROMPT_DOWNKEY_TYPE_ERR       "密钥类型错误!"
#define PROMPT_DOWNKEY_LENGTHERR      "密钥长度错误!"
#define PROMPT_DOWNKEY_AUTHENFAIL     "认证失败!"
#define PROMPT_DOWNKEY_UNAUTHENTICATE "未认证"
#define PROMPT_AUTHENKEYISSUER_ERROR  "公钥证书不匹配"
#define PROMPT_KEYFILENOTUSE          "密钥未下载"
#define PROMPT_DOWNKEY_OTHERERROR     "下载密钥失败!"
#define PROMPT_NOLOGFILE              "没有LOG文件"
#define PROMPT_SENDLOGFILE            "发送log文件..."
#define PROMPT_SENDOVERTIME           "发送超时"
#define PROMPT_SENDLOGSUCCSS          "发送log文件成功!"



#define KEYTYPE_SK_MACK 0x09
#define KEYTYPE_PIN_MK  0x10
#define KEYTYPE_MAC_MK  0x12
#define KEYTYPE_FIXEDK  0x14
#define KEYTYPE_MAGKEY  0x30
#define KEYTYPE_END     0xff


#define MAXLOGFILELEN   0x2800  //sxl  先设置成10K


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


