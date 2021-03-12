#ifndef __DRV_CL_PARAMETER_H__
#define __DRV_CL_PARAMETER_H__

#define CL_PARAM_LEN_MAX  32
#define CL_PARAM_SIZE   4
#define CL_PAR_ModGsp    0x90
#define CL_PAR_RFCfg_A    0x06
#define CL_PAR_RFCfg_B    0x06
#define CL_PAR_RFOLEVEL    0x00

typedef __packed struct _CL_PARAM
{
    u8 ModGsP;           //MODGSP  TypeB卡的调制深度()
    u8 RFCfg_A;          //RFCfgReg,控制TypeA卡的接收增益(b7~b5)  
    u8 RFCfg_B;          //RFCfgReg,控制TypeB卡的接收增益(b4~b2)
    u8 RFOLevel;         //RFONormalLevel,调节RFO输出功率
    //u8 RxTreshold_A;
    u8 RFU[27];//6];             //预留
    u8 crc;  
}CL_PARAM;
extern CL_PARAM g_cl_param;

s32 drv_cl_read_param(u8 *buf,u32 size);
s32 drv_cl_store_param(u8 *buf,u32 size);
void drv_cl_default_param(void);
s32 drv_cl_param_init(void);
u8 crc8_cal(u8 *pdata,u32 len);
void calculate_param_checksum(CL_PARAM *param);

#endif

