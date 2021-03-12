

#ifndef _FS_SYSTEMBASE_H_
#define _FS_SYSTEMBASE_H_



typedef struct _SysFsPra
{
    u8  dir[8];
    u32 BLen;
    u32 StrAdd;
    u32 Len;
    u32 BlockNum;
    u32 EndAdd;
    //����
    s32 (*read)( u32 addr,u8* data_buf,u32 len);
    s32 (*write)( u32 addr,u8* data_buf,u32 len);      //write ��������� erase ����
    s32 (*erase)(u32 addr);//�����Ĵ�С���� BLen
    s32 (*writespec)( u32 addr,u8* data_buf,u32 len);  //special write mode for write system data
}strSysFsPra;

#define SYSDIRLEN           31                      //ϵͳĿ¼����

//д���Ĳ������ڿ��С��4K������£�ֻ�ܹ���50���ļ���690��ռ�
#define FileNum         25
#define FileSysBlockNum 300//ϵͳ�������ܹ���������������ڿ��С��4096�������
                           //��ʵ���ļ�ϵͳ�Ŀ�����һ����ʵ���ļ�ϵͳ����С�ڵ������ֵ
                           
#define UNBlock       0xffff    //δ�ÿ�
#define BadBlock      0xff00    //����
#define LogBlock      0Xff55    //��־��
#define FileFlag      0x55aa    //��Ч�ļ�����־
#define FileBFlag     0x5a      //Ϊ�ļ����־  ����Ϊ12

#define SOK           0x00  //�����ɹ�
#define SER           0x01  //����ʧ��

#define WFL           0x01  //д����ʧ��
#define RFL           0x02  //������ʧ��
#define NFS           0x03  //δ�����ļ�ϵͳ
#define RNE           0x04  //д���ݿռ䲻��
#define AER           0x05  //���ݵ�ַ����
#define NFN           0x06  //�ļ���û����
#define FILECSR           0x07  //����У�����
#define NNF           0x08  //�ļ��Ų�����ȷ���ļ�
#define FEnd          0x09  //���ļ��ѵ�β
#define FNF           0x0a  //Ŀ¼�ļ�����,�����ٴ����ļ�!
#define LST           0x0b  //Ŀ¼�ļ���̫��,�����ٴ����ļ�

#define EQU           0   //���
#define BIG           1   //����
#define LIT           2   //С��

#define NSD           0//�����ļ�ϵͳ���� 
#define ISF           1//Ŀ¼���� 
#define ISL           2//��������
//===========================================================================
//  Flash ����Ϣ Ĭ�Ͽ鳤�Ȳ������32K
struct _FileLink
{
   u8 NextBlack[2];     //0x00~BlockLen               Ϊ������д�ĳ��� ,
                          //BlockLen + FileSysBLen   ��ʾ�ļ�ϵͳ�� ��ѧ�� ע
                          //0x8000~ 0x8000 + BlockNum   Ϊ��һ���� + 0x8000 ,
                          //0xffff                      δ�ÿ� 
                          //0xff00                      ����
   u8 CS;            //һ�����ݵ��ۼӺ�
};
#define  FileLink sizeof(struct _FileLink)

//===========================================================================
//                       �ļ�����
struct _FileSys
{
    u8 FileF[2];        //0  0x55AA Ϊ��ȷ���ļ�
    u8 Name[SYSDIRLEN]; //2  �ļ���
    u8 StartLink[2];    //33 ���ӿ�    0xffff     δ����ռ�
    u8 FileLen[4];      //35 �ļ�����  0x00       δ����ռ�
    u8 CS;              //39 У���
};                        //Len 40
#define  FileNLen sizeof(struct _FileSys)
//============================================================================
//                    �ļ��������ݽṹ
struct _FileSysB
{
    u8              FileBF[12];         //0  0x5a5a 5a5a 5a5a 5a5a 5a5a 5a5a Ϊ�ļ����־
    u8              WdateTime[6];       //12 д����
    u8              RBlock[2];          //18 ���д�Ŀ��
    u8              FileBN[2];          //20 �ļ�ϵͳ���
    u8              FileBBN[2];         //22 �ļ�ϵͳ���ݿ�� 
    struct _FileSys   File[FileNum];  //24 �ļ�����ͷ(֧��25���ļ�)
    struct _FileLink  Block[FileSysBlockNum];   //1024 = 24 + 40 *25   //�����
    u8              CS;                 //1924 = 1024 + 300*3 +1//1��4K�����������ܹ���690�� 2760K

};                                        //len3159 < 4096
#define  FileSysBLen sizeof(struct _FileSysB)
#define _FileSysLink_BUFSIZE 2048
//============================================================================
union _FileSysLink
{
   u8               Buf[_FileSysLink_BUFSIZE];//�����Ķ��壬��������ݣ�������ϵĴ�С������С��һ��ĳ���
                              //Ŀǰ�ļ�ϵͳֻ�ܹ���鳤��Ϊ4K��
   u16              U16Buf[_FileSysLink_BUFSIZE>>1];
   struct _FileSysB   FileSB;    //�ļ��������ݽṹ
  // struct _FileLinkB  FileLB;    //�ļ��������ݽṹ
};


#define FileSysBlock_BUFSIZE 4096
union _FileSysBlockBuf
{
	
	u8               Buf[FileSysBlock_BUFSIZE];//�����Ķ��壬��������ݣ�������ϵĴ�С������С��һ��ĳ���
	                                                           //Ŀǰ�ļ�ϵͳֻ�ܹ���鳤��Ϊ4K��
	u16              U16Buf[FileSysBlock_BUFSIZE>>1];
	u32              U32Buf[FileSysBlock_BUFSIZE>>2];
  // struct _FileLinkB  FileLB;    //�ļ��������ݽṹ
  
};

//============================================================================
//�ļ�ϵͳ����
struct _FileSysP
{
    u32  PrepaidSpace;   //���ÿռ�
    u32  DoSpace;        //���ÿռ�
    u32  BadSpace;       //�ѻ��ռ�
    u8   PrepaidFileNum; //�����ļ���
    u8   DoFileNum;      //�����ļ���
}; 
//============================================================================
struct _TEMPSYSFILEINFO
{
    u8    WdateTime[6]; 
    u16     BLKNo;
};
//===========================================================================
extern struct _FileSysB   FileSysB;  //�ļ��������ݽṹ
extern struct _FileSysP   FileSysP;  //�ļ�ϵͳ����
//===========================================================================


//extern void pciarith_BcdAdd(unsigned char *Dest,unsigned char *Src,unsigned char Len);
//extern void pciarith_BcdSub(u8 *Dest,u8 *Src,u32 Len);

extern s32 fs_systembase_mount(void);
extern u8 SysFile_AmendFileNameFnum(u8 FNum, u8 *NewFileName);

extern u32 SysFile_File_GetLen(u8 Fnum);
extern u8 SysFile_ReadFileData(u8 FNum,u8 *RD,u32 StrA,u32 *DLen);
extern u8 SysFile_WriteFileData(u8 FNum,u32 StrA, u8 *WD,u32 DLen);
extern u8 SysFile_DeleteFileNum(u8 FNum);
extern void SysFile_ClearFileData(u8 FNum);
extern u8 SysFile_FoundFile(u8 *FileN,u8 *FNUM);
extern u8 SysFile_CloseFile(void);
extern u8 SysFile_OpenFile(u8 *FileN,u8 *FNum);


#endif

