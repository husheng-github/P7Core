

#ifndef _PITTABLE_H_
#define _PITTABLE_H_


#define COREPITTABLEOFFSET 0x1018800
#define BOOTPITTABLEOFFSET 0x1001800



#define  TRENDIT_TAG_BIOS       0x42494F53
#define  TRENDIT_TAG_CORE       0x434F5245
#define  TRENDIT_TAG_APPL       0x4150504C





struct trendit_sp_header_s{       //头部
unsigned int   tag;           //BISO  CORE  APPL
unsigned int  startAddr;      //Core映像链接地址
unsigned int  imageMaxSize;   //映像最大尺寸
unsigned int  entryAddr;      //映像入口地址
unsigned int  version;        //当前版本
unsigned int  depHighVer;    //能够支持的最高版本，加载时程序根据此域作版本匹配
unsigned int  depLowVer;     //能够支持的最低版本
unsigned int  apiTableAddr;   // BIOS,此域无效，保存API表地址
unsigned char   verStr[32]  ;   //包含日期，YYMMDD，以NULL结尾
unsigned char   topShaVal[32]   ;   //前半部分SHA256
//用于校验完整性信息，此处采用HASH256算法
unsigned char   bottomShaVal[32];  //后半部分SHA256
unsigned char   desc[64]; 
//后面区域预留
};



extern void pitver_get_corever(unsigned char *ver);
extern void pitver_get_bootver(unsigned char *ver);
extern void pitver_get_hardwarever(unsigned char *ver);


#endif


