#ifndef __DDI_CONFIG_H
#define __DDI_CONFIG_H

#include "devglobal.h"

//定义返回值
#define DDI_OK          (0)//成功
#define DDI_ERR         (-1)//错误
#define DDI_ETIMEOUT    (-2)//超时
#define DDI_EBUSY       (-3)//设备繁忙
#define DDI_ENODEV      (-4)//设备不存在
#define DDI_EACCES      (-5)//无权限
#define DDI_EINVAL      (-6)//参数无效
#define DDI_EIO         (-7)//设备未打开或设备操作出错
#define DDI_EDATA       (-8)//数据错误
#define DDI_EPROTOCOL   (-9)//协议错误
#define DDI_ETRANSPORT  (-10)//传输错误
#define DDI_ESIM		(-11)//SIM卡检测错误
#define DDI_EOPERATION	(-12)//注册网络错误
#define DDI_ESET_APN 	(-13)//设置APN失败
#define DDI_EWOULD_BLOCK (-14)//被阻塞
#define DDI_EOVERFLOW	(-15)//内存溢出
#define DDI_EUNKOWN_CMD  (-16)//未知命令
#define DDI_EINVALID_IP  (-17)//IP地址非法
#define DDI_ELOW_CSQ     (-18)//信号低
#define DDI_EWIRE_PDP_ACTIVING  (-19)//正在激活数据网络
#define DDI_EUN_SUPPORT         (-20)//不支持
#define DDI_EREG_DENIED         (-21)//注册网络被拒
#define DDI_EHIGHTEMP           (-22)//温度过高
#define DDI_ENOFILE             (-23)//文件不存在
#define DDI_EHAVE_DONE          (-24)//命令重复，已执行过
#define DDI_EDNS_FAIL           (-30)//域名解析失败




#define DDI_COM_ENABLE
//#define DDI_ICCPSAM_ENABLE
//#define DDI_MAG_ENABLE               //sxl?
//#define DDI_RF_ENABLE
//#define DDI_MIFARE_ENABLE  
#define DDI_AUDIO_ENABLE
#define DDI_LED_ENABLE             //sxl?
//#define DDI_WATCHDOG_ENABLE
//#define DDI_DUKPT_ENABLE

#if(KEYPAD_EXIST==1)
#define DDI_KEY_ENABLE
#endif

#if(LCD_EXIST==1)
#define DDI_LCD_ENABLE
#endif

#if(PRINTER_EXIST==1)
#define DDI_THMPRN_ENABLE
#endif

#if(BT_EXIST==1)  
#define DDI_BT_ENABLE
#endif

#if(WIFI_EXIST == 1)
#define DDI_WIFI_ENABLE
#endif

//包含文件
#define WATCHDOG_EN     1

#ifndef TRUE_TRENDIT
#define TRUE_TRENDIT 1
#endif

#ifndef FALSE_TRENDIT
#define FALSE_TRENDIT 0
#endif

typedef enum{
    AT_API_NO_BLOCK,     //非阻塞获取，主要是由模块控制策略调用
    AT_API_BLOCK         //阻塞获取，主要是由给应用层的api调用
}AT_API_GET_TYPE_E;


#endif
