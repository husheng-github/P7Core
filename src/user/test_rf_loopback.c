#include "devglobal.h"
#include "test_rf_loopback.h"
#include "test_interface.h"
#if 0
//#ifdef EMV_CL_TEST
#ifndef ulong
typedef unsigned long ulong; 
//#endif
//#define EMV_TEST_FLOW       1//如果用于EMV测试，请定义为1，出货程序定义为0
//extern u8 g_rf_lp_step;
//extern u8 g_rf_lp_flg;

static s32 g_emvrf_pcd_bmp_xsta = EMVRF_PCD_BMP_XSTA;
static s32 g_emvrf_pcd_bmp_ysta = EMVRF_PCD_BMP_YSTA;

#define delayms     dev_user_delay_ms//rf_delayms//dev_delay_ms //

static u8 loopback_mode_flag = 0;

#define COMMAND_LOOPBACK   0xF9

extern s32 dev_lcd_show_bmp_ram(u32 x, u32 y, u32 xlen, u32 ylen, const u8 *BmpFileName);


void lcd_initdisp(u8 mode)
{
    
}

void dev_loopback_led_blue(u8 status)
{
//    dev_misc_led_blue(status);
}     
void dev_loopback_led_red(u8 status)
{
//    dev_misc_led_red(status);
}
void dev_loopback_led_green(u8 status)
{
//    dev_misc_led_green(status);
}
void dev_loopback_led_yellow(u8 status)
{
//    dev_misc_led_yellow(status);
}

s32 lp_dev_lcd_show_bmp_ram(u32 x, u32 y, u32 xlen, u32 ylen, s8 *BmpFileName)
{
  #if 0  
    strRect lp_strRect;

    lp_strRect.m_x0 = x;
    lp_strRect.m_y0 = y;
    
    lp_strRect.m_x1 = x+xlen;
    lp_strRect.m_y1 = y+ylen;
    
    dev_lcd_show_picture_file(&lp_strRect,BmpFileName);
 #endif    
}

#define EMVRF_LOOPBACK_DISP_POLL        0
#define EMVRF_LOOPBACK_DISP_MORECARD    1
#define EMVRF_LOOPBACK_DISP_ACTIVE      2
#define EMVRF_LOOPBACK_DISP_ACTIVE_ERR  3
#define EMVRF_LOOPBACK_DISP_APDU        4
#define EMVRF_LOOPBACK_DISP_APDU_OK     5
#define EMVRF_LOOPBACK_DISP_APDU_ERR    6
#define EMVRF_LOOPBACK_DISP_REMOVE      7

void dev_loopback_disp_type(u32 type)
{
#ifdef EMVRF_DISPLAY_F
    //lcd_interface_clear_ram();
    lcd_interface_clear_rowram(DISPAGE1); 
    switch(type)
    {
    case EMVRF_LOOPBACK_DISP_POLL: 
        lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"POLLING", FDISP|CDISP);
        break;
    case EMVRF_LOOPBACK_DISP_MORECARD:
        lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"MoreCard", FDISP|CDISP);
        break;
    case EMVRF_LOOPBACK_DISP_ACTIVE_ERR: 
        lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"ActiveErr", FDISP|CDISP);
        break;
    case EMVRF_LOOPBACK_DISP_APDU: 
        lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"APUD Exchange", FDISP|CDISP);
        break;
    case EMVRF_LOOPBACK_DISP_APDU_OK: 
        lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"APUD OK", FDISP|CDISP);
        break;
    case EMVRF_LOOPBACK_DISP_APDU_ERR: 
        lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"APUD Err", FDISP|CDISP);
        break;
    case EMVRF_LOOPBACK_DISP_REMOVE:   
        lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"REMOVE", FDISP|CDISP); 
        break;
    }
    lcd_interface_brush_screen(); 
#endif    
}

//==================================================================
#define COMMDATATXMAXLEN  (1024+8)
void transflow_commu_SendPacket(s32 protocolcmd,s32 serialno,u8 *senddata,s32 senddatalen,ulong arg)
{
 #if 0   
    s32 i;
    u8 gPCIdrvsendbuf[COMMDATATXMAXLEN];
    s32 gPCIdrvsendbufLen;


    i = 0;
    gPCIdrvsendbuf[i++] = 0x02;

    gPCIdrvsendbuf[i++] = (u8)(serialno);
    memcpy(&gPCIdrvsendbuf[i], "\x0E\x01\x0B\x01",4);   // add for new specification about XGD_RFU
    i+=4;

    gPCIdrvsendbuf[i++] = (u8)(protocolcmd);
    gPCIdrvsendbuf[i++] = (u8)(senddatalen>>8);
    gPCIdrvsendbuf[i++] = (u8)(senddatalen);
    memcpy(&gPCIdrvsendbuf[i],senddata,senddatalen);
    i += senddatalen;
    //Crc16CCITT(&gPCIdrvsendbuf[1],senddatalen+4+4,&gPCIdrvsendbuf[i]);
    gPCIdrvsendbuf[i] = 0x41;
    gPCIdrvsendbuf[i+1] = 0x41;
    i += 2;
    gPCIdrvsendbuf[i++] = 0x03;


    gPCIdrvsendbufLen = i;

#ifdef SXL_DEBUG
    printf("\r\n send data: %d \r\n",gPCIdrvsendbufLen);
    for(i = 0; i<gPCIdrvsendbufLen; i++)
    {
        printf("%02x ",gPCIdrvsendbuf[i]);
    }
    printf("\r\n");
#endif

    dev_com_write(PORT_PC,gPCIdrvsendbuf,gPCIdrvsendbufLen);   //sxl?
  #endif
//dev_debug_printf("send:%d\r\n", senddatalen);  
//PrintFormat(senddata, senddatalen);
}

/*******************************************************************
Author:  
Function Name: 
Function Purpose:deal loopback command
Input Paramters: 
Output Paramters:N/A
Return Value:       N/A
Remark: N/A
Modify: N/A      1002
********************************************************************/
#if 0
void loopback_deal_command(u8 *wbuf, u16 wlen)
{
    u8 tmp[128];
    u8 m,n;

  #ifdef EMV_DEBUG_EN
    dev_debug_printf("loopback_deal_command\r\n");
  #endif
    dev_loopback_led_yellow(1);
    if((LOOPBACK_SETPARAM_COMM!=wbuf[0]) && (LOOPBACK_READPARAM_COMM!=wbuf[0]))
    {
        dev_loopback_led_blue(0);
        dev_loopback_led_green(0);
    }
    tmp[0]=wbuf[0]; 
    switch(wbuf[0])
    {
    case LOOPBACK_START_COMM:         //0x40
        n = wbuf[1];
        if(0 != (n&0x01))           //需要查询A卡
        {
            m = PICC_TYPEA;
        }
        else if(0 != (n&0x02))      //需要查询B卡
        {
            m = PICC_TYPEB;
        }
      #if 0  
        else if(0 != (n&0x04))      //需要查询C卡
        {
            m = PICC_TYPEC;
        }
        else if(0 != (n&0x08))
        {
            m = PICC_TYPE24;
        }
      #endif  
        else
        {
            tmp[1] = 0x01;          //参数错误
            transflow_commu_SendPacket(COMMAND_LOOPBACK,0xFF,tmp,2,0);
            break;
        }  

  #ifdef EMV_DEBUG_EN
    dev_debug_printf(" before dev_pcd_open\r\n");
  #endif
        
        dev_pcd_open();

  #ifdef EMV_DEBUG_EN
    dev_debug_printf("\r\n after dev_pcd_open \r\n");
  #endif

        if(MI_OK == dev_pcd_set_para(n, m))
        {

  #ifdef EMV_DEBUG_EN
    dev_debug_printf("\r\n dev_pcd_set_para == MI_OK \r\n");
  #endif

            tmp[1] = 0x00;          
            rf_loopback_init();
            rf_loopback_ctl(1);
            lcd_interface_bl_ctl(1);
            #if 0  
            dev_lcd_clear_screen();

            if(0x01 == n)
            {
                dev_fill_row_ram(1, 0, "      Type A    ", NOFDISP); 
            }
            else if(0x02 == n)
            {
                dev_fill_row_ram(1, 0, "      Type B    ", NOFDISP); 
            }
           #if 0 
            else if(0x08 == n)
            {
                dev_fill_row_ram(1, 0, "       2.4G     ", NOFDISP); 
            }
           #endif
            else
            {
                dev_fill_row_ram(0, 0, "  RF LOOPBACK   ", NOFDISP);    
                dev_fill_row_ram(1, 0, "     20101118   ", NOFDISP);
            }

            lcd_interface_brush_screen();
            #endif
        }
        else
        {
            tmp[1] = 0x02;              //初始化失败
        }                    
        loopback_mode_flag = 0;
        transflow_commu_SendPacket(COMMAND_LOOPBACK,0xFF,tmp,2,0);
        
        dev_loopback_led_red(1);
        break;
    case LOOPBACK_END_COMM:           //0x41
        dev_pcd_close();
        rf_loopback_init();
        lcd_interface_bl_ctl(0);
        lcd_initdisp(0);
        tmp[1] = 0x00;
        loopback_mode_flag = 0;
        transflow_commu_SendPacket(COMMAND_LOOPBACK,0xFF,tmp,2,0); 
        dev_loopback_led_red(0);
        break;
   #if 0     
    case LOOPBACK_REPORT_COMM:        //0x42
        break;
    case LOOPBACK_ACTIVE_COMM:        //0x43
        break;
    case LOOPBACK_OPERATE_COMM:       //0x44
        break;
    case LOOPBACK_REMOVE_COMM:        //0x45
        break;
   #endif     
    case LOOPBACK_SETPARAM_COMM:       //0x46
        //调制深度
        tmp[1] = dev_pcd_set_param(&wbuf[1], wlen-1);
        transflow_commu_SendPacket(COMMAND_LOOPBACK,0xFF,tmp,2,0);
        break;
    case LOOPBACK_READPARAM_COMM:      //0x47
        tmp[1] = dev_pcd_read_param(&tmp[2], &m);   
        transflow_commu_SendPacket(COMMAND_LOOPBACK,0xFF,tmp,m+2,0);
        break;
    case LOOPBACK_CONTROL_CARRIER:    //0x48     //控制载波
        if(0==dev_pcd_ctrl_carrier(wbuf[1]))
        {
            tmp[1] = 0x00;
        }
        else
        {
            tmp[1] = 0x01;
        }                             
        loopback_mode_flag = 0;
        transflow_commu_SendPacket(COMMAND_LOOPBACK,0xFF,tmp,2,0);
        if(wbuf[1] == 0)
        {
             dev_loopback_led_blue(0);
        }
        else
        {                              
             dev_loopback_led_blue(1);
        }
        break;
    case LOOPBACK_RESET:              //0x49     //RESET
        tmp[1] = 0x00;
        transflow_commu_SendPacket(COMMAND_LOOPBACK,0xFF,tmp,2,0); 
        if(wlen>2)
        {
            dev_pcd_loopback_reset(((u16)wbuf[1]<<8)+wbuf[2]);
        }
        else
        {
            dev_pcd_loopback_reset(1);
        }
        break;
    case LOOPBACK_WUP_R:              //0x4A  
        n = wbuf[1];
        if(0 != (n&0x01))           //需要查询A卡
        {
            m = PICC_TYPEA;
        }
        else if(0 != (n&0x02))      //需要查询B卡
        {
            m = PICC_TYPEB;
        }
      #if 0  
        else if(0 != (n&0x04))      //需要查询C卡
        {
            m = PICC_TYPEC;
        }
        else if(0 != (n&0x08))
        {
            m = PICC_TYPE24;
        }
      #endif  
        else
        {
            tmp[1] = 0x01;          //参数错误
            transflow_commu_SendPacket(COMMAND_LOOPBACK,0xFF,tmp,2,0);
            break;
        }  
        dev_pcd_open();
        if(MI_OK == dev_pcd_set_para(n, m))
        {
            tmp[1] = 0x00;          
            rf_loopback_init();
            rf_loopback_ctl(1);
            lcd_interface_bl_ctl(1);
    
        #if 0
            dev_lcd_clear_screen();
            if(0x01 == n)
            {
                dev_fill_row_ram(1, 0, "      Type A    ", NOFDISP); 
            }
            else if(0x02 == n)
            {
                dev_fill_row_ram(1, 0, "      Type B    ", NOFDISP); 
            }
        #if 0 
            else if(0x08 == n)
            {
                dev_fill_row_ram(1, 0, "       2.4G     ", NOFDISP); 
            }
        #endif
            else
            {
                dev_fill_row_ram(0, 0, "  RF LOOPBACK   ", NOFDISP);    
                dev_fill_row_ram(1, 0, "     20101118   ", NOFDISP);
            }

            lcd_interface_brush_screen();
        #endif
        }
        else
        {
            tmp[1] = 0x02;              //初始化失败
        }                                                
        loopback_mode_flag = 1;
        transflow_commu_SendPacket(COMMAND_LOOPBACK,0xFF,tmp,2,0);  
        dev_loopback_led_red(1);
        break;
    }
    dev_loopback_led_yellow(0);
    
}
#endif

#if 0
#define SOH             0x01        /*帧头标识*/
#define EOT             0x04        /*帧结束标识*/
#define ETX             0x03        /*正文结束标识*/
#define KEY_TEXT        0x12        /*键盘正文帧标识*/
#define KEY_ORDER       0X13        /*键盘命令帧标识*/
#define KEY_LTEXT       0x15        /*键盘扩展正文帧标识*/
void SendFrameData(u8 comm, u8* src, u32 len)
{
    u32 i;
    u8 TxBCC;
    u32 Txlen = 0;
    u8 TxBuf[512];
    
    if(len > 0x200) return;                                
    switch(comm)
    {
        case 0x00:
            break;
   /*
        case DDATAOK:           //0x20  //上帧正确  
        case DDATAOPRERR:       //0x21    //操作失败    
        case DDATAERR:          //0x2f  //上帧错误，请求重发//
        case DBRUSHERR:         //0x37  //回传读磁卡数据错误    //命令
        case DREQURERAND:       //0x82  //回传请求随机数
        case DSERIALCONOK:      //0x84  //密码键盘确认已完成串口控制转换
        case DDELMAINKEYOK:     //0xe1  //删除主密钥成功
        case DDELMAINKEYERR:    //0xe2  //删除主密钥失败
        case DENCRYPTPINERR:    //0xe4  //PIN加密失败
        case DRESETKEYICERR:    //0xe6  //加密芯片复位失败
        case DDECODEKEYERR:     //0xf3  //解出工作密钥或更新工作密钥失败
        case DUPDATEKEYOK:      //0xf4  //更新工作密钥成功
        case DENDRYPTERR:       //0xf6  //加密失败
        case DUPDATEKEYEN:      //0xfe  //更新密钥握手
        case DINITKEYICOK:      //0xf7  //初始化密钥芯片成功
            Tx_Valid=5;         //发送5次S0H
            TxBuf[0] = KEY_ORDER;
            TxBCC = SOH + KEY_ORDER;
            TxBuf[1] = comm;
            TxBCC += comm;
            TxBuf[2] = TxBCC;
            Txlen=0x03;
            break;  
        case DMAGDATA:          //0x31  //磁道的数据
        case DICCARDDATA:       //0x71  //操作IC卡后响应的数?       case DRESETICCARDERR:   //0x72  //IC卡复位失败
        case DQUERYICCARD:      //0X73    //回传查询IC卡，      
        case DPLUGICCARD:       //0x76  //有卡插入  
        case DDRAWICCARD:       //0X77  //卡被拔出  
        case DRESETICCARDOK:    //0x78  //回传IX卡复位数据 
        case DENDICCARD:        //0x7d  //回传IC卡结束处理//数据
        case DINPUTKEY:         //0x81  //回传输入的密码包
        case DREPLYEXPEND:      //0x83  //回传消费金额
        case DSOFT_VER:         //0xb1  //回传软件版本      //数据 
        case DEXPENDRET:        //0xbc  //回传显示金额确认
        case DREADHARDWARE:     //0xaf    //回传硬件版本 
        //--->pengxuebin, 20080722
        case DCHECKVALUE:       //0xc1    //新的MAC算法
        case DNEWENCRYPTDATA:   //0xc4    //新的加密数据
        //<----
        case DICMEMORYDEAL:     //0xca    //操作ICmemory卡  //20071119          
        case DDOWNTPKTAK:       //0xcb    //回传ICmemory卡   
        case DENCRYDATA:        //0xcc    //新加密数据  
        case DCLEARKEY:         //0xcd    //删除密钥
        case DREADKEY:          //0xd0    //读密钥
        case DENCRYPTTAK:       //0xe0  //回传加密后的TAK
        case DENCRYPTPIN:       //0xe5  //加密PIN后的结果
        case DRESETKEYIC:       //0xe7  //加密芯片复位信息
        case DDECODEDATA:       //0xe8  //解密后的明文  
        case DCRYPTOGRAPH:      //0xf0  //加密后的密文
        case DINITKEYICERR:     //0xf1  //初始化密钥芯片失败
        case DVERIFYMAC:        //0xf2  //回传用于验证的MAC  
       */
        case DEXTCMD_DD:        //0Xdd  //扩展指令
        //case EXT_COMM_DB:       //0xDB  //扩展指令
            if(len < 250)
            {
                TxBuf[0] = KEY_TEXT;
                TxBCC = SOH + KEY_TEXT + ETX;
                TxBuf[1] = comm;
                TxBCC += comm;
                TxBuf[2] = len;
                TxBCC += TxBuf[2];
                for(i=0; i<len; i++){
                    TxBuf[3+i]=src[i];
                    TxBCC += src[i];
                }
                i=len+3;
            }
            else
            {
                TxBuf[0] = KEY_LTEXT;
                TxBCC = SOH + KEY_LTEXT + ETX;
                TxBuf[1] = comm;
                TxBCC += comm;
                TxBuf[2] = len/0x100; 
                TxBCC += TxBuf[2];
                TxBuf[3] = len;
                TxBCC += TxBuf[3];
                for(i=0; i<len; i++){
                    TxBuf[4+i]=src[i];
                    TxBCC += src[i];
                }
                i = len+4;
            }
            TxBuf[i] = ETX;
            i++;
            TxBuf[i] = TxBCC;
            Txlen=i+1;                       
            break;
        default:
            break;
    }               
    {
        TxBCC = SOH;
        for(i=0;i<5;i++)
        {
            dev_com_write(PC_PORT, (s8 *)&TxBCC, 1);
        }
        dev_com_write(PC_PORT, (s8 *)TxBuf, Txlen);  
    } 
}      
#define EXT_COMM_DB         0xDB    //扩展命令帧0XDB

///////////////////定义扩展帧0x次级命令
#define EXT_DB_RF_POWERON   0x40    // 给RF上电,自动查询
#define EXT_DB_RF_RESET     0x41    // 复位及激活RF
#define EXT_DB_RF_EXCHANGE  0x42    // 操作RF
#define EXT_DB_RF_POWERDWN  0x43    // 给RF下电,
#define EXT_DB_RF_SETPARA   0x44    // 设置RF参数 
#define EXT_DB_RF_REMOVAL   0x45    // 移出卡
#define EXT_DB_RF_NEWCARD   0x46    // 上报RF查到新卡 
#define EXT_DB_RF_STARTLP   0x47    // 启动LOOPBACK  
#define EXT_DB_RF_ENDLP     0x48    // 结束LOOPBACK
#define EXT_DB_RF_MODU_DEP  0x49    // 设置调制深度



#define EXT_DB_DOMOBILE24           0x60   // 操作中国移动2.4G卡操作
#define EXT_DB_QUERYMOBILE24STATUS  0x61   // 查询连接的状态
#define EXT_DB_READMOBILEVERSION    0x62   // 查看中国移动2.4G模块版本
#define EXT_DB_VERIFYMOBILE24DATA   0x63   // 读写器认证报文
#define EXT_DB_UPDATEKEY            0x64   // 更新密钥
#define EXT_DB_GETRANDOM            0x65   // 获取随机数
#define EXT_DB_TRANSFORM24G         0x66   // 直接向2.4G发指令

#define APP_DOWNLOAD                0x70    // 应用程序中下载字库和语音库
#define LED_FLASH                   0x71    // 闪烁LED灯
#define RESET_SIM_CARD              0x72    // 复位SIM卡
#define DEAL_SIM_CARD               0x73    // SIM卡操作
#define END_SIM_CARD                0x74    // SIM下电
          
#endif
#if 1
#define LOOPBACK_OPT_CNT_MAX       2//0
u8 const PSETabl[]={0x00,0xA4,0x04,0x00,0x0E,0x32,0x50,0x41,0x59,0x2E,0x53,0x59,0x53,
                    0x2E,0x44,0x44,0x46,0x30,0x31,0x00};
//#define LOOPBACK_OPT_CNT_MAX2       14
u8 const PSETabl2[]={0x00,0xA4,0x04,0x00,0x08,0xA0,0x00,0x00,0x03,0x33,0x01,0x01,0x02,0x00 };

//u8 rf_rxtmp[300];
//u8 rf_txtmp[300];                    
u16  rf_txlen;
u16  rf_rxlen;
u8 g_loopback_led_exchange_cnt;
u8 loopback_err_flg=0;
u8 rf_rxtmp[300];
u8 rf_txtmp[300]; 
s32 lpcnt;
void rf_loopback(void)
{
    u16 i;
    u16 rlen;
    u16 rlen1;
    s32 status;
    static u8 lp_flg_bak=0xff;

    if(g_rf_lp_flg == 1)
    {
      #if 0
        if(g_rf_lp_step != RF_LP_STEP_POWERON)
        {
            dev_debug_printf("loop:step=%d\r\n", g_rf_lp_step);
        } 
      #endif  
      #if 1
        if(g_rf_lp_step != lp_flg_bak)
        {
            dev_debug_printf("loop:step=%d\r\n", g_rf_lp_step);
            lp_flg_bak = g_rf_lp_step;
        }
      #endif
        switch(g_rf_lp_step)
        {
            case RF_LP_STEP_POWERON:             //上电
                checkparam.checkcontinue = 1;
                checkparam.checkremovflg = 0;
                g_rf_lp_step = RF_LP_STEP_QUERY; 
                dev_loopback_led_green(0);   
                dev_loopback_led_blue(0);     
                dev_loopback_led_blue(1);     
                //dev_misc_led_blue(1);
                dev_pcd_reset(6);
                loopback_err_flg = 0;
                break;
            case RF_LP_STEP_POWERON1:             //上电
                checkparam.checkcontinue = 1;
                checkparam.checkremovflg = 0;
                g_rf_lp_step = RF_LP_STEP_QUERY; 
                dev_loopback_led_green(0);   
                dev_loopback_led_blue(0);     
                dev_loopback_led_blue(1);     
                //dev_misc_led_blue(1);
                dev_pcd_reset(20);
                loopback_err_flg = 0;
                break;
                ////////////////////////        
            case RF_LP_STEP_QUERY:             //搜寻卡
                rlen=0;
                rlen1=0;
                if(0 == loopback_mode_flag)
                {
                    status = dev_pcd_check_picc(&rf_rxtmp[2], &rf_rxtmp[3], &rlen);
                }
                else
                {
                    status = dev_pcd_check_picc1(&rf_rxtmp[2], &rf_rxtmp[3], &rlen);    
                //uart_printf("status=%d\r\n", status);
                }
                if(MI_OK == status)
                {
//dev_debug_printf("check_picc:%d\r\n", rlen);  
//PrintFormat(&rf_rxtmp[3], rlen); 
                    status = dev_pcd_active_picc(rf_rxtmp[2], &rf_rxtmp[3+rlen], &rlen1);
                    rf_rxtmp[0] = LOOPBACK_REPORT_COMM;
 //dev_debug_printf("active:%d\r\n", rlen1);  
                    if(MI_OK == status)
                    {
//PrintFormat(&rf_rxtmp[3+rlen], rlen1);
                        rf_rxtmp[1] = 0;         //有卡
                        if(0 == loopback_mode_flag)
                        {
                            lpcnt = 0;
                            g_rf_lp_step = RF_LP_STEP_OPERATE;    //需要操作卡 
                            rf_txlen = 0x14;
                            for(i=0; i<0x14; i++)rf_txtmp[i] = PSETabl[i];    
                            dev_loopback_led_green(1);
                            g_loopback_led_exchange_cnt=0;
                            dev_loopback_disp_type(EMVRF_LOOPBACK_DISP_APDU);   
                        }
                        else
                        {
                            g_rf_lp_step = RF_LP_STEP_REMOVE;    //需要操作卡
                            dev_loopback_disp_type(EMVRF_LOOPBACK_DISP_REMOVE);
                        }
                    }
                    else
                    {
                        rf_rxtmp[1] = 1;                         //激活失败
                        dev_loopback_disp_type(EMVRF_LOOPBACK_DISP_ACTIVE_ERR);
                    }
                    transflow_commu_SendPacket(COMMAND_LOOPBACK,0xFF,rf_rxtmp,2,0);
                    
                }
                else if((MI_COLLERR == status))// || (MI_ANTICOLL_ERR == status))   //20090907
                {
                    //继续寻卡
                    loopback_err_flg ++;
                    if(loopback_err_flg>10)
                    {
                        loopback_err_flg = 0;
                        rf_rxtmp[0] = LOOPBACK_REPORT_COMM;
                        rf_rxtmp[1] = 2;
                        transflow_commu_SendPacket(COMMAND_LOOPBACK,0xFF,rf_rxtmp,2,0);
                        dev_loopback_disp_type(EMVRF_LOOPBACK_DISP_MORECARD);                        
                    }
                }
                break;
            ////////////////////////    
            case RF_LP_STEP_OPERATE:             //操作卡
                status = dev_pcd_exchange_data(0, rf_txtmp, rf_txlen, &rf_rxtmp[2], &rf_rxlen);
                if(MI_OK == status)
                { 
                  #if 0  
                    dev_debug_printf("optok%d:%d\r\n", lpcnt, rf_rxlen);  
                    PrintFormat(&rf_rxtmp[2], rf_rxlen);
                  #endif 
                 #if 1           
                    rf_rxtmp[0] = LOOPBACK_OPERATE_COMM; 
                    //rf_rxtmp[2] = 0x00;
                    if(rf_rxlen < 4)
                    { 
                      //lpcnt++;
                      //if(lpcnt >= LOOPBACK_OPT_CNT_MAX)
                      {
                        rf_rxtmp[1] = 0x01;
                        g_rf_lp_step = RF_LP_STEP_REMOVE;    //需要操作卡
                        //SendFrameData(EXT_COMM_DB, rf_rxtmp, 3);
                        transflow_commu_SendPacket(COMMAND_LOOPBACK,0xFF,rf_rxtmp,2,0);
                        dev_loopback_disp_type(EMVRF_LOOPBACK_DISP_APDU_OK);   
                      }
                    #if 0  
                      else
                      {
                        rf_txlen = 0x14;
                        for(i=0; i<0x14; i++)rf_txtmp[i] = PSETabl[i];    
                  
                      }
                    #endif  

                    }
                    else if(rf_rxtmp[2+1] == 0x70)
                    {
                        rf_rxtmp[1] = 0x01;
                        g_rf_lp_step = RF_LP_STEP_REMOVE;
                        //SendFrameData(EXT_COMM_DB, rf_rxtmp, 3);
                        transflow_commu_SendPacket(COMMAND_LOOPBACK,0xFF,rf_rxtmp,2,0);
                        dev_loopback_disp_type(EMVRF_LOOPBACK_DISP_APDU_OK);                        

                    }
                  #if 1  
                    else if(rf_rxtmp[2+1] == 0x72)
                    {
                        rf_rxtmp[1] = 0x01;
                        g_rf_lp_step = RF_LP_STEP_POWERON1;
                        //delayms(4); 
                        //SendFrameData(EXT_COMM_DB, rf_rxtmp, 3);
                        transflow_commu_SendPacket(COMMAND_LOOPBACK,0xFF,rf_rxtmp,2,0);
                        dev_loopback_disp_type(EMVRF_LOOPBACK_DISP_APDU_OK);                        

                    }
                  #endif  
                    else
                    {
                        rf_rxtmp[1] = 0x00;
                        memcpy(rf_txtmp, &rf_rxtmp[2], rf_rxlen-2);
                        rf_txlen = rf_rxlen-2;
                        //SendFrameData(EXT_COMM_DB, rf_rxtmp, rf_rxlen+3);
                        transflow_commu_SendPacket(COMMAND_LOOPBACK,0xFF,rf_rxtmp,rf_rxlen+2,0);

                    }        
                  #else
                      lpcnt++;
                      if(lpcnt >= LOOPBACK_OPT_CNT_MAX)
                      {
                        rf_rxtmp[1] = 0x01;
                        g_rf_lp_step = RF_LP_STEP_REMOVE;    //需要操作卡
                        //SendFrameData(EXT_COMM_DB, rf_rxtmp, 3);
                        transflow_commu_SendPacket(COMMAND_LOOPBACK,0xFF,rf_rxtmp,2,0);
                        dev_loopback_disp_type(EMVRF_LOOPBACK_DISP_APDU_OK);   
                      }
                      else
                      {
                        rf_txlen = 14;
                        for(i=0; i<14; i++)rf_txtmp[i] = PSETabl2[i];    
                  
                      }
                    
                  #endif
                    dev_loopback_led_green(g_loopback_led_exchange_cnt);
                    if(g_loopback_led_exchange_cnt)g_loopback_led_exchange_cnt=0;
                    else g_loopback_led_exchange_cnt = 1;
                }
                else
                {
                    g_rf_lp_step = RF_LP_STEP_REMOVE;    //操作不成功移除卡
                    rf_rxtmp[0] = LOOPBACK_OPERATE_COMM;
                    rf_rxtmp[1] = 0x02;
                    //rf_rxtmp[2] = 0x00;
                    //SendFrameData(EXT_COMM_DB, rf_rxtmp, 3);
                    transflow_commu_SendPacket(COMMAND_LOOPBACK,0xFF,rf_rxtmp,2,0);
                    dev_loopback_disp_type(EMVRF_LOOPBACK_DISP_APDU_ERR); 
                }
                break;
            ///////////////////////
                case RF_LP_STEP_REMOVE:             //移除,下电
                delayms(2); 
                dev_pcd_reset(6);
                checkparam.checkcontinue = 0; 
                checkparam.checkremovflg = 1;   //需要检测卡被移除
                g_rf_lp_step = RF_LP_STEP_REMOVE1; 
                dev_loopback_led_green(0);   
                dev_loopback_led_blue(1);
                break;
            ///////////////////////    
            case RF_LP_STEP_REMOVE1:            //移除
                status = dev_pcd_checkremov_picc();
                if(0 == status)
                {
                    rf_rxtmp[0] = LOOPBACK_REMOVE_COMM;
                    rf_rxtmp[1] = 0;
                    //SendFrameData(EXT_COMM_DB, rf_rxtmp, 2);
                    g_rf_lp_step = RF_LP_STEP_POWERON;
                    transflow_commu_SendPacket(COMMAND_LOOPBACK,0xFF,rf_rxtmp,2,0);
                    dev_loopback_disp_type(EMVRF_LOOPBACK_DISP_POLL); 
                    delayms(5);
                }
                break;    
            //////////////////////
            default:
                g_rf_lp_step = RF_LP_STEP_POWERON;
                break;   
        }
    }
}
#endif
/***********************************************
***********************************************/
s32 emvcl_digital_test(u8 type, u8 mode)
{
//    u8 disflg = 1;
//    u8 tmp[2];
    u8 m;
    u8 key;
    u8 typeflg = 0;
    s32 ret;

    if(type==3)
    {
        type = 0;
        typeflg = 1;
    }

    if(0 != (mode&0x01))
    {
        m = PICC_TYPEA;
    }
    else if(0 != (mode&0x02))
    {
        m = PICC_TYPEB;
    }
    else
    {
        return -1;
    }
    dev_pcd_open();
    if(MI_OK == dev_pcd_set_para(mode, m))
    {
        rf_loopback_init();
        rf_loopback_ctl(1);
        lcd_interface_clear_ram(); 
      #ifndef EMVRF_BMP_ICO   
        if(0x01 == mode)
        {
            if(0 == type)
            {
                lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"TypeA", NOFDISP|CDISP);
            }
            else
            {
                lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"TypeA-1", NOFDISP|CDISP);
            }
        }
        else if(0x02 == mode)
        {
            if(0 == type)
            {
                lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"TypeB", NOFDISP|CDISP);
            }
            else
            {
                lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"TypeB-1", NOFDISP|CDISP);
            }
        }
        else
        {
            if(typeflg == 1)
            {
                lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"DIGITAL", NOFDISP|CDISP);
            }
            else
            {
                lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"POLLING", NOFDISP|CDISP);
            }
        } 
      #endif  
      #ifdef EMVRF_BMP_ICO
        lp_dev_lcd_show_bmp_ram(g_emvrf_pcd_bmp_xsta, g_emvrf_pcd_bmp_ysta, EMVRF_PCD_BMP_XLEN, EMVRF_PCD_BMP_YLEN, EMVRF_PCD_ICO_PATH);
      #endif
        lcd_interface_brush_screen();
    }
    else
    {
        dev_debug_printf("%s(%d):RF initialize ERR!", __FUNCTION__, __LINE__);
        
        return -1;
    }
    if(type)
    {
        loopback_mode_flag = 1;
    }
    else
    {
        loopback_mode_flag = 0;
    }
    while(1)
    {
        rf_loopback();
        ret = keypad_interface_read_beep(&key);
        if(ret)
        {
            if(ESC == key)
            {
                dev_pcd_close();
                rf_loopback_init();
                return 0;
            }
        }
    }
}
s32 emvcl_digital_test1(u8 type, u8 mode)
{
#if 0    
    u8 disflg = 1;
    u8 tmp[2];
    u8 m;
    u8 key;

    if(0 != (mode&0x01))
    {
        m = PICC_TYPEA;
    }
    else if(0 != (mode&0x02))
    {
        m = PICC_TYPEB;
    }
    else
    {
        return -1;
    }
    dev_pcd_open();
    if(MI_OK == dev_pcd_set_para(mode, m))
    {
        rf_loopback_init();
        rf_loopback_ctl(1);
        lcd_interface_clear_ram();
        if(0x01 == mode)
        {
            lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"TypeA", NOFDISP|CDISP);
        }
        else if(0x02 == mode)
        {
            lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"TypeB", NOFDISP|CDISP);
        }
        else
        {
            lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"POLLING", NOFDISP|CDISP);
        }
        lcd_interface_brush_screen();
    }
    else
    {
        uart_printf("emvcl_digital_test:RF initialize ERR!");
        
        return -1;
    }
    if(type)
    {
        loopback_mode_flag = 1;
    }
    else
    {
        loopback_mode_flag = 0;
    }
    while(1)
    {
        rf_loopback1();
        key = dev_get_key();
        if(ESC == key)
        {
            dev_pcd_close();
            rf_loopback_init();
            return 0;
        }
    }
#endif    
    return 0;
}
/***********************************************
***********************************************/
s32 emvcl_test_mifare(void)
{
#if 0    
    u8 disflg = 1;
//    u8 m;
    u8 key;
    u16 i;
    u16 rlen;
    u16 rlen1;
    s32 status;
    u8 uid[16];
    s32 ret;

//    m = PICC_TYPEA;
    dev_pcd_open();
    if(MI_OK == dev_pcd_powerup(0x03))
    {
        //rf_loopback_init();
        //dev_pcd_powerup();
        lcd_interface_clear_ram();
        lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"Mifare", NOFDISP|CDISP);
        lcd_interface_brush_screen();
    }
    else
    {
        dev_debug_printf("emvcl_test_mifare:RF initialize ERR!");
        return -1;
    }
    //寻卡
    dev_pcd_reset(6);
    while(1)
    {
        rlen = 0; 
        rlen1 = 0;
        status = dev_pcd_check_picc(&rf_rxtmp[2], &rf_rxtmp[3], &rlen);
        if(MI_OK == status)
        {
            dev_debug_printf("RF Search OK:len =%d,type=%02X\r\n", rlen, rf_rxtmp[3]);            
            PrintFormat(&rf_rxtmp[i+3], rlen);
            break;
        }
        ret = posdev_keypad_read(&key);
        if(ret)
        {
            if(ESC == key)
            {
                rf_loopback_init();
                dev_pcd_close();
                return 0;
            }
        }
    }
    lcd_interface_clear_ram();
    lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"Mifare", NOFDISP|CDISP);
    lcd_interface_fill_rowram(DISPAGE2, 0, (u8*)"1.rd 2.auth 3.wr", NOFDISP|CDISP);
    lcd_interface_brush_screen();
    disflg = 1;
    while(1)
    {
        if(disflg==1)
        {
            disflg = 0;
            dev_debug_printf("********************************\r\n");
            dev_debug_printf("1.Read   2.Authent   3.Write\r\n");
            //uart_printf("");
            dev_debug_printf("********************************\r\n");
        }
        ret = posdev_keypad_read(&key);
        if(ret)
        {
            switch(key)
            {
            case DIGITAL1:
                disflg = 1;
                dev_pcd_setdebug(1);
                memset(rf_rxtmp, 0, 16);
                status = drv_mifare_read_binary(0, rf_rxtmp);
                dev_pcd_setdebug(0);
                dev_debug_printf("read binary: %d\r\n", status);
                if(MI_OK == status)
                {
                    PrintFormat(rf_rxtmp, 16);
                }
                break;
            case DIGITAL2:
                disflg = 1;
                dev_pcd_setdebug(1);
                dev_pcd_get_uid(uid, 4);
                status = dev_mifare_auth(0x60, "\xFF\xFF\xFF\xFF\xFF\xFF",uid,4);
                dev_pcd_setdebug(0);
                dev_debug_printf("auth:%d\r\n", status);
                break;
            case DIGITAL3: 
                disflg = 1;
                dev_pcd_setdebug(1);
                memset(rf_rxtmp, 0, 16);
                status = dev_mifare_read_binary(4, rf_rxtmp);
                dev_pcd_setdebug(0);
                dev_debug_printf("read:%d\r\n", status);
                PrintFormat(&rf_rxtmp, 16);
                break;
            case DIGITAL4:
                disflg = 1;
                for(i=0; i<16; i++)
                {
                    rf_rxtmp[i] = 0x31+i;
                }
                dev_pcd_setdebug(1);
                status = dev_mifare_write_binary(4, rf_rxtmp);
                dev_pcd_setdebug(0);
                dev_debug_printf("write44:%d\r\n", status);            
                break;
            case DIGITAL5:
                disflg = 1;
                for(i=0; i<16; i++)
                {
                    rf_rxtmp[i] = 0x31;
                }
                dev_pcd_setdebug(1);
                status = dev_mifare_write_binary(4, rf_rxtmp);
                dev_pcd_setdebug(0);
                dev_debug_printf("write55:%d\r\n", status);  
                break;
            case DIGITAL6:
                disflg = 1;
                break;
            case DIGITAL7:
                disflg = 1;
                break;
            case DIGITAL8:
                disflg = 1;
                break;
            case DIGITAL9:
                disflg = 1;
                break;
                
            case ESC:
                rf_loopback_init();
                dev_pcd_close();
                return 0;
            }
        }
    }
#endif    
}
/***********************************************
***********************************************/
void emvcl_carrier_test(void)
{
    u8 key;
    s32 ret;
    
    lcd_interface_clear_ram();
    lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"CarrierTest", NOFDISP|CDISP);
    //lcd_interface_fill_rowram(DISPAGE2, 0, (u8*)"CarrierOn", FDISP|LDISP);
    lcd_interface_brush_screen(); 
    
    dev_pcd_open();
    dev_pcd_ctrl_carrier(1);   
    while(1)
    {
        ret = keypad_interface_read_beep(&key);
        if(ret)
        {
            switch(key)
            {
            case ESC:
                dev_pcd_ctrl_carrier(0);
                return;
            }
        }
    }
}
void emvcl_reset_test(void)
{
    u8 key;
    s32 ret;
    
    lcd_interface_clear_ram();
    lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"ResetTest", NOFDISP|CDISP);
    lcd_interface_brush_screen(); 
    dev_pcd_open();
    while(1)
    {
        dev_pcd_ctrl_carrier(1);
        delayms(100);
        dev_pcd_ctrl_carrier(0);
//        delayms(6);
        
        ret = keypad_interface_read_beep(&key);
        if(ret)
        {
            switch(key)
            {
            case ESC:
                dev_pcd_ctrl_carrier(0);
                return;
            }
        }
    }
    
}



/***********************************************
***********************************************/
void emvcl_test_analogue(void)
{
    u8 key;
    u8 disflg = 1;
//    u8 tmp[64];
//    u8 flg1 = 0;
    u8 dismenu=0;
    s32 ret;

    dismenu = 0;
    while(1)
    {
        if(1 == disflg)
        {       
            disflg = 0;
            lcd_interface_clear_ram();
            if(dismenu>2)dismenu=0;
            if(0 == dismenu)
            {
                lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"1.Carrier", FDISP|LDISP);
                lcd_interface_fill_rowram(DISPAGE2, 0, (u8*)"2.polling", FDISP|LDISP);
            }
            else if(1 == dismenu)
            {
                lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"3.Reset", FDISP|LDISP);
                lcd_interface_fill_rowram(DISPAGE2, 0, (u8*)"4.WUPA  5.WUPB", FDISP|LDISP);
            }
            else if(2 == dismenu)
            {
                lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"6.WUPA-1", FDISP|LDISP);
                lcd_interface_fill_rowram(DISPAGE2, 0, (u8*)"7.WUPB-1", FDISP|LDISP);
            }
            
            //lcd_interface_fill_rowram(DISPAGE4, 0, (u8*)"3.wupa", FDISP|LDISP);
            lcd_interface_brush_screen(); 
        }
        ret = keypad_interface_read_beep(&key);
        if(ret)
        {
            switch(key)
            {
            case DIGITAL1:
                emvcl_carrier_test();
                disflg = 1;
                break;
            case DIGITAL2:
                emvcl_digital_test(0, 0x03); 
                disflg = 1;
                break;
            case DIGITAL3:
                emvcl_reset_test();
                disflg = 1;
                break;
            case DIGITAL4:
                emvcl_digital_test(0, 0x01);
                disflg = 1;
                break;
            case DIGITAL5:
                emvcl_digital_test(0, 0x02);
                disflg = 1;
                break;
            case DIGITAL6:
                emvcl_digital_test(1, 0x01);
                disflg = 1;
                break;
            case DIGITAL7:
                emvcl_digital_test(1, 0x02);
                disflg = 1;
                break;
            //-----------------------------------    
            case DIGITAL8://PGUP
                if(dismenu<2)
                {
                    dismenu++;
                    disflg = 1;
                }
                break;
            case DIGITAL0://PGDWON:
                if(dismenu>0)
                {
                    dismenu--;
                    disflg = 1;
                }
                break;
            case ESC:
                return;
                
            }
        }
    }
}
/***********************************************
***********************************************/
s32 emvcl_param_input(u8 type, u8 min, u8 max, u8 curr)
{
    u8 dispstr[32];
    u8 dispstr1[32];
    u8 key;
    u8 disflg = 1;
    u16 n;
    s32 ret;

    n = curr;
    switch(type)
    {
    case 0:
        sprintf(dispstr, "调制深度(%02X)", curr);
        break;
    case 1:
        sprintf(dispstr, "TypeA增益(%02X)", curr);
        break;
    case 2:
        sprintf(dispstr, "TypeB增益(%02X)", curr);
        break;
    case 3:
        sprintf(dispstr, "TypeA门限(%02X)", curr);
        break;
    case 4:
        sprintf(dispstr, "TypeA门限L(%02X)", curr);
        break;
    case 5:
        sprintf(dispstr, "TypeB门限(%02X)", curr);
        break;
    case 6:
        sprintf(dispstr, "TypeB门限L(%02X)", curr);
        break;
    default:
        return -1;
    }
    while(1)
    {
        if(1 == disflg)
        {       
            disflg = 0;
            sprintf(dispstr1, "%02X", n); 
            lcd_interface_clear_ram();
            lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)dispstr, FDISP|CDISP);
            lcd_interface_fill_rowram(DISPAGE2, 0, (u8*)dispstr1, FDISP|LDISP);
            lcd_interface_brush_screen();
        }
    
        ret = keypad_interface_read_beep(&key);
        if(ret)
        {
            switch(key)
            {
            //-----------------------------------    
            case DIGITAL2://PGUP:
                if(n<max)
                {
                    n++;
                    disflg = 1;
                }
                break;
            case DIGITAL4:
                if((n>(min+0x10)))
                {
                    n-=0x10;
                    disflg = 1;
                }
                break;
            case DIGITAL6:
                if((n+0x10)<max)
                {
                    n+=0x10;
                    disflg = 1;
                }
                break;
                
            case DIGITAL8://PGDWON:
                if(n>min)
                {
                    n--;
                    disflg = 1;
                }
                break;
            case ESC:
                return -1;
            case ENTER:
                return n;
            }
        }
    }
    
}
void emvcl_param_set(void)
{
    RF_PARAM lp_rf_param;
    s32 ret;

    dev_pcd_readallparam((u8*)&lp_rf_param, 32);
    
    ret = emvcl_param_input(0, 0, 0xff, lp_rf_param.rfmodgsp);
    if(ret>=0)
    {
        lp_rf_param.rfmodgsp = ret;
    }
    ret = emvcl_param_input(1, 0, 0xff, lp_rf_param.rfcfgregA);
    if(ret>=0)
    {
        lp_rf_param.rfcfgregA = ret;
    }
    ret = emvcl_param_input(2, 0, 0xff, lp_rf_param.rfcfgregB);
    if(ret>=0)
    {
        lp_rf_param.rfcfgregB = ret;
    }
  #if 0 
    ret = emvcl_param_input(3, 0, 0xf, lp_rf_param.rxthreshold_a>>4);
    if(ret>=0)
    {
        lp_rf_param.rxthreshold_a = (lp_rf_param.rxthreshold_a&0x0f)| ((ret&0x0f)<<4);
    }
    ret = emvcl_param_input(4, 0, 0xf, lp_rf_param.rxthreshold_a&0x0f);
    if(ret>=0)
    {
        lp_rf_param.rxthreshold_a = (lp_rf_param.rxthreshold_a&0xf0)| (ret&0x0f);
    }

    
    ret = emvcl_param_input(5, 0, 0xf, lp_rf_param.rxthreshold_b>>4);
    if(ret>=0)
    {
        lp_rf_param.rxthreshold_b = (lp_rf_param.rxthreshold_b&0x0f)| ((ret&0x0f)<<4);
    }
    ret = emvcl_param_input(6, 0, 0xf, lp_rf_param.rxthreshold_b&0x0f);
    if(ret>=0)
    {
        lp_rf_param.rxthreshold_b = (lp_rf_param.rxthreshold_b&0xf0)| (ret&0x0f);
    }
   #else
    ret = emvcl_param_input(3, 0, 0xff, lp_rf_param.rxthreshold_a);
    if(ret>=0)
    {
        lp_rf_param.rxthreshold_a = ret;
    }
    ret = emvcl_param_input(5, 0, 0xff, lp_rf_param.rxthreshold_b);
    if(ret>=0)
    {
        lp_rf_param.rxthreshold_b = ret;
    }
   #endif 
    dev_pcd_writeallparam((u8*)&lp_rf_param, 32);
    
}
void emvcl_test_file(void)//s32 drv_cl_store_param(u8 *buf,u32 size)
{
#if 0    
    u8 tmp[32];
    s32 i;
    s32 disflg=1;
    u8 key;
    s32 ret;
    
    while(1)
    {
        if(1 == disflg)
        {       
            disflg = 0;
            lcd_interface_clear_ram();
            lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"测试文件", FDISP|CDISP);
            lcd_interface_brush_screen(); 
        }
        ret = posdev_keypad_read(&key);
        switch(key)
        {
        case '1':
            for(i=0; i<32; i++)
            {
                tmp[i] = (i&0x0f) +0x30;
            }
            drv_cl_store_param(tmp, 32);
            disflg = 1;
            break;
        case '2':
            for(i=0; i<32; i++)
            {
                tmp[i] = 0x55;
            }
            drv_cl_store_param(tmp, 32); 
            disflg = 1;
            break;
        case '3':
            for(i=0; i<32; i++)
            {
                tmp[i] = 0xaa;
            }
            drv_cl_store_param(tmp, 32);
            disflg = 1;
            break;
        case '4':
            memset(tmp, 0, 32);
            drv_cl_read_param(tmp, 32);
printf("read:");
PrintFormat(tmp, 32);
            disflg = 1;
            break;
        case ESC:
            return;
            
        }
    }
#endif    
}
/***********************************************
***********************************************/
void emvl1_param_set_default(void)
{
    u8 key;
    u8 disflg =1;
    u8 tmp[16];
    u8 pin[10];
    u8 pinlen;
    s32 ret;
    u8 data;
    

    pinlen = 0;
    while(1)
    {
        if(disflg)
        {
            disflg = 0;
            lcd_interface_clear_ram();
            lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"恢复默认参数", NOFDISP|CDISP);
            memset(tmp, 0, 16);
            if(pinlen)
            {
                memset(tmp, '*', pinlen);
            }
            lcd_interface_fill_rowram(DISPAGE3, 0, tmp, FDISP|LDISP);
            lcd_interface_brush_screen(); 
            keypad_interface_clear();
        }
        
        ret = keypad_interface_read_beep(&key);
        if(ret)
        {
            data = 0;
            switch(key)
            {
            case DIGITAL0:
                data = 0x30;
                break;
            case DIGITAL1:
                data = 0x31;
                break;
            case DIGITAL2:
                data = 0x32;
                break;
            case DIGITAL3:
                data = 0x33;
                break;
            case DIGITAL4:
                data = 0x34;
                break;
            case DIGITAL5:
                data = 0x35;
                break;
            case DIGITAL6:
                data = 0x36;
                break;
            case DIGITAL7: 
                data = 0x37;
                break; 
            case DIGITAL8:
                data = 0x38;
                break;
        	case DIGITAL9:
                data = 0x39;
                break;
            //-------------------------------------    
            case ENTER:
                if(pinlen == 6)
                {
                    if(0 == memcmp(pin, "667788", 6))
                    {
                        dev_pcd_param_set_default();
                        data = 1;
                    }
                    else
                    {
                        data = 0;
                    }
                    lcd_interface_clear_ram();
                    lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"恢复默认参数", NOFDISP|CDISP);
                    if(data==1)
                    {
                        lcd_interface_fill_rowram(DISPAGE3, 0, (u8*)"参数恢复成功", FDISP|CDISP);
                    }
                    else
                    {
                        lcd_interface_fill_rowram(DISPAGE3, 0, (u8*)"密码错", FDISP|CDISP);
                    }
                    lcd_interface_brush_screen(); 
            
                    rf_delayms(2000);//dev_delay_ms(2000);
                    return;
                }
                break;
            //-------------------------------------
            case CLEAR:
                if(pinlen)
                {
                    pinlen--;
                }
                disflg = 1;
                break;
            //-------------------------------------
            case ESC:
                return;
            }
            if(data != 0)
            {
                if(pinlen<6)
                {
                    pin[pinlen++] = data;
                }
                disflg = 1;
            }
        }
    }
}
/***********************************************
***********************************************/
void emvcl_param_manage(void)
{
    u8 key;
    u8 disflg = 1;
    s32 ret;
    RF_PARAM lp_rf_param;
    u8 tmp[32];

    
    while(1)
    {
        if(1 == disflg)
        {       
            disflg = 0;
            lcd_interface_clear_ram();
            lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"RF Param Manage", NOFDISP|CDISP);
            lcd_interface_fill_rowram(DISPAGE2, 0, (u8*)"1.Set Param", FDISP|LDISP);
            lcd_interface_fill_rowram(DISPAGE3, 0, (u8*)"2.Default Param", FDISP|LDISP);
            dev_pcd_readallparam((u8*)&lp_rf_param, 32);
            sprintf(tmp, "(%02X %02X %02X %02X %02X)", lp_rf_param.rfmodgsp, lp_rf_param.rfcfgregA,
                    lp_rf_param.rfcfgregB, lp_rf_param.rxthreshold_a, lp_rf_param.rxthreshold_b);
            lcd_interface_fill_rowram(DISPAGE5, 0, tmp, FDISP|CDISP);
            lcd_interface_brush_screen(); 
        }
        ret = keypad_interface_read_beep(&key);
        if(ret)
        {
            switch(key)
            {
            case DIGITAL1:
                emvcl_param_set();
                disflg = 1;
                break;
            case DIGITAL2:
                emvl1_param_set_default();
                disflg = 1;
                break;
            case DIGITAL0:
                lcd_interface_clear_ram();
                lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"LOOPBACK SUPPORT", NOFDISP|CDISP);
                dev_pcd_readallparam((u8*)&lp_rf_param, 32);
            
              #ifndef EMVCL1_2_5A
                lcd_interface_fill_rowram(DISPAGE3, 0, (u8*)"EMV CL1 2.3A", FDISP|CDISP);
              #else
                lcd_interface_fill_rowram(DISPAGE3, 0, (u8*)"EMV CL1 2.5A", FDISP|CDISP);
              #endif
                lcd_interface_brush_screen();    
                rf_delayms(2000);//dev_delay_ms(2000);
                disflg = 1;
                break;
            case ESC:
                return;
                break;
            }
        }
    }
}
/***********************************************
***********************************************/
void emvcl_icon_set(void)
{
    u8 key;
    u8 disflg =1;
    u8 tmp[32];
    u8 func=0;   //0:粗调,  1:细调
    u8 step = 10;
    s32 ret;

    while(1)
    {
        if(disflg)
        {
            disflg = 0;
            lcd_interface_clear_ram();
            lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"ADJUST ICON PLACE", NOFDISP|CDISP);
            if(func==0)
            {
                sprintf(tmp, "粗调(%3d,%3d)", g_emvrf_pcd_bmp_xsta, g_emvrf_pcd_bmp_ysta);
            }
            else
            {
                sprintf(tmp, "细调(%3d,%3d)", g_emvrf_pcd_bmp_xsta, g_emvrf_pcd_bmp_ysta);
            }
            lcd_interface_fill_rowram(EMVRF_SEARCH_ROW, 0, tmp, FDISP|CDISP);
            lp_dev_lcd_show_bmp_ram(g_emvrf_pcd_bmp_xsta, g_emvrf_pcd_bmp_ysta, EMVRF_PCD_BMP_XLEN, EMVRF_PCD_BMP_YLEN, EMVRF_PCD_ICO_PATH);
            lcd_interface_brush_screen(); 
            keypad_interface_clear();
        }
        
        ret = keypad_interface_read_beep(&key);
        if(ret)
        {
            switch(key)
            {
            case DIGITAL4:
                //左移
            
                if(g_emvrf_pcd_bmp_xsta<step)
                {
                    g_emvrf_pcd_bmp_xsta = 0;
                }
                else
                {
                    g_emvrf_pcd_bmp_xsta -= step;
                }
                disflg = 1;
                break;
            //--------------------------------------    
            case DIGITAL6:
                //右移
                if((g_emvrf_pcd_bmp_xsta+step) > LCD_MAX_COL)
                {
                    g_emvrf_pcd_bmp_xsta = LCD_MAX_COL;
                }
                else
                {
                    g_emvrf_pcd_bmp_xsta += step;
                }
                    
                disflg = 1;
                break;
            //--------------------------------------
        	case DIGITAL2:
                //上移
                if(g_emvrf_pcd_bmp_ysta<step)
                {
                    g_emvrf_pcd_bmp_ysta = 0;
                }
                else
                {
                    g_emvrf_pcd_bmp_ysta -= step;
                }
                disflg = 1;
                break;
            //--------------------------------------
            case DIGITAL8:
                //下移
                if((g_emvrf_pcd_bmp_ysta+step) > LCD_MAX_LINE)
                {
                    g_emvrf_pcd_bmp_ysta = LCD_MAX_LINE;
                }
                else
                {
                    g_emvrf_pcd_bmp_ysta += step;
                }
                disflg = 1;
                break;
            //-------------------------------------    
            case FUNCTION:
            case EMV_FUNCTION:
                //下移 //F1
                if(0 == func)
                {
                    func = 1;
                    step = 1;
                }
                else
                {
                    func = 0;
                    step = 10;
                }
                disflg = 1;
                break;
            //-------------------------------------
            case ESC:
                return;
            }
        }
    }
}
/***********************************************
***********************************************/
void emvcl_test(void)
{
    u8 key;
    u8 disflg = 1;
//    u8 flg1 = 0;
    u8 dismenu=0;
    s32 ret;
    u8 tmp[8];
    u8 timerid;


//    dev_lcd_open(); 
dev_debug_printf("emvcl_test:%s %s\r\n", __DATE__, __TIME__);
  #if 0
    lcd_interface_clear_ram();
    lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"EMVCL1 LOOPBACK", FDISP|CDISP);
    lcd_interface_fill_rowram(DISPAGE2, 0, (u8*)"(20160324)", FDISP|CDISP);
    lcd_interface_brush_screen(); 
  #endif
    dev_pcd_open();
    lcd_interface_bl_ctl(1);
//    posdev_keypad_open();
//	ddi_audio_open();
    dev_pcd_reset(6);
//    dev_delay_ms(1000);
    
    emvcl_digital_test(3, 0x03);
    dismenu = 0;
    while(1)
    {
        if(1 == disflg)
        {       
            disflg = 0;
            lcd_interface_clear_ram();
            lcd_interface_fill_rowram(DISPAGE1, 0, (u8*)"EMVCL1 LOOPBACK", NOFDISP|CDISP);
            lcd_interface_fill_rowram(DISPAGE2, 0, (u8*)"(20170104)", FDISP|CDISP);
            lcd_interface_fill_rowram(DISPAGE3, 0, (u8*)"1.Analogue", FDISP|LDISP);
            lcd_interface_fill_rowram(DISPAGE4, 0, (u8*)"2.Digital", FDISP|LDISP);
            lcd_interface_brush_screen(); 
        }
        ret = keypad_interface_read_beep(&key);
        if(ret)
        {
            switch(key)
            {
            case DIGITAL1:
                emvcl_test_analogue();
                disflg = 1;
                break;
            case DIGITAL2:
                emvcl_digital_test(3, 0x03); //emvcl_digital_test(0, 0x03); 
                disflg = 1;
                break;
            case DIGITAL3:
               // emvcl_param_set();
                emvcl_param_manage();
                disflg = 1;
                break;
            case DIGITAL4:
    //            emvcl_test_file();
//                dev_rc663_read_version(tmp);
    
                break;
            case DIGITAL9:
                emvl1_param_set_default();
                break;
          #ifdef EMVRF_BMP_ICO
            case DIGITAL0:
                emvcl_icon_set();
                disflg = 1;
                break;
          #endif  
            case POWER:
              #if 0  
                while(1)
                {
                    //判断松键
                    drv_power_check_powerkey(&key);
                    if(key==0)
                    {
                        lcd_interface_bl_ctl(0);
                        lcd_interface_clear_ram();
                        lcd_interface_brush_screen();
                        ddi_misc_poweroff();
                        break;
                    }
                }
                while(1)
                {
                    //有外电时程序关不掉,等待按键按下重启
		            ret = drv_power_check_powerkey(&key);
                    if(key == 1)
                    {
                        rf_delayms(200);//dev_delay_ms(200);
                        ret = drv_power_check_powerkey(&key);
                        if(key == 1)
                        {
                            ddi_misc_reboot();
                        }
                    }
                }
              #endif  
                break;
                
            }
        }
    }
}


#endif
#endif
