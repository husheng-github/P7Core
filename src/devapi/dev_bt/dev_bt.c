/********************************Copyright ( c ) ********************************
**                  DAQU 
** 						
**                        
**
**Created By:		 
** Created Date:	 
** Version:		
** Description:	该文件包含蓝牙相关接口
  
****************************************************************************/
#include "devglobal.h"
#include "drv_bt_yc1021.h"
#include "drv_bt_bk3231s.h"

#if(BT_EXIST==1)  
static s32 g_bt_fd = -1;

/****************************************************************************
**Description:	   蓝牙参数设置
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170610
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_bt_param_set(u8 btmod, u8 *btname, u8 *btmac, u8 *nvram)
{  
  #if(BTCHIP_TYPE == BTCHIP_YC1021)
    return drv_bt_param_set(btmod, btname, btmac, nvram);
  #elif(BTCHIP_TYPE == BTCHIP_BK3231S)
    s32 ret;
    ret = drv_bt_bk3231s_param_set(btmod, btname, btmac, nvram);
    if(ret < 0)
    {
        return -1;
    }
    if(btmod&BTPARAM_NAME_BIT)
    {
        ret = drv_bt_bk3231s_set_btname(btname);
    }
    return ret;
  #endif
}
/****************************************************************************
**Description:	   蓝牙参数读取
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170610
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_bt_param_get(u8 *btmod, u8 *btname, u8 *btmac, u8 *nvram)
{
  #if(BTCHIP_TYPE == BTCHIP_YC1021)
    return drv_bt_param_get(btmod, btname, btmac, nvram);
  #elif(BTCHIP_TYPE == BTCHIP_BK3231S)
    return drv_bt_bk3231s_param_get(btmod, btname, btmac, nvram);
  #endif
}
 
/****************************************************************************
**Description:	   蓝牙设备初始化
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170610
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void dev_bt_init(void)
{
  #if(BTCHIP_TYPE == BTCHIP_YC1021)
    drv_bt_pin_init();
  #elif(BTCHIP_TYPE == BTCHIP_BK3231S)
    drv_bt_bk3231s_pin_init();
  #endif    
}
#if 1
s32 dev_bt_check(void)
{
    s32 hwver;
    
    //判断版本和蓝牙芯片是否匹配
  #if 0  
    hwver = dev_misc_get_hwver();
  #if(BTCHIP_TYPE == BTCHIP_YC1021)
    if((hwver != MACHINE_HWVER_M3V2)
     &&(hwver != MACHINE_HWVER_M5V2))
    {
        return -1;
    }
  #elif(BTCHIP_TYPE == BTCHIP_BK3231S)
    if((hwver != MACHINE_HWVER_M3V1)
     &&(hwver != MACHINE_HWVER_M5V1))
    {
        return -1;
    }
  #endif   
  #endif
  #if(BTCHIP_TYPE == BTCHIP_YC1021)
    //YC判断 
    if(dev_misc_bt_check()!=1)
    {
        return -1;
    }
  #elif(BTCHIP_TYPE == BTCHIP_BK3231S)
    //BK判断 
    if(dev_misc_bt_check()!=0)
    {
        return -1;   
    }
  #endif
    return  0;
}
#endif
/****************************************************************************
**Description:	   打开蓝牙设备
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170610
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_bt_open(void)
{ 
    s32 ret;
    
    if(g_bt_fd < 0)
    {
      #if(BTCHIP_TYPE == BTCHIP_YC1021)
        //drv_bt_pin_init();
        ret = drv_bt_yc1021_patch();
        if(ret < 0)
        {
            drv_bt_yc1021_poweroff();
            return ret;
        }
      #ifndef BT_FEQTEST_EN  
        ret = drv_bt_yc1021_param_init();
      #endif
	  BT_DEBUG("\r\nbt param init = %d",ret);
        if(ret < 0)
        {
            drv_bt_yc1021_poweroff();
            return ret;
        }
        else
        {
            //drv_bt_task_ctl_dealflg(1);
            g_bt_fd = 0;
        }
		
      #elif(BTCHIP_TYPE == BTCHIP_BK3231S)
 //       drv_bt_bk3231s_init();
//        drv_bt_bk3231s_hdrst();
        ret = drv_bt_bk3231s_poweron();
//BT_DEBUG("ret=%d\r\n", ret);      
        if(ret < 0)
        {
            drv_bt_bk3231s_poweroff();
            return -1;
        }
        ret = drv_bt_bk3231s_param_init();
//BT_DEBUG("ret=%d\r\n", ret);      
        if(ret<0)
        {
            drv_bt_bk3231s_poweroff();
            return -1;
        }
        g_bt_fd = 0;
      #endif
    }
    return g_bt_fd;
}
/****************************************************************************
**Description:	   关闭蓝牙设备
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170610
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_bt_close(void)
{   
    s32 ret;
    
    if(g_bt_fd == 0)
    {
      #if(BTCHIP_TYPE == BTCHIP_YC1021)
        //drv_bt_wakeup_ctl(0);   //进入低功耗
        //ret = drv_bt_hci_cmd_sleep_mode();
        ret = drv_bt_yc1021_poweroff();
      #elif(BTCHIP_TYPE == BTCHIP_BK3231S)
        drv_bt_bk3231s_poweroff();
      #endif
        g_bt_fd = -1;
    } 
    return 0;
}
/****************************************************************************
**Description:	   蓝牙设备任务处理
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170610
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void dev_bt_task_process(void)
{
    if(g_bt_fd < 0)
        return;
    
  #if(BTCHIP_TYPE == BTCHIP_YC1021)
    drv_bt_task_process();
  #elif(BTCHIP_TYPE == BTCHIP_BK3231S)
    drv_bt_bk3231s_task_process();
  #endif
  
}
/****************************************************************************
**Description:	   蓝牙设备发送数据
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_bt_write(u8 *wbuf, s32 wlen)
{    
    if(g_bt_fd < 0)
    {
        return DDI_EIO;
    }
        
  #if(BTCHIP_TYPE == BTCHIP_YC1021)
    return drv_bt_send_data(wbuf, wlen);
  #elif(BTCHIP_TYPE == BTCHIP_BK3231S)
    return drv_bt_bk3231s_send_data(wbuf, wlen);
  #endif
}
/****************************************************************************
**Description:	   蓝牙设备接收数据
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_bt_read(u8 *rbuf, s32 rlen)
{
    if(g_bt_fd < 0)
    {
        return DDI_EIO;
    }
  #if(BTCHIP_TYPE == BTCHIP_YC1021)  
    return drv_bt_read_data(rbuf, rlen);
  #elif(BTCHIP_TYPE == BTCHIP_BK3231S)
    return drv_bt_bk3231s_read_data(rbuf, rlen);
  #endif    
}
/****************************************************************************
**Description:	   蓝牙设备状态
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_bt_get_status(void)
{   
    s32 ret;
    u8 connect;

    if(g_bt_fd < 0)
    {
        return DDI_EIO;
    }
  #if(BTCHIP_TYPE == BTCHIP_YC1021)
    ret = drv_bt_get_connect_status(&connect);
  #elif(BTCHIP_TYPE == BTCHIP_BK3231S)
    ret = drv_bt_bk3231s_get_connect_status(&connect);
  #endif    
//  BT_DEBUG("ret=%d,connect=%d\r\n", ret, connect);
    if(ret < 0)
    {
        return DDI_ERR;
    }
    if((connect==1)||(connect==2))
    {        
        return DDI_OK;
    }
    else
    {
        return DDI_ERR;
    }   
}
/****************************************************************************
**Description:	   蓝牙断开连接
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
s32 dev_bt_disconnect(const u8 *mac)
{  
    if(g_bt_fd < 0)
    {
        return DDI_EIO;
    }
  #if(BTCHIP_TYPE == BTCHIP_YC1021)
    return drv_bt_disconnect();
  #elif(BTCHIP_TYPE == BTCHIP_BK3231S)
    return drv_bt_bk3231s_disconnect();
  #endif    
}
/****************************************************************************
**Description:	   蓝牙断开连接
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170615
**----------------------------------------------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static s32 dev_bt_get_btparam(u8 type, u8 *rbuf)
{
    u8 btmod;
    u8 btname[35];
    u8 btmac[6];
    u8 btnvram[120];
    s32 ret;

    ret = dev_bt_param_get(&btmod, btname, btmac, btnvram);
    if(ret != 0)
    {
        return DDI_ERR;
    }

    if(type == 0)
    {
        //读取蓝牙名称
        if(btmod&BTPARAM_NAME_BIT)
        {
            strcpy(rbuf, btname);
        }
        else
        {
            return DDI_ERR;
        }
    }
    else if(type == 1)
    {
        //读取MAC地址
      #if(BTCHIP_TYPE == BTCHIP_YC1021)
        if(btmod&BTPARAM_MAC_BIT)
        {
            memcpy(rbuf, btmac, 6);
        }
        else
        {
            return DDI_ERR;
        }
      #elif(BTCHIP_TYPE == BTCHIP_BK3231S)
        ret = drv_bt_bk3231s_cmd_get_btmac(rbuf);
        if(ret != 0)
        {
            ret = DDI_EIO;
        }
      #endif
    }
    else 
    {
        //暂不处理
        ret = DDI_EIO;
    }
    return ret;
}
s32 dev_bt_firmwareupdate(u8 *filename, u32 updateaddr,u32 updatelen)
{
    u8 tmp[8];
    s32 ret;

    memset(tmp, 0, 8);
    ret = fs_read_file(filename,tmp,8,updateaddr+updatelen-8);
    if(ret != 8)
    {
        return DDI_ERR;
    }
  #if(BTCHIP_TYPE == BTCHIP_YC1021)
    if(0 == strcmp(tmp,"YC1021"))   //59 43 31 30 32 31
    {
        ret = drv_bt_yc1021_firmwareupdate(filename,updateaddr,updatelen);
    }
  #elif(BTCHIP_TYPE == BTCHIP_BK3231S)
    if(0 == strcmp(tmp,"BK3231"))   //42 4B 33 32 33 31
    {
        ret = drv_bt_bk3231s_firmwareupdate(filename,updateaddr,updatelen);  //蓝牙固件通过串口升级
    }
  #endif 
    else
    {
        ret = DDI_ERR;
    }
    return ret;
}
s32 dev_bt_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    s32 ret=DDI_EINVAL;
    u8 *lp_tmp;
    u8 n;
	PARAM_BTFIRMWAREUPDATE *param_btfirmwareupdate;
 
    switch(nCmd)
    {
	case DDI_BT_CTL_VER:				//0	//获取蓝牙控制程序版本
	    lp_tmp = (u8*)wParam;
      #if(BTCHIP_TYPE == BTCHIP_YC1021)
        strcpy((s8*)lp_tmp, "YC1021");
      #elif(BTCHIP_TYPE == BTCHIP_BK3231S)
        strcpy((s8*)lp_tmp, "BK3X31S");
      #endif   
        ret = 0;
        break;
    //-----------------------------------
    case DDI_BT_CTL_MVER:				//1	//获取蓝牙模块固件版本
	    lp_tmp = (u8*)wParam;
      #if(BTCHIP_TYPE == BTCHIP_YC1021)
        ret = drv_bt_hci_cmd_version_request(lp_tmp, &n);
      #elif(BTCHIP_TYPE == BTCHIP_BK3231S)
        ret = drv_bt_bk3231s_cmd_get_btversion(lp_tmp, &n);
      #endif  
        if(ret < 0)
        {
            ret = DDI_ERR;
        }
        else
        {
            lp_tmp[n] = 0;
        }  
	    break;
    //-----------------------------------
    case DDI_BT_CTL_GET_ADDR:              //获取蓝牙地址
        ret = dev_bt_get_btparam(1, (u8*)wParam);
        if(ret != 0)
        {
            ret = DDI_EIO;
        }
	    break;
    //-----------------------------------
    case DDI_BT_CTL_RPIN:                    //取PIN CODE
	    break;
    //-----------------------------------
    case DDI_BT_CTL_RNAME:                   //获取蓝牙设备名称
        ret = dev_bt_get_btparam(0, (u8*)wParam);
	    break;
    //-----------------------------------
    case DDI_BT_CTL_PAIRED_LIST:             //获取已配对列表
	    break;
    //-----------------------------------
    case DDI_BT_CTL_PAIRED_DEL:              //删除已配对设备信息
	    break;
    //-----------------------------------
    case DDI_BT_CTL_REMOTE_INFO:             //获取远端设备的信息
	    break;
    //-----------------------------------
    case DDI_BT_CTL_WPIN:                    //设置PIN CODE
	    break;
    //-----------------------------------
    case DDI_BT_CTL_WNAME:                   //设置蓝牙设备名称
        ret = dev_bt_param_set(BTPARAM_NAME_BIT, (u8*)lParam, NULL, NULL);
        if(ret < 0)
        {
            ret = DDI_EIO;
        }
	    break;
    //-----------------------------------
    case DDI_BT_CTL_DISCOVERABLE:            //设置是否可被扫描
	    break;
    //-----------------------------------
    case DDI_BT_CTL_PAIRED_MODE_SET:	     //设置配对模式
	    break;
    //-----------------------------------
    case DDI_BT_CTL_PAIRED_MODE_GET:	     //获取配对模式
	    break;
    //-----------------------------------
    case DDI_BT_CTL_BT_STACK_VER:            //获取蓝牙协议栈版本
        
	    break;
    //-----------------------------------
    case DDI_BT_CTL_LOCAL_INFO:              
	    break;
    //-----------------------------------
    case DDI_BT_CTL_FIRMWAREUPDATE:          //固件升级
        param_btfirmwareupdate = (PARAM_BTFIRMWAREUPDATE *)lParam;
        ret = dev_bt_firmwareupdate(param_btfirmwareupdate->filename,param_btfirmwareupdate->offset,wParam);
        if(ret!= 0)
		{
			ret = DDI_ERR;
		}
    	break;
    //-----------------------------------
    case DDI_BT_CTL_SET_ADDR:               //设置蓝牙地址
        ret = dev_bt_param_set(BTPARAM_MAC_BIT, NULL, (u8*)lParam, NULL);
        if(ret < 0)
        {
            ret = DDI_EIO;
        }
	    break;
    //-----------------------------------
    }   
	return ret;
}
#endif
