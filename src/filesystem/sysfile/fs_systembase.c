

#include "fs.h"

strSysFsPra SysFsMtdPra = 
{
	"/mtd0",
    4096,
    0x0058000,
    0x14000,   // 1M���ļ�ϵͳ,�ӿ��ϵ��ٶ�
    0, //��������Զ�����
    0,//��������Զ�����
    //����
    dev_flash_read,
    dev_flash_write,  //write ��������� erase ����
    dev_flash_erasesector,//�����Ĵ�С���� BLen
    dev_flash_writespec
};



strSysFsPra* SysFsHdPra = &SysFsMtdPra;

static u8 SysFsUpdatFlag = 0;//�ر��ļ�ϵͳʱ�Ƿ���Ҫ���£�0 ����Ҫ��1 ��Ҫ


#define BlockLen            ((SysFsHdPra)->BLen)     //�鳤��
#define HDStrAdd            ((SysFsHdPra)->StrAdd)   //Ӳ����ʼ��ַ
#define SYSFILE_BLOCKNUM    ((SysFsHdPra)->BlockNum) //�ļ�ϵͳ�ܿ���
#define SYSFILE_HDENDADD    ((SysFsHdPra)->EndAdd)   //ϵͳӲ�̽�����ַ  

#define MaxBNum             (SYSFILE_BLOCKNUM - 5)    //һ���ļ����ÿ���  
#define MaxFL               (BlockLen*MaxBNum)       //�ļ���󳤶�



struct _FileSysB   FileSysB;  //�ļ��������ݽṹ
struct _FileSysP   FileSysP;  //�ļ�ϵͳ����

//===================================================================
u16 SysFile_CCU16(u8 *pb)  //   // 2λuint8ת��Ϊuin16
{
    u16 d;

    d=*pb;
    pb++;
    d=d*0x100+*pb;

    return d;
}
//==================================================================
void SysFile_U16CC(u8 *pb,u16 d) //  //2λuint8ת��Ϊuin16
{
	
	*pb=d>>8;
	pb++;
	*pb=d>>0;
	pb++;
	
}
//=================================================================
u32 FS_CCU32(u8 *pb)           ////4λuint8ת��Ϊuin32
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
void U32CC(u8 *pb,u32 d)   ////4λuint8ת��Ϊuin32
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
//��������:   �Ƚ������Ƿ���� ��
//��ڲ���:   uint16 *PX1,uint16 *PX2 ��uint32 Len
//�� �� ֵ:   0:PX1=PX2  1:PX1>PX2 -1:PX1<PX2
/*
static u8 DataComp(u8 *PX1,u8 *PX2,u16 Len)
{
    u16 i;
    for(i=0; i<Len; i++)
    {
        if((*PX1)>(*PX2)) return BIG; //����
        if((*PX1)<(*PX2)) return LIT; //С��
        PX1++;
        PX2++;
    }
    return EQU;  //���
}
*/
//=======================================================================
//            �Ƚ�һ�����Ƿ�Ϊ��ͬһ����
//��ڲ���:   uint16 *PX,uint16 DF ��uint32 Len
//�� �� ֵ:   0:PX = DF  1:PX > DF 2:PX < DF
static u8 DataCompOne(u8 *PX, u8 DF, u16 Len)
{
    u16 i;
    for(i=0; i<Len; i++)
    {
        if((*PX)>(DF)) return BIG; //����
        if((*PX)<(DF)) return LIT; //С��
        PX++;
    }
    return EQU;  //���
}
//======================================================================
//                   �������У��
//����:Da ����,DLen,���ݳ���
//���:
//����:char�����
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


//-------------- ��ȡһЩϵͳ������ԭ�Ӳ���----д------------

/****************************************************************************
**Description:      ��¼���д�Ŀ��
**Input parameters:
**Output parameters:
**
**Returned value: 0 �ɹ�
**
**Created by:
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
static s32 SysFile_RBlock_Set(u16 blocknum) // 20160815
{
	
    SysFile_U16CC(FileSysB.RBlock, blocknum);
    FsDebugLog("\r\n--->>>>>logs L1: ����޸Ŀ�:%d---\r\n", blocknum);
    SysFile_UpdataFlag_Set();
    return 0;
	
}
//--- ��������� ---
/****************************************************************************
**Description:      ����һ�����״̬
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
        FsDebug("---�����鳬��ϵͳ��Χ\r\n");
        return -1;
    }

    SysFile_U16CC(FileSysB.Block[blocknum].NextBlack, status);
    FileSysB.Block[blocknum].CS = cs;
    //FsDebug("---������״̬:0x%02x 0x%02x--\r\n", FileSysB.Block[blocknum].NextBlack[0],1);
    FsDebugLog("\r\n--->>>>>logs L1: �޸Ŀ�:%d, s:0x%04x, cs:0x%02x---\r\n", blocknum, status, cs);
    SysFile_UpdataFlag_Set();
    return 0;
}

/****************************************************************************
**Description:      ��ʼ���ļ�����
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
        FsDebug("FoundFile:�ļ�������!\r\n");
        return LST;
    }

    memset(&FileSysB.File[FNum].FileF[0], 0, FileNLen);
    SysFile_U16CC(FileSysB.File[FNum].FileF, FileFlag);      // ��־
    strcpy((s8 *)FileSysB.File[FNum].Name, (s8 *)FName);         // �ļ���
    FileSysB.File[FNum].Name[len] = 0;              //
    SysFile_U16CC(FileSysB.File[FNum].StartLink,UNBlock);   // ���ӿ�    0xffff     δ����ռ�

    FileSysB.File[FNum].CS = CountCheckSum(&FileSysB.File[FNum].FileF[0],FileNLen - 1);// ���У��

    FsDebugLog("\r\n--->>>>>logs L1: ��ʼ���ļ�:%d, %s---\r\n", FNum, FName);
    SysFile_UpdataFlag_Set();
    return SOK;
}

//============================================================================
//����:�޸��ļ���
//����:filename Newfilename�ļ���
//���:FNUM�ļ��� >= FileNum  //�ļ�������
//����:0:�ɹ� ����:ʧ�ܴ���
//���: heyi
u8 SysFile_AmendFileNameFnum(u8 FNum, u8 *NewFileName)   // 20160815
{
    FsDebugLog("\r\n\r\n--->>>>>logs L2: AmendFileNameFnum---\r\n");
    memset(FileSysB.File[FNum].Name,0,sizeof(FileSysB.File[FNum].Name));
    strcpy((s8 *)FileSysB.File[FNum].Name, (s8 *)NewFileName);         // �ļ���
    // memcpy(FileSysB.FileName[Fnum].dateTime,&Sys.Year,6);     // YY MM DD hh mm ss �������ʱ��
    FileSysB.File[FNum].CS = CountCheckSum(&FileSysB.File[FNum].FileF[0],FileNLen - 1);// У���
    FsDebugLog("\r\n--->>>>>logs L1: �޸��ļ���:%d, %s---\r\n", FNum, NewFileName);
    SysFile_UpdataFlag_Set();
    return SOK;
}

/****************************************************************************
**Description:      ɾ���ļ�����
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
    memset(&FileSysB.File[FNum].FileF[0], 0xff, FileNLen);//ɾ���ļ�
    FsDebugLog("\r\n--->>>>>logs L1: ɾ���ļ�����:%d---\r\n", FNum);
    SysFile_UpdataFlag_Set();
    return 0;
}

/****************************************************************************
**Description:      ����ϵͳ���
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
    FsDebugLog("\r\n--->>>>>logs L1: ����BN:%04x---\r\n", blocknum);
    SysFile_UpdataFlag_Set();
    return 0;
}

static s32 SysFile_Set_BBN(u16 blocknum)   // 20160815
{
    SysFile_U16CC(FileSysB.FileBBN, blocknum);
    FsDebugLog("\r\n--->>>>>logs L1: ����BBN:%04x---\r\n", blocknum);
    SysFile_UpdataFlag_Set();
    return 0;
}

/****************************************************************************
**Description:      ����ϵͳ�������У��ͣ�������
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
    FsDebugLog("\r\n--->>>>>logs L1: ����У���:%02x---\r\n", FileSysB.CS);
    SysFile_UpdataFlag_Set();
    return 0;
}

/****************************************************************************
**Description:      ����ڴ��е��ļ�ϵͳ
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
    FsDebugLog("\r\n--->>>>>logs L1: ��������ļ�ϵͳ---\r\n");
    SysFile_UpdataFlag_Set();
    return 0;
}

/****************************************************************************
**Description:      �����ļ�����ʼ��״̬
**Input parameters:  StrLink �ļ�����ʼ�飬���Ϊ0XFFFF,˵���ǿ��ļ�
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
    FsDebugLog("\r\n--->>>>>logs L1: �����ļ���ʼ��:%d, %04x---\r\n", FNum, StrLink);
    SysFile_UpdataFlag_Set();
    return 0;
}

/****************************************************************************
**Description:      �����ļ�����
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
    //�����ļ��ĳ���
    U32CC(FileSysB.File[FNum].FileLen, Len);
    FsDebugLog("\r\n--->>>>>logs L1: �����ļ�����:%d, %04x---\r\n", FNum, Len);
    SysFile_UpdataFlag_Set();
    return 0;

}


/****************************************************************************
**Description:      �����ļ�������У��
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
    FileSysB.File[FNum].CS = CountCheckSum(&FileSysB.File[FNum].FileF[0], FileNLen - 1);// У���
    FsDebugLog("\r\n--->>>>>logs L1: �����ļ�У��:%d, %02x---\r\n", FNum, FileSysB.File[FNum].CS);
    SysFile_UpdataFlag_Set();
    return 0;
}
/****************************************************************************
**Description:       �����Լ�
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
    dev_maths_bcd_add(FileSysB.WdateTime ,"\x00\x00\x00\x00\x00\x01", 6); //��ʹ�ô�����1
    FsDebugLog("\r\n--->>>>>logs L1: �޸�ϵͳ��������---\r\n");
    SysFile_UpdataFlag_Set();
    return 0;
}
//-------------- ��ȡһЩϵͳ������ԭ�Ӳ���----��------------
//============================================================================
//����:�õ��ļ��ĳ���
//����: �ļ���
//����: �ļ�����
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
**Description:      �ж�ָ�����ļ��������Ƿ����ļ�
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
**Description:      ��ȡ�ļ�����ʼ���
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
**Description:      ��ȡ���д�Ŀ��
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
**Description:      ��ȡһ�����״̬
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
**Description:      ��ȡһ�����У���
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
**Description:      ��ȡ����ϵͳ�������У��
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
**Description:      ��ȡ��ǰϵͳ���
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
**Description:      ��ȡϵͳ���ݿ��
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
    return SysFile_CCU16(FileSysB.FileBBN)&0x7fff;//���һ������ FileSysB �ӿ�
}

//-----------------------------------------------------------
//=======================================================================
//����: flash �����
//����: LBNum ���
//����: �ļ���������
static u8 SysFile_EraseFileSysBlock(u16 LBNum)   // 20160815
{
    u8 Re;
    u32 i;
    u32 AD;

    Re = SOK;
    LBNum = LBNum & 0x7fff;
    if(LBNum >= SYSFILE_BLOCKNUM)
        return AER;

    AD = HDStrAdd + (u32)LBNum * BlockLen;   //flash ��ַ
    if((AD < HDStrAdd)||(AD > (SYSFILE_HDENDADD - BlockLen)))
        return AER;

    for(i = 0; i < 3; i++)
    {

        FsDebug("---�����LBNum:%u i:%u...",LBNum,i);

        (SysFsHdPra)->erase(AD);
        Re= SOK;

        if(Re == SOK)
        {
            break;
        }
    }

    if(Re == SOK)
    {
        FsDebug("�ɹ�\r\n");
    }
    else
    {
        FsDebug("ʧ��\r\n");
        SysFile_Block_SetStatus(LBNum, BadBlock, 0xff);
    }

    return Re;
}

/****************************************************************************
**Description:      ���ļ�ϵͳָ��λ��ֱ��д���ݣ������в���
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

    wadd = HDStrAdd + (LBNum * BlockLen) + shift; //�����ַflash ��ַ
    endadd = wadd + LBLen;
    if((wadd < HDStrAdd)||(endadd > (SYSFILE_HDENDADD)))
    {
        return AER;
    }  //��ַ����

   
    
    ret = (SysFsHdPra)->write(wadd, Bdata,  LBLen);

	if (0 != ret)
	{
		FsDebug("IFLASHдʧ��\r\n");
	}
	
    return SOK;
}
//=======================================================================
//����:д������
//����:LBNum:��� ,Bdata:Ҫд������, LBLen:Ҫд�����ݳ���
//����:0:�ɹ� ����:ʧ�ܴ���
static u8 SysFile_WriteFileSysBlock(u16 LBNum, const u8 *Bdata,u16 LBLen)  // 20160815
{
    u8 Re;//;,i
    u32 time;//д���������β��ɹ����˳�
    u16 WD;
    u32 WLen;
    u32 WAdd;
	//s32 ret=0;
    LBNum = LBNum & 0x7fff;
    if(LBNum >= SYSFILE_BLOCKNUM)//���ܴ���Ӳ�̿���
        return AER;

    if(LBLen > BlockLen)//���Ƴ��ȣ�һ��ֻ��4K
        WLen = BlockLen;
    else
        WLen = LBLen;

    WAdd = HDStrAdd + (LBNum * BlockLen); //�����ַflash ��ַ
    if((WAdd < HDStrAdd)||(WAdd > (SYSFILE_HDENDADD-BlockLen)))
    {
        return AER;
    }  //��ַ����

    for(time=0; time<3; time++)
    {
		FsDebug("---д������LBNum:%u,LBLen:0x%04x,time:%u ...", LBNum, LBLen, time);

		// (SysFsHdPra)->write(Bdata, WAdd, WLen);

		//ֱ��д���ɵײ�����ɲ����Ķ���
		(SysFsHdPra)->write( WAdd,(u8 *)Bdata, WLen);//����u32һ�ֽ�  ret = 
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
        //дʧ�ܣ���Ϊ�����ǻ���
        FsDebug("ʧ��\r\n");

        WD = BadBlock;
        SysFile_Block_SetStatus(LBNum, WD, 0xff);
    }

    return Re;
}



static u8 SysFile_WriteFileSysBlockspec(u16 LBNum, const u8 *Bdata,u16 LBLen)  // 20160815
{
    u8 Re;//;,i
    u32 time;//д���������β��ɹ����˳�
    u16 WD;
    u32 WLen;
    u32 WAdd;
	//s32 ret=0;
    LBNum = LBNum & 0x7fff;
    if(LBNum >= SYSFILE_BLOCKNUM)//���ܴ���Ӳ�̿���
        return AER;

    if(LBLen > BlockLen)//���Ƴ��ȣ�һ��ֻ��4K
        WLen = BlockLen;
    else
        WLen = LBLen;

    WAdd = HDStrAdd + (LBNum * BlockLen); //�����ַflash ��ַ
    if((WAdd < HDStrAdd)||(WAdd > (SYSFILE_HDENDADD-BlockLen)))
    {
        return AER;
    }  //��ַ����

    for(time=0; time<3; time++)
    {
		FsDebug("---д������LBNum:%u,LBLen:0x%04x,time:%u ...", LBNum, LBLen, time);

		// (SysFsHdPra)->write(Bdata, WAdd, WLen);

		//ֱ��д���ɵײ�����ɲ����Ķ���
		(SysFsHdPra)->write( WAdd,(u8 *)Bdata, WLen);//����u32һ�ֽ�  ret = 
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
        //дʧ�ܣ���Ϊ�����ǻ���
        FsDebug("ʧ��\r\n");

        WD = BadBlock;
        SysFile_Block_SetStatus(LBNum, WD, 0xff);
    }

    return Re;
}


//=======================================================================
//����:��������
//����:LBNum:��� ,Bdata:Ҫд������, LBLen:Ҫд�����ݳ���
//����:0:�ɹ� ����:ʧ�ܴ���
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

    RAdd = HDStrAdd + (u32)LBNum * BlockLen;  //flash ��ַ
    if((RAdd < HDStrAdd)||(RAdd > (SYSFILE_HDENDADD - BlockLen)))
    {

        FsDebug("!!!---�������ݴ���LBNum:%d\r\n",LBNum);

        return AER;
    }  //��ַ����

    //SpiFlashFastReadMoreByteData(Bdata, RAdd, RLen);
    (SysFsHdPra)->read( RAdd, Bdata,RLen);
    return SOK;
	
}
//=======================================================================
//����:������һ����õĿ��
//���� :0xffff:û�п��ÿ� / <0xffff:���
//�������ұȽϣ��ڿ������ʱ����ܻ�ȽϷ�ʱ��
static u16 SysFile_LookupUseBlock(void)  // 20160815
{
    u32 i,j;
    u16 BN;
    u16 blockstatus;

    BN = SysFile_RBlock_Get();

    BN = BN & 0x7fff;
    i = BN & 0x7fff;//�ܲ���ֱ�ӵ���BN?

    j = 0;

    while(1)
    {
        i++;
        j++;

        if( i >= SYSFILE_BLOCKNUM)//��ѯӲ���ϵĿ�
        {
            i = 0;
        }

        blockstatus = SysFile_Block_GetStatus(i);

        if(blockstatus == UNBlock)
        {
            break;
        } //���ÿ�

        if((i == BN)||(j >= SYSFILE_BLOCKNUM))
        {
            i = 0xffff;//�Ҳ������п�
            break;
        }
    }

#ifdef Test_SysPrint
    if(i == 0xffff)
    {
        FsDebug("!!!---�޿��ÿ�\r\n");
    }
    else
    {
        FsDebug("---���ÿ��:%d \r\n",i);
    }
#endif

    return (i|0x8000);//���ؿ�ţ��޿��ÿ��򷵻�0XFFFF. ΪʲôҪ����0x8000
}
//============================================================================
//����:����ļ�ϵͳ�ξ�
//
//���:
//����:0:�ɹ� ����:ʧ�ܴ���
u8 SysFile_AcquireFileSysParameter(void)   // 20160815
{
    u32 i;
    u16 Num;

    FileSysP.PrepaidSpace = 0;   //���ÿռ�(���ݿ�)
    FileSysP.DoSpace = 0;        //���ÿռ�(���ݿ�)
    FileSysP.BadSpace = 0;       //�ѻ��ռ�(���ݿ�)
    FileSysP.PrepaidFileNum = 0; //�����ļ���
    FileSysP.DoFileNum = 0;      //�����ļ���

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

    FileSysP.PrepaidSpace *= BlockLen;   //���ÿռ�
    FileSysP.DoSpace      *= BlockLen;   //���ÿռ�
    FileSysP.BadSpace     *= BlockLen;   //�ѻ��ռ�

    for(i = 0; i < FileNum; i++)
    {
        Num = SysFile_File_GetFlag(i);
        if(Num == FileFlag)
        {
            FileSysP.PrepaidFileNum++;//�����ļ���
        }
        else
        {
            FileSysP.DoFileNum++;       //�����ļ���

        }
    }
    return SOK;
}
//=======================================================================
//����:�ж������Ƿ�Ϊ�ļ�ϵͳ����
//����: 0:�����ļ�ϵͳ���� 1:Ŀ¼���� 2:��������
/*
char JudgeDataIsFileSys(u16 BN)
{
   u16 j,CS;
   char Re;
   Re = NSD;
   CS = 0xffff;

   if(DataCompOne(&FileSL.FileSB.FileBF[0],FileBFlag,12)== EQU)  //�ļ���
   {
      Re = ReadFileSysBlock(BN,FileSL.Buf,4096);  //��ȡ12���ֽڵ�ͷ����
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
**Description:          ��һ��ϵͳ������ɾ��(ֻҪɾ����ʶ�Ϳ�����)
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

    AD = HDStrAdd + (u32)LBNum * BlockLen;   //flash ��ַ
    if((AD < HDStrAdd)||(AD > (SYSFILE_HDENDADD - BlockLen)))
        return AER;

	//ֱ�Ӳ��飬������д�Ŀ�ֱ��д��д���� 2015.10.27 caoliang
  (SysFsHdPra)->erase(AD);  //ret = 
//  SpiFlashWrite(data, AD, 12);
    Re = SOK;

    if(Re != SOK)//������ʶ���ɹ�������п����
    {
        FsDebug("!!!---��ϵͳ���ʶʧ��LBNum:%u\r\n",LBNum);

        for(i = 0; i < 3; i++)
        {
            FsDebug("---�����LBNum:%u i:%u \r\n",LBNum,i);

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
        FsDebug("---ɾ��ϵͳ��...�ɹ���LBNum:%u\r\n",LBNum);

        WD = UNBlock;
    }
    else
    {
        FsDebug("---ɾ��ϵͳ��...ʧ�ܡ�LBNum:%u\r\n", LBNum);

        WD = BadBlock;
    }

    SysFile_Block_SetStatus(LBNum, WD, 0xff);

    return Re;
}

//============================================================================
//����:����ļ�ʹ�õ�ȫ�����
//����:FNum�ļ���
//���:FBN����б� BNum���� FLen�ļ�����
//����:0:�ɹ� ����:ʧ�ܴ���
static u8 SysFile_FileBlackLish(u8 FNum, u16 *FBN, u16 *BNum, u32 *FLen)  // 20160815
{
    u8 Re;
    u16 BN;
    u16 FBNum;
    u32 Len, len_tmp;

    Re = SOK;

    if(SysFile_File_GetFlag(FNum) != FileFlag)
    {

        FsDebug("!!!!!!--FileBlackLish:�����ļ�!\r\n");

        Re = NNF;
    }  //�����������ļ�
    else
    {
        Len = 0x00;
        FBNum = 0x00;
        BN = SysFile_File_GetHead(FNum); //��ʼ���

        while(1)
        {
            if(BN == BadBlock)
            {
                FsDebug("!!!!!---FileBlackLish:�ļ�ָ�򻵿�!\r\n");

                Re = NNF;
                break;
            } //����

            if(BN == UNBlock)
            {
                //�ҵ��ļ�β��
                //������һ�����ļ����յ�
                FsDebug("!!!!!---FileBlackLish:���ļ�!\r\n");
                break;
            }                   //δ�ÿ�

            BN = BN & 0x7fff;    //���
            *(FBN + FBNum) = BN;   //���
            FBNum++;               //����

            //��һ��ţ��������û���꣬�õ��Ľ��Ǳ����Ѿ�ʹ�õ��ֽ���
            BN = SysFile_Block_GetStatus(BN);
            if(BN < 0x8000)  //�����飬С��0X8000��˵������û����
            {
                Len += BN;   //(�ļ�)���ݳ���
                FsDebug("!!!!!---FileBlackLish:���һ�� %d!\r\n", BN);
                break;
            }

            Len += BlockLen;
        }

        if(Re == SOK)
        {
            len_tmp = SysFile_File_GetLen(FNum);
            if(Len != len_tmp)//�ļ����ȳ���
            {
                FsDebug("!!!!!!!--FileBlackLish:У���ļ����ȴ���%d, %d!\r\n",Len, len_tmp);
            }
            else
            {}
        }
    }

    *FLen = Len;    //�ļ�����
    *BNum = FBNum;  //����
    return Re;
}
//============================================================================
//����:����ļ�����
//����:�ļ���
//����:0:�ɹ� ����:ʧ�ܴ���
static void SysFile_ClearFileData_Block(u8 FNum)  // 20160815
{
    u16 BN,BN1;
    u8 FN;

    if(FNum >= FileNum)
    {
        FsDebug("ɾ���� �ļ��Ŵ���\r\n");
        return;
    }

    FN = FNum;
    BN = SysFile_File_GetHead(FN);
    while(1)
    {
        if(SysFile_File_GetFlag(FN)!= FileFlag)
        {
            FsDebug("ɾ���ļ�����ʧ��:1\r\n");
            break;
        }  //�����������ļ�

        if((BN == BadBlock)||(BN == UNBlock))
        {

            FsDebug("ɾ���ļ�:���飬δʹ�ÿ�\r\n");

            break;
        }               //δ�ÿ�/����

        BN = BN & 0x7fff;
        //����һ��򱾿鳤��)
        BN1 = SysFile_Block_GetStatus(BN);
        //����Ϊδ�ÿ�
        SysFile_Block_SetStatus(BN, UNBlock, 0xff);

        if(BN1 < 0x8000)//���һ��?
        {
            SysFile_Starlink_Set(FNum, UNBlock);
            break;    //����
        }

        BN = BN1;//����������һ���ļ���
    }

    FsDebug("ɾ���ļ�����\r\n");

    return;
}

/****************************************************************************
**Description:      ��ӡ�ļ�ϵͳ��Ϣ�����ڲ���     
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
    
    VfsDebug("������ȡ�ļ�ϵͳ����\r\n");
    VfsDebug("\r\n--------------- �ļ�ϵͳ---------\r\n");
    VfsDebug("��ʼ��ַ:%08x\r\n", HDStrAdd);
    VfsDebug("������ַ:%08x\r\n", SYSFILE_HDENDADD);
    VfsDebug("����:%d\r\n", SYSFILE_BLOCKNUM);
    VfsDebug("���С:%x\r\n", BlockLen);
    VfsDebug("�ļ��б�:\r\n");

    for(i = 0; i < FileNum; i++)
    {

        if(SysFile_File_GetFlag(i) == FileFlag)
        {
            VfsDebug("�ļ���:%d, %s, %02x%02x,%02x%02x%02x%02x\r\n", i, FileSysB.File[i].Name,
                    FileSysB.File[i].StartLink[0], FileSysB.File[i].StartLink[1],
                    FileSysB.File[i].FileLen[0],FileSysB.File[i].FileLen[1],FileSysB.File[i].FileLen[2],FileSysB.File[i].FileLen[3]);
        }
        else
        {
            VfsDebug("�ļ���:%d, δʹ��\r\n", i);
        }
    }

    FsDebug("���б�:\r\n");
    for(i = 0; i < SYSFILE_BLOCKNUM; i++)
    {

        tmp = SysFile_Block_GetStatus(i);

        FsDebug("(%04d, 0x%04x) ", i, tmp);

        if(tmp == UNBlock)
        {
            FsDebug("δʹ��\r\n");
        }
        else if(tmp == BadBlock)
        {
            FsDebug("����\r\n");
        }
        else if(tmp == LogBlock)
        {
            FsDebug("**��־��\r\n");
        }
        else if(tmp <= BlockLen)
        {
            FsDebug("������ʹ��\r\n");
        }
        else if(tmp == BlockLen + FileSysBLen)
        {
            FsDebug("�ļ�ϵͳ��\r\n");
        }
        else if(tmp > 0X8000)
        {
            FsDebug("���꣬����%d\r\n", tmp - 0X8000);
        }
        else
        {
            FsDebug("δ֪\r\n");
        }
    }
    FsDebug("\r\n--------------- �ļ�ϵͳ---------\r\n\r\n");
#endif

}

//=======================================================================
//����:��������ļ�ϵͳ����
//����:0:�ɹ� ����:ʧ�ܴ���
//sxl   �ҵ��ļ�Ŀ¼�б����Ҹ����ļ�Ŀ¼������
//��ԭ�ļ�Ŀ¼����,û���õ��Ŀ�Ҫ����
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
    //FsDebug("\r\nָ��洢�ռ�����%d���ļ�ϵͳ��Ŀǰָ���%d���ļ�ϵͳ\r\n", XgdMtdNum, XgdMtdIndex);
    FsDebug("\r\n��ַ%08x��������ַ%0x ������%d\r\n", HDStrAdd, SYSFILE_HDENDADD,SYSFILE_BLOCKNUM);

    if((FileSysBLen+2)>BlockLen)//�ļ��ṹ���ܴ���һ�����ݿ飬���ң��鳤�Ȳ��ܴ���0X8000(32K)
    {
        while(1)
        {
            FsDebug("�ļ�ϵͳ�ṹ���ݳ���:%04����\r\n", FileSysBLen);
        }
    }

    FSL = (union _FileSysLink *)k_malloc(sizeof(union _FileSysLink));
    if(FSL == NULL)
    {
        while(1)
        {
            FsDebug("�ڴ�����ʧ�� AcquireFileSysData\r\n");
        }
    }

    //RS = NFS;
    //RL = NFS;

    for(i = 0; i < SYSFILE_BLOCKNUM ; i++) //��Ѱ���п飬��ѯϵͳ������
    {
        BN=i;
        Re = SysFile_ReadFileSysBlock(BN, FSL->Buf, 12); //��ȡ12���ֽڵ�ͷ����
        if(Re == SOK)  //�����ݳɹ�
        {
            RSD = NSD;
            CS = 0xffff;
            if(DataCompOne(&FSL->FileSB.FileBF[0], FileBFlag, 12)== EQU)  //�ļ�����12��5A��ͷ
            {
                //�Ѿ��ҵ��ļ�ϵͳ�Ĳ����飬�����������

                FsDebug("\r\n�ҵ�ϵͳ�ļ���LBNum:%u\r\n",BN);

                Re = SysFile_ReadFileSysBlock(BN, FSL->Buf, FileSysBLen);//��ȡ����ϵͳ�����飬

                if(Re == SOK)
                {
                    CS = 0;
                    CS = CountCheckSum(&FSL->Buf[0],FileSysBLen - 1);//����У��ֵ��ʹ�����У��
                    if(CS == FSL->FileSB.CS)
                    {
                        RSD = ISF;
                    }
                    else
                    {
                        FsDebug("ϵͳ������У��Ͳ���\r\n");
                    }
                    //ÿ�ζ���������ô����ֻ�������һ�ε����ݶ���У�鲻��Ҳ����?

                    FsDebug("����ϵͳ�ļ���LBNum:%u\r\n",BN);

                    memcpy(&FileSysB.FileBF[0],&FSL->Buf[0],FileSysBLen);//��ϵͳ���������ڴ�(ȫ��)�У��������
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
        //��δ�����5���ļ�ϵͳ���BUG?
        if((RSD == ISF) && (tempfileno <5))//�ļ�Ŀ¼���ݲ��ᳬ��5����ʵ��ʹ���У�Ӧ��ֻ������
        {
            tempsysfileinfo[tempfileno].BLKNo = BN;//��ʱ��ס�ļ�ϵͳ���
            memcpy(tempsysfileinfo[tempfileno].WdateTime, FSL->FileSB.WdateTime, 6);//����д�����ŵ���ʱ��Ϣ��

            FsDebug("BLOCK:%d WdateTime----\r\n", BN);
            //sys_debug_format_u8(FSL->FileSB.WdateTime, 6);

            tempfileno++;
        }
    }
    FsDebug("����ϵͳ��\r\n");
    //�Զ����������ļ�ϵͳ���������
    j = 0;//tempfileno �϶�С�� 5
    while(j < tempfileno)   //��ԭLINK����,ֻ��һ�����õ�
    {
        for(i =(j + 1); i< tempfileno; i++)//������������j��
        {
            mFlag = 0;
            if((memcmp(tempsysfileinfo[j].WdateTime, tempsysfileinfo[i].WdateTime, 6))>0)//jָ��Ŀ��ʹ�ô��� > iָ��Ŀ�
            {
                memcpy(tempbcddata, tempsysfileinfo[j].WdateTime, 6);
                dev_maths_bcd_sub(tempbcddata, tempsysfileinfo[i].WdateTime, 6); //tempbcddata = j - i   //    j   i   i  i  i  i
                if(memcmp(tempbcddata, "\x00\x10\x00\x00\x00\x00", 6) > 0) //(j - i) > 1000000000  //0   97  5   99 98 7  6     97- 5(j-i) = 92 > 50  Ҫ��
                {
                    //   //1   5   97  99      99- 5(i-j) = 94 > 50  ��Ҫ��
                    mFlag = 1; //��������ѭ�� //2 5  98  98- 5(i-j) = 93 > 50 ��Ҫ��
                }   //  //3   5   7  7 - 5(i-j) = 2  < 50  Ҫ��
            }                                                   //  //4   7             5  6     7 - 6(j-i) = 1  < 50  ��Ҫ��
            else //j <= i  //5   7   97  99 98 5  6     //�������
            {
                memcpy(tempbcddata,tempsysfileinfo[i].WdateTime,6);
                dev_maths_bcd_sub(tempbcddata,tempsysfileinfo[j].WdateTime,6);      //tempbcddata = i - j
                if(memcmp(tempbcddata,"\x00\x10\x00\x00\x00\x00",6) < 0) //(i - j) < 1000000000
                {
                    mFlag = 1;                                            //������δѭ��
                }
            }

            if(mFlag == 1)   //��j������С��Ҫ����
            {
                FsDebug("�ļ����������\r\n");

                temp.BLKNo = tempsysfileinfo[j].BLKNo;   //�ݱ�j������
                memcpy(temp.WdateTime,tempsysfileinfo[j].WdateTime,6);

                tempsysfileinfo[j].BLKNo = tempsysfileinfo[i].BLKNo;      //����
                memcpy(tempsysfileinfo[j].WdateTime,tempsysfileinfo[i].WdateTime,6);

                tempsysfileinfo[i].BLKNo = temp.BLKNo;
                memcpy(tempsysfileinfo[i].WdateTime,temp.WdateTime,6);
            }
        }


        FsDebug("�ļ���������:%d\r\n", tempfileno);
        if(tempfileno < 2)
        {
            //˵��ϵͳ�������⣬���ܱ������ˣ�������û�н����ļ�ϵͳ
            FsDebug("����:�ļ�ϵͳ�鲻�㣬��ע��\r\n");

        }

        BN = tempsysfileinfo[j].BLKNo;//��ȡ���µĿ�

        FsDebug("ʹ���ļ�������:%d\r\n", BN);
        //��������ĵ������ٶ�һ��
        Re = SysFile_ReadFileSysBlock(BN, FSL->Buf, FileSysBLen);
        if(Re == SOK)   //���ﲻ�ٿ��Ƕ��ļ���������
        {
            CS = CountCheckSum(&FSL->Buf[0], FileSysBLen - 1);//�ٴμ���У�飬Ϊʲô?
            if(CS == FSL->FileSB.CS)
            {
                memcpy(&FileSysB.FileBF[0], &FSL->Buf[0], FileSysBLen);
                //ɾ��ԭ��û��ɾ���Ŀ�
                //�����ļ�ϵͳ��ֻ���������ļ�ϵͳ�飬��ˣ��ڼ���ļ�ϵͳ��ʱ��
                //�����ͷţ�������ɴ���
                tempblockaddr1 = SysFile_Get_BN();
                tempblockaddr2 = SysFile_Get_BBN();
                FsDebug("--ɾ������ϵͳ��---");
                for(i = 0; i< tempfileno; i++)
                {
                    if(tempsysfileinfo[i].BLKNo != tempblockaddr1 &&
                            tempsysfileinfo[i].BLKNo != tempblockaddr2)
                    {
                        BLKNo = tempsysfileinfo[i].BLKNo;
                        SysFile_SysFileDelSysBlock(BLKNo);//.ɾ��ԭ����ϵͳ������
                        {

                        }
                    }
                }

                //Ϊ�˶����ϼ��ݣ�ͨ���ж���־���Ƿ���ڣ������Ƿ�������־��
                #ifdef LOG_MODE
                SysFile_Logs_Found();
                //ϵͳ��У��ͨ������ô�͸�ϵͳ��־�Ƚϣ����Ƿ������µ�ϵͳ�������������
                //���лָ�����
                #endif
                
#if 0
                //����������ļ�ϵͳ�Ѿ����������ļ�ϵͳ��ֻ���������ļ�ϵͳ������
                //�����ļ�ϵͳ��
                Re = RenewalFileSys(0);//���ڴ��е��ļ�ϵͳ�������浽FLASH��,����ʼ��һЩ����
#else
                Re = SOK;
#endif
                if(Re == SOK)
                {
                    tempblockaddr1 = SysFile_Get_BN();
                    tempblockaddr2 = SysFile_Get_BBN();

                    for(k = 0; k< SYSFILE_BLOCKNUM; k++)
                    {
                        //����һЩ�ļ��� ��ʶ�� (BlockLen + FileSysBLen)
                        //����ط���������ֲ������NextBlack��ȡֵ��Χ�����˹����ֵ�
                        if(SysFile_Block_GetStatus(k) == (BlockLen + FileSysBLen)
                                &&(k != tempblockaddr1)
                                &&(k != tempblockaddr2)) //�ļ���/�����
                        {
                            FsDebug("���տ���:%d\r\n", k);

                            SysFile_Block_SetStatus(k, UNBlock, 0xff);

                        }
                    }
                }


#if 0 /*��ѧ�� 2014-1-29*/
                //�����ǰϵͳ�������
                tempblockaddr1 = SysFile_CCU16(FileSysB.FileBN) & 0x7fff;
                tempblockaddr2 = CCU16(FileSysB.FileBBN)& 0x7fff;
                //�����ʶ�ó�(BlockLen + FileSysBLen),˵�����ļ�ϵͳ��
                SysFile_Block_SetStatus(tempblockaddr1, BlockLen + FileSysBLen, 0xff);
                SysFile_Block_SetStatus(tempblockaddr2, BlockLen + FileSysBLen, 0xff);
#endif

                //����ϵͳ����,V30�Ѿ��޸���BUG,2012.12.22 ��ѧ�ѣ�add
                SysFile_AcquireFileSysParameter();
                SysFile_Show_detai();
                //�ͷ��ڴ�
                k_free(FSL);
                return Re;
            }
            else
            {
                //���LOG���ڣ���Ҫ���лָ�
                //���ñ�����Ϊ���滹�ᴦ��������ϵͳ�����飬���籸�ݵĲ����顣
            }
        }
        //--��ѯ��һ��
        j += 1;
        //��ԭLINK��
    }

    SysFile_Clr();
    //�ͷ��ڴ�
    k_free(FSL);
    return NFS;  //δ�����ļ�ϵͳ

}

//=======================================================================
//����:�����ļ�ϵͳ
//����:0:�ɹ� ����:ʧ�ܴ���
//�޸�:������LINK��LINK��Ҫͨ���ļ�������������
// ����һ�����������ھ����Ƿ񴴽������ļ�ϵͳ��
// 1 ����
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

    //�ݱ�ԭ���Ŀ��
    SaFileBN = SysFile_Get_BN();
    SaFileBBN = SysFile_Get_BBN();

    FsDebug("\r\n--------------�����ļ�ϵͳ------------------\r\n");
    FsDebug(" File Sys Block Len:%04x\r\n", FileSysBLen);
    FsDebug("---Ŀǰϵͳ�������:%d, ���ݿ�:%d\r\n", SaFileBN, SaFileBBN);

    Re = SER;
    //Re1 = SER;
    //Re2 = SER;

    while(1)
    {
        Re = SER;
        //Re1 = SER;
        //--��ȡһ�����п���Ϊ�ļ�ϵͳ��
        NextB = SysFile_LookupUseBlock();//��ȡ��һ����
        if(NextB == 0XFFFF)
        {
            FsDebug("�� ���п�!\r\n");
            break;
        }
        //����ϵͳ���
        SysFile_Set_BN(NextB);
        SysFile_RBlock_Set(NextB & 0x7fff);
        //---
        WBN = SysFile_Get_BN();

        memset(&FileSysB.FileBF[0], FileBFlag, 12);  //Ϊ������־
        SysFile_WdateTime_Add();//��ʹ�ô�����1

        //��ԭ���Ŀ���Ϊ���ݿ�
        SysFile_Set_BBN(SaFileBN);
        //�޸ı��ݿ�Ŀ�״̬������Ϊ���п�
        SysFile_Block_SetStatus(SaFileBBN, UNBlock, 0xff);//�޸Ŀ�״̬

        //�޸�����Ŀ��״̬
#if 0
        cs = CountCheckSum(&FileSysB.FileBF[0],FileSysBLen);
#else
        cs = 0xff;
#endif

        //����и��������⣬��ˣ�����ϵͳ����˵����У�����û�õ�
        //ֻ�к����ϵͳ��У������ã���У���ֻ���ļ�������
        SysFile_Block_SetStatus(WBN, BlockLen + FileSysBLen, cs);

        //����У���
        SysFile_SetCS();

        //--���ڴ��е�ϵͳ������д���ո�����Ŀ���
        //memcpy(FSL.Buf, &FileSysB.FileBF[0], BlockLen);//ΪʲôҪ����һ��?

        Re = SysFile_WriteFileSysBlock(WBN, &FileSysB.FileBF[0], FileSysBLen);  //д�ļ���

        if(Re == SOK)
        {
#if 0
            //д�ɹ����޸��ڴ��е�ϵͳ������ָ���ո�д�Ŀ���һ��ϵͳ������
            //ΪʲôҪд���ˣ����޸��ڴ��еĲ���?
            //����ط����߼���ϵ��Ҫ�úÿ��ǡ�����������ʱ�ŵ�д֮ǰ
            //cs = CountCheckSum(&FileSysB.FileBF[0],FileSysBLen);
            //SysFile_Block_SetStatus(WBN, BlockLen + FileSysBLen, cs);
#endif

            //���û�����ļ�ɾ��ԭ��������
            if(SaFileBN != SysFile_Get_BN())//���Ӧ���ǰٷְٲ����ڣ���Ϊ���·������
            {
                //SysFileDelSysBlock(SaFileBN);//�Ż�����ֻҪ����ϵͳ�������ʶ�Ϳ����ˣ������������������Ҫ���Ƚ϶�ʱ��
                {

                }
                //��������·�������ô�Ͳ��ǲ���SaFileBN�飬���ǲ��������һ��ϵͳ������ ���ݿ�
                #ifdef LOG_MODE
                SysFile_SysFileDelSysBlock(SaFileBBN);
                #else
                SysFile_SysFileDelSysBlock(SaFileBN);
                #endif
            }

            SaveFileSuc  = 1;//���³ɹ�
            break;
        }
        else//дʧ�ܣ����ϳ��ԣ�ֻ�е����䲻�����ÿ��ʱ����˳�
        {
            //-- ���ﲻ�ô���дʧ�ܣ���ᱻ��Ϊ����
            continue;
        }
    }
    //�����ڳ�ʼ���ļ�ϵͳ��ʱ��Żᴴ�����ݿ�
    #ifndef LOG_MODE
    mode = 1;
    FsDebug("\r\n\r\n\r\n------------��ģʽ!--------\r\n\r\n\r\n");
    #endif
    
    while((SaveFileSuc == 1) && (mode == 1))
    {
        FsDebug("\r\n\r\n\r\n------------�������ݿ�!--------\r\n\r\n\r\n");

        Re = SER;
        //Re2 = SER;
        //==============����ռ�=================
        NextB = SysFile_LookupUseBlock();//��ȡ��һ����
        if(NextB == 0XFFFF)
        {
            FsDebug("�� ���п�!\r\n");
            break;
        }

        //����ϵͳ���ݿ��
        SysFile_Set_BBN(NextB);
        SysFile_RBlock_Set(NextB & 0x7fff);

        //==========�ļ����鱸��==============
        memset(&FileSysB.FileBF[0],FileBFlag,12);   //Ϊ������־

        //�������������ÿ��д�ļ�ϵͳ������ͻ����
        SysFile_WdateTime_Add();//��ʹ�ô�����1

        //������ţ�ǰ��д����Ϊ���ݿ�
        WBN = SysFile_Get_BN();
        SysFile_Set_BN(NextB);
        SysFile_Set_BBN( WBN|0x8000);
        //-------------------
        WBN = SysFile_Get_BN();
        //�޸�����Ŀ��״̬
#if 0
        cs = CountCheckSum(&FileSysB.FileBF[0],FileSysBLen);
        //����и��������⣬��ˣ�����ϵͳ����˵����У�����û�õ�
        //ֻ�к����ϵͳ��У������ã���У���ֻ���ļ�������
#else
        cs = 0XFF;
#endif
        SysFile_Block_SetStatus(WBN, BlockLen + FileSysBLen, cs);

        //����У���
        SysFile_SetCS();

        //---------д����---------------
        //memcpy(FSL.Buf,&FileSysB.FileBF[0],BlockLen);

        Re = SysFile_WriteFileSysBlock(WBN, &FileSysB.FileBF[0], FileSysBLen);
        if(Re == SOK)
        {
            //cs = CountCheckSum(&FileSysB.FileBF[0],FileSysBLen);
            //SysFile_Block_SetStatus(WBN, BlockLen + FileSysBLen, cs);
            FsDebug("д���ݿ�ɹ�...");
            if(SaFileBBN != WBN)
            {
                //�Ż�����ֻҪ����ϵͳ�������ʶ�Ϳ����ˣ������������������Ҫ���Ƚ϶�ʱ��
                SysFile_SysFileDelSysBlock(SaFileBBN);
                FsDebug("ɾ��ԭ���ı��ݿ�...%d...", SaFileBBN);
            }
            break;
        }
        else //����ڶ���д��Ҫ���ڴ��У�������������ݲ�����Ŀ�Ż���ȥ��
            //Ȼ���ٳ��ԡ�ʧ�ܵĻ�Ҫ����ȥ
        {
            NextB = SysFile_Get_BN();
            WBN = SysFile_Get_BBN();

            SysFile_Set_BN(WBN);
            SysFile_Set_BBN(NextB);
            continue;
        }
    }

    FsDebug("׼�����²���...\r\n");

    SysFile_AcquireFileSysParameter();//����ϵͳ����
    SysFile_Show_detai();
#ifdef Test_SysPrint
    if(Re != SOK)
    {
        FsDebug("ʧ��\r\n");
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
//����:�����ļ�ϵͳ
//���ԭ�������ļ�ϵͳ����Ϊ���¸�ʽ��
//����:0:�ɹ� ����:ʧ�ܴ���
u8 SysFile_FoundFileSys(void)  // 20160815
{
    u32 i;

    FsDebugLog("\r\n\r\n--->>>>>logs L2: �����ļ�ϵͳ---\r\n");

    //���ڴ��е��ļ�ϵͳ�飬ȫ���ó�0XFF
    //ǰ��18���ֽڣ���12���ֽ��Ǳ�ʶ��0X5A,
    memset(&FileSysB.RBlock[0], 0xff, FileSysBLen-18);

    for(i = 0; i < 6; i++)
    {
        //�����ʼû���ļ�ϵͳ��wdateTime����ʲôֵ������ΪʲôҪ���ж�?
        //10����?
        if(((FileSysB.WdateTime[i] & 0xf0) > 0x90) || ((FileSysB.WdateTime[i] & 0x0f) > 0x09))
        {
            FsDebug("reset WdateTime: %d\r\n",i);
            memset(&FileSysB.WdateTime[0],0x00,6);  //������ 0x00
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

    //�����ļ�ϵͳ��ʱ��ȡ���һ����Ϊ���д�Ŀ��
    SysFile_RBlock_Set(SYSFILE_BLOCKNUM);
    
    #ifdef LOG_MODE
    //Ϊ�˶����ϼ��ݣ�ͨ���ж���־���Ƿ���ڣ������Ƿ�������־��
    SysFile_Logs_Found();
    #endif
    
    return SysFile_RenewalFileSys(1) ;  //�����ļ�ϵͳ
}
/*

//--------------------------- ����ӿ�-----------------------------------


*/
//============================================================================
//����:�����ļ�(���ļ�)
//���: FileN:�ļ���
//���: FNum �ļ��� >= FileNum  //�ļ�������
//����: 0:�ɹ� ����:ʧ�ܴ���
u8 SysFile_OpenFile(u8 *FileN,u8 *FNum)  // 20160815
{
    u8 Re;
    u32 i;
    Re = NFS;

    //�Ƿ���Ҫ�ж��ļ�ϵͳ�Ƿ����?
    {
        Re = NFN;
        for(i = 0; i < FileNum; i++)
        {
            if((strcmp((s8 *)FileSysB.File[i].Name, (s8 *)FileN) == 0)
                    &&(SysFile_File_GetFlag(i)==FileFlag))
            {
                *FNum = i;//�ļ���

                Re = SOK;
                break;
            }            //�ҵ��ļ�
        }
    }

    if(Re == NFN)
    {
        FsDebug("���ļ�OpenFile:%s:������\r\n",FileN);
    }
    else
    {

    }

    return Re;   //�Ҳ����ļ�
}
//============================================================================
//����:�ر��ļ�(�����ļ�ϵͳ)
//����:0:�ɹ� ����:ʧ�ܴ���
u8 SysFile_CloseFile(void)  // 20160815
{
    u8 ret;
    
    FsDebugLog("\r\n\r\n--->>>>>logs L2: CloseFile---\r\n");

    if(SysFsUpdatFlag == 0)
    {
        //FsDebugLog("����Ҫ�����ļ�ϵͳ\r\n");
        //uart_printf("����Ҫ�����ļ�ϵͳ\r\n");
        return 0;
    }

    ret = SysFile_RenewalFileSys(0);
    
    SysFsUpdatFlag = 0;//����±�ʶ����������CLOSE������ֱ����дһ�顣
    return ret;
}
//============================================================================
//����:����һ���ļ�
//����:FileN�ļ���
//���:FNum�ļ��� >= FileNum  //�ļ�������
//����:0:�ɹ� ����:ʧ�ܴ���
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
        FsDebug("FoundFile:�ļ���̫��!\r\n");
        return LST;
    }

    fn = FileNum + 1;
    Flag = 0;

    for(i = 0; i < FileNum; i++)
    {
        if(SysFile_File_GetFlag(i)== FileFlag)
        {
            if(strcmp((s8 *)FileSysB.File[i].Name, (s8 *)FileN) == 0) //�ҵ��ļ�(�Ѵ������ļ�)
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
                //����Ӧ��ֻ�����һ�Σ��������˼�ǽ���һ��
                //��Ч���ļ����ռ䵱����Ҫʹ�õĿռ䣬
                fn = i; //��һ���յ��ļ���
            }
        }
    }

    if((Flag == 0)&&(fn < FileNum))   //�ļ������� && ���п��ļ�����
    {

        Re = SysFile_File_Init(fn, FileN);//����ʧ��
        Re = SOK;

        FsDebug("FoundFile:�ɹ�!�ļ���:%d\r\n", fn);

    }
    else if(Flag == 1)
    {

        FsDebug("FoundFile:�ļ��Ѵ���!�ļ���:%d\r\n", fn);

    }
    else
    {

        FsDebug("FoundFile:Ŀ¼��!�ļ���:%d\r\n", fn);

        Re = FNF;
    }

    *FNUM = fn;

    FsDebug("\r\n-------------------------------\r\n");

    return Re;
}

//============================================================================
//����:����ļ�����
//����:�ļ���
//����:0:�ɹ� ����:ʧ�ܴ���
void SysFile_ClearFileData(u8 FNum)  // 20160815
{

    FsDebugLog("\r\n\r\n--->>>>>logs L2: ClearFileData---\r\n");
    SysFile_ClearFileData_Block(FNum);
    SysFile_File_SetLen(FNum, 0);//����ļ����ݺ�Ҫ���ļ���������Ϊ0
    return;
}
//============================================================================
//����:ɾ��һ���ļ�
//����:�ļ���
//����:0:�ɹ� ����:ʧ�ܴ���
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
**Description:      ����:д�ļ�����
**Input parameters: FNum�ļ���,StrAλ��,WD����,DLen ���ݳ���
**Output parameters:
**
**Returned value:   0:�ɹ� ����:ʧ�ܴ���
**
**Created by:

ע:��ʼ��ַҪ��ԭ���ļ��У���������һ����ַд������׷�ӣ���ѧ�� 2012.12.22
**--------------------------------------------------------------------------
**Modified by:
**Modified by:
****************************************************************************/
u8 SysFile_WriteFileData(u8 FNum,u32 StrA, u8 *WD,u32 DLen) // 20160815
{
    u8 Re,Rf;
    u16 i;

    u16 *FileBNum;
    u16 WBN,RBN;                      //��ǰ�������
    u16 j;                            //��ǰ��������ǵ�j��
    u16 BNum;                          //���ļ����õĿ���
    u16 WBSA;                         //�鿪ʼд��ַ
    u32 FLen;                         //�ļ�����
    u32 WAdd;                //д��ַ
    u32 WDLen;                        //Ҫд�����ݳ���
    u8 CS;

    u8 need_erase = 0;//�Ƿ���Ҫ������ʶ
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
            FsDebug("�ڴ�����ʧ�� WriteFileData\r\n");
        }
    }
    
    FileBNum = (u16 *)k_malloc(sizeof(u16)*(MaxBNum));
    if(FileBNum == NULL)
    {
        FsDebug("�ڴ�����ʧ�� ReadFileData\r\n");
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

    FsDebug("---δʹ�ÿռ� %08x \r\n", FileSysP.DoSpace);//�������д������жϲ��������á�

    if(FileSysP.DoSpace < 6 * BlockLen)//6��ʲô��˼? Ԥ���������ڱ��ݣ�������?
    {
        Re = RNE;   //���ݿռ䲻��

        FsDebug("!!!!!!!!----���ݿռ䲻��д�ļ��ռ� %08x %08x %08x", FNum, StrA, FileSysP.DoSpace);

        goto END;
    }

    Re = SysFile_FileBlackLish(FNum, FileBNum, &BNum, &FLen);
    if(Re != SOK)
    {
        FsDebug("!!!!!!!!----��ȡ�ļ����ʧ��!\r\n");
        goto END;
    }

    if((WAdd > FLen)||(BNum >= MaxBNum)) //д���ݳ����ļ�β���ļ�����Ҳ���ܴ��ڵ���Ӳ�̿���(Ӧ�ò��ᷢ��)
    {
        Re = NNF;
        goto END;
    }

    memset(FSL->Buf, 0xff, FileSysBlock_BUFSIZE);//

    while(1)
    {
        FsDebug("дһ������----");

        if(WDLen==0)//д�ļ�����
        {
            break;
        }

        WBSA = 0;

        j = WAdd/BlockLen;      //Ҫд���������ļ��ĵ�j��

        if(j < BNum)           //Ҫд��������ԭ���ļ���
        {
            WBN = FileBNum[j];      //��ǰд�Ŀ���
            WBSA = WAdd%BlockLen;   //��ǰд�Ŀ��ƫ����

            need_erase = 0;
		
            FsDebug("---����Ҫ���ӿ�");

            if(WBN < SYSFILE_BLOCKNUM)   //��ԭ���Ŀ����ȶ�����,���ж����ڷ�ֹ����
            {
                Re = SysFile_ReadFileSysBlock(WBN, FSL->Buf,FileSysBlock_BUFSIZE);   //��ȡ��ǰ������
                if(Re != SOK)
                {
                    goto ConverFile;
                }                  //�����ݳ���
            }
            else
            {
				
            }
			
        }
        else   //Ҫ�����¿�
        {
            if(BNum <(MaxBNum-1))
            {
                FileBNum[BNum] = 0xffff;       //��һ���
                BNum++ ;                        //����
                //��û�а취�ж�������Ŀ��Ƿ���Բ�����?
                memset(FSL->Buf,0xff,FileSysBlock_BUFSIZE);
                need_erase = 1;//���趨Ϊ��Ҫ����

                FsDebug("---��Ҫ�����...");

            }
            else
            {
                goto ConverFile;
            }
        }

        //������д��RAM�еĻ�����

        FsDebug("��仺��...");

        i = WBSA;//�ڿ��е�ƫ��  ����������¿飬��Ϊ0

        FsDebug("ƫ�� %d...", i);
		
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

            //�����ݿ�����RAM�еĻ������
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

        //changdatalen = i - WBSA;//�����޸�(��ӵ�����),����ֱ��д���ʱ��

        //�������ʹ�õ����ݳ���
        if(j < (BNum-1))//�������һ�飬Ҫ����ȫ����ֵ
        {
            i = BlockLen;//����ʹ��
        }
        else   //���һ��Ƚϸ���
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
            FsDebug("������ֱ�Ӵ�0 ��ʼд:%d...", i);
            //д����
            while(1)
            {
                WBN = SysFile_LookupUseBlock();       //��ȡһ������д
                if(WBN == 0xffff)//�Ҳ������ÿ�
                {
                    FsDebug("----�Ҳ������ÿ�---");
                    goto ConverFile;
                }

                WBN = WBN & 0x7fff; //���
                //�Ƿ���Բ�дһ����?
                //Re = WriteFileSysBlock(WBN,FSL.U16Buf,BlockLen); //д����//sxl 1127
                Re = SysFile_WriteFileSysBlockspec(WBN,FSL->Buf,FileSysBlock_BUFSIZE);//ֻд�õ����ֽ�//sxl20151102
                if(Re == SOK)
                {
                    break;
                }     //д���ݳɹ�
            }
        }

        //������ݿ�û�л����Ͳ�Ҫ�������д�Ŀ飬����ѯ��Ӧ����˵��������Ŀ飬��Ӧ����д
        if(1 == need_erase)
        {
            //��¼���д�Ŀ��
            SysFile_RBlock_Set(WBN);
            //--ֻ���������¿��ʱ�򣬲Ż�ı�����
            if(j == 0)//�ļ���һ��(�ļ�����)
            {
                //���¿�ʼ���
                SysFile_Starlink_Set(FNum, WBN|0x8000);
            }
            else
            {
                RBN = FileBNum[j - 1];   //ǰ��һ���ż�¼����Ŀ��

                CS = SysFile_Block_GetCS(RBN);
                SysFile_Block_SetStatus(RBN, WBN|0x8000, CS);

            }
        }

#if 0 /*��ѧ�� 2014-1-28*/
        FileSysB.Block[WBN].CS = CountCheckSum(FSL.Buf,i); //�����У��:���У��
        FsDebug("---����У��͵ĳ���:%d, ���:%d, CS:%d\r\n", i, WBN, FileSysB.Block[WBN].CS);
#endif

#if 0 /*��ѧ�� 2014-1-29*/
        if(j == 0)//�ļ���һ��(�ļ�����)
        {
            SysFile_U16CC(FileSysB.File[FNum].StartLink,WBN|0x8000); //���¿�ʼ���
        }
        else
        {
            RBN = FileBNum[j - 1];   //ǰ��һ���ż�¼����Ŀ��

            CS = SysFile_Block_GetCS(RBN);
            SysFile_Block_SetStatus(RBN, WBN|0x8000, CS);

        }
#endif

        //���ñ���״̬
        //sys_debug_format_u8(FSL->Buf, i);
        CS = CountCheckSum(FSL->Buf,i);//�����У��:���У��
        FsDebug("����У��͵ĳ���:%d, ���:%d, CS:0x%02x...", i, WBN, CS);

        if(j < (BNum-1))
        {
            //��ǰ�����鲻�����һ��
            //��¼��һ��Ŀ��
            FsDebug("�����һ�飬�޸Ŀ�������У���\r\n");
            SysFile_Block_SetStatus(WBN, FileBNum[j+1]|0x8000, CS);
        }
        else
        {
            //���һ��
            ////���ݵĳ���
            FsDebug("���һ�飬�޸ĳ�����У���\r\n");
            SysFile_Block_SetStatus(WBN, i, CS);
        }


        if(FileBNum[j] < SYSFILE_BLOCKNUM)
        {
            //�ͷ�ԭ���Ŀ�??????
            if(WBN != FileBNum[j])//��ǰд�Ŀ鲻��ԭ���Ŀ�,��ԭ���Ŀ��ͷ�
            {

                SysFile_Block_SetStatus(FileBNum[j], UNBlock, 0XFF);

            }
        }

        FileBNum[j] = WBN;

        if(WDLen==0)//д����
        {
            break;
        }
    }
    //========д���ݳɹ�=======================
    if(WAdd > FLen)
    {
        //�ļ�����
        FsDebug("�޸��ļ�����\r\n");
        SysFile_File_SetLen(FNum, WAdd);//�����ļ��ĳ���
    }
    else
    {
        FsDebug("����Ҫ�޸��ļ�����\r\n");
    }

    SysFile_File_SetCs(FNum);

    goto END;

ConverFile: //�������û�н�����Ҫ��ԭ��ԭ��������
//�����һ���ļ�������ݣ������ݶ������ˣ��ָ��������?
    FsDebug("������ʧ��\r\n");

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
        FsDebug("ϵͳ�ָ�ʧ��\r\n");
    }

END:
    k_free(FSL);
    k_free(FileBNum);
    FsDebug("д�ļ�����end:%u Re:%u \r\n",FNum,Re);
    FsDebug("\r\n**********************************\r\n");

    return Re;
}

//===========================================================================
//����:���ļ�����
//����:FNum�ļ���,StrAλ��,DLen Ҫ�������ݳ���
//���:RD����,DLen ��ȡ�����ݳ���
//����:0:�ɹ� ����:ʧ�ܴ���
u8 SysFile_ReadFileData(u8 FNum,u8 *RD,u32 StrA,u32 *DLen) // 20160815
{
    u8 Re;
    u8 CS;
    u16 i,CsLen;
    
    //u16 FileBNum[FileSysBlockNum+20]; //���ļ����õĿ��
    u16 *FileBNum;
    u16 RBN;                          //��ǰ�������
    u16 j;                            //��ǰ��������ǵ�j��
    u16 BNum;                         //���ļ����õĿ���
    u16 RBSA;                         //�鿪ʼ����ַ
    u32 FLen;                         //�ļ�����
    u32 RAdd;                         //д��ַ
    u32 RDLen;                        //Ҫ�������ݳ���
    u32 OutLen;
    union _FileSysBlockBuf *FSL;

    OutLen = 0;   //��ȡ���ݳ���

    FsDebug("ReadFileData len :%d\r\n", *DLen);

    if((*DLen) == 0)
        return SOK;

    FSL = (union _FileSysBlockBuf *)k_malloc(sizeof(union _FileSysBlockBuf));
    if(FSL == NULL)
    {
        FsDebug("�ڴ�����ʧ�� ReadFileData\r\n");
        while(1)
        {}
    }

    FileBNum = (u16 *)k_malloc(sizeof(u16)*(MaxBNum+20));
    if(FileBNum == NULL)
    {
        FsDebug("�ڴ�����ʧ�� ReadFileData\r\n");
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

            j = RAdd/BlockLen;      //Ҫ�����������ļ��ĵ�j��
            if(j >= BNum || RAdd >=FLen)   //sxl 1117
            {
                Re = FEnd;
                break;
            }
            
            RBN = FileBNum[j];      //��ǰ���Ŀ���
            RBSA = RAdd%BlockLen;   //��ǰ���Ŀ��ƫ����
            
            FsDebug("ReadFileData block :%d\r\n", RBN);
            
            Re = SysFile_ReadFileSysBlock(RBN,FSL->Buf,BlockLen);//��ȡһ������, ������ζ���һ�飬��ʱ
            if(Re == SOK )
            {
                FsDebug("ReadFileData block :OK\r\n", RBN);
                CsLen = SysFile_Block_GetStatus(RBN);
                if(CsLen >= BlockLen)
                    CsLen = BlockLen;
                    
                //sys_debug_format_u8(FSL->Buf, CsLen);
                
                CS = CountCheckSum(FSL->Buf,CsLen);//

                //��������
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
                    Re = FILECSR;    //����У�����
                    //sys_debug_format_u8(FSL->Buf, CsLen);
                    
                    FsDebug("ReadFileData1(������У�����):CS:%04x %04x Re:%u \r\n",CS, SysFile_Block_GetCS(RBN), Re);

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
    //printf("ReadFileData1:CS:%04x %04x Re:%u \r\n",CS,SysFile_CCU16(FileLinkB.Block[RBN].CS),Re);  //sxl �����ã�Ҫɾ��
    return Re;
}

/****************************************************************************
**Description:         У�������ļ�ϵͳ  
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
    //u16 RBN;                          //��ǰ�������
    //u16 j;                            //��ǰ��������ǵ�j��

    union _FileSysBlockBuf *FSL;
    u16 tmp;

    FSL = (union _FileSysBlockBuf *)k_malloc(sizeof(union _FileSysBlockBuf));
    if(FSL == NULL)
    {
        FsDebug("�ڴ�����ʧ�� ReadFileData\r\n");
        while(1)
        {}
    }

    FileBNum = (u16 *)k_malloc(sizeof(u16)*(MaxBNum+20));
    if(FileBNum == NULL)
    {
        FsDebug("�ڴ�����ʧ�� ReadFileData\r\n");
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
            FsDebug("δʹ��--");
        }
        else if(tmp == BadBlock)
        {
            FsDebug("����--");
        }
        else if(tmp == LogBlock)
        {
            FsDebug("**��־��--");
        }
        else if(tmp <= BlockLen)
        {
            FsDebug("������ʹ��---");
        }
        else if(tmp == BlockLen + FileSysBLen)
        {
            FsDebug("�ļ�ϵͳ��---");
        }
        else if(tmp > 0X8000)
        {
            FsDebug("���꣬����%d---", tmp - 0X8000);
        }
        else
        {
            FsDebug("δ֪--");
        }
     
        Re = SysFile_ReadFileSysBlock(i,FSL->Buf,BlockLen);//��ȡһ������, ������ζ���һ�飬��ʱ
        if(Re == SOK )
        {
            FsDebug("OK--");
            CsLen = SysFile_Block_GetStatus(i);//�����ݳ���
            if(CsLen >= BlockLen)
                CsLen = BlockLen;
            FsDebug("%d--", CsLen);    
            //sys_debug_format_u8(FSL->Buf, CsLen);
            CS = CountCheckSum(FSL->Buf,CsLen);//

            if(CS != SysFile_Block_GetCS(i))
            {
                Re = FILECSR;    //����У�����

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
    //printf("ReadFileData1:CS:%04x %04x Re:%u \r\n",CS,SysFile_CCU16(FileLinkB.Block[RBN].CS),Re);  //sxl �����ã�Ҫɾ��
    return Re;
}

//-----------------����ϵͳ��־�Ĳ���-------------------
#ifdef LOG_MODE

#define SysFile_Log_MAX 64//һ�������32��LOG

struct _FileSysLog
{
    u8 Flag[4];//log,�������LOG�ռ��Ƿ������һ��LOG
    u8 type;//�������ͣ��뿴�궨��
    u8 cs;//����LOG��У���
};
typedef  struct _FileSysLog SysFileLog;

struct _FileSysLogB
{
    u8 Flag[16];//"xgdfs_logs.log",�ַ�����ʶ
    u8 SysBT[6];//ϵͳ���д����������LOG��ϵͳ�����
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

//ÿ��LOG�ĳ��ȶ��Ƕ�ֵ
struct _sLogDetail
{
    u8 name[32];
    u8 len;
};


#define XGDFS_LOGB_FLAG "xgdfs_logs.log"//��־���ʶ

static u16 LogsBlock1 = 0XFFFF;//��
static u16 LogsBlock2 = 0XFFFF;//��

/****************************************************************************
**Description:      ��һ��ָ���Ŀ��ʼ��ΪLOG��
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

    FsDebugLog("\r\n\r\n--->>>>>logs L2: ������־:%04x---\r\n", blocknum);

    if(sizeof(struct _FileSysLogB) >= BlockLen)
    {
        while(1)
        {
            FsDebug("log block too len!!!");
        }
    }

    //��ʼ�� FileSysLogB
    memset(&FileSysLogB, 0xff, sizeof(FileSysLogB));
    memcpy(&(FileSysLogB.Flag[0]), XGDFS_LOGB_FLAG, strlen(XGDFS_LOGB_FLAG) + 1);

    //������LOG��ʱ���ļ�ϵͳ����д��LOG����
    memcpy(&(FileSysLogB.SysBT[0]), &(FileSysB.WdateTime[0]), 6);

    //�� FileSysLogB д���ոշ���Ŀ���
    Re = SysFile_WriteFileSysBlock(blocknum, &(FileSysLogB.Flag[0]), 22);//ֻд�õ����ֽ�
    if(Re == SOK)
    {
        //���ÿ�״̬
        SysFile_Block_SetStatus(blocknum, LogBlock, 0XFF);//log�鲻��У��
        FsDebug("...�����ļ�ϵͳLOG�ɹ� %d...", blocknum);
        ret = 0;
    }
    else
    {
        FsDebug("...�����ļ�ϵͳLOGʧ�� %d...", blocknum);
        ret = -1;
    }

    return ret;

}

/****************************************************************************
**Description:      ���һ��ָ�����Ƿ���LOG��
**Input parameters:
**Output parameters: WdateTime ʱ���
**
**Returned value: 0 �ɹ� -1 ʧ��
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
        FsDebug("��־�� %d...", blocknum);
        //�����LOG��ı�ʶ���ٴ��ж�
        Re = SysFile_ReadFileSysBlock(blocknum, buff, 22); //��ȡ12���ֽڵ�ͷ����

        if(Re == SOK)  //�����ݳɹ�
        {
            //sys_debug_format_u8(buff, 22);

            if(0 == strcmp(buff, XGDFS_LOGB_FLAG))
            {
                FsDebug("��ʶ��ȷ...");
                //sys_debug_format_u8(&buff[16], 6);
                memcpy(WdateTime, &buff[16], 6);
                ret = 0;
            }
            else
            {
                FsDebug("��ʶ����...");
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
**Description:      �����־�ļ�������ޣ�����  �������ڻ�ȡ�ļ�ϵͳʱ����
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

    //��ʼ��Ϊ0
    memset(WdateTime1, 0x00, sizeof(WdateTime1));
    memset(WdateTime2, 0x00, sizeof(WdateTime2));

    FsDebug("\r\n�����־�ļ�...");

    LogsBlock1 = 0XFFFF;//������ҵ������ӿ��
    LogsBlock2 = 0XFFFF;

    for(i = 0; i < SYSFILE_BLOCKNUM; i++)
    {
        relblocknum = 0xffff;

        ret = SysFile_Logs_CheckBlock(i, WdateTimet);
        if(ret == 0)
        {
            //��δ������������
            if(memcmp(WdateTimet, WdateTime2, 6) >= 0)//WdateTimet��2��
            {
                if(LogsBlock1 != 0XFFFF)
                {
                    FsDebug("�Ѿ��ҵ�����...");
                    relblocknum =  LogsBlock1;//��BLOCK 1 �ż�ס�������ͷ�
                }

                memcpy(WdateTime1, WdateTime2, 6);//����1
                LogsBlock1 = LogsBlock2;
                memcpy(WdateTime2, WdateTimet, 6);
                LogsBlock2 = i;//��ס���
            }
            else
            {

                if(memcmp(WdateTimet, WdateTime1, 6) >= 0)//WdateTimet��2С�����Ǳ�1��
                {
                    if(LogsBlock1 != 0XFFFF)
                    {
                        relblocknum =  LogsBlock1;
                    }

                    memcpy(WdateTime1, WdateTimet, 6);
                    LogsBlock1 = i;//��ס���
                }
                else
                {
                    relblocknum =  i;
                }
            }

            //�ͷŶ���Ŀ飬�����޸�ϵͳ�����ͷţ�������LOG�鲢û�в�
            if(relblocknum != 0xffff)
            {
                FsDebugLog("\r\n\r\n--->>>>>logs L2: �ͷŶ�����־:%d---\r\n", relblocknum);

                FsDebug("�ͷ���־�� :%d...", relblocknum);
                SysFile_Block_SetStatus(relblocknum, UNBlock, 0xff);
            }

            cnt++;

        }
    }
    FsDebug("������־�� :%d...", cnt);

    if(cnt == 0)
    {
        //�᲻����ϵͳ�鱻�Ҹ���? �����Զ��?
        FsDebug("��Ҫ������־�ļ�...");

        while(cnt <2)
        {
            WBN = SysFile_LookupUseBlock();
            if(WBN == 0xffff)
            {
                FsDebug("�޿��ÿ�...");
                break;
            }
            else
            {
                //��һ��������Ϊ��־��
                ret = SysFile_Logs_init(WBN);
                if(ret == -1)
                {
                    FsDebug("log����ʧ��...");
                    break;
                }
                else
                {
                    FsDebug("log�����ɹ�...");
                    LogsBlock1 = LogsBlock2;
                    LogsBlock2 = i;
                }
            }

            cnt++;

        }
    }
    else if(cnt == 2)
    {
        FsDebug("��־�ļ�OK...");
    }
    else if(cnt >2)
    {
        FsDebug("��־�����...���޸�...");
    }

    FsDebug("������\r\n");
}
/****************************************************************************
**Description:      ���һ��LOG
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

    FsDebug("���һ��LOG\r\n");
}
/****************************************************************************
**Description:      ��ѯϵͳ��־
**Input parameters:  Index ��־������0-MAX��0Ϊ����
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
**Description:      ����ϵͳ��Ϣ����system�ļ�ϵͳ
**Input parameters:
**Output parameters:
**
**Returned value: -1 ����ʧ��  0 ���سɹ�
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

	if((SysFsHdPra)->BlockNum > FileSysBlockNum)//���ܳ���ϵͳ����
	{
		(SysFsHdPra)->BlockNum = FileSysBlockNum;
	}

	(SysFsHdPra)->EndAdd = (SysFsHdPra)->StrAdd + (SysFsHdPra)->Len;

	FsDebug("\r\nBlockNum = %d %08x %08x\r\n",(SysFsHdPra)->BlockNum,(SysFsHdPra)->BLen,(SysFsHdPra)->EndAdd);
	
	ret = SysFile_AcquireFileSysData();//��������ļ�ϵͳ
    FsDebug("\r\nSysFile_AcquireFileSysData ret = %d\r\n",ret);
	if(ret != 0)
	{
		SysFile_FoundFileSys();
	}

	return 0;
	
}

