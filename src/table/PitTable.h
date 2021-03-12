

#ifndef _PITTABLE_H_
#define _PITTABLE_H_


#define COREPITTABLEOFFSET 0x1018800
#define BOOTPITTABLEOFFSET 0x1001800



#define  TRENDIT_TAG_BIOS       0x42494F53
#define  TRENDIT_TAG_CORE       0x434F5245
#define  TRENDIT_TAG_APPL       0x4150504C





struct trendit_sp_header_s{       //ͷ��
unsigned int   tag;           //BISO  CORE  APPL
unsigned int  startAddr;      //Coreӳ�����ӵ�ַ
unsigned int  imageMaxSize;   //ӳ�����ߴ�
unsigned int  entryAddr;      //ӳ����ڵ�ַ
unsigned int  version;        //��ǰ�汾
unsigned int  depHighVer;    //�ܹ�֧�ֵ���߰汾������ʱ������ݴ������汾ƥ��
unsigned int  depLowVer;     //�ܹ�֧�ֵ���Ͱ汾
unsigned int  apiTableAddr;   // BIOS,������Ч������API���ַ
unsigned char   verStr[32]  ;   //�������ڣ�YYMMDD����NULL��β
unsigned char   topShaVal[32]   ;   //ǰ�벿��SHA256
//����У����������Ϣ���˴�����HASH256�㷨
unsigned char   bottomShaVal[32];  //��벿��SHA256
unsigned char   desc[64]; 
//��������Ԥ��
};



extern void pitver_get_corever(unsigned char *ver);
extern void pitver_get_bootver(unsigned char *ver);
extern void pitver_get_hardwarever(unsigned char *ver);


#endif


