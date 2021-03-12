/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    该文件包含LED的驱动接口
  
****************************************************************************/
#include "devglobal.h"
#include "devapi/dev_rf/dev_rf_io.h"


void char2long(u8 endian,u8 *character,u32 *longval)
{
    if(endian) //Big-endian
    {
        *longval = character[0];
        *longval <<= 8;
        *longval |= character[1];
        *longval <<= 8;
        *longval |= character[2];
        *longval <<= 8;
        *longval |= character[3];
    }
    else //little-endian
    {
        *longval = character[3];
        *longval <<= 8;
        *longval |= character[2];
        *longval <<= 8;
        *longval |= character[1];
        *longval <<= 8;
        *longval |= character[0];
    }
}
void long2char(u8 endian,u8 *character,u32 *longval)
{
    u32 lval = *longval;
    if(endian) //Big-endian
    {
        character[3] = lval & 0xFF;
        lval >>= 8;
        character[2] = lval & 0xFF;
        lval >>= 8;
        character[1] = lval & 0xFF;
        lval >>= 8;
        character[0] = lval & 0xFF;
    }
    else //little-endian
    {
        character[0] = lval & 0xFF;
        lval >>= 8;
        character[1] = lval & 0xFF;
        lval >>= 8;
        character[2] = lval & 0xFF;
        lval >>= 8;
        character[3] = lval & 0xFF;
    }
}

/*******************************************************************************
 ** Descriotions:   Mifare认证
                     auth_mode  specify key A or key B -- MIFARE_AUTHENT_A or MIFARE_AUTHENT_A
                     key        the buffer stored the key(6 bytes)
                     snr        the buffer stored the selected card's UID
                     addr       the block address of a sector                                            
 ** Returned value:  0: 成功
                    其它:失败 
 ** Created By:     彭学斌 20170503
 ** Remarks:                                                                                   
*******************************************************************************/
s32 drv_mifare_auth(u8 auth_mode, u8 *key, u8 *snr, u8 addr)
{
    return dev_pcd_mifare_auth(auth_mode, key, snr, addr);
}
/*******************************************************************************
 ** Descriotions:    read 16 bytes data from a block
 ** parameters:     block       the address of the block
                    rdata       the buffer to save the 16 bytes data               
 ** Returned value:  0: 成功
                    其它:失败 
 ** Created By:     彭学斌 20170503
 ** Remarks:       
*******************************************************************************/
s32 drv_mifare_read_binary(u8 block, u8 *data)
{
    s32 status;
    u8 databfr[2];
    MfCmdInfo mfcmdinfotmp;

    //准备数据
    databfr[0] = MIFARE_READ;       //读MIFARE标识
    databfr[1] = block;              //address to authentication
    //prepare data for common transceive
    dev_pcd_reset_mfinfo(&mfcmdinfotmp);    //复位结构 
    mfcmdinfotmp.nBytesToSend = 2;          //数据长度
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_100US_1;
    mfcmdinfotmp.timeout = 200;             //待定，pengxuebin，????
    mfcmdinfotmp.nmifareflg = 1;
    status = dev_pcd_execute_cmd(RF_CMDNUM_TRANSCEIVE, &mfcmdinfotmp, databfr, data);
//uart_printf("status=%d\r\n", status);
    if(MI_OK == status)
    {
        if(16 != mfcmdinfotmp.nBytesReceived)
        {
            status = MI_BYTECOUNTERR;
        }
    }
    return status;
}
/*******************************************************************************
 ** Descriotions:    write 16 bytes data to a block
 ** parameters:     block       the address of the block
                    wdata       the buffer to save the 16 bytes data               
 ** Returned value:  0: success
                     other:fail
 ** Created By:     彭学斌 20170503
 ** Remarks:                                                                                 
*******************************************************************************/
s32 drv_mifare_write_binary(u8 block, u8 *data)
{
    S32 status;
    u8 databfr[2];
    u8 rtmp[32];
    MfCmdInfo mfcmdinfotmp;
    
    //准备数据
    databfr[0] = MIFARE_WRITE;       //写MIFARE标识
    databfr[1] = block;              //address to authentication
    
    //disable RxCRC  
    dev_pcd_ctl_crc(DEV_RF_RXCRC_MASK, DEV_RF_RXCRC_DIS);  
    //prepare data for common transceive
    dev_pcd_reset_mfinfo(&mfcmdinfotmp);    //复位结构
    mfcmdinfotmp.nBytesToSend = 2;          //数据长度
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_100US_1;
    mfcmdinfotmp.timeout = 500;             //待定，pengxuebin，????
    mfcmdinfotmp.nmifareflg = 1;
//    mfcmdinfotmp.cmd = JCMD_TRANSCEIVE;
    status = dev_pcd_execute_cmd(RF_CMDNUM_TRANSCEIVE, &mfcmdinfotmp, databfr, rtmp);
    if(MI_OK == status)
    {
        if(4 != mfcmdinfotmp.nBitsReceived)
        {
            status = MI_BITCOUNTERR;
        }
        else
        {
            rtmp[0] &= 0x0f;
            if(0x0a != rtmp[0])
            {
                status = MI_CODEERR;
            } 
        }
    }
    
    if(MI_OK == status)
    {
        //prepare data for common transceive
        dev_pcd_reset_mfinfo(&mfcmdinfotmp);    //复位结构
        mfcmdinfotmp.nBytesToSend = 16;         //数据长度
        mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_100US_1;
        mfcmdinfotmp.timeout = 50;             //待定，pengxuebin，????
        mfcmdinfotmp.nmifareflg = 1;
//        mfcmdinfotmp.cmd = JCMD_TRANSCEIVE;
        status = dev_pcd_execute_cmd(RF_CMDNUM_TRANSCEIVE, &mfcmdinfotmp, data, rtmp);
        
        if(MI_OK == status)
        {
            if(4 != mfcmdinfotmp.nBitsReceived)
            {
                status = MI_BITCOUNTERR;
            }
            else
            {
                rtmp[0] &= 0x0f;
                if(rtmp[0] != 0x0a)
                {
                    status = MI_CODEERR;
                }
            }
        }
    }
    //enable RxCRC
    //dev_spi2_modify_reg(JREG_RXMODE, 1, JBIT_CRCEN);
    dev_pcd_ctl_crc(DEV_RF_RXCRC_MASK, DEV_RF_RXCRC_EN);   
    return status;
}
/*******************************************************************************
 ** Descriotions:    write MIFARE 
 ** parameters:     operation    data mode
                    block       the address of the block
                    wdata       the buffer to save the 16 bytes data
                    trans_addr 
 ** Returned value:  0: 成功
                    其它:失败 
 ** Created By:     彭学斌 20170503
 ** Remarks:       
*******************************************************************************/
s32 dev_pcd_mifare_value(u8 operation, u8 block, u8 *wbuf, u8 trans_addr)
{
    S32 status = MI_OK;
    u8 databfr[12];
    u8 rtmp[32];
    MfCmdInfo mfcmdinfotmp;
    
    //准备数据
    databfr[0] = operation;      
    databfr[1] = block;  
    //disable RxCRC
    //dev_spi2_modify_reg(JREG_RXMODE, 0, JBIT_CRCEN); 
    dev_pcd_ctl_crc(DEV_RF_RXCRC_MASK, DEV_RF_RXCRC_DIS);    
    //prepare data for common transceive
    dev_pcd_reset_mfinfo(&mfcmdinfotmp);    //复位结构
    mfcmdinfotmp.nBytesToSend = 2;          //数据长度
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_100US_1;
    mfcmdinfotmp.timeout = 500;              //待定，pengxuebin，????
    mfcmdinfotmp.nmifareflg = 1;
//    mfcmdinfotmp.cmd = JCMD_TRANSCEIVE;
    status = dev_pcd_execute_cmd(RF_CMDNUM_TRANSCEIVE, &mfcmdinfotmp, databfr, rtmp);
    if(MI_OK == status)
    {
        if(4 != mfcmdinfotmp.nBitsReceived)
        {
            status = MI_BITCOUNTERR;
        }
        else
        {
            rtmp[0] &= 0x0f;                // mask out upper nibble
            switch(rtmp[0])
            {
                case 0x00:
                    status = MI_NOTAUTHERR;
                    break;
                case 0x0a:
                    status = MI_OK;
                    break;
                case 0x01:
                    status = MI_VALERR;
                    break;
                default:
                    status = MI_CODEERR;
                    break;    
            }
        }
    }
    if(MI_OK == status)
    {
        //准备数据
        memcpy(databfr, wbuf, 4);
        //prepare data for common transceive
        dev_pcd_reset_mfinfo(&mfcmdinfotmp);    //复位结构
        mfcmdinfotmp.nBytesToSend = 4;          //数据长度
        mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_100US_1;
        mfcmdinfotmp.timeout = 200;              //待定，pengxuebin，????
        mfcmdinfotmp.nmifareflg = 1;
//        mfcmdinfotmp.cmd = JCMD_TRANSCEIVE;
        status = dev_pcd_execute_cmd(RF_CMDNUM_TRANSCEIVE, &mfcmdinfotmp, databfr, rtmp);
        if(MI_OK == status)
        { 
            if(4 != mfcmdinfotmp.nBitsReceived)
            {
                status = MI_BITCOUNTERR;
            }
            else 
            {
                rtmp[0] &= 0x0f;
                switch(rtmp[0])
                {
                    case 0x00:
                        status = MI_NOTAUTHERR;
                        break;
                    case 0x0a:
                        status = MI_OK;
                        break;
                    case 0x01:
                        status = MI_VALERR;
                        break;
                    default:
                        status = MI_CODEERR;
                        break;
                }
            }
        }
        if(MI_NOTAGERR == status)
        {
            status = MI_OK;
        }
    }
    if(MI_OK == status)
    {
        //准备数据
        databfr[0] = MIFARE_TRANSFER;   // transfer command code
        databfr[1] = trans_addr;
        //prepare data for common transceive
        dev_pcd_reset_mfinfo(&mfcmdinfotmp);    //复位结构
        mfcmdinfotmp.nBytesToSend = 2;          //数据长度
        mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_100US_1;
        mfcmdinfotmp.timeout = 500;              //待定，pengxuebin，????
        mfcmdinfotmp.nmifareflg = 1;
//        mfcmdinfotmp.cmd = JCMD_TRANSCEIVE;
        status = dev_pcd_execute_cmd(RF_CMDNUM_TRANSCEIVE, &mfcmdinfotmp, databfr, rtmp);
        if(MI_OK == status)
        {
            if(4 != mfcmdinfotmp.nBitsReceived)
            {
                status = MI_BITCOUNTERR;
            }
            else
            {
                rtmp[0] &= 0x0f;    // mask out upper nibble
                switch(rtmp[0])
                {
                    case 0x00:
                        status = MI_NOTAUTHERR;
                        break;
                    case 0x0a:
                        status = MI_OK;
                        break;
                    case 0x01:
                        status = MI_VALERR;
                        break;
                    default:
                        status = MI_CODEERR;
                        break;
                }
            }
        }
    }
    //enable RxCRC 
    dev_pcd_ctl_crc(DEV_RF_RXCRC_MASK, DEV_RF_RXCRC_EN);  
    return status;
}
/*******************************************************************************
 ** Descriotions:   MIFARE debit (MIFARE借贷)
 ** parameters:     dd_mode    data mode
                    addr       the address of the block
                    value      借贷数据(4Bytes)
 ** Returned value:  0: 成功
                    其它:失败 
 ** Created By:     彭学斌 20170503
 ** Remarks:       
*******************************************************************************/
S32 dev_pcd_mifare_debit(u8 dd_mode, u8 addr, u8 *value)
{
    S32 status;
    u8 databfr[12];
    u8 rtmp[32];
    MfCmdInfo mfcmdinfotmp;
    
    //disable RxCRC
//    dev_spi2_modify_reg(JREG_RXMODE, 0, JBIT_CRCEN); 
    dev_pcd_ctl_crc(DEV_RF_RXCRC_MASK, DEV_RF_RXCRC_DIS); 
    //准备数据
    databfr[0] = dd_mode;   // transfer command code
    databfr[1] = addr;
    //prepare data for common transceive
    dev_pcd_reset_mfinfo(&mfcmdinfotmp);    //复位结构
    mfcmdinfotmp.nBytesToSend = 2;          //数据长度
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_100US_1;
    mfcmdinfotmp.timeout = 500;              //待定，pengxuebin，????
    mfcmdinfotmp.nmifareflg = 1;
//    mfcmdinfotmp.cmd = JCMD_TRANSCEIVE;
    status = dev_pcd_execute_cmd(RF_CMDNUM_TRANSCEIVE, &mfcmdinfotmp, databfr, rtmp);
    if(MI_OK == status)
    {
        if(4 != mfcmdinfotmp.nBitsReceived)
        {
            status = MI_BITCOUNTERR;
        }
        else
        {
            rtmp[0] &= 0x0f;
            if(rtmp[0] != 0x0a)
            {
                status = MI_CODEERR;
            }
        } 
    }
    if(MI_OK == status)
    {
        //prepare data for common transceive
        dev_pcd_reset_mfinfo(&mfcmdinfotmp);    //复位结构
        mfcmdinfotmp.nBytesToSend = 4;          //数据长度
        mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_100US_1;
        mfcmdinfotmp.timeout = 500;              //待定，pengxuebin，????
        mfcmdinfotmp.nmifareflg = 1;
//        mfcmdinfotmp.cmd = JCMD_TRANSCEIVE;
        status = dev_pcd_execute_cmd(RF_CMDNUM_TRANSCEIVE, &mfcmdinfotmp, value, rtmp);
        if(MI_OK == status)
        {
            if(4 != mfcmdinfotmp.nBitsReceived)
            {
                    status = MI_BITCOUNTERR;
            }
            else
            {
                rtmp[0] &= 0x0f;
                if(0x0a != rtmp[0])
                {
                    status = MI_CODEERR;
                } 
            }
        }
    }
    //enable RxCRC
//    dev_spi2_modify_reg(JREG_RXMODE, 1, JBIT_CRCEN);  
    dev_pcd_ctl_crc(DEV_RF_RXCRC_MASK, DEV_RF_RXCRC_EN);  
    return status;
}
/*******************************************************************************
 ** Descriotions:    MIFARE 减值
 ** parameters:     block       the address of the block
                    value       
 ** Returned value:  0: 成功
                    其它:失败 
 ** Created By:     彭学斌 20170503
 ** Remarks:       
*******************************************************************************/
s32 drv_mifare_decreament(u8 block, u8 *value)
{
    s32 status;
    status = dev_pcd_mifare_value(MIFARE_DECREMENT, block, value, block);
    return status;
}
/*******************************************************************************
 ** Descriotions:    MIFARE 增值
 ** parameters:     block       the address of the block
                    value       
 ** Returned value:  0: 成功
                    其它:失败 
 ** Created By:     彭学斌 20170503
 ** Remarks:       
*******************************************************************************/
s32 dev_pcd_mifare_increament(u8 block, u8 *value)
{
    s32 status;
    status = dev_pcd_mifare_value(MIFARE_INCREMENT, block, value, block);
    return status;
}

/*******************************************************************************
 ** Descriotions:    MIFARE restore
 ** parameters:     block       the address of the block
                    value       
 ** Returned value:  0: 成功
                    其它:失败 
 ** Created By:     彭学斌 20170503
 ** Remarks:       
*******************************************************************************/
s32 drv_pcd_mifare_backup(u8 src_blk,u8 dst_blk)
{
    s32 status;
    u8 databfr[12];
    u8 rtmp[32];
    MfCmdInfo mfcmdinfotmp;

    //disable RxCRC
//    dev_spi2_modify_reg(JREG_RXMODE, 0, JBIT_CRCEN);
    dev_pcd_ctl_crc(DEV_RF_RXCRC_MASK, DEV_RF_RXCRC_DIS);
    
    databfr[0] = MIFARE_RESTORE;    // restore command code
    databfr[1] = src_blk;            //source block address
    dev_pcd_reset_mfinfo(&mfcmdinfotmp);    
    mfcmdinfotmp.nBytesToSend = 2;            
    mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_100US_1;
    mfcmdinfotmp.timeout = 500;//60;            
    mfcmdinfotmp.nmifareflg = 1;
//    mfcmdinfotmp.cmd = JCMD_TRANSCEIVE;
    status = dev_pcd_execute_cmd(RF_CMDNUM_TRANSCEIVE, &mfcmdinfotmp, databfr, rtmp);
    if(MI_OK == status)
    {
        if(4 != mfcmdinfotmp.nBitsReceived)
        {
            status = MI_BITCOUNTERR;
        }
        else
        {
            rtmp[0] &= 0x0f;
            if(rtmp[0] != 0x0a)
            {
                status = MI_CODEERR;
            }
        } 
    }
    if(MI_OK == status)
    {
        dev_pcd_reset_mfinfo(&mfcmdinfotmp);    
        mfcmdinfotmp.nBytesToSend = 4;            
        mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_100US_1;
        mfcmdinfotmp.timeout = 60;        
        mfcmdinfotmp.nmifareflg = 1;    
//        mfcmdinfotmp.cmd = JCMD_TRANSCEIVE;
        status = dev_pcd_execute_cmd(RF_CMDNUM_TRANSCEIVE, &mfcmdinfotmp, databfr, rtmp);
        if(MI_NOTAGERR == status)        
        {
            databfr[0] = MIFARE_TRANSFER;
            databfr[1] = dst_blk;
            dev_pcd_reset_mfinfo(&mfcmdinfotmp);    
            mfcmdinfotmp.nBytesToSend = 2;            
            mfcmdinfotmp.timeoutmode = PCD_TIMER_MODE_100US_1;
            mfcmdinfotmp.timeout = 500;//120;         
            mfcmdinfotmp.nmifareflg = 1;    
//            mfcmdinfotmp.cmd = JCMD_TRANSCEIVE;
            status = dev_pcd_execute_cmd(RF_CMDNUM_TRANSCEIVE, &mfcmdinfotmp, databfr, rtmp);
            if(MI_OK == status)
            {
                if(4 != mfcmdinfotmp.nBitsReceived)
                {
                    status = MI_BITCOUNTERR;
                }
                else
                {
                    rtmp[0] &= 0x0f;
                    if(rtmp[0] != 0x0a)
                    {
                        status = MI_CODEERR;
                    }
                } 
            }
        }
    }
    //enable RxCRC 
    dev_pcd_ctl_crc(DEV_RF_RXCRC_MASK, DEV_RF_RXCRC_EN);
    return status;
}
/*******************************************************************************
 ** Descriotions:    数据封装
 ** parameters:     block   
                    value 
                    buffer
                    restore_addr      the address of the block
 ** Returned value:  0: 成功
                    其它:失败 
 ** Created By:     彭学斌 20170503
 ** Remarks:       
*******************************************************************************/
void drv_mifare_value_format(u8 block, u32 value, u8 *buffer)
{
    unsigned char  i=0;
     /* init 1st 4 bytes with the specified value */
     buffer[i++] = value&0xFF;
     buffer[i++] = (value>>8)&0xFF;
     buffer[i++] = (value>>16)&0xFF;
     buffer[i++] = (value>>24)&0xFF;
    
     /* 2nd 4 bytes must have bit-inverted value */
     for( ; i<8; i++)
     {
         buffer[i] = (u8)(~buffer[i-4]);
     }
    
     /* 3rd 4 bytes must have same value as 1st 4 bytes */
     for( ; i<12; i++)
     {
         buffer[i] = buffer[i-8];
     }
    
     /* 4th 4 byte contain the address, twice plain, twice bit inverted */
     buffer[i++] = block;
     buffer[i++] = (u8)(~block);
     buffer[i++] = block;
     buffer[i++] = (u8)(~block);

}
/***********************************************************
* Function: u32 ConvertToAddr(void)
*Author:    zhaoying
*Version : 1.0
*Date:9/10/2008
*Input:      unsigned char sector,unsigned char block
*
*Output:void
*Return:   vaild address, return converted address
*                Invaild addreass,return -1
* Description:
***********************************************************/
s32 ConvertToAddr(u8 CurrentSector,u8 CurrentBlock)
{
    u32 block;

    if((CurrentSector >40) ||(CurrentBlock >16)) {
        return -1;/* i.e. s70 */
    }
    if((CurrentSector <32)&&(CurrentBlock >4)) {
        return -1;    /* i.e. s50 */
    }

    if(CurrentSector <= 31)
    {
        block = CurrentSector*4 +CurrentBlock;
    }
    else if(CurrentSector >= 32)
    {
        block = 32*4 + (CurrentSector -32)*16 + CurrentBlock;
    }

    return block;
}

/**
  * @brief  To check whether data of a block is value format
  * @param  block_datas:16 bytes block data
  *            value:value returned if the block data is value format
  * @retval 
  * @auther    
  */
s32 drv_mifare_value_check(u8 *block_data,u32 *value)
{
    s32 ret;
    u32 *val = (u32 *)(block_data + 4);
    *value = *(u32 *)block_data;
    if(*val != ~*value)
    {
        ret = 1;
    }
    else if((block_data[12] != block_data[14]) || (block_data[13] != block_data[15])\
        || (block_data[12] != (u8)~block_data[13]))
    {
        ret = 2;
    }
    else
    {
        ret = 0;
    }
    return ret;
}
/**
  * @brief  To perform mifare authentication 
  * @param  
  *            
  * @retval 
  * @auther    Sandy
  */
s32 dev_mifare_auth(u8 key_type,u8 *key, u8 *uid, u8 block_num)
{
    s32 ret = 0;
    if((key == NULL) || (uid == NULL))
    {
        ret = MF_EC_PARAM_INVALID;
    }
    else if((key_type != MF_KEY_A) && (key_type != MF_KEY_B))
    {
        ret = MF_EC_KEY_TYPE_UNKNOW;
    }
    else if(MI_OK == (ret = dev_pcd_mifare_auth(key_type,key,uid,block_num)))
    {
        ret = MF_EC_NO_ERROR;
    }
    else
    {
        ret = MF_EC_AUTH_FAILED;
    }
    return ret; 
}   
/**
  * @brief  
  * @param  
  *            
  * @retval 
  * @auther    
  */
s32 dev_mifare_read_binary(u8 block_num,u8 *rdata)
{
    s32 ret;
    if(NULL == rdata)
    {
        ret = MF_EC_PARAM_INVALID;
    }
    else if(MI_OK == (ret = drv_mifare_read_binary(block_num, rdata)))
    {
        ret = MF_EC_NO_ERROR;
    }
    else
    {
        ret = MF_EC_EXECUTE_FAILED;
    }
    return ret;
}
/**
  * @brief  
  * @param  
  *            
  * @retval 
  * @auther    
  */
s32 dev_mifare_write_binary(u8 block_num,u8 *wdata)
{
    s32 ret;
    if(NULL == wdata)
    {
        ret = MF_EC_PARAM_INVALID;
    }
    else if(MI_OK == (ret = drv_mifare_write_binary(block_num, wdata)))
    {
        ret = MF_EC_NO_ERROR;
    }
    else
    {
        ret = MF_EC_EXECUTE_FAILED;
    }
    return ret;
}
/**
  * @brief  
  * @param  
  *            
  * @retval 
  * @auther    
  */
s32 dev_mifare_read_value(u8 block_num,u32 *value)
{
    u8 rbuf[16];
    s32 ret;
    if(MF_EC_NO_ERROR == (ret = dev_mifare_read_binary(block_num,rbuf)))
    {
        if(0 == drv_mifare_value_check(rbuf,value))
        {
            ret = MF_EC_NO_ERROR;
        }
        else
        {
            ret = MF_EC_DATA_FORMAT_INCORRECT;
        }
    }
    return ret;
}
/**
  * @brief  
  * @param  
  *            
  * @retval 
  * @auther    
  */
s32 dev_mifare_write_value(u8 block_num,u32 value)
{
    u8 wbuf[16];
    s32 ret;
    drv_mifare_value_format(block_num,value,wbuf);
    ret = dev_mifare_write_binary(block_num,wbuf);
    return ret;
}
/**
  * @brief  
  * @param  
  *            
  * @retval 
  * @auther    Sandy
  */
s32 dev_mifare_inc_value(u8 block_num, u32 value)
{
    s32 ret;
    u8 val[4];

    long2char(0,val,&value);

    if(MI_OK == dev_pcd_mifare_value(MIFARE_INCREMENT, block_num, val, block_num))
    {
        ret = MF_EC_NO_ERROR;
    }
    else
    {
        ret = MF_EC_EXECUTE_FAILED;
    }
    return ret;
}
/**
  * @brief  
  * @param  
  *            
  * @retval 
  * @auther    Sandy
  */
s32 dev_mifare_dec_value(u8 block_num, u32 value)
{
    s32 ret;
    u8 val[4];
    long2char(0,val,&value);

    if(MI_OK == dev_pcd_mifare_value(MIFARE_DECREMENT, block_num, val, block_num))
    {
        ret = MF_EC_NO_ERROR;
    }
    else
    {
        ret = MF_EC_EXECUTE_FAILED;
    }
    return ret;
}
/**
  * @brief  
  * @param  
  *            
  * @retval 
  * @auther    Sandy
  */
s32 dev_mifare_backup_value(u8 src_block_num, u8 dst_block_num)
{
    s32 ret;
    if(MI_OK == drv_pcd_mifare_backup(src_block_num,dst_block_num))
    {
        ret = MF_EC_NO_ERROR;
    }
    else
    {
        ret = MF_EC_EXECUTE_FAILED;
    }
    return ret;
}




s32 mifare_test(u8 block)
{
    u8 cmd_buf[20];
    u8 rsp_buf[20];
    u8 uid[16];
    s32 ret;
    u32 len;
    u32 val = 0;
    s32 i;

    if(((block+1)%4 == 0)||((block+2)%4 == 0))  //the test block MUST NOT be the trailer block!
        return 1;    
    for(len=0;len<16;len++)
        cmd_buf[len] = len;
    memset(rsp_buf,0,sizeof(rsp_buf));
    dev_pcd_open();
    ret = api_rf_poweron(0x03);
    
RF_DEBUG("mifare_test:ret=%d\r\n", ret);   
    while(1)
    {
        ret = api_rf_get_status();
        if(ret > PICC_STATUS_NOTPRESENT)
        {
           RF_DEBUG("ret=%d\r\n", ret);
           break;
        }
        else if(ret < 0)
        {
           RF_DEBUG("ret=%d\r\n", ret);
           break;
        }
    }
    if(ret < 0)
    {
        dev_pcd_powerdown();
        return 1;
    }
    do{
//        ret = dev_cl_poll(0x03,rsp_buf,&len,sizeof(rsp_buf));
#if 0
        ret = api_rf_get_status();
RF_DEBUG("poll:ret=%d\r\n", ret);        
        if(ret)
        {
            ret = 2;
            break;
        }
#endif        
        delayms(500);
       #if 1  //20140414
        dev_pcd_get_uid(uid, 4);
        ret = dev_mifare_auth(0x60,"\xFF\xFF\xFF\xFF\xFF\xFF",uid,block);
RF_DEBUG("auth:ret=%d\r\n", ret);        
       #endif
        if(ret)
        {
            ret = 3;
            break;
        }
        ret = dev_mifare_write_binary(block,cmd_buf);
RF_DEBUG("write_binary:ret=%d\r\n", ret);        
        if(ret)
        {
            ret = 4;
            break;
        }
        ret = dev_mifare_read_binary(block,rsp_buf);
RF_DEBUG("read_binary:ret=%d\r\n", ret);
RF_DEBUGHEX(NULL, rsp_buf, 16);
        if(ret)
        {
            ret = 5;
            break;
        }
        if(0 != memcmp(cmd_buf,rsp_buf,16))
        {
            ret = 6;
            break;
        }
        ret = dev_mifare_write_value(block,0x11223344);
RF_DEBUG("write_value:ret=%d\r\n", ret);   
        if(ret)
        {
            ret = 7;
            break;
        }
        ret = dev_mifare_read_value(block,&val);
RF_DEBUG("read_value:ret=%d, %d\r\n", ret, val);   
        if(ret)
        {
            ret = 8;
            break;
        }
        if(val != 0x11223344)
        {
            ret = 9;
            break;
        }
         //   dev_pcd_setdebug(1);
        ret = dev_mifare_inc_value(block,1);
         //   dev_pcd_setdebug(0);
RF_DEBUG("inc_value:ret=%d\r\n", ret);   
        if(ret)
        {
            ret = 10;
            break;
        }
        ret = dev_mifare_read_value(block,&val);
RF_DEBUG("read_value2:ret=%d, %d\r\n", ret, val);  
        if(ret)
        {
            ret = 11;
            break;
        }
        if(val != 0x11223345)
        {
            ret = 12;
            break;
        }        
        ret = dev_mifare_dec_value(block,1);
RF_DEBUG("dec_value:ret=%d\r\n", ret);   
        if(ret)
        {
            ret = 13;
            break;
        }
        ret = dev_mifare_read_value(block,&val);
RF_DEBUG("read_value3:ret=%d, %d\r\n", ret, val);  
        if(ret)
        {
            ret = 14;
            break;
        }
        if(val != 0x11223344)
        {
            ret = 15;
            break;
        }
        ret =  dev_mifare_write_binary(block+1,cmd_buf);
RF_DEBUG("write_value:ret=%d\r\n", ret);   
        if(ret)
        {
            ret = 16;
            break;
        }
        ret = dev_mifare_backup_value(block,block+1);
RF_DEBUG("backup_value:ret=%d\r\n", ret);   
        if(ret)
        {
            ret = 17;
            break;
        }
        val = 0;
        ret = dev_mifare_read_value(block+1,&val);
RF_DEBUG("read_value4:ret=%d, %d\r\n", ret, val);  
        if(ret)
        {
            ret = 18;
            break;
        }
        if(val != 0x11223344)
        {
            ret = 19;
            break;
        }
    }while(0);  
//    dev_cl_close();
    return ret;
}

