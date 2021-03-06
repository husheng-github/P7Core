#include <string.h>
#include "dev.h"
//#include "ams_types.h"
#include "dev_cl_hal.h"
//#include "dev_as3911.h"

#include "drv_cl_parameter.h"
#include "debug.h"

static const u8  cl_par_path[] = "cl_par.cfg";
CL_PARAM g_cl_param;

const u8 crc8_tab[256]={
    0x0,0x7,0xe,0x9,0x1c,0x1b,0x12,0x15,    
    0x38,0x3f,0x36,0x31,0x24,0x23,0x2a,0x2d,    
    0x70,0x77,0x7e,0x79,0x6c,0x6b,0x62,0x65,    
    0x48,0x4f,0x46,0x41,0x54,0x53,0x5a,0x5d,    
    0xe0,0xe7,0xee,0xe9,0xfc,0xfb,0xf2,0xf5,    
    0xd8,0xdf,0xd6,0xd1,0xc4,0xc3,0xca,0xcd,    
    0x90,0x97,0x9e,0x99,0x8c,0x8b,0x82,0x85,    
    0xa8,0xaf,0xa6,0xa1,0xb4,0xb3,0xba,0xbd,    
    0xc7,0xc0,0xc9,0xce,0xdb,0xdc,0xd5,0xd2,    
    0xff,0xf8,0xf1,0xf6,0xe3,0xe4,0xed,0xea,    
    0xb7,0xb0,0xb9,0xbe,0xab,0xac,0xa5,0xa2,    
    0x8f,0x88,0x81,0x86,0x93,0x94,0x9d,0x9a,    
    0x27,0x20,0x29,0x2e,0x3b,0x3c,0x35,0x32,    
    0x1f,0x18,0x11,0x16,0x3,0x4,0xd,0xa,    
    0x57,0x50,0x59,0x5e,0x4b,0x4c,0x45,0x42,    
    0x6f,0x68,0x61,0x66,0x73,0x74,0x7d,0x7a,    
    0x89,0x8e,0x87,0x80,0x95,0x92,0x9b,0x9c,    
    0xb1,0xb6,0xbf,0xb8,0xad,0xaa,0xa3,0xa4,    
    0xf9,0xfe,0xf7,0xf0,0xe5,0xe2,0xeb,0xec,    
    0xc1,0xc6,0xcf,0xc8,0xdd,0xda,0xd3,0xd4,    
    0x69,0x6e,0x67,0x60,0x75,0x72,0x7b,0x7c,    
    0x51,0x56,0x5f,0x58,0x4d,0x4a,0x43,0x44,    
    0x19,0x1e,0x17,0x10,0x5,0x2,0xb,0xc,    
    0x21,0x26,0x2f,0x28,0x3d,0x3a,0x33,0x34,    
    0x4e,0x49,0x40,0x47,0x52,0x55,0x5c,0x5b,    
    0x76,0x71,0x78,0x7f,0x6a,0x6d,0x64,0x63,    
    0x3e,0x39,0x30,0x37,0x22,0x25,0x2c,0x2b,    
    0x6,0x1,0x8,0xf,0x1a,0x1d,0x14,0x13,    
    0xae,0xa9,0xa0,0xa7,0xb2,0xb5,0xbc,0xbb,    
    0x96,0x91,0x98,0x9f,0x8a,0x8d,0x84,0x83,    
    0xde,0xd9,0xd0,0xd7,0xc2,0xc5,0xcc,0xcb,    
    0xe6,0xe1,0xe8,0xef,0xfa,0xfd,0xf4,0xf3
}; 
u8 crc8_cal(u8 *pdata,u32 len)
{
    u8  crc=0;   
    while (len--!=0)   
    {      
        crc = crc8_tab[*pdata^crc];   
        pdata++;   
    }   
    return crc;      
}

void calculate_param_checksum(CL_PARAM *param)
{
    u8 n;
    for(n=0;n<sizeof(CL_PARAM)-CL_PARAM_SIZE;n++)
        param->RFU[n] = 0;
    param->crc = crc8_cal((u8 *)param,sizeof(CL_PARAM)-1); 
}

s32 verify_param_checksum(CL_PARAM *param)
{
    if(param->crc == crc8_cal((u8 *)param,sizeof(CL_PARAM)-1))
        return 0;
    return 1;
}

s32 drv_cl_read_param(u8 *buf,u32 size)
{
    s32 fd;
    s32 ret = 0;
    s8 FNum;
    s8 re;
//return -1;
    if(SOK == (re = AccessExternalFile((s8 *)cl_par_path)))
    {
        if(SOK == (re =ReadExternalFile(cl_par_path,buf,0,&size)))
        {
            ret = 0;
        }
        else
        {
    printd("Read cl par fail\r\n");        
            ret = -2;
        }
    }
    else
    {
printd("Open cl par fail\r\n");        
        ret = -1;
    }
    return ret;//0410
}

s32 drv_cl_store_param(u8 *buf,u32 size)
{
    s32 fd;
    s32 ret = 0;
    s8 re;
//return -1;

    if(SOK != (re = WriteExternalFile((u8 *)cl_par_path,buf,size)))
    {
printd("Store cl par failed\r\n");
        ret = -1;
    }
    
    return ret;//0410
}

void drv_cl_default_param(void)
{
    memset((u8*)&g_cl_param, 0, sizeof(g_cl_param));
    g_cl_param.ModGsP = CL_PAR_ModGsp;
    g_cl_param.RFCfg_A = CL_PAR_RFCfg_A; 
    g_cl_param.RFCfg_B = CL_PAR_RFCfg_B;
    g_cl_param.RFOLevel = CL_PAR_RFOLEVEL;
    calculate_param_checksum(&g_cl_param);
}
/**************************************************************
 ** Descriotions:  ??????????????
 ** parameters:                                            
 ** Returned value:                                                                          
 ** Created By:     ?????? 2013-5-18 20:43:03           
 ** Remarks:                                                                                 
***************************************************************/
s32 drv_cl_param_init(void)
{
    u32 len;
    u8 n;
    u8 buf[sizeof(g_cl_param)];
    len = sizeof(g_cl_param);

//    fs_init();
//    if(AcquireFileSysData())
//        FoundFileSys();

    memset(buf,0,sizeof(buf));
    buf[0] = 0xFF;
    drv_cl_read_param( buf, CL_PARAM_LEN_MAX);
    if(crc8_cal(buf,sizeof(g_cl_param)))  //crc check fail
//if(1)
    {
        printd("cl:par invalid\r\n");
        drv_cl_default_param();
        if(drv_cl_store_param((u8 *)&g_cl_param,CL_PARAM_LEN_MAX))
            printd("cl:par write fail\r\n");
    }
    else
    {
        memcpy((u8 *)&g_cl_param,buf,sizeof(g_cl_param));
        printd("cl parameter:");
//        printd("%02X %02X %02X %02X %02X\r\n",g_cl_param.ModGsP,g_cl_param.RFCfg_A,g_cl_param.RFCfg_B,g_cl_param.RFOLevel,g_cl_param.crc);
        printd("pn512:par read done\r\n");
    }
    return 0;
}


