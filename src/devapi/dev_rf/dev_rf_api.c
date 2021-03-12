#include "devglobal.h"

#include "devapi/dev_rf/dev_rf_flow.h"
#include "devapi/dev_rf/dev_rf_io.h"
#include "devapi/dev_rf/pn512/dev_pn512.h"
#include "devapi/dev_rf/rc663/dev_rc663.h"


#define trace(...)

#define CL_CTL_STEP_IDLE        0
#define CL_CTL_STEP_POLL        1
#define CL_CTL_STEP_ACTIVATE    2
#define CL_CTL_STEP_APDU        3
#define CL_CTL_STEP_REMOVE      4

typedef struct {
    u8 step;
    u8 card_type;
}cl_ctl_t;
static cl_ctl_t cl_ctl;

extern nPICCData PICCData[PICC_CIDCNTMAX];
static nPICCData *g_picc_info = &PICCData[0];
static const u8 cl_drv_ver[] = "V9ADE020130815001";
s32 api_rf_init(void)
{
    return dev_pcd_init();
}

s32 api_rf_open(void)
{
    if(0 > dev_pcd_open())
    {
        return DDI_ERR;
    }
    cl_ctl.step = CL_CTL_STEP_IDLE;
    return DDI_OK;
}

s32 api_rf_close(void)
{
    trace("rf close\r\n");
    dev_pcd_close();
    cl_ctl.step = CL_CTL_STEP_IDLE;
    return DDI_OK;
}

s32 api_rf_poweron (u32  nType)
{
    s32 ret;
    cl_ctl.card_type = 0;
    
    trace("RF Power on\r\n");
    
    if(nType & 0xFFFFFFF8)
        return DDI_EINVAL;

    if(nType & 0x01) 
        cl_ctl.card_type = CARD_TYPE_A;
    else if(nType & 0x02)
        cl_ctl.card_type = CARD_TYPE_B;
    else
    {
        return DDI_EINVAL;
    }
    
    if(MI_OK == dev_pcd_set_para(nType,cl_ctl.card_type))
    {
        checkparam.checkcontinue = 1;
    }
    else
    {
        return DDI_ERR;
    }

//    trace("Start poll %d :", nType);
    
    cl_ctl.card_type = nType;
    cl_ctl.step = CL_CTL_STEP_POLL;
//RF_DEBUG("step=%d\r\n", cl_ctl.step);
    return DDI_OK;
}

s32 api_rf_poweroff(void)
{   
    trace("RF Power off\r\n");
//    dev_pcd_softpowerdown();    
    dev_pcd_ctrl_carrier(0);
    checkparam.checkcontinue = 0;
    cl_ctl.step = CL_CTL_STEP_IDLE;
    return DDI_OK;
}

s32 api_rf_get_status(void)
{
    s32 ret;
    u32 n;
    u8 rbuf[50];
    u32 rsize = 0;
                        
    if(cl_ctl.step == CL_CTL_STEP_POLL)
    {
      #if 0
        ret = dev_rf_get_lpmod();
        if(ret == 1)
        {
            ret = dev_pcd_poll_1(cl_ctl.card_type, rbuf, &rsize, sizeof(rbuf));
        }
        else   
      #endif      
        {
            ret = dev_pcd_poll(cl_ctl.card_type, rbuf, &rsize, sizeof(rbuf));
        }

        if(0 == ret)
        {
            RF_DEBUG("Get a picc\r\n");
            if(CARD_TYPE_A == dev_pcd_get_cardtype())
            {
                if(g_picc_info->PiccSAK & 0x20)
                {
                    RF_DEBUG("PICC_STATUS_SINGLECPUA\r\n");
                    return PICC_STATUS_SINGLECPUA;
                }
                else
                {
                    RF_DEBUG("PICC_STATUS_MEMA\r\n");
                    return PICC_STATUS_MEMA;
                }
            }
            else if(CARD_TYPE_B == dev_pcd_get_cardtype())
            {
                RF_DEBUG("PICC_STATUS_SINGLECPUB\r\n");
                return PICC_STATUS_SINGLECPUB;
            }
            else
            {
                RF_DEBUG("Unkown picc type\r\n");
                cl_ctl.step = CL_CTL_STEP_IDLE;
                return DDI_ERR;
            }
        }
        else if(-2 == ret)
        {
            RF_DEBUG("PICC_STATUS_MULTICARD\r\n");
            return PICC_STATUS_MULTICARD;
        }
#if 0
        else if(-1 != ret)
        {
            trace("Poll process err:%d\r\n",ret);
            return DDI_ERR;
        }
#endif
        return PICC_STATUS_NOTPRESENT;

    }
    else if(CL_CTL_STEP_REMOVE == cl_ctl.step)
    {
        //ret = dev_cl_remove();
        if(dev_pcd_get_cardtype() == CARD_TYPE_A)
        {
            ret = dev_pcd_typeARemove();
        }
        else
        {
            ret = dev_pcd_typeBRemove();
        }
        if(0 == ret)
        {
            cl_ctl.step = CL_CTL_STEP_IDLE;
            RF_DEBUG("PICC_STATUS_NOTPRESENT\r\n");
            return PICC_STATUS_NOTPRESENT;
        }
        else if(1 == ret)
        {
            //remove in processing.
        }
        else if(0 > ret)
        {
            RF_DEBUG("Remove process err:%d\r\n",ret);
            cl_ctl.step = CL_CTL_STEP_IDLE;
            return DDI_ERR;
        }
        if(CARD_TYPE_A == dev_pcd_get_cardtype())
        {
            if(g_picc_info->PiccSAK & 0x20)
            {
                RF_DEBUG("PICC_STATUS_SINGLECPUA\r\n");
                return PICC_STATUS_SINGLECPUA;
            }
            else
            {
                RF_DEBUG("PICC_STATUS_MEMA\r\n");
                return PICC_STATUS_MEMA;
            }
        }
        else if(CARD_TYPE_B == dev_pcd_get_cardtype())
        {
            RF_DEBUG("PICC_STATUS_SINGLECPUB\r\n");
            return PICC_STATUS_SINGLECPUB;
        }
        else
        {
            RF_DEBUG("Unkonw picc type\r\n");
            cl_ctl.step = CL_CTL_STEP_IDLE;
            return DDI_ERR;
        }
    }
    else
    {
        RF_DEBUG("ERR Step:\r\n");
    }
    return DDI_EIO;
}

s32 api_rf_remove(void)
{
    s32 ret;

    trace("Start remove :");
    if(0 != (ret = dev_pcd_checkremov_sta()))
    {
        trace("err:%d\r\n",ret);
        cl_ctl.step = CL_CTL_STEP_IDLE;
        return DDI_ERR;
    }
    trace("done\r\n");
    cl_ctl.step = CL_CTL_STEP_REMOVE;

    return DDI_OK;
}

s32 api_rf_activate(void)
{
    s32 ret;
    u8 ats[50];
    u16 rlen;
    
    trace("Start Activate\r\n");
    //ret = dev_cl_active(ats, &rlen, sizeof(ats));
    ret = dev_pcd_active_picc(0, ats, &rlen);
    trace("Activate done\r\n");
    if(0 == ret)
    {
        return DDI_OK;
    }
    else
    {
        return DDI_ERR;
    }
}

s32 api_rf_exchange_apdu(const u8*lpCApdu, u32 nCApduLen, u8*lpRApdu, u32 *lpRApduLen, u32 nRApduSize)
{
//    u16 clen;
//    u32 n;
    s32 ret;
    u16 len;
    
    trace("APDU exchange :");

    if(!lpCApdu || !lpRApdu)
        return DDI_EINVAL;
    if(nCApduLen > 0xFF)
        return DDI_EINVAL;

    //ret = dev_cl_apdu((u8 *)lpCApdu,nCApduLen,lpRApdu,&n,nRApduSize);
    ret = dev_pcd_exchange_data(0, (u8 *)lpCApdu, nCApduLen, lpRApdu, &len);
    trace("APDU done\r\n");
    if(0 == ret)
    {
        *lpRApduLen = len;
        return DDI_OK;
    }
    *lpRApduLen = 0;
    return DDI_ERR;
}

s32 api_rf_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
    s32 ret = DDI_ERR;
//    u8 buf[50];
    u8 len = 0;
//    CL_PARAM ctler_param;
    strMfAuth *pmfauth;
//    s32 val;
    u8 *ap;
        
    switch(nCmd)
    {
        case DDI_RF_CTL_VER:
            strcpy((char *)wParam,(char const*)cl_drv_ver);
            ret = DDI_OK;
            break;
        case DDI_RF_CTL_SAK:
             if(CARD_TYPE_A == dev_pcd_get_cardtype())
            {
                *(u8 *)wParam = g_picc_info->PiccSAK;
                ret = DDI_OK;
            }
            else
            {
                trace("SAK not exist:%d\r\n", dev_pcd_get_cardtype());
                ret = DDI_ERR;
            }
            break;
        case DDI_RF_CTL_UID:
            if(CARD_TYPE_A == dev_pcd_get_cardtype())
            {
                if((g_picc_info->MLastSelectedsnrLen != 4) && (g_picc_info->MLastSelectedsnrLen != 7) &&\
                    (g_picc_info->MLastSelectedsnrLen != 10))
                    ret = DDI_ERR;
                else
                {
                    (((u8 *)wParam))[0] = g_picc_info->MLastSelectedsnrLen;
                    memcpy(&((u8 *)wParam)[1],g_picc_info->MLastSelectedSnr,g_picc_info->MLastSelectedsnrLen);
                    ret = DDI_OK;
                }
            }
            else if(CARD_TYPE_B == dev_pcd_get_cardtype())
            {
                ((u8 *)wParam)[0] = 4;
                memcpy(&((u8 *)wParam)[1],g_picc_info->pupi,4);
                ret = DDI_OK;
            }
            else
            {
                trace("UID not exist:%d\r\n", dev_pcd_get_cardtype());
                ret = DDI_ERR;
            }
            break;
        case DDI_RF_CTL_MF_AUTH:
            pmfauth = (strMfAuth *)lParam;
            ret = dev_mifare_auth(pmfauth->m_authmode,pmfauth->m_key,pmfauth->m_uid,pmfauth->m_block);
            if(MF_EC_NO_ERROR == ret)
                ret = DDI_OK;
            else if(MF_EC_PARAM_INVALID == ret)
                ret = DDI_EINVAL;
            else
                ret = DDI_ERR;
            break;

        case DDI_RF_CTL_MF_READ_RAW:
            ret = dev_mifare_read_binary(lParam,(u8 *)wParam);
            if(MF_EC_NO_ERROR == ret)
                ret = DDI_OK;
            else if(MF_EC_PARAM_INVALID == ret)
                ret = DDI_EINVAL;
            else
                ret = DDI_ERR;
            break;

        case DDI_RF_CTL_MF_WRITE_RAW:
            ret = dev_mifare_write_binary(lParam,(u8 *)wParam);
            if(MF_EC_NO_ERROR == ret)
                ret = DDI_OK;
            else if(MF_EC_PARAM_INVALID == ret)
                ret = DDI_EINVAL;
            else
                ret = DDI_ERR;
            break;

        case DDI_RF_CTL_MF_READ_VALUE:
            ret = dev_mifare_read_value(lParam,(u32 *)(wParam));
            if(MF_EC_NO_ERROR == ret)
                ret = DDI_OK;
            else if(MF_EC_PARAM_INVALID == ret)
                ret = DDI_EINVAL;
            else
                ret = DDI_ERR;          
            break;

        case DDI_RF_CTL_MF_WRITE_VALUE:
            ret = dev_mifare_write_value(lParam,*(u32 *)(wParam));
            if(MF_EC_NO_ERROR == ret)
                ret = DDI_OK;
            else if(MF_EC_PARAM_INVALID == ret)
                ret = DDI_EINVAL;
            else
                ret = DDI_ERR;  
            break;

        case DDI_RF_CTL_MF_INC_VALUE:
            ret = dev_mifare_inc_value(lParam,*(u32 *)(wParam));
            if(MF_EC_NO_ERROR == ret)
                ret = DDI_OK;
            else if(MF_EC_PARAM_INVALID == ret)
                ret = DDI_EINVAL;
            else
                ret = DDI_ERR;  
            break;

        case DDI_RF_CTL_MF_DEC_VALUE:
            ret = dev_mifare_dec_value(lParam,*(u32 *)(wParam));
            if(MF_EC_NO_ERROR == ret)
                ret = DDI_OK;
            else if(MF_EC_PARAM_INVALID == ret)
                ret = DDI_EINVAL;
            else
                ret = DDI_ERR;              
            break;

        case DDI_RF_CTL_MF_BACKUP_VALUE:
            ret = dev_mifare_backup_value(lParam,wParam);
            if(MF_EC_NO_ERROR == ret)
                ret = DDI_OK;
            else if(MF_EC_PARAM_INVALID == ret)
                ret = DDI_EINVAL;
            else
                ret = DDI_ERR;
            break;
            
        case DDI_RF_CTL_SET_PARAM:
            ap = (u8*)lParam;
            len = ap[0];
            ret = dev_pcd_writeallparam(&ap[1], len);
            if(ret < 0)
            {
                return DDI_ERR;
            }
            else
            {
                return DDI_OK;
            }
            break;
        case DDI_RF_CTL_GET_PARAM:
            len = 5;
            ap = (u8*)wParam;
            *ap = len;
            ret = dev_pcd_readallparam(&ap[1], len);
            if(ret < 0)
            {
                return DDI_ERR;
            }
            else
            {
                return DDI_OK;
            }
            break;
       #if 0   
        case DDI_RF_CTL_NFCBUILTIN:
            if(is_pcd_open())  //not open befor
            {
                if(dev_pcd_open())
                    ret = DDI_ENODEV;
                else
                    ret = DDI_OK;
                dev_pcd_close();
            }
            else
                ret = DDI_OK;
            break;
            
        case DDI_RF_CTL_RF_POWER_TEST:
            dev_rc663_write_reg(J663REG_DrvMod, 0x8F);
            dev_rc663_write_reg(J663REG_Command, J663CMD_IDLE);
            break;
        #endif   
      #if 0  
        case DDI_RF_CTL_SET_LPMOD://        (14) //设置循环模式(主要用于测试)
            dev_rf_set_lpmod(lParam);
            ret = DDI_OK;
            break;
      #endif      
        case DDI_RF_CTL_TEST_RTSA:  //
            dev_rf_test_rtsa(lParam);   
            ret = DDI_OK;
            break;
            
        default:
            ret = DDI_ERR;
            break;
    }
    return ret;
}

