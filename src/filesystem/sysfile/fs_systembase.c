

#include "fs.h"

strSysFsPra SysFsMtdPra = 
{
	"/mtd0",
    4096,
    0x0058000,
    0x14000,   // 1M的文件系统,加快上电速度
    0, //后面程序自动计算
    0,//后面程序自动计算
    //方法
    dev_flash_read,
    dev_flash_write,  //write 函数里包含 erase 方法
    dev_flash_erasesector,//擦除的大小等于 BLen
    dev_flash_writespec
};



strSysFsPra* SysFsHdPra = &SysFsMtdPra;

static u8 SysFsUpdatFlag = 0;//关闭文件系统时是否需要更新，0 不需要，1 需要


#define BlockLen            ((SysFsHdPra)->BLen)     //块长度
#define HDStrAdd            ((SysFsHdPra)->StrAdd)   //硬盘起始地址
#define SYSFILE_BLOCKNUM    ((SysFsHdPra)->BlockNum) //文件系统总块数
#define SYSFILE_HDENDADD    ((SysFsHdPra)->EndAdd)   //系统硬盘结束地址  

#define MaxBNum             (SYSFILE_BLOCKNUM - 5)    //一个文件所用块数  
#define MaxFL               (BlockLen*MaxBNum)       //文件最大长度



struct _FileSysB   FileSysB;  //文件名块数据结构
struct _FileSysP   FileSysP;  //文件系统参数

//===================================================================
u16 SysFile_CCU16(u8 *pb)  //   // 2位uint8转换为uin16
{
    u16 d;

    d=*pb;
    pb++;
    d=d*0x100+*pb;

    return d;
}
//==================================================================
void SysFile_U16CC(u8 *pb,u16 d) //  //2位uint8转换为uin16
{
	
	*pb=d>>8;
	pb++;
	*pb=d>>0;
	pb++;
	
}
//=================================================================
u32 FS_CCU32(u8 *pb)           ////4位uint8转换为uin32
{
    u32 i;
    u32 d;
    d=0;
    for(i = 0; i < 4; i++)
    {
        d = d*0x100 + *(pb+i);
    }
    return d;
}
//==================================================================
void U32CC(u8 *pb,u32 d)   ////4位uint8转换为uin32
{
	
    *pb=d>>24;
    pb++;
    *pb=d>>16;
    pb++;
    *pb=d>>8;
    pb++;
    *pb=d>>0;
    pb++;
	
}
//=============================================================
//函数功能:   比较两数是否相等 ，
//入口参数:   uint16 *PX1,uint16 *PX2 ，uint32 Len
//返 回 值:   0:PX1=PX2  1:PX1>PX2 -1:PX1<PX2
/*
static u8 DataComp(u8 *PX1,u8 *PX2,u16 Len)
{
    u16 i;
    for(i=0; i<Len; i++)
    {
        if((*PX1)>(*PX2)) return BIG; //大于
        if((*PX1)<(*PX2)) return LIT; //小于
        PX1++;
        PX2++;
    }
    return EQU;  //相等
}
*/
//=======================================================================
//            比较一数组是否为等同一个数
//入口参数:   uint16 *PX,uint16 DF ，uint32 Len
//返 回 值:   0:PX = DF  1:PX > DF 2:PX < DF
static u8 DataCompOne(u8 *PX, u8 DF, u16 Len)
{
    u16 i;
    for(i=0; i<Len; i++)
    {
        if((*PX)>(DF)) return BIG; //大于
        if((*PX)<(DF)) return LIT; //小于
        PX++;
    }
    return EQU;  //相等
}
//======================================================================
//                   计算异或校验
//输入:Da 数据,DLen,数据长度
//输出:
//返回:char的异或
static u8 CountCheckSum(u8 *Da,u16 DLen)
{
    u8 UCS;
    u16 i;
    UCS = 0;
    for(i = 0; i < DLen ; i++)
    {
        UCS ^= *(Da+i);
    }
    return UCS;
}


/****************************************************************************
**Description:           
**Input parameters:  
**Output parameters: 
**                   
**Returned value:        
**                   
**Created by:
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
static void SysFile_UpdataFlag_Set(void)
{
    SysFsUpdatFlag = 1;
}


//-------------- 提取一些系统参数块原子操作----写------------

/****************************************************************************
**Description:      记录最近写的块号
**Input parameters:
**Output parameters:
**
**Returned value: 0 成功
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static s32 SysFile_RBlock_Set(u16 blocknum) // 20160815
{
	
    SysFile_U16CC(FileSysB.RBlock, blocknum);
    FsDebugLog("\r\n--->>>>>logs L1: 最近修改块:%d---\r\n", blocknum);
    SysFile_UpdataFlag_Set();
    return 0;
	
}
//--- 块参数操作 ---
/****************************************************************************
**Description:      设置一个块的状态
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by: 
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static s32 SysFile_Block_SetStatus(u16 blocknum, u16 status, u8 cs)  // 20160815
{
    blocknum = blocknum & 0x7fff;

    if(blocknum >= SYSFILE_BLOCKNUM)
    {
        FsDebug("---操作块超出系统范围\r\n");
        return -1;
    }

    SysFile_U16CC(FileSysB.Block[blocknum].NextBlack, status);
    FileSysB.Block[blocknum].CS = cs;
    //FsDebug("---操作块状态:0x%02x 0x%02x--\r\n", FileSysB.Block[blocknum].NextBlack[0],1);
    FsDebugLog("\r\n--->>>>>logs L1: 修改块:%d, s:0x%04x, cs:0x%02x---\r\n", blocknum, status, cs);
    SysFile_UpdataFlag_Set();
    return 0;
}

/****************************************************************************
**Description:      初始化文件索引
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static s32 SysFile_File_Init(u8 FNum, u8* FName) // 20160815
{
    u16 len;

    len = strlen((s8 *)FName);
    if((len == 0)||(len > 30))
    {
        FsDebug("FoundFile:文件名出错!\r\n");
        return LST;
    }

    memset(&FileSysB.File[FNum].FileF[0], 0, FileNLen);
    SysFile_U16CC(FileSysB.File[FNum].FileF, FileFlag);      // 标志
    strcpy((s8 *)FileSysB.File[FNum].Name, (s8 *)FName);         // 文件名
    FileSysB.File[FNum].Name[len] = 0;              //
    SysFile_U16CC(FileSysB.File[FNum].StartLink,UNBlock);   // 链接块    0xffff     未分配空间

    FileSysB.File[FNum].CS = CountCheckSum(&FileSysB.File[FNum].FileF[0],FileNLen - 1);// 异或校验

    FsDebugLog("\r\n--->>>>>logs L1: 初始化文件:%d, %s---\r\n", FNum, FName);
    SysFile_UpdataFlag_Set();
    return SOK;
}

//============================================================================
//功能:修改文件名
//输入:filename Newfilename文件名
//输出:FNUM文件号 >= FileNum  //文件不存在
//返回:0:成功 其它:失败代码
//设计: heyi
u8 SysFile_AmendFileNameFnum(u8 FNum, u8 *NewFileName)   // 20160815
{
    FsDebugLog("\r\n\r\n--->>>>>logs L2: AmendFileNameFnum---\r\n");
    memset(FileSysB.File[FNum].Name,0,sizeof(FileSysB.File[FNum].Name));
    strcpy((s8 *)FileSysB.File[FNum].Name, (s8 *)NewFileName);         // 文件名
    // memcpy(FileSysB.FileName[Fnum].dateTime,&Sys.Year,6);     // YY MM DD hh mm ss 最近操作时间
    FileSysB.File[FNum].CS = CountCheckSum(&FileSysB.File[FNum].FileF[0],FileNLen - 1);// 校验和
    FsDebugLog("\r\n--->>>>>logs L1: 修改文件名:%d, %s---\r\n", FNum, NewFileName);
    SysFile_UpdataFlag_Set();
    return SOK;
}

/****************************************************************************
**Description:      删除文件索引
**Input parameters:
**Output parameters:
**
**Returned value:  0
**
**Created by: 
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static s32 SysFile_File_Clr(u8 FNum)   // 20160815
{
    memset(&FileSysB.File[FNum].FileF[0], 0xff, FileNLen);//删除文件
    FsDebugLog("\r\n--->>>>>logs L1: 删除文件索引:%d---\r\n", FNum);
    SysFile_UpdataFlag_Set();
    return 0;
}

/****************************************************************************
**Description:      设置系统块号
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by: 
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static s32 SysFile_Set_BN(u16 blocknum)   // 20160815
{
    SysFile_U16CC(FileSysB.FileBN, blocknum);
    FsDebugLog("\r\n--->>>>>logs L1: 设置BN:%04x---\r\n", blocknum);
    SysFile_UpdataFlag_Set();
    return 0;
}

static s32 SysFile_Set_BBN(u16 blocknum)   // 20160815
{
    SysFile_U16CC(FileSysB.FileBBN, blocknum);
    FsDebugLog("\r\n--->>>>>logs L1: 设置BBN:%04x---\r\n", blocknum);
    SysFile_UpdataFlag_Set();
    return 0;
}

/****************************************************************************
**Description:      计算系统参数块的校验和，并设置
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static s32 SysFile_SetCS(void)  // 20160815
{
    u8 *PX;

    PX = &FileSysB.FileBF[0];
    FileSysB.CS = CountCheckSum(PX, FileSysBLen -1);
    FsDebugLog("\r\n--->>>>>logs L1: 设置校验和:%02x---\r\n", FileSysB.CS);
    SysFile_UpdataFlag_Set();
    return 0;
}

/****************************************************************************
**Description:      清除内存中的文件系统
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static s32 SysFile_Clr(void)  // 20160815
{
    memset(&FileSysB.FileBF[0], 0xff, FileSysBLen);
    FsDebugLog("\r\n--->>>>>logs L1: 清除整个文件系统---\r\n");
    SysFile_UpdataFlag_Set();
    return 0;
}

/****************************************************************************
**Description:      设置文件的起始块状态
**Input parameters:  StrLink 文件的起始块，如果为0XFFFF,说明是空文件
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static s32 SysFile_Starlink_Set(u8 FNum, u16 StrLink)  // 20160815
{
    SysFile_U16CC(FileSysB.File[FNum].StartLink, StrLink);
    FsDebugLog("\r\n--->>>>>logs L1: 设置文件起始块:%d, %04x---\r\n", FNum, StrLink);
    SysFile_UpdataFlag_Set();
    return 0;
}

/****************************************************************************
**Description:      设置文件长度
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static s32 SysFile_File_SetLen(u8 FNum, u32 Len)  // 20160815
{
    //设置文件的长度
    U32CC(FileSysB.File[FNum].FileLen, Len);
    FsDebugLog("\r\n--->>>>>logs L1: 设置文件长度:%d, %04x---\r\n", FNum, Len);
    SysFile_UpdataFlag_Set();
    return 0;

}


/****************************************************************************
**Description:      设置文件索引的校验
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static s32 SysFile_File_SetCs(u8 FNum)  // 20160815
{
    FileSysB.File[FNum].CS = CountCheckSum(&FileSysB.File[FNum].FileF[0], FileNLen - 1);// 校验和
    FsDebugLog("\r\n--->>>>>logs L1: 设置文件校验:%d, %02x---\r\n", FNum, FileSysB.File[FNum].CS);
    SysFile_UpdataFlag_Set();
    return 0;
}
/****************************************************************************
**Description:       次数自加
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static s32 SysFile_WdateTime_Add(void)  // 20160815
{
    dev_maths_bcd_add(FileSysB.WdateTime ,"\x00\x00\x00\x00\x00\x01", 6); //块使用次数加1
    FsDebugLog("\r\n--->>>>>logs L1: 修改系统操作计数---\r\n");
    SysFile_UpdataFlag_Set();
    return 0;
}
//-------------- 提取一些系统参数块原子操作----读------------
//============================================================================
//功能:得到文件的长度
//输入: 文件号
//返回: 文件长度
//u32 GetFileLength(u8 Fnum)
u32 SysFile_File_GetLen(u8 Fnum)   // 20160815
{
    if(Fnum >= FileNum)
    {
        return -1;
    }
    return FS_CCU32(FileSysB.File[Fnum].FileLen);
}

/****************************************************************************
**Description:      判断指定的文件索引下是否有文件
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
u16 SysFile_File_GetFlag(u8 FNum)   // 20160815
{
    return SysFile_CCU16(FileSysB.File[FNum].FileF);
}

/****************************************************************************
**Description:      获取文件的起始块号
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static u16 SysFile_File_GetHead(u8 FNum)   // 20160815
{
    return SysFile_CCU16(FileSysB.File[FNum].StartLink);
}
/****************************************************************************
**Description:      获取最近写的块号
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static u16 SysFile_RBlock_Get(void)   // 20160815
{
    return SysFile_CCU16(FileSysB.RBlock);
}

/****************************************************************************
**Description:      获取一个块的状态
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static u16 SysFile_Block_GetStatus(u16 blocknum)   // 20160815
{
    return SysFile_CCU16(FileSysB.Block[blocknum].NextBlack);
}

/****************************************************************************
**Description:      获取一个块的校验和
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static u8 SysFile_Block_GetCS(u16 blocknum)   // 20160815
{
    return FileSysB.Block[blocknum].CS;
}

/****************************************************************************
**Description:      读取整个系统参数块的校验
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static u8 SysFile_GetCS(void)   // 20160815
{
    return FileSysB.CS;
}

/****************************************************************************
**Description:      读取当前系统块号
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by: 
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static u16 SysFile_Get_BN(void)   // 20160815
{
    return SysFile_CCU16(FileSysB.FileBN)&0x7fff;
}

/****************************************************************************
**Description:      读取系统备份块号
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by: 
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static u16 SysFile_Get_BBN(void)   // 20160815
{
    return SysFile_CCU16(FileSysB.FileBBN)&0x7fff;//最后一个操作 FileSysB 接口
}

//-----------------------------------------------------------
//=======================================================================
//功能: flash 块擦除
//输入: LBNum 块号
//反回: 文件操作代码
static u8 SysFile_EraseFileSysBlock(u16 LBNum)   // 20160815
{
    u8 Re;
    u32 i;
    u32 AD;

    Re = SOK;
    LBNum = LBNum & 0x7fff;
    if(LBNum >= SYSFILE_BLOCKNUM)
        return AER;

    AD = HDStrAdd + (u32)LBNum * BlockLen;   //flash 地址
    if((AD < HDStrAdd)||(AD > (SYSFILE_HDENDADD - BlockLen)))
        return AER;

    for(i = 0; i < 3; i++)
    {

        FsDebug("---块擦除LBNum:%u i:%u...",LBNum,i);

        (SysFsHdPra)->erase(AD);
        Re= SOK;

        if(Re == SOK)
        {
            break;
        }
    }

    if(Re == SOK)
    {
        FsDebug("成功\r\n");
    }
    else
    {
        FsDebug("失败\r\n");
        SysFile_Block_SetStatus(LBNum, BadBlock, 0xff);
    }

    return Re;
}

/****************************************************************************
**Description:      向文件系统指定位置直接写数据，不进行擦除
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/

static u8 SysFile_WriteFileSysData(u16 LBNum, u32 shift, u8 *Bdata, u16 LBLen)  // 20160815
{
    u32 wadd;
    u32 endadd;
	int ret;

    wadd = HDStrAdd + (LBNum * BlockLen) + shift; //计算地址flash 地址
    endadd = wadd + LBLen;
    if((wadd < HDStrAdd)||(endadd > (SYSFILE_HDENDADD)))
    {
        return AER;
    }  //地址出错

   
    
    ret = (SysFsHdPra)->write(wadd, Bdata,  LBLen);

	if (0 != ret)
	{
		FsDebug("IFLASH写失败\r\n");
	}
	
    return SOK;
}
//=======================================================================
//功能:写块数据
//输入:LBNum:块号 ,Bdata:要写的数据, LBLen:要写的数据长度
//返回:0:成功 其它:失败代码
static u8 SysFile_WriteFileSysBlock(u16 LBNum, const u8 *Bdata,u16 LBLen)  // 20160815
{
    u8 Re;//;,i
    u32 time;//写次数，三次不成功则退出
    u16 WD;
    u32 WLen;
    u32 WAdd;
	//s32 ret=0;
    LBNum = LBNum & 0x7fff;
    if(LBNum >= SYSFILE_BLOCKNUM)//不能大于硬盘块数
        return AER;

    if(LBLen > BlockLen)//限制长度，一块只有4K
        WLen = BlockLen;
    else
        WLen = LBLen;

    WAdd = HDStrAdd + (LBNum * BlockLen); //计算地址flash 地址
    if((WAdd < HDStrAdd)||(WAdd > (SYSFILE_HDENDADD-BlockLen)))
    {
        return AER;
    }  //地址出错

    for(time=0; time<3; time++)
    {
		FsDebug("---写块数据LBNum:%u,LBLen:0x%04x,time:%u ...", LBNum, LBLen, time);

		// (SysFsHdPra)->write(Bdata, WAdd, WLen);

		//直接写，由底层来完成擦除的动作
		(SysFsHdPra)->write( WAdd,(u8 *)Bdata, WLen);//按照u32一字节  ret = 
		Re = SOK;
		
        if(Re == SOK)
            break;
    }

    if(Re == SOK)
    {
        FsDebug("Re:%u \r\n",Re);
    }
    else
    {
        //写失败，认为本块是坏块
        FsDebug("失败\r\n");

        WD = BadBlock;
        SysFile_Block_SetStatus(LBNum, WD, 0xff);
    }

    return Re;
}



static u8 SysFile_WriteFileSysBlockspec(u16 LBNum, const u8 *Bdata,u16 LBLen)  // 20160815
{
    u8 Re;//;,i
    u32 time;//写次数，三次不成功则退出
    u16 WD;
    u32 WLen;
    u32 WAdd;
	//s32 ret=0;
    LBNum = LBNum & 0x7fff;
    if(LBNum >= SYSFILE_BLOCKNUM)//不能大于硬盘块数
        return AER;

    if(LBLen > BlockLen)//限制长度，一块只有4K
        WLen = BlockLen;
    else
        WLen = LBLen;

    WAdd = HDStrAdd + (LBNum * BlockLen); //计算地址flash 地址
    if((WAdd < HDStrAdd)||(WAdd > (SYSFILE_HDENDADD-BlockLen)))
    {
        return AER;
    }  //地址出错

    for(time=0; time<3; time++)
    {
		FsDebug("---写块数据LBNum:%u,LBLen:0x%04x,time:%u ...", LBNum, LBLen, time);

		// (SysFsHdPra)->write(Bdata, WAdd, WLen);

		//直接写，由底层来完成擦除的动作
		(SysFsHdPra)->write( WAdd,(u8 *)Bdata, WLen);//按照u32一字节  ret = 
		Re = SOK;
		
        if(Re == SOK)
            break;
    }

    if(Re == SOK)
    {
        FsDebug("Re:%u \r\n",Re);
    }
    else
    {
        //写失败，认为本块是坏块
        FsDebug("失败\r\n");

        WD = BadBlock;
        SysFile_Block_SetStatus(LBNum, WD, 0xff);
    }

    return Re;
}


//=======================================================================
//功能:读块数据
//输入:LBNum:块号 ,Bdata:要写的数据, LBLen:要写的数据长度
//返回:0:成功 其它:失败代码
static u8 SysFile_ReadFileSysBlock(u16 LBNum,u8 *Bdata,u16 LBLen) // 20160815
{
    //u32 a,b;

    u16 RLen;
    u32 RAdd;
    //int j;

    if(LBLen == 0)
        return SOK;

    LBNum = LBNum & 0x7fff;
    if(LBNum >= SYSFILE_BLOCKNUM)
    {
        return AER;
    }

    if(LBLen > BlockLen)
    {
        RLen = BlockLen;
    }
    else
    {
        RLen = LBLen;
    }

    RAdd = HDStrAdd + (u32)LBNum * BlockLen;  //flash 地址
    if((RAdd < HDStrAdd)||(RAdd > (SYSFILE_HDENDADD - BlockLen)))
    {

        FsDebug("!!!---读块数据错误LBNum:%d\r\n",LBNum);

        return AER;
    }  //地址出错

    //SpiFlashFastReadMoreByteData(Bdata, RAdd, RLen);
    (SysFsHdPra)->read( RAdd, Bdata,RLen);
    return SOK;
	
}
//=======================================================================
//功能:查找下一块可用的块号
//返回 :0xffff:没有可用块 / <0xffff:块号
//遍历并且比较，在块数多的时候可能会比较费时间
static u16 SysFile_LookupUseBlock(void)  // 20160815
{
    u32 i,j;
    u16 BN;
    u16 blockstatus;

    BN = SysFile_RBlock_Get();

    BN = BN & 0x7fff;
    i = BN & 0x7fff;//能不能直接等于BN?

    j = 0;

    while(1)
    {
        i++;
        j++;

        if( i >= SYSFILE_BLOCKNUM)//轮询硬盘上的块
        {
            i = 0;
        }

        blockstatus = SysFile_Block_GetStatus(i);

        if(blockstatus == UNBlock)
        {
            break;
        } //可用块

        if((i == BN)||(j >= SYSFILE_BLOCKNUM))
        {
            i = 0xffff;//找不到空闲块
            break;
        }
    }

#ifdef Test_SysPrint
    if(i == 0xffff)
    {
        FsDebug("!!!---无可用块\r\n");
    }
    else
    {
        FsDebug("---可用块号:%d \r\n",i);
    }
#endif

    return (i|0x8000);//返回块号，无可用块则返回0XFFFF. 为什么要或上0x8000
}
//============================================================================
//功能:获得文件系统参据
//
//输出:
//返回:0:成功 其它:失败代码
u8 SysFile_AcquireFileSysParameter(void)   // 20160815
{
    u32 i;
    u16 Num;

    FileSysP.PrepaidSpace = 0;   //已用空间(数据块)
    FileSysP.DoSpace = 0;        //可用空间(数据块)
    FileSysP.BadSpace = 0;       //已坏空间(数据块)
    FileSysP.PrepaidFileNum = 0; //已用文件数
    FileSysP.DoFileNum = 0;      //可用文件数

    FsDebug("--------AcquireFileSysParameter-----\r\n");

    for(i = 0; i < SYSFILE_BLOCKNUM; i++)
    {
        Num = SysFile_Block_GetStatus(i);

        if(Num == UNBlock)
        {
            FileSysP.DoSpace++;
        }
        else if(Num == BadBlock)
        {
            FileSysP.BadSpace++;
        }
        else
        {
            FileSysP.PrepaidSpace++;
        }
    }

    FileSysP.PrepaidSpace *= BlockLen;   //已用空间
    FileSysP.DoSpace      *= BlockLen;   //可用空间
    FileSysP.BadSpace     *= BlockLen;   //已坏空间

    for(i = 0; i < FileNum; i++)
    {
        Num = SysFile_File_GetFlag(i);
        if(Num == FileFlag)
        {
            FileSysP.PrepaidFileNum++;//已用文件数
        }
        else
        {
            FileSysP.DoFileNum++;       //可用文件数

        }
    }
    return SOK;
}
//=======================================================================
//功能:判断数据是否为文件系统数据
//返回: 0:不是文件系统数据 1:目录数据 2:链表数据
/*
char JudgeDataIsFileSys(u16 BN)
{
   u16 j,CS;
   char Re;
   Re = NSD;
   CS = 0xffff;

   if(DataCompOne(&FileSL.FileSB.FileBF[0],FileBFlag,12)== EQU)  //文件名
   {
      Re = ReadFileSysBlock(BN,FileSL.Buf,4096);  //读取12个字节的头数据
      if(Re != SOK)
      {
            return NSD;
      }
      CS = 0;
      for(j = 0;j< (FileSysBLen -4);j++) CS += FileSL.Buf[j];
      if((CS == SysFile_CCU16(FileSL.FileSB.CS)) &&(SysFile_CCU16(FileSL.FileSB.CS1) == (u16)(CS+CS)))
      {
         Re = ISF;
      }
   }

   return Re;
}
*/
/****************************************************************************
**Description:          将一块系统参数快删除(只要删除标识就可以了)
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by: 
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static u8 SysFile_SysFileDelSysBlock(u16 LBNum)  // 20160815
{
    u8 Re;
    //u32 j;
    u32 i;
    u16 WD;
    u32 AD;
    u8 data[12];
	//s32 ret =0;

    memset(data, 0x00, sizeof(data));
    Re = SOK;

    LBNum = LBNum & 0x7fff;
    if(LBNum >= SYSFILE_BLOCKNUM)
        return AER;

    AD = HDStrAdd + (u32)LBNum * BlockLen;   //flash 地址
    if((AD < HDStrAdd)||(AD > (SYSFILE_HDENDADD - BlockLen)))
        return AER;

	//直接擦块，测试已写的块直接写会写错误 2015.10.27 caoliang
  (SysFsHdPra)->erase(AD);  //ret = 
//  SpiFlashWrite(data, AD, 12);
    Re = SOK;

    if(Re != SOK)//如果请标识不成功，则进行块擦除
    {
        FsDebug("!!!---改系统块标识失败LBNum:%u\r\n",LBNum);

        for(i = 0; i < 3; i++)
        {
            FsDebug("---块擦除LBNum:%u i:%u \r\n",LBNum,i);

            (SysFsHdPra)->erase(AD);
            Re = SOK;

            if(Re == SOK)
                break;
        }
    }
    else
    {}

    if(Re == SOK)
    {
        FsDebug("---删除系统块...成功。LBNum:%u\r\n",LBNum);

        WD = UNBlock;
    }
    else
    {
        FsDebug("---删除系统块...失败。LBNum:%u\r\n", LBNum);

        WD = BadBlock;
    }

    SysFile_Block_SetStatus(LBNum, WD, 0xff);

    return Re;
}

//============================================================================
//功能:获得文件使用的全部块号
//输入:FNum文件号
//输出:FBN块号列表 BNum块数 FLen文件长度
//返回:0:成功 其它:失败代码
static u8 SysFile_FileBlackLish(u8 FNum, u16 *FBN, u16 *BNum, u32 *FLen)  // 20160815
{
    u8 Re;
    u16 BN;
    u16 FBNum;
    u32 Len, len_tmp;

    Re = SOK;

    if(SysFile_File_GetFlag(FNum) != FileFlag)
    {

        FsDebug("!!!!!!--FileBlackLish:错误文件!\r\n");

        Re = NNF;
    }  //不是正常的文件
    else
    {
        Len = 0x00;
        FBNum = 0x00;
        BN = SysFile_File_GetHead(FNum); //开始块号

        while(1)
        {
            if(BN == BadBlock)
            {
                FsDebug("!!!!!---FileBlackLish:文件指向坏块!\r\n");

                Re = NNF;
                break;
            } //坏块

            if(BN == UNBlock)
            {
                //找到文件尾巴
                //或者是一个新文件，空的
                FsDebug("!!!!!---FileBlackLish:空文件!\r\n");
                break;
            }                   //未用块

            BN = BN & 0x7fff;    //块号
            *(FBN + FBNum) = BN;   //块号
            FBNum++;               //块数

            //下一块号，如果本块没用完，得到的将是本块已经使用的字节数
            BN = SysFile_Block_GetStatus(BN);
            if(BN < 0x8000)  //结束块，小于0X8000，说明本快没用完
            {
                Len += BN;   //(文件)数据长度
                FsDebug("!!!!!---FileBlackLish:最后一块 %d!\r\n", BN);
                break;
            }

            Len += BlockLen;
        }

        if(Re == SOK)
        {
            len_tmp = SysFile_File_GetLen(FNum);
            if(Len != len_tmp)//文件长度出错
            {
                FsDebug("!!!!!!!--FileBlackLish:校验文件长度错误%d, %d!\r\n",Len, len_tmp);
            }
            else
            {}
        }
    }

    *FLen = Len;    //文件长度
    *BNum = FBNum;  //块数
    return Re;
}
//============================================================================
//功能:清除文件数据
//输入:文件号
//返回:0:成功 其它:失败代码
static void SysFile_ClearFileData_Block(u8 FNum)  // 20160815
{
    u16 BN,BN1;
    u8 FN;

    if(FNum >= FileNum)
    {
        FsDebug("删除文 文件号错误\r\n");
        return;
    }

    FN = FNum;
    BN = SysFile_File_GetHead(FN);
    while(1)
    {
        if(SysFile_File_GetFlag(FN)!= FileFlag)
        {
            FsDebug("删除文件数据失败:1\r\n");
            break;
        }  //不是正常的文件

        if((BN == BadBlock)||(BN == UNBlock))
        {

            FsDebug("删除文件:坏块，未使用块\r\n");

            break;
        }               //未用块/坏块

        BN = BN & 0x7fff;
        //求下一块或本块长度)
        BN1 = SysFile_Block_GetStatus(BN);
        //设置为未用块
        SysFile_Block_SetStatus(BN, UNBlock, 0xff);

        if(BN1 < 0x8000)//最后一块?
        {
            SysFile_Starlink_Set(FNum, UNBlock);
            break;    //结束
        }

        BN = BN1;//继续处理下一块文件块
    }

    FsDebug("删除文件结束\r\n");

    return;
}

/****************************************************************************
**Description:      打印文件系统信息，用于测试     
**Input parameters:  
**Output parameters: 
**                   
**Returned value:        
**                   
**Created by:
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
void SysFile_Show_detai(void)  // 20160815
{

#if 1//def Test_SysPrint
    u32 i;
    u16 tmp;
    
    VfsDebug("开机获取文件系统结束\r\n");
    VfsDebug("\r\n--------------- 文件系统---------\r\n");
    VfsDebug("起始地址:%08x\r\n", HDStrAdd);
    VfsDebug("结束地址:%08x\r\n", SYSFILE_HDENDADD);
    VfsDebug("块数:%d\r\n", SYSFILE_BLOCKNUM);
    VfsDebug("块大小:%x\r\n", BlockLen);
    VfsDebug("文件列表:\r\n");

    for(i = 0; i < FileNum; i++)
    {

        if(SysFile_File_GetFlag(i) == FileFlag)
        {
            VfsDebug("文件号:%d, %s, %02x%02x,%02x%02x%02x%02x\r\n", i, FileSysB.File[i].Name,
                    FileSysB.File[i].StartLink[0], FileSysB.File[i].StartLink[1],
                    FileSysB.File[i].FileLen[0],FileSysB.File[i].FileLen[1],FileSysB.File[i].FileLen[2],FileSysB.File[i].FileLen[3]);
        }
        else
        {
            VfsDebug("文件号:%d, 未使用\r\n", i);
        }
    }

    FsDebug("块列表:\r\n");
    for(i = 0; i < SYSFILE_BLOCKNUM; i++)
    {

        tmp = SysFile_Block_GetStatus(i);

        FsDebug("(%04d, 0x%04x) ", i, tmp);

        if(tmp == UNBlock)
        {
            FsDebug("未使用\r\n");
        }
        else if(tmp == BadBlock)
        {
            FsDebug("坏块\r\n");
        }
        else if(tmp == LogBlock)
        {
            FsDebug("**日志块\r\n");
        }
        else if(tmp <= BlockLen)
        {
            FsDebug("本快已使用\r\n");
        }
        else if(tmp == BlockLen + FileSysBLen)
        {
            FsDebug("文件系统块\r\n");
        }
        else if(tmp > 0X8000)
        {
            FsDebug("用完，链到%d\r\n", tmp - 0X8000);
        }
        else
        {
            FsDebug("未知\r\n");
        }
    }
    FsDebug("\r\n--------------- 文件系统---------\r\n\r\n");
#endif

}

//=======================================================================
//功能:开机获得文件系统数据
//返回:0:成功 其它:失败代码
//sxl   找到文件目录列表，并且根据文件目录数据来
//还原文件目录链表,没有用到的块要回收
u8 SysFile_AcquireFileSysData(void)  // 20160815
{
    u8 Re,RSD,mFlag;  //RS,RL,
    u32 i,j,BN,k;
    struct _TEMPSYSFILEINFO tempsysfileinfo[6];
    struct _TEMPSYSFILEINFO temp;

    u8 tempbcddata[16];
    int tempfileno = 0;
    u16 CS;
    u16 tempblockaddr1,tempblockaddr2,BLKNo;

    union _FileSysLink *FSL;

    //u16 tmp;

    FsDebugLog("\r\n\r\n--->>>>>logs L2: AcquireFileSysData---\r\n");
    //FsDebug("\r\n指向存储空间上有%d个文件系统，目前指向第%d个文件系统\r\n", XgdMtdNum, XgdMtdIndex);
    FsDebug("\r\n地址%08x，结束地址%0x 块总数%d\r\n", HDStrAdd, SYSFILE_HDENDADD,SYSFILE_BLOCKNUM);

    if((FileSysBLen+2)>BlockLen)//文件结构不能大于一个数据块，并且，块长度不能大于0X8000(32K)
    {
        while(1)
        {
            FsDebug("文件系统结构数据长度:%04超长\r\n", FileSysBLen);
        }
    }

    FSL = (union _FileSysLink *)k_malloc(sizeof(union _FileSysLink));
    if(FSL == NULL)
    {
        while(1)
        {
            FsDebug("内存申请失败 AcquireFileSysData\r\n");
        }
    }

    //RS = NFS;
    //RL = NFS;

    for(i = 0; i < SYSFILE_BLOCKNUM ; i++) //搜寻所有块，查询系统参数块
    {
        BN=i;
        Re = SysFile_ReadFileSysBlock(BN, FSL->Buf, 12); //读取12个字节的头数据
        if(Re == SOK)  //读数据成功
        {
            RSD = NSD;
            CS = 0xffff;
            if(DataCompOne(&FSL->FileSB.FileBF[0], FileBFlag, 12)== EQU)  //文件名，12个5A开头
            {
                //已经找到文件系统的参数块，将其整块读出

                FsDebug("\r\n找到系统文件块LBNum:%u\r\n",BN);

                Re = SysFile_ReadFileSysBlock(BN, FSL->Buf, FileSysBLen);//读取整块系统参数块，

                if(Re == SOK)
                {
                    CS = 0;
                    CS = CountCheckSum(&FSL->Buf[0],FileSysBLen - 1);//计算校验值，使用异或校验
                    if(CS == FSL->FileSB.CS)
                    {
                        RSD = ISF;
                    }
                    else
                    {
                        FsDebug("系统参数块校验和不对\r\n");
                    }
                    //每次都拷贝，那么这里只会是最后一次的数据而已校验不过也拷贝?

                    FsDebug("拷贝系统文件块LBNum:%u\r\n",BN);

                    memcpy(&FileSysB.FileBF[0],&FSL->Buf[0],FileSysBLen);//将系统参数读到内存(全局)中，方便操作
                }

            }

            if(RSD != ISF)
            {
                continue;
            }
        }
        else
        {
            continue;
        }
        //如何处理超过5个文件系统块的BUG?
        if((RSD == ISF) && (tempfileno <5))//文件目录数据不会超过5个，实际使用中，应该只有两个
        {
            tempsysfileinfo[tempfileno].BLKNo = BN;//暂时记住文件系统块号
            memcpy(tempsysfileinfo[tempfileno].WdateTime, FSL->FileSB.WdateTime, 6);//将块写次数放到临时信息中

            FsDebug("BLOCK:%d WdateTime----\r\n", BN);
            //sys_debug_format_u8(FSL->FileSB.WdateTime, 6);

            tempfileno++;
        }
    }
    FsDebug("处理系统块\r\n");
    //对读到的所有文件系统块进行排序
    j = 0;//tempfileno 肯定小于 5
    while(j < tempfileno)   //还原LINK链表,只找一个有用的
    {
        for(i =(j + 1); i< tempfileno; i++)//把最大的数换到j组
        {
            mFlag = 0;
            if((memcmp(tempsysfileinfo[j].WdateTime, tempsysfileinfo[i].WdateTime, 6))>0)//j指向的块的使用次数 > i指向的块
            {
                memcpy(tempbcddata, tempsysfileinfo[j].WdateTime, 6);
                dev_maths_bcd_sub(tempbcddata, tempsysfileinfo[i].WdateTime, 6); //tempbcddata = j - i   //    j   i   i  i  i  i
                if(memcmp(tempbcddata, "\x00\x10\x00\x00\x00\x00", 6) > 0) //(j - i) > 1000000000  //0   97  5   99 98 7  6     97- 5(j-i) = 92 > 50  要换
                {
                    //   //1   5   97  99      99- 5(i-j) = 94 > 50  不要换
                    mFlag = 1; //计数器已循环 //2 5  98  98- 5(i-j) = 93 > 50 不要换
                }   //  //3   5   7  7 - 5(i-j) = 2  < 50  要换
            }                                                   //  //4   7             5  6     7 - 6(j-i) = 1  < 50  不要换
            else //j <= i  //5   7   97  99 98 5  6     //最后排列
            {
                memcpy(tempbcddata,tempsysfileinfo[i].WdateTime,6);
                dev_maths_bcd_sub(tempbcddata,tempsysfileinfo[j].WdateTime,6);      //tempbcddata = i - j
                if(memcmp(tempbcddata,"\x00\x10\x00\x00\x00\x00",6) < 0) //(i - j) < 1000000000
                {
                    mFlag = 1;                                            //计数还未循环
                }
            }

            if(mFlag == 1)   //第j组数是小数要调换
            {
                FsDebug("文件参数块调换\r\n");

                temp.BLKNo = tempsysfileinfo[j].BLKNo;   //暂保j组数据
                memcpy(temp.WdateTime,tempsysfileinfo[j].WdateTime,6);

                tempsysfileinfo[j].BLKNo = tempsysfileinfo[i].BLKNo;      //调换
                memcpy(tempsysfileinfo[j].WdateTime,tempsysfileinfo[i].WdateTime,6);

                tempsysfileinfo[i].BLKNo = temp.BLKNo;
                memcpy(tempsysfileinfo[i].WdateTime,temp.WdateTime,6);
            }
        }


        FsDebug("文件参数块数:%d\r\n", tempfileno);
        if(tempfileno < 2)
        {
            //说明系统块有问题，可能被串改了，或者是没有建立文件系统
            FsDebug("警告:文件系统块不足，请注意\r\n");

        }

        BN = tempsysfileinfo[j].BLKNo;//获取最新的块

        FsDebug("使用文件参数块:%d\r\n", BN);
        //经过上面的调整，再读一次
        Re = SysFile_ReadFileSysBlock(BN, FSL->Buf, FileSysBLen);
        if(Re == SOK)   //这里不再考虑读文件错的情况了
        {
            CS = CountCheckSum(&FSL->Buf[0], FileSysBLen - 1);//再次计算校验，为什么?
            if(CS == FSL->FileSB.CS)
            {
                memcpy(&FileSysB.FileBF[0], &FSL->Buf[0], FileSysBLen);
                //删除原来没有删除的块
                //整个文件系统，只会有两块文件系统块，因此，在检测文件系统的时候，
                //必须释放，以免造成错乱
                tempblockaddr1 = SysFile_Get_BN();
                tempblockaddr2 = SysFile_Get_BBN();
                FsDebug("--删除多余系统块---");
                for(i = 0; i< tempfileno; i++)
                {
                    if(tempsysfileinfo[i].BLKNo != tempblockaddr1 &&
                            tempsysfileinfo[i].BLKNo != tempblockaddr2)
                    {
                        BLKNo = tempsysfileinfo[i].BLKNo;
                        SysFile_SysFileDelSysBlock(BLKNo);//.删除原来的系统参数块
                        {

                        }
                    }
                }

                //为了对新老兼容，通过判断日志块是否存在，决定是否申请日志块
                #ifdef LOG_MODE
                SysFile_Logs_Found();
                //系统块校验通过，那么就跟系统日志比较，看是否是最新的系统参数，如果不是
                //进行恢复操作
                #endif
                
#if 0
                //到这里，整个文件系统已经检测结束，文件系统中只会有两块文件系统参数块
                //更新文件系统块
                Re = RenewalFileSys(0);//将内存中的文件系统参数保存到FLASH中,并初始化一些参数
#else
                Re = SOK;
#endif
                if(Re == SOK)
                {
                    tempblockaddr1 = SysFile_Get_BN();
                    tempblockaddr2 = SysFile_Get_BBN();

                    for(k = 0; k< SYSFILE_BLOCKNUM; k++)
                    {
                        //回收一些文件块 标识是 (BlockLen + FileSysBLen)
                        //这个地方可能有移植隐患，NextBlack的取值范围，是人工划分的
                        if(SysFile_Block_GetStatus(k) == (BlockLen + FileSysBLen)
                                &&(k != tempblockaddr1)
                                &&(k != tempblockaddr2)) //文件名/链表块
                        {
                            FsDebug("回收块数:%d\r\n", k);

                            SysFile_Block_SetStatus(k, UNBlock, 0xff);

                        }
                    }
                }


#if 0 /*黄学佳 2014-1-29*/
                //求出当前系统参数块号
                tempblockaddr1 = SysFile_CCU16(FileSysB.FileBN) & 0x7fff;
                tempblockaddr2 = CCU16(FileSysB.FileBBN)& 0x7fff;
                //将块标识置成(BlockLen + FileSysBLen),说明是文件系统块
                SysFile_Block_SetStatus(tempblockaddr1, BlockLen + FileSysBLen, 0xff);
                SysFile_Block_SetStatus(tempblockaddr2, BlockLen + FileSysBLen, 0xff);
#endif

                //更新系统参数,V30已经修复的BUG,2012.12.22 黄学佳，add
                SysFile_AcquireFileSysParameter();
                SysFile_Show_detai();
                //释放内存
                k_free(FSL);
                return Re;
            }
            else
            {
                //如果LOG存在，需要进行恢复
                //不用报错，因为后面还会处理其他的系统参数块，例如备份的参数块。
            }
        }
        //--查询下一块
        j += 1;
        //还原LINK表
    }

    SysFile_Clr();
    //释放内存
    k_free(FSL);
    return NFS;  //未创建文件系统

}

//=======================================================================
//功能:更新文件系统
//返回:0:成功 其它:失败代码
//修改:不保存LINK块LINK块要通过文件块数据来计算
// 增加一个参数，用于决定是否创建备份文件系统块
// 1 创建
static u8 SysFile_RenewalFileSys(u8 mode)  // 20160815
{
    u8 Re;   //,Re1,Re2
    //u8 *PX;
    u8 cs;
    u16 SaFileBN,SaFileBBN;
    u16 WBN;
    int SaveFileSuc = 0;
    //union _FileSysLink FSL;
    u16 NextB;

    FsDebugLog("\r\n\r\n--->>>>>logs L2: RenewalFileSys %d---\r\n", mode);

    //暂保原来的块号
    SaFileBN = SysFile_Get_BN();
    SaFileBBN = SysFile_Get_BBN();

    FsDebug("\r\n--------------更新文件系统------------------\r\n");
    FsDebug(" File Sys Block Len:%04x\r\n", FileSysBLen);
    FsDebug("---目前系统参数块号:%d, 备份块:%d\r\n", SaFileBN, SaFileBBN);

    Re = SER;
    //Re1 = SER;
    //Re2 = SER;

    while(1)
    {
        Re = SER;
        //Re1 = SER;
        //--获取一个空闲块作为文件系统块
        NextB = SysFile_LookupUseBlock();//获取下一块块号
        if(NextB == 0XFFFF)
        {
            FsDebug("无 空闲块!\r\n");
            break;
        }
        //设置系统块号
        SysFile_Set_BN(NextB);
        SysFile_RBlock_Set(NextB & 0x7fff);
        //---
        WBN = SysFile_Get_BN();

        memset(&FileSysB.FileBF[0], FileBFlag, 12);  //为链表块标志
        SysFile_WdateTime_Add();//块使用次数加1

        //将原来的块做为备份块
        SysFile_Set_BBN(SaFileBN);
        //修改备份块的块状态，设置为空闲块
        SysFile_Block_SetStatus(SaFileBBN, UNBlock, 0xff);//修改块状态

        //修改申请的块的状态
#if 0
        cs = CountCheckSum(&FileSysB.FileBF[0],FileSysBLen);
#else
        cs = 0xff;
#endif

        //这个有个死锁问题，因此，对于系统块了说，块校验和是没用的
        //只有后面的系统块校验和有用，块校验和只有文件才有用
        SysFile_Block_SetStatus(WBN, BlockLen + FileSysBLen, cs);

        //计算校验和
        SysFile_SetCS();

        //--将内存中的系统参数块写到刚刚申请的块内
        //memcpy(FSL.Buf, &FileSysB.FileBF[0], BlockLen);//为什么要拷贝一次?

        Re = SysFile_WriteFileSysBlock(WBN, &FileSysB.FileBF[0], FileSysBLen);  //写文件块

        if(Re == SOK)
        {
#if 0
            //写成功，修改内存中的系统参数，指明刚刚写的块是一个系统参数块
            //为什么要写完了，才修改内存中的参数?
            //这个地方的逻辑关系还要好好考虑。。。。，暂时放到写之前
            //cs = CountCheckSum(&FileSysB.FileBF[0],FileSysBLen);
            //SysFile_Block_SetStatus(WBN, BlockLen + FileSysBLen, cs);
#endif

            //如果没有来的及删掉原来的链表
            if(SaFileBN != SysFile_Get_BN())//这个应该是百分百不等于，因为重新分配块了
            {
                //SysFileDelSysBlock(SaFileBN);//优化程序，只要擦除系统参数块标识就可以了，不用整块擦除，擦除要花比较多时间
                {

                }
                //如果换成新方案，那么就不是擦除SaFileBN块，而是擦除更早的一块系统参数块 备份块
                #ifdef LOG_MODE
                SysFile_SysFileDelSysBlock(SaFileBBN);
                #else
                SysFile_SysFileDelSysBlock(SaFileBN);
                #endif
            }

            SaveFileSuc  = 1;//更新成功
            break;
        }
        else//写失败，不断尝试，只有当分配不到可用块的时候才退出
        {
            //-- 这里不用处理，写失败，块会被置为坏块
            continue;
        }
    }
    //仅仅在初始化文件系统的时候才会创建备份块
    #ifndef LOG_MODE
    mode = 1;
    FsDebug("\r\n\r\n\r\n------------旧模式!--------\r\n\r\n\r\n");
    #endif
    
    while((SaveFileSuc == 1) && (mode == 1))
    {
        FsDebug("\r\n\r\n\r\n------------创建备份块!--------\r\n\r\n\r\n");

        Re = SER;
        //Re2 = SER;
        //==============分配空间=================
        NextB = SysFile_LookupUseBlock();//获取下一块块号
        if(NextB == 0XFFFF)
        {
            FsDebug("无 空闲块!\r\n");
            break;
        }

        //设置系统备份块号
        SysFile_Set_BBN(NextB);
        SysFile_RBlock_Set(NextB & 0x7fff);

        //==========文件名块备份==============
        memset(&FileSysB.FileBF[0],FileBFlag,12);   //为链表块标志

        //这个计数好像是每次写文件系统参数块就会更新
        SysFile_WdateTime_Add();//块使用次数加1

        //调换块号，前面写的作为备份块
        WBN = SysFile_Get_BN();
        SysFile_Set_BN(NextB);
        SysFile_Set_BBN( WBN|0x8000);
        //-------------------
        WBN = SysFile_Get_BN();
        //修改申请的块的状态
#if 0
        cs = CountCheckSum(&FileSysB.FileBF[0],FileSysBLen);
        //这个有个死锁问题，因此，对于系统块了说，块校验和是没用的
        //只有后面的系统块校验和有用，块校验和只有文件才有用
#else
        cs = 0XFF;
#endif
        SysFile_Block_SetStatus(WBN, BlockLen + FileSysBLen, cs);

        //计算校验和
        SysFile_SetCS();

        //---------写数据---------------
        //memcpy(FSL.Buf,&FileSysB.FileBF[0],BlockLen);

        Re = SysFile_WriteFileSysBlock(WBN, &FileSysB.FileBF[0], FileSysBLen);
        if(Re == SOK)
        {
            //cs = CountCheckSum(&FileSysB.FileBF[0],FileSysBLen);
            //SysFile_Block_SetStatus(WBN, BlockLen + FileSysBLen, cs);
            FsDebug("写备份块成功...");
            if(SaFileBBN != WBN)
            {
                //优化程序，只要擦除系统参数块标识就可以了，不用整块擦除，擦除要花比较多时间
                SysFile_SysFileDelSysBlock(SaFileBBN);
                FsDebug("删除原来的备份块...%d...", SaFileBBN);
            }
            break;
        }
        else //如果第二次写，要把内存中，主参数块跟备份参数块的块号换回去，
            //然后再尝试。失败的话要换回去
        {
            NextB = SysFile_Get_BN();
            WBN = SysFile_Get_BBN();

            SysFile_Set_BN(WBN);
            SysFile_Set_BBN(NextB);
            continue;
        }
    }

    FsDebug("准备更新参数...\r\n");

    SysFile_AcquireFileSysParameter();//更新系统参数
    SysFile_Show_detai();
#ifdef Test_SysPrint
    if(Re != SOK)
    {
        FsDebug("失败\r\n");
    }
    else
    {
        FsDebug("Re:%u\r\n",Re);
    }
    FsDebug("-------------------end-------------------\r\n");
#endif

    return Re;
}
//======================================================================
//功能:创建文件系统
//如果原来存在文件系统，则为重新格式化
//返回:0:成功 其它:失败代码
u8 SysFile_FoundFileSys(void)  // 20160815
{
    u32 i;

    FsDebugLog("\r\n\r\n--->>>>>logs L2: 建立文件系统---\r\n");

    //将内存中的文件系统块，全部置成0XFF
    //前面18个字节，有12个字节是标识，0X5A,
    memset(&FileSysB.RBlock[0], 0xff, FileSysBLen-18);

    for(i = 0; i < 6; i++)
    {
        //如果开始没有文件系统，wdateTime会是什么值，这里为什么要做判断?
        //10进制?
        if(((FileSysB.WdateTime[i] & 0xf0) > 0x90) || ((FileSysB.WdateTime[i] & 0x0f) > 0x09))
        {
            FsDebug("reset WdateTime: %d\r\n",i);
            memset(&FileSysB.WdateTime[0],0x00,6);  //数据置 0x00
            break;
        }
    }

    for(i = 0; i < SYSFILE_BLOCKNUM; i++)
    {
        if(SysFile_EraseFileSysBlock(i) == SOK)
        {
            SysFile_Block_SetStatus(i, UNBlock, 0xff);
            FsDebug("Erase File Sys Block sui: %u\r\n",i);

        }
        else
        {

            FsDebug("Found File Sys Err i: %u\r\n",i);

        }
    }

    //建立文件系统的时候，取最后一块作为最近写的块号
    SysFile_RBlock_Set(SYSFILE_BLOCKNUM);
    
    #ifdef LOG_MODE
    //为了对新老兼容，通过判断日志块是否存在，决定是否申请日志块
    SysFile_Logs_Found();
    #endif
    
    return SysFile_RenewalFileSys(1) ;  //更新文件系统
}
/*

//--------------------------- 对外接口-----------------------------------


*/
//============================================================================
//功能:查找文件(打开文件)
//入口: FileN:文件名
//输出: FNum 文件号 >= FileNum  //文件不存在
//返回: 0:成功 其它:失败代码
u8 SysFile_OpenFile(u8 *FileN,u8 *FNum)  // 20160815
{
    u8 Re;
    u32 i;
    Re = NFS;

    //是否需要判断文件系统是否存在?
    {
        Re = NFN;
        for(i = 0; i < FileNum; i++)
        {
            if((strcmp((s8 *)FileSysB.File[i].Name, (s8 *)FileN) == 0)
                    &&(SysFile_File_GetFlag(i)==FileFlag))
            {
                *FNum = i;//文件号

                Re = SOK;
                break;
            }            //找到文件
        }
    }

    if(Re == NFN)
    {
        FsDebug("打开文件OpenFile:%s:不存在\r\n",FileN);
    }
    else
    {

    }

    return Re;   //找不到文件
}
//============================================================================
//功能:关闭文件(更新文件系统)
//返回:0:成功 其它:失败代码
u8 SysFile_CloseFile(void)  // 20160815
{
    u8 ret;
    
    FsDebugLog("\r\n\r\n--->>>>>logs L2: CloseFile---\r\n");

    if(SysFsUpdatFlag == 0)
    {
        //FsDebugLog("不需要更新文件系统\r\n");
        //uart_printf("不需要更新文件系统\r\n");
        return 0;
    }

    ret = SysFile_RenewalFileSys(0);
    
    SysFsUpdatFlag = 0;//清更新标识，连续调用CLOSE，不会直接重写一遍。
    return ret;
}
//============================================================================
//功能:创建一个文件
//输入:FileN文件名
//输出:FNum文件号 >= FileNum  //文件不存在
//返回:0:成功 其它:失败代码
u8 SysFile_FoundFile(u8 *FileN,u8 *FNUM)  // 20160815
{
    u8 Re;
    u32 Flag;
    u32 fn;
    u32 i;
    u16 Len;

    FsDebugLog("\r\n\r\n--->>>>>logs L2: FoundFile---\r\n");

    Len = strlen((s8 *)FileN);
    if((Len == 0)||(Len > 30))
    {
        FsDebug("FoundFile:文件名太长!\r\n");
        return LST;
    }

    fn = FileNum + 1;
    Flag = 0;

    for(i = 0; i < FileNum; i++)
    {
        if(SysFile_File_GetFlag(i)== FileFlag)
        {
            if(strcmp((s8 *)FileSysB.File[i].Name, (s8 *)FileN) == 0) //找到文件(已创建了文件)
            {
                fn = i;
                Flag = 1;
                break;
            }
        }
        else
        {
            if(fn > FileNum)
            {
                //这里应该只会进来一次，这里的意思是将第一个
                //无效的文件名空间当做将要使用的空间，
                fn = i; //第一个空的文件号
            }
        }
    }

    if((Flag == 0)&&(fn < FileNum))   //文件不存在 && 还有空文件索引
    {

        Re = SysFile_File_Init(fn, FileN);//不会失败
        Re = SOK;

        FsDebug("FoundFile:成功!文件号:%d\r\n", fn);

    }
    else if(Flag == 1)
    {

        FsDebug("FoundFile:文件已存在!文件号:%d\r\n", fn);

    }
    else
    {

        FsDebug("FoundFile:目录满!文件号:%d\r\n", fn);

        Re = FNF;
    }

    *FNUM = fn;

    FsDebug("\r\n-------------------------------\r\n");

    return Re;
}

//============================================================================
//功能:清除文件数据
//输入:文件号
//返回:0:成功 其它:失败代码
void SysFile_ClearFileData(u8 FNum)  // 20160815
{

    FsDebugLog("\r\n\r\n--->>>>>logs L2: ClearFileData---\r\n");
    SysFile_ClearFileData_Block(FNum);
    SysFile_File_SetLen(FNum, 0);//清除文件内容后，要将文件长度设置为0
    return;
}
//============================================================================
//功能:删除一个文件
//输入:文件号
//返回:0:成功 其它:失败代码
u8 SysFile_DeleteFileNum(u8 FNum)  // 20160815
{
    FsDebugLog("\r\n\r\n--->>>>>logs L2: DeleteFileNum---\r\n");

    if(FNum >= FileNum)
    {
        return NNF;
    }

    SysFile_ClearFileData(FNum);

    SysFile_File_Clr(FNum);

    return SysFile_CloseFile();
}

/****************************************************************************
**Description:      功能:写文件数据
**Input parameters: FNum文件号,StrA位置,WD数据,DLen 数据长度
**Output parameters:
**
**Returned value:   0:成功 其它:失败代码
**
**Created by:

注:起始地址要在原来文件中，如果从最后一个地址写，则是追加，黄学佳 2012.12.22
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
u8 SysFile_WriteFileData(u8 FNum,u32 StrA, u8 *WD,u32 DLen) // 20160815
{
    u8 Re,Rf;
    u16 i;

    u16 *FileBNum;
    u16 WBN,RBN;                      //当前操作块号
    u16 j;                            //当前操作块号是第j块
    u16 BNum;                          //该文件所用的块数
    u16 WBSA;                         //块开始写地址
    u32 FLen;                         //文件长度
    u32 WAdd;                //写地址
    u32 WDLen;                        //要写的数据长度
    u8 CS;

    u8 need_erase = 0;//是否需要擦除标识
    //u16 changdatalen;

    union _FileSysBlockBuf *FSL;

	if(FNum>=FileNum)
	{
		return SER;
	}
    
    
    FSL = (union _FileSysBlockBuf *)k_malloc(sizeof(union _FileSysBlockBuf));
    if(FSL == NULL)
    {
        while(1)
        {
            FsDebug("内存申请失败 WriteFileData\r\n");
        }
    }
    
    FileBNum = (u16 *)k_malloc(sizeof(u16)*(MaxBNum));
    if(FileBNum == NULL)
    {
        FsDebug("内存申请失败 ReadFileData\r\n");
        while(1)
        {}
    }
    
    FsDebugLog("\r\n\r\n--->>>>>logs L2: WriteFileData---\r\n");
    FsDebug("\r\n**********************************\r\n");
    FsDebug("WriteFileData FNum : %u StrA = 0x%08x  ", FNum, StrA);
    FsDebug("Datalen = 0x%08x \r\n", DLen);



    WAdd = StrA;
    WDLen = DLen;
    if(WDLen == 0)
    {
        Re = SOK;
        goto END;
    }

    BNum = 0;
    FLen = 0;

    memset(FileBNum, 0xffff, MaxBNum);

    FsDebug("---未使用空间 %08x \r\n", FileSysP.DoSpace);//如果连续写，这个判断不会起作用。

    if(FileSysP.DoSpace < 6 * BlockLen)//6是什么意思? 预留两块用于备份，其余呢?
    {
        Re = RNE;   //数据空间不够

        FsDebug("!!!!!!!!----数据空间不够写文件空间 %08x %08x %08x", FNum, StrA, FileSysP.DoSpace);

        goto END;
    }

    Re = SysFile_FileBlackLish(FNum, FileBNum, &BNum, &FLen);
    if(Re != SOK)
    {
        FsDebug("!!!!!!!!----获取文件块号失败!\r\n");
        goto END;
    }

    if((WAdd > FLen)||(BNum >= MaxBNum)) //写数据超过文件尾，文件块数也不能大于等于硬盘块数(应该不会发生)
    {
        Re = NNF;
        goto END;
    }

    memset(FSL->Buf, 0xff, FileSysBlock_BUFSIZE);//

    while(1)
    {
        FsDebug("写一块数据----");

        if(WDLen==0)//写文件结束
        {
            break;
        }

        WBSA = 0;

        j = WAdd/BlockLen;      //要写的数据在文件的第j块

        if(j < BNum)           //要写的数据在原来文件中
        {
            WBN = FileBNum[j];      //当前写的块块号
            WBSA = WAdd%BlockLen;   //当前写的块的偏移数

            need_erase = 0;
		
            FsDebug("---不需要增加块");

            if(WBN < SYSFILE_BLOCKNUM)   //从原来的块中先读数据,本判断用于防止意外
            {
                Re = SysFile_ReadFileSysBlock(WBN, FSL->Buf,FileSysBlock_BUFSIZE);   //读取当前块数据
                if(Re != SOK)
                {
                    goto ConverFile;
                }                  //读数据出错
            }
            else
            {
				
            }
			
        }
        else   //要申请新块
        {
            if(BNum <(MaxBNum-1))
            {
                FileBNum[BNum] = 0xffff;       //下一块号
                BNum++ ;                        //块数
                //有没有办法判断新申请的块是否可以不擦除?
                memset(FSL->Buf,0xff,FileSysBlock_BUFSIZE);
                need_erase = 1;//先设定为需要擦除

                FsDebug("---需要申请块...");

            }
            else
            {
                goto ConverFile;
            }
        }

        //将数据写到RAM中的缓存中

        FsDebug("填充缓冲...");

        i = WBSA;//在块中的偏移  如果申请了新块，则为0

        FsDebug("偏移 %d...", i);
		
        while(1)
        {
            
            if(need_erase == 0)  
            {
                //uart_printf("[%02x %02x] ", FSL.Buf[i], *WD);
                if(FSL->Buf[i] != (*WD))
                {
                    need_erase = 1;
                }
			}

            //将数据拷贝到RAM中的缓冲块内
            FSL->Buf[i] = *WD;
            WD++ ;
            WAdd++;
            i++;
            WDLen--;

            //printf("%d...", i);

            if((WDLen==0)||(i >= BlockLen))
            {
                break;
            }
        }

        FsDebug("OK...");

        //changdatalen = i - WBSA;//本次修改(添加的数据),用于直接写入的时候

        //求出本块使用的数据长度
        if(j < (BNum-1))//不是最后一块，要计算全部的值
        {
            i = BlockLen;//整块使用
        }
        else   //最后一块比较复杂
        {
            if(WAdd < FLen)
            {
                if(FLen >0 && (FLen % BlockLen == 0))
                {
                    i = BlockLen;
                }
                else
                {
                    i =  FLen%BlockLen;
                }
            }
        }
		
		
        if(0 == need_erase)
        {
            //all data the same,not need write
            FsDebug("does not need write:...");
			continue;
        }

        if(1 == need_erase)
        {
            FsDebug("擦除，直接从0 开始写:%d...", i);
            //写数据
            while(1)
            {
                WBN = SysFile_LookupUseBlock();       //获取一块用于写
                if(WBN == 0xffff)//找不到可用块
                {
                    FsDebug("----找不到可用块---");
                    goto ConverFile;
                }

                WBN = WBN & 0x7fff; //块号
                //是否可以不写一整块?
                //Re = WriteFileSysBlock(WBN,FSL.U16Buf,BlockLen); //写数据//sxl 1127
                Re = SysFile_WriteFileSysBlockspec(WBN,FSL->Buf,FileSysBlock_BUFSIZE);//只写用到的字节//sxl20151102
                if(Re == SOK)
                {
                    break;
                }     //写数据成功
            }
        }

        //如果数据块没有换，就不要更换最近写的块，块轮询，应该是说最近擦除的块，不应该是写
        if(1 == need_erase)
        {
            //记录最近写的块号
            SysFile_RBlock_Set(WBN);
            //--只有有申请新款的时候，才会改变链条
            if(j == 0)//文件第一块(文件创建)
            {
                //更新开始块号
                SysFile_Starlink_Set(FNum, WBN|0x8000);
            }
            else
            {
                RBN = FileBNum[j - 1];   //前面一块块号记录本块的块号

                CS = SysFile_Block_GetCS(RBN);
                SysFile_Block_SetStatus(RBN, WBN|0x8000, CS);

            }
        }

#if 0 /*黄学佳 2014-1-28*/
        FileSysB.Block[WBN].CS = CountCheckSum(FSL.Buf,i); //计算块校验:异或校验
        FsDebug("---计算校验和的长度:%d, 块号:%d, CS:%d\r\n", i, WBN, FileSysB.Block[WBN].CS);
#endif

#if 0 /*黄学佳 2014-1-29*/
        if(j == 0)//文件第一块(文件创建)
        {
            SysFile_U16CC(FileSysB.File[FNum].StartLink,WBN|0x8000); //更新开始块号
        }
        else
        {
            RBN = FileBNum[j - 1];   //前面一块块号记录本块的块号

            CS = SysFile_Block_GetCS(RBN);
            SysFile_Block_SetStatus(RBN, WBN|0x8000, CS);

        }
#endif

        //设置本块状态
        //sys_debug_format_u8(FSL->Buf, i);
        CS = CountCheckSum(FSL->Buf,i);//计算块校验:异或校验
        FsDebug("计算校验和的长度:%d, 块号:%d, CS:0x%02x...", i, WBN, CS);

        if(j < (BNum-1))
        {
            //当前操作块不是最后一块
            //记录下一块的块号
            FsDebug("非最后一块，修改块链接与校验和\r\n");
            SysFile_Block_SetStatus(WBN, FileBNum[j+1]|0x8000, CS);
        }
        else
        {
            //最后一块
            ////数据的长度
            FsDebug("最后一块，修改长度与校验和\r\n");
            SysFile_Block_SetStatus(WBN, i, CS);
        }


        if(FileBNum[j] < SYSFILE_BLOCKNUM)
        {
            //释放原来的块??????
            if(WBN != FileBNum[j])//当前写的块不是原来的块,把原来的块释放
            {

                SysFile_Block_SetStatus(FileBNum[j], UNBlock, 0XFF);

            }
        }

        FileBNum[j] = WBN;

        if(WDLen==0)//写结束
        {
            break;
        }
    }
    //========写数据成功=======================
    if(WAdd > FLen)
    {
        //文件长度
        FsDebug("修改文件长度\r\n");
        SysFile_File_SetLen(FNum, WAdd);//设置文件的长度
    }
    else
    {
        FsDebug("不需要修改文件长度\r\n");
    }

    SysFile_File_SetCs(FNum);

    goto END;

ConverFile: //如果链表没有建起来要还原成原来的链表，
//如果是一个文件最佳数据，，数据都被擦了，恢复链表何用?
    FsDebug("链表建立失败\r\n");

    RBN = SysFile_Get_BN();

    Rf = SysFile_ReadFileSysBlock(RBN,FSL->Buf,FileSysBlock_BUFSIZE);
    if(Rf == SOK)
    {
        memcpy(&FileSysB.FileBF[0], FSL->Buf, FileSysBLen);

        CS = CountCheckSum(&FSL->Buf[0],FileSysBLen-1);
        if(CS == SysFile_GetCS())
        {
            goto END;
        }
        else
        {

        }
    }
    else
    {
        FsDebug("系统恢复失败\r\n");
    }

END:
    k_free(FSL);
    k_free(FileBNum);
    FsDebug("写文件数据end:%u Re:%u \r\n",FNum,Re);
    FsDebug("\r\n**********************************\r\n");

    return Re;
}

//===========================================================================
//功能:读文件数据
//输入:FNum文件号,StrA位置,DLen 要读的数据长度
//输出:RD数据,DLen 读取的数据长度
//返回:0:成功 其它:失败代码
u8 SysFile_ReadFileData(u8 FNum,u8 *RD,u32 StrA,u32 *DLen) // 20160815
{
    u8 Re;
    u8 CS;
    u16 i,CsLen;
    
    //u16 FileBNum[FileSysBlockNum+20]; //该文件所用的块号
    u16 *FileBNum;
    u16 RBN;                          //当前操作块号
    u16 j;                            //当前操作块号是第j块
    u16 BNum;                         //该文件所用的块数
    u16 RBSA;                         //块开始读地址
    u32 FLen;                         //文件长度
    u32 RAdd;                         //写地址
    u32 RDLen;                        //要读的数据长度
    u32 OutLen;
    union _FileSysBlockBuf *FSL;

    OutLen = 0;   //读取数据长度

    FsDebug("ReadFileData len :%d\r\n", *DLen);

    if((*DLen) == 0)
        return SOK;

    FSL = (union _FileSysBlockBuf *)k_malloc(sizeof(union _FileSysBlockBuf));
    if(FSL == NULL)
    {
        FsDebug("内存申请失败 ReadFileData\r\n");
        while(1)
        {}
    }

    FileBNum = (u16 *)k_malloc(sizeof(u16)*(MaxBNum+20));
    if(FileBNum == NULL)
    {
        FsDebug("内存申请失败 ReadFileData\r\n");
        while(1)
        {}
    }

    BNum = 0;    //sxl 1117
    FLen = 0;
    memset(FileBNum,0xffff, MaxBNum);

    Re = SysFile_FileBlackLish(FNum,FileBNum,&BNum,&FLen);

    if(FLen == 0)
    {
        FsDebug("ReadFileData file end\r\n");
        Re = FEnd;
    }
    
    if(Re == SOK)
    {
        RAdd = StrA;
        RDLen = *DLen;
        Re = FEnd;
        while(1)
        {
            if(RDLen==0)
                break;

            j = RAdd/BlockLen;      //要读的数据在文件的第j块
            if(j >= BNum || RAdd >=FLen)   //sxl 1117
            {
                Re = FEnd;
                break;
            }
            
            RBN = FileBNum[j];      //当前读的块块号
            RBSA = RAdd%BlockLen;   //当前读的块的偏移数
            
            FsDebug("ReadFileData block :%d\r\n", RBN);
            
            Re = SysFile_ReadFileSysBlock(RBN,FSL->Buf,BlockLen);//读取一块数据, 无论如何都读一块，耗时
            if(Re == SOK )
            {
                FsDebug("ReadFileData block :OK\r\n", RBN);
                CsLen = SysFile_Block_GetStatus(RBN);
                if(CsLen >= BlockLen)
                    CsLen = BlockLen;
                    
                //sys_debug_format_u8(FSL->Buf, CsLen);
                
                CS = CountCheckSum(FSL->Buf,CsLen);//

                //拷贝数据
                for(i = RBSA; i < BlockLen; i++)
                {
                    *RD = FSL->Buf[i];
                    RD++ ;
                    RAdd++;
                    RDLen--;
                    OutLen++;
                    if(RDLen==0)
                        break;

                    if(RAdd >= FLen)
                    {
                        Re = FEnd;
                        break;
                    }
                }
                if(CS != SysFile_Block_GetCS(RBN))
                {
                    Re = FILECSR;    //数据校验出错
                    //sys_debug_format_u8(FSL->Buf, CsLen);
                    
                    FsDebug("ReadFileData1(块数据校验出错):CS:%04x %04x Re:%u \r\n",CS, SysFile_Block_GetCS(RBN), Re);

                }
            }
            
            if(Re != SOK )
            {
                FsDebug("ReadFileData re:%d\r\n", Re);
                break;
            }
        }
    }
    *DLen = OutLen;

    k_free(FSL);
    k_free(FileBNum);
    //printf("ReadFileData1:CS:%04x %04x Re:%u \r\n",CS,SysFile_CCU16(FileLinkB.Block[RBN].CS),Re);  //sxl 测试用，要删除
    return Re;
}

/****************************************************************************
**Description:         校验整个文件系统  
**Input parameters:  
**Output parameters: 
**                   
**Returned value:        
**                   
**Created by:
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
s32 SysFile_verify(void)  // 20160815
{
    u8 Re;
    u8 CS;
    u16 i,CsLen;
    u16 *FileBNum;
    //u16 RBN;                          //当前操作块号
    //u16 j;                            //当前操作块号是第j块

    union _FileSysBlockBuf *FSL;
    u16 tmp;

    FSL = (union _FileSysBlockBuf *)k_malloc(sizeof(union _FileSysBlockBuf));
    if(FSL == NULL)
    {
        FsDebug("内存申请失败 ReadFileData\r\n");
        while(1)
        {}
    }

    FileBNum = (u16 *)k_malloc(sizeof(u16)*(MaxBNum+20));
    if(FileBNum == NULL)
    {
        FsDebug("内存申请失败 ReadFileData\r\n");
        while(1)
        {}
    }

    FsDebug("----------------------------\r\n");
    
    for(i = 0; i < SYSFILE_BLOCKNUM; i++)
    {

        tmp = SysFile_Block_GetStatus(i);

        FsDebug("(%04d, 0x%04x) ", i, tmp);

        if(tmp == UNBlock)
        {
            FsDebug("未使用--");
        }
        else if(tmp == BadBlock)
        {
            FsDebug("坏块--");
        }
        else if(tmp == LogBlock)
        {
            FsDebug("**日志块--");
        }
        else if(tmp <= BlockLen)
        {
            FsDebug("本快已使用---");
        }
        else if(tmp == BlockLen + FileSysBLen)
        {
            FsDebug("文件系统块---");
        }
        else if(tmp > 0X8000)
        {
            FsDebug("用完，链到%d---", tmp - 0X8000);
        }
        else
        {
            FsDebug("未知--");
        }
     
        Re = SysFile_ReadFileSysBlock(i,FSL->Buf,BlockLen);//读取一块数据, 无论如何都读一块，耗时
        if(Re == SOK )
        {
            FsDebug("OK--");
            CsLen = SysFile_Block_GetStatus(i);//块数据长度
            if(CsLen >= BlockLen)
                CsLen = BlockLen;
            FsDebug("%d--", CsLen);    
            //sys_debug_format_u8(FSL->Buf, CsLen);
            CS = CountCheckSum(FSL->Buf,CsLen);//

            if(CS != SysFile_Block_GetCS(i))
            {
                Re = FILECSR;    //数据校验出错

                FsDebug("cs err\r\n");

            }
            else
            {
                FsDebug("cs ok\r\n");    
            }
        }
        else
        {
            FsDebug("ERR--\r\n");
            break;
        }
    }


    k_free(FSL);
    k_free(FileBNum);
    //printf("ReadFileData1:CS:%04x %04x Re:%u \r\n",CS,SysFile_CCU16(FileLinkB.Block[RBN].CS),Re);  //sxl 测试用，要删除
    return Re;
}

//-----------------增加系统日志的操作-------------------
#ifdef LOG_MODE

#define SysFile_Log_MAX 64//一个块最多32条LOG

struct _FileSysLog
{
    u8 Flag[4];//log,表明这个LOG空间是否真的有一个LOG
    u8 type;//操作类型，请看宏定义
    u8 cs;//本条LOG的校验和
};
typedef  struct _FileSysLog SysFileLog;

struct _FileSysLogB
{
    u8 Flag[16];//"xgdfs_logs.log",字符串标识
    u8 SysBT[6];//系统块的写次数，用于LOG跟系统块对照
};


typedef enum
{
    LogL2_WriteFileData,
    LogL2_RenewalFileSys,
} LogL2Type;

typedef enum
{
    LogL1_,
} LogL1Type;

//每个LOG的长度都是定值
struct _sLogDetail
{
    u8 name[32];
    u8 len;
};


#define XGDFS_LOGB_FLAG "xgdfs_logs.log"//日志块标识

static u16 LogsBlock1 = 0XFFFF;//旧
static u16 LogsBlock2 = 0XFFFF;//新

/****************************************************************************
**Description:      将一个指定的块初始化为LOG块
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static s32 SysFile_Logs_init(u16 blocknum)
{
    u8 Re;
    s32 ret;

    struct _FileSysLogB FileSysLogB;

    u16 cs;

    FsDebugLog("\r\n\r\n--->>>>>logs L2: 创建日志:%04x---\r\n", blocknum);

    if(sizeof(struct _FileSysLogB) >= BlockLen)
    {
        while(1)
        {
            FsDebug("log block too len!!!");
        }
    }

    //初始化 FileSysLogB
    memset(&FileSysLogB, 0xff, sizeof(FileSysLogB));
    memcpy(&(FileSysLogB.Flag[0]), XGDFS_LOGB_FLAG, strlen(XGDFS_LOGB_FLAG) + 1);

    //将建立LOG块时的文件系统块编号写到LOG块内
    memcpy(&(FileSysLogB.SysBT[0]), &(FileSysB.WdateTime[0]), 6);

    //将 FileSysLogB 写到刚刚分配的块内
    Re = SysFile_WriteFileSysBlock(blocknum, &(FileSysLogB.Flag[0]), 22);//只写用到的字节
    if(Re == SOK)
    {
        //设置块状态
        SysFile_Block_SetStatus(blocknum, LogBlock, 0XFF);//log块不做校验
        FsDebug("...建立文件系统LOG成功 %d...", blocknum);
        ret = 0;
    }
    else
    {
        FsDebug("...建立文件系统LOG失败 %d...", blocknum);
        ret = -1;
    }

    return ret;

}

/****************************************************************************
**Description:      检测一个指定块是否是LOG块
**Input parameters:
**Output parameters: WdateTime 时间戳
**
**Returned value: 0 成功 -1 失败
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
s32 SysFile_Logs_CheckBlock(u16 blocknum, u8 * WdateTime)
{
    u16 tmp;
    u8 Re;
    u8 buff[64];
    s32 ret = -1;

    tmp = SysFile_Block_GetStatus(blocknum);

    if(tmp == LogBlock)
    {
        FsDebug("日志块 %d...", blocknum);
        //读这个LOG块的标识，再次判断
        Re = SysFile_ReadFileSysBlock(blocknum, buff, 22); //读取12个字节的头数据

        if(Re == SOK)  //读数据成功
        {
            //sys_debug_format_u8(buff, 22);

            if(0 == strcmp(buff, XGDFS_LOGB_FLAG))
            {
                FsDebug("标识正确...");
                //sys_debug_format_u8(&buff[16], 6);
                memcpy(WdateTime, &buff[16], 6);
                ret = 0;
            }
            else
            {
                FsDebug("标识错误...");
            }

        }
        else
        {

        }

        if(ret == -1)
        {
            SysFile_Block_SetStatus(blocknum, UNBlock, 0xff);
        }
    }

    return ret;
}


/****************************************************************************
**Description:      检测日志文件，如果无，则建立  本函数在获取文件系统时调用
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
s32 SysFile_Logs_Found(void)
{
    u32 i;

    u16 cnt = 0;
    u16 WBN;
    s32 ret;
    u16 relblocknum;

    u8 WdateTime1[6];
    u8 WdateTime2[6];
    u8 WdateTimet[6];

    //初始化为0
    memset(WdateTime1, 0x00, sizeof(WdateTime1));
    memset(WdateTime2, 0x00, sizeof(WdateTime2));

    FsDebug("\r\n检测日志文件...");

    LogsBlock1 = 0XFFFF;//清空已找到的日子块号
    LogsBlock2 = 0XFFFF;

    for(i = 0; i < SYSFILE_BLOCKNUM; i++)
    {
        relblocknum = 0xffff;

        ret = SysFile_Logs_CheckBlock(i, WdateTimet);
        if(ret == 0)
        {
            //暂未考虑溢出的情况
            if(memcmp(WdateTimet, WdateTime2, 6) >= 0)//WdateTimet比2大
            {
                if(LogsBlock1 != 0XFFFF)
                {
                    FsDebug("已经找到两块...");
                    relblocknum =  LogsBlock1;//将BLOCK 1 号记住，等下释放
                }

                memcpy(WdateTime1, WdateTime2, 6);//丢弃1
                LogsBlock1 = LogsBlock2;
                memcpy(WdateTime2, WdateTimet, 6);
                LogsBlock2 = i;//记住块号
            }
            else
            {

                if(memcmp(WdateTimet, WdateTime1, 6) >= 0)//WdateTimet比2小，但是比1大
                {
                    if(LogsBlock1 != 0XFFFF)
                    {
                        relblocknum =  LogsBlock1;
                    }

                    memcpy(WdateTime1, WdateTimet, 6);
                    LogsBlock1 = i;//记住块号
                }
                else
                {
                    relblocknum =  i;
                }
            }

            //释放多余的块，仅仅修改系统参数释放，真正的LOG块并没有擦
            if(relblocknum != 0xffff)
            {
                FsDebugLog("\r\n\r\n--->>>>>logs L2: 释放多余日志:%d---\r\n", relblocknum);

                FsDebug("释放日志块 :%d...", relblocknum);
                SysFile_Block_SetStatus(relblocknum, UNBlock, 0xff);
            }

            cnt++;

        }
    }
    FsDebug("共有日志块 :%d...", cnt);

    if(cnt == 0)
    {
        //会不会是系统块被乱改了? 可能性多大?
        FsDebug("需要建立日志文件...");

        while(cnt <2)
        {
            WBN = SysFile_LookupUseBlock();
            if(WBN == 0xffff)
            {
                FsDebug("无可用块...");
                break;
            }
            else
            {
                //将一个块设置为日志块
                ret = SysFile_Logs_init(WBN);
                if(ret == -1)
                {
                    FsDebug("log建立失败...");
                    break;
                }
                else
                {
                    FsDebug("log建立成功...");
                    LogsBlock1 = LogsBlock2;
                    LogsBlock2 = i;
                }
            }

            cnt++;

        }
    }
    else if(cnt == 2)
    {
        FsDebug("日志文件OK...");
    }
    else if(cnt >2)
    {
        FsDebug("日志块多余...已修复...");
    }

    FsDebug("检测结束\r\n");
}
/****************************************************************************
**Description:      添加一条LOG
**Input parameters:
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
s32 SysFile_Logs_Add(u8 type, u8 Num, u8 Len, const u8* data)
{
    u8 write_buf[64];
    u8 index = 0;

    write_buf[index++] = 'L';
    write_buf[index++] = 'O';
    write_buf[index++] = 'G';

    memcpy(&write_buf[index], &FileSysB.WdateTime[0], 6);
    index += 6;

    FsDebug("添加一条LOG\r\n");
}
/****************************************************************************
**Description:      查询系统日志
**Input parameters:  Index 日志索引，0-MAX，0为最新
**Output parameters:
**
**Returned value:
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
s32 SysFile_Logs_Check(u32 Index)
{
    if(Index > 256)
    {
        return -1;
    }
}
#endif


/***************************************************************
**Description:      根据系统信息挂载system文件系统
**Input parameters:
**Output parameters:
**
**Returned value: -1 挂载失败  0 挂载成功
**
**Created by: 
**--------------------------------------------------------------
**Modified by:
**Modified by:
***************************************************************/
s32 fs_systembase_mount(void)
{
	u8 ret;

//  #if(EXFS_EXIST == 0)
//	SysFsHdPra = &SysFsMtdPra;
//  #endif
	(SysFsHdPra)->BlockNum = ((SysFsHdPra)->Len)/((SysFsHdPra)->BLen);

	if((SysFsHdPra)->BlockNum > FileSysBlockNum)//不能超出系统块数
	{
		(SysFsHdPra)->BlockNum = FileSysBlockNum;
	}

	(SysFsHdPra)->EndAdd = (SysFsHdPra)->StrAdd + (SysFsHdPra)->Len;

	FsDebug("\r\nBlockNum = %d %08x %08x\r\n",(SysFsHdPra)->BlockNum,(SysFsHdPra)->BLen,(SysFsHdPra)->EndAdd);
	
	ret = SysFile_AcquireFileSysData();//开机获得文件系统
    FsDebug("\r\nSysFile_AcquireFileSysData ret = %d\r\n",ret);
	if(ret != 0)
	{
		SysFile_FoundFileSys();
	}

	return 0;
	
}

