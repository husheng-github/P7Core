
#include "devglobal.h"

#include "table/PitTable.h"


#define VER_CSTCODE_R5_CORE      "C"   //代表CORE
#define VER_CSTCODE_R5_BOOT      "B"   //代表BOOT


#if defined(SYS_DEBUG_EN)
#define VER_CSTCODE_R4      "D" 
#else
#define VER_CSTCODE_R4      "R"     
#endif


//安全CPU版本,Z1兆讯1902
#define VER_CSTCODE_R3      "Z"  
#define VER_CSTCODE_R2      "1"  


//安全已启用版本
#define VER_CSTCODE_R1      "S" 






extern int main(void);

#ifdef TRENDIT_CORE



#define TRENDIT_RESERV_VER_ZONE_CORE VER_CSTCODE_R5_CORE VER_CSTCODE_R4 VER_CSTCODE_R3 \
                                VER_CSTCODE_R2 VER_CSTCODE_R1
                                

//#define IMAGE_VERSION_CORE   "V10" TRENDIT_RESERV_VER_ZONE_CORE "170421" "001"
//#define IMAGE_VERSION_CORE   "V10" TRENDIT_RESERV_VER_ZONE_CORE "170807" "001"  //修改DDI接口及updateflg保存位置
//#define IMAGE_VERSION_CORE   "V10" TRENDIT_RESERV_VER_ZONE_CORE "170809" "001"  //修改DDI接口及updateflg保存位置
//#define IMAGE_VERSION_CORE   "V10" TRENDIT_RESERV_VER_ZONE_CORE "170918" "001"  //修改低功耗和打印机
//#define IMAGE_VERSION_CORE   "V10" TRENDIT_RESERV_VER_ZONE_CORE "170920" "001"  //修改非接和打印机程序
//#define IMAGE_VERSION_CORE   "V10" TRENDIT_RESERV_VER_ZONE_CORE "171101" "001"  //1.修改非接和打印机程序,
//#define IMAGE_VERSION_CORE   "V10" TRENDIT_RESERV_VER_ZONE_CORE "180125" "002"     //修改触发配置
//#define IMAGE_VERSION_CORE   "V10" TRENDIT_RESERV_VER_ZONE_CORE "180131" "002"     //
//#define IMAGE_VERSION_CORE   "V10" TRENDIT_RESERV_VER_ZONE_CORE "180420" "002"     //增加硬件版本(ADC)
//#define IMAGE_VERSION_CORE   "V10" TRENDIT_RESERV_VER_ZONE_CORE "180426" "002"     
//#define IMAGE_VERSION_CORE   "V10" TRENDIT_RESERV_VER_ZONE_CORE "180504" "002"         //修改ADC读不准问题，Open后需要延时20ms
//#define IMAGE_VERSION_CORE   "V10" TRENDIT_RESERV_VER_ZONE_CORE "180709" "002"         //解锁后将安全等级设为2
//#define IMAGE_VERSION_CORE   "V10" TRENDIT_RESERV_VER_ZONE_CORE "180716" "003"      //增加启动触发判断条件
//#define IMAGE_VERSION_CORE     "V10" TRENDIT_RESERV_VER_ZONE_CORE "180726" "001"      //增加启动触发判断条件,上版没有判断开机时会恢复密钥的密钥
//#define IMAGE_VERSION_CORE     "V10" TRENDIT_RESERV_VER_ZONE_CORE "181229" "001"      //
//#define IMAGE_VERSION_CORE     "V10" TRENDIT_RESERV_VER_ZONE_CORE "190307" "001"      //普瑞特帮助调整了打印补偿参数，解决黑块异响问题
//#define IMAGE_VERSION_CORE     "V10" TRENDIT_RESERV_VER_ZONE_CORE "190417" "001"      //兼容M6程序
//#define IMAGE_VERSION_CORE     "V10" TRENDIT_RESERV_VER_ZONE_CORE "190529" "001"        //兼容M6程序 
//#define IMAGE_VERSION_CORE     "V10" TRENDIT_RESERV_VER_ZONE_CORE "190709" "001"        //兼容T6程序 
//#define IMAGE_VERSION_CORE     "V10" TRENDIT_RESERV_VER_ZONE_CORE "190812" "001"          //针对工厂三磁道刷不出来的问题，修改磁卡库mh1902_msr.lib 
//#define IMAGE_VERSION_CORE     "V10" TRENDIT_RESERV_VER_ZONE_CORE "190819" "001"        //1.SP主频由96Mhz改为72Mhz, 2.增加了SM4和分段加密处理  
//#define IMAGE_VERSION_CORE     "V10" TRENDIT_RESERV_VER_ZONE_CORE "190826" "001"        //对于参数新版本更新到老版本，不恢复默认参数。
//#define IMAGE_VERSION_CORE     "V10" TRENDIT_RESERV_VER_ZONE_CORE "190829" "001"        //1.兼容T3、T5,  2.处理机型变量gMachineID存在于不同文件问题。
//#define IMAGE_VERSION_CORE     "V10" TRENDIT_RESERV_VER_ZONE_CORE "191008" "001"        //1.处理打印缺纸
//#define IMAGE_VERSION_CORE     "V10" TRENDIT_RESERV_VER_ZONE_CORE "191010" "001"        //1.处理刷卡问题
//#define IMAGE_VERSION_CORE     "V10" TRENDIT_RESERV_VER_ZONE_CORE "191012" "001"        //1.兼容T1程序
//#define IMAGE_VERSION_CORE     "V56" TRENDIT_RESERV_VER_ZONE_CORE "200115" "005"        //1.按键改中断触发
//#define IMAGE_VERSION_CORE     "V" MACHINE_P7 TRENDIT_RESERV_VER_ZONE_CORE "201107" "002"        //1.USB支持双设备(CDC\Print)
#define IMAGE_VERSION_CORE     "V" MACHINE_P7 TRENDIT_RESERV_VER_ZONE_CORE "210112" "004"  //1.解决插拔USB会导致机器死机的问题


//放在第一个包的2K的位置，好判断版本
const struct trendit_sp_header_s  corePitTable __attribute__((at(COREPITTABLEOFFSET)))  =
{
    TRENDIT_TAG_CORE,   // tag     BISO CORE  APPL
    0x1018000,//startAddr;        //Core映像链接地址
    0x30000,  //imageMaxSize;     //映像最大尺寸
    (unsigned int)main,//entryAddr;      //映像入口地址
    0x00000000,        //version;          //当前版本，暂时未使用
    0xFFFFFFFF,     // depHighVer;      //能够支持的最高版本，加载时程序根据此域作版本匹配
    0x00000000,     //depLowVer;    //能够支持的最低版本
    0x00000000,   //apiTableAddr; // BIOS,此域无效，保存API表地址，APP的表地址也无效
    IMAGE_VERSION_CORE,  //包含日期，YYMMDD，以NULL结尾
    "00000000", //前半部分SHA256
    "00000000",  //后半部分SHA256
    __DATE__" "__TIME__" ",          //desc
};


#else

#define TRENDIT_RESERV_VER_ZONE_BOOT VER_CSTCODE_R5_BOOT VER_CSTCODE_R4 VER_CSTCODE_R3 \
                                VER_CSTCODE_R2 VER_CSTCODE_R1
                                

//#define IMAGE_VERSION_BOOT   "V10" TRENDIT_RESERV_VER_ZONE_BOOT "170421" "001"
//#define IMAGE_VERSION_BOOT   "V10" TRENDIT_RESERV_VER_ZONE_BOOT "170807" "001"  //修改updateflg保存位置
//#define IMAGE_VERSION_BOOT   "V10" TRENDIT_RESERV_VER_ZONE_BOOT "170821" "001"  //修改低功耗
//#define IMAGE_VERSION_BOOT   "V10" TRENDIT_RESERV_VER_ZONE_BOOT "170920" "001"  //
//#define IMAGE_VERSION_BOOT   "V10" TRENDIT_RESERV_VER_ZONE_BOOT "171101" "001"  //
//#define IMAGE_VERSION_BOOT   "V10" TRENDIT_RESERV_VER_ZONE_BOOT "171222" "001" 
//#define IMAGE_VERSION_BOOT   "V10" TRENDIT_RESERV_VER_ZONE_BOOT "190626" "001" 
//#define IMAGE_VERSION_BOOT   "V10" TRENDIT_RESERV_VER_ZONE_BOOT "190709" "001" //兼容T6
//#define IMAGE_VERSION_BOOT   "V10" TRENDIT_RESERV_VER_ZONE_BOOT "190812" "001"  //针对工厂三磁道刷不出来的问题，修改磁卡库mh1902_msr.lib 
//#define IMAGE_VERSION_BOOT   "V10" TRENDIT_RESERV_VER_ZONE_BOOT "190819" "001"  //SP主频由96Mhz改为72Mhz  
//#define IMAGE_VERSION_BOOT   "V10" TRENDIT_RESERV_VER_ZONE_BOOT "190826" "001"  //对于参数新版本更新到老版本，不恢复默认参数。
//#define IMAGE_VERSION_BOOT   "V10" TRENDIT_RESERV_VER_ZONE_BOOT "190829" "001"  //1.兼容T3、T5,  2.处理机型变量gMachineID存在于不同文件问题。
//#define IMAGE_VERSION_BOOT   "V10" TRENDIT_RESERV_VER_ZONE_BOOT "190930" "001"  //1.1902S 750K
//#define IMAGE_VERSION_BOOT   "V10" TRENDIT_RESERV_VER_ZONE_BOOT "191008" "001"  //1.处理打印缺纸问题
//#define IMAGE_VERSION_BOOT   "V10" TRENDIT_RESERV_VER_ZONE_BOOT "191010" "001"  //1.处理刷卡问题
//#define IMAGE_VERSION_BOOT   "V10" TRENDIT_RESERV_VER_ZONE_BOOT "191012" "001"  //1.兼容T1程序
//#define IMAGE_VERSION_BOOT   "V56" TRENDIT_RESERV_VER_ZONE_BOOT "200115" "005"
#define IMAGE_VERSION_BOOT   "V" MACHINE_P7 TRENDIT_RESERV_VER_ZONE_BOOT "200917" "001"


//放在第一个包的2K的位置，好判断版本
const struct trendit_sp_header_s  bootPitTable __attribute__((at(BOOTPITTABLEOFFSET)))  =
{
    TRENDIT_TAG_BIOS,   // tag     BISO CORE  APPL
    0x1001000,//startAddr;        //Core映像链接地址
    0x17000,  //imageMaxSize;     //映像最大尺寸
    (unsigned int)main,//entryAddr;      //映像入口地址
    0x00000000,        //version;          //当前版本，暂时未使用
    0xFFFFFFFF,     // depHighVer;      //能够支持的最高版本，加载时程序根据此域作版本匹配
    0x00000000,     //depLowVer;    //能够支持的最低版本
    0x00000000,   //apiTableAddr; // BIOS,此域无效，保存API表地址，APP的表地址也无效
    IMAGE_VERSION_BOOT,  //包含日期，YYMMDD，以NULL结尾
    "00000000", //前半部分SHA256
    "00000000",  //后半部分SHA256
    __DATE__" "__TIME__" ",          //desc
};


#endif


//从物理地址读取版本信息
void pitver_get_corever(unsigned char *ver)
{
    unsigned int len;
    
     struct trendit_sp_header_s *corever;
    corever = (struct trendit_sp_header_s *)COREPITTABLEOFFSET;
    
    len = strlen(corever->verStr);
    if(len >= 20)
    {
        len = 19;
    }
    
    memcpy(ver,corever->verStr,len);
    ver[len] = 0;
    
}


void pitver_get_bootver(unsigned char *ver)
{
       unsigned int len;
    
     struct trendit_sp_header_s *bootver;
    bootver = (struct trendit_sp_header_s *)BOOTPITTABLEOFFSET;
    
    len = strlen(bootver->verStr);
    if(len >= 20)
    {
        len = 19;
    }
    
    memcpy(ver,bootver->verStr,len);
    ver[len] = 0;
    
}


void pitver_get_hardwarever(unsigned char *ver)
{
    s32 ret;

    ret = dev_misc_get_hwver();
    if(ret < 0)
    {
        ret = dev_misc_get_hwver();
    }
    //strcpy(ver,"VH1.01");
    if(ret < 0)
    {
        strcpy(ver, "VHFF");
    }
    else
    {
        sprintf(ver, "VH1.%02d", ret);
    }
}
void pitver_get_appver(unsigned char *ver)
{
    unsigned int len;
    u8 *appver;
    
    appver = (u8*)(APP_START_ADDR+MCU_BASE_ADDR);
    
    len = strlen(appver);
    if(len >= 20)
    {
        len = 19;
    }
    
    memcpy(ver,appver,len);
    ver[len] = 0;
    
}

