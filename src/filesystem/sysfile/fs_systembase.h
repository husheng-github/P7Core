

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
    //方法
    s32 (*read)( u32 addr,u8* data_buf,u32 len);
    s32 (*write)( u32 addr,u8* data_buf,u32 len);      //write 函数里包含 erase 方法
    s32 (*erase)(u32 addr);//擦除的大小等于 BLen
    s32 (*writespec)( u32 addr,u8* data_buf,u32 len);  //special write mode for write system data
}strSysFsPra;

#define SYSDIRLEN           31                      //系统目录长度

//写死的参数，在块大小是4K的情况下，只能管理50个文件跟690块空间
#define FileNum         25
#define FileSysBlockNum 300//系统参数块能管理的最大块数，基于块大小是4096的情况下
                           //跟实际文件系统的块数不一样，实际文件系统块数小于等于这个值
                           
#define UNBlock       0xffff    //未用块
#define BadBlock      0xff00    //坏块
#define LogBlock      0Xff55    //日志块
#define FileFlag      0x55aa    //有效文件名标志
#define FileBFlag     0x5a      //为文件块标志  长度为12

#define SOK           0x00  //操作成功
#define SER           0x01  //操作失败

#define WFL           0x01  //写数据失败
#define RFL           0x02  //读数据失败
#define NFS           0x03  //未创建文件系统
#define RNE           0x04  //写数据空间不够
#define AER           0x05  //数据地址不对
#define NFN           0x06  //文件还没创建
#define FILECSR           0x07  //数据校验出错
#define NNF           0x08  //文件号不是正确的文件
#define FEnd          0x09  //读文件已到尾
#define FNF           0x0a  //目录文件已满,不能再创建文件!
#define LST           0x0b  //目录文件名太长,不能再创建文件

#define EQU           0   //相等
#define BIG           1   //大于
#define LIT           2   //小于

#define NSD           0//不是文件系统数据 
#define ISF           1//目录数据 
#define ISL           2//链表数据
//===========================================================================
//  Flash 块信息 默认块长度不会大于32K
struct _FileLink
{
   u8 NextBlack[2];     //0x00~BlockLen               为本块已写的长度 ,
                          //BlockLen + FileSysBLen   表示文件系统块 黄学佳 注
                          //0x8000~ 0x8000 + BlockNum   为下一块块号 + 0x8000 ,
                          //0xffff                      未用块 
                          //0xff00                      坏块
   u8 CS;            //一块数据的累加和
};
#define  FileLink sizeof(struct _FileLink)

//===========================================================================
//                       文件数据
struct _FileSys
{
    u8 FileF[2];        //0  0x55AA 为正确的文件
    u8 Name[SYSDIRLEN]; //2  文件名
    u8 StartLink[2];    //33 链接块    0xffff     未分配空间
    u8 FileLen[4];      //35 文件长度  0x00       未分配空间
    u8 CS;              //39 校验和
};                        //Len 40
#define  FileNLen sizeof(struct _FileSys)
//============================================================================
//                    文件名块数据结构
struct _FileSysB
{
    u8              FileBF[12];         //0  0x5a5a 5a5a 5a5a 5a5a 5a5a 5a5a 为文件块标志
    u8              WdateTime[6];       //12 写次数
    u8              RBlock[2];          //18 最近写的块号
    u8              FileBN[2];          //20 文件系统块号
    u8              FileBBN[2];         //22 文件系统备份块号 
    struct _FileSys   File[FileNum];  //24 文件描述头(支持25个文件)
    struct _FileLink  Block[FileSysBlockNum];   //1024 = 24 + 40 *25   //块管理
    u8              CS;                 //1924 = 1024 + 300*3 +1//1块4K的情况下最多能管理690块 2760K

};                                        //len3159 < 4096
#define  FileSysBLen sizeof(struct _FileSysB)
#define _FileSysLink_BUFSIZE 2048
//============================================================================
union _FileSysLink
{
   u8               Buf[_FileSysLink_BUFSIZE];//这样的定义，不方便兼容，这个联合的大小，不能小于一块的长度
                              //目前文件系统只能管理块长度为4K，
   u16              U16Buf[_FileSysLink_BUFSIZE>>1];
   struct _FileSysB   FileSB;    //文件名块数据结构
  // struct _FileLinkB  FileLB;    //文件名块数据结构
};


#define FileSysBlock_BUFSIZE 4096
union _FileSysBlockBuf
{
	
	u8               Buf[FileSysBlock_BUFSIZE];//这样的定义，不方便兼容，这个联合的大小，不能小于一块的长度
	                                                           //目前文件系统只能管理块长度为4K，
	u16              U16Buf[FileSysBlock_BUFSIZE>>1];
	u32              U32Buf[FileSysBlock_BUFSIZE>>2];
  // struct _FileLinkB  FileLB;    //文件名块数据结构
  
};

//============================================================================
//文件系统参数
struct _FileSysP
{
    u32  PrepaidSpace;   //已用空间
    u32  DoSpace;        //可用空间
    u32  BadSpace;       //已坏空间
    u8   PrepaidFileNum; //已用文件数
    u8   DoFileNum;      //可用文件数
}; 
//============================================================================
struct _TEMPSYSFILEINFO
{
    u8    WdateTime[6]; 
    u16     BLKNo;
};
//===========================================================================
extern struct _FileSysB   FileSysB;  //文件名块数据结构
extern struct _FileSysP   FileSysP;  //文件系统参数
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

