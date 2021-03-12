
#include <string.h>
#include <stdio.h>
//#include "mhscpu.h"
#include "sdk\mhscpu_sdk.h"
#include "devglobal.h"
#include "ddi_mag.h"

//#include "core\coremanage.h"
//#include "app_utils.h"
#include "ddi.h"
#include "ddi_misc.h"
#include "ddi_wifi.h"
#include "ddi_led.h"
#include "filesystem/fs.h"

#if 0
s32 testgetkey(u32 *key)
{
  #if 0  
    u8 ch;
    if(1 == dev_com_read(DEBUG_PORT_NO, &ch, 1))
    {
        *key = ch;
        return 1;
    }
    else
    {
        return 0;
    }
  #else
    s32 ret;
    ret = dev_keypad_read(key);
    if(ret > 0)
    {
        dev_audio_ioctl(DDI_AUDIO_CTL_BUZZER, 0, 200);
    }
    return ret;
  #endif
}
#endif

//iomux_pad_config_t test_gpio_tab[];

void gpio_test(void)
{
    #if 1
        
        u8 flg;
        u32 key;
        s32 ret;
        u32 rlen;
        s32 i;
    
        flg = 1;
        while(1)
        {
            if(flg == 1)
            {
                flg = 0;
                lcd_interface_clear_ram();
                lcd_interface_fill_rowram(0, 0, "FLASH TEST", NOFDISP|CDISP);
                lcd_interface_fill_rowram(1, 0, "1.flash cs high", FDISP);
                lcd_interface_fill_rowram(2, 0, "2.flash cs low", FDISP);
                //lcd_interface_fill_rowram(3, 0, "3.write2", FDISP);
                //lcd_interface_fill_rowram(4, 0, "4.write3", FDISP);
                lcd_interface_brush_screen();
                keypad_interface_clear();
            }
            if(keypad_interface_read_beep(&key)>0)
            {
                switch(key)
                {
                case DIGITAL1:
                    dev_gpio_direction_output(GPIO_PIN_PTA7, 1);
                    break;
                    
                case DIGITAL2:
                    dev_gpio_direction_output(GPIO_PIN_PTA7, 0);
                    break;
                    
                case DIGITAL3:

                    break;
                case DIGITAL5: 

                case ESC:
                    flg = 1;
                    return;
                    break;
                }
            }
        }

    #else
    iomux_pad_config_t testpin;
    s32 i;
    s32 old=-1;

    testpin = GPIO_PIN_PTC3;//GPIO_PIN_PTB12;

	dev_gpio_config_mux(GPIO_PIN_PTC8, MUX_CONFIG_GPIO);
	dev_gpio_set_pad(GPIO_PIN_PTC8, PAD_CTL_PULL_UP);
    dev_gpio_direction_output(GPIO_PIN_PTC8, 0);
	
    dev_gpio_config_mux(testpin, MUX_CONFIG_ALT1);   //
    dev_gpio_set_pad(testpin, PAD_CTL_PULL_NONE);

    dev_gpio_direction_output(testpin, 1);
//    dev_gpio_direction_input(testpin);
dev_debug_printf("%s(%d):%s %s\r\n", __FUNCTION__, __LINE__, __DATE__, __TIME__);    
    while(1)
    {
#if 1        
        dev_user_delay_us(100);
        dev_gpio_set_value(testpin, 0);
        dev_user_delay_us(100);
        dev_gpio_set_value(testpin, 1);
#else
        i = dev_gpio_get_value(testpin);
        if(0 == i)
        {
            if(old!=0)
            {
                old = 0;
dev_debug_printf("%s:%d\r\n", GPIO_PIN_PTA6, old);    
                
            }
        }
        else
        {
            if(old!=1)
            {
                old = 1;
dev_debug_printf("%s:%d\r\n", GPIO_PIN_PTA6, old);  
            }
        }
#endif
        
    }
    #endif
}

#define TESTPORT0       0
#define TESTPORT1       5
#define PEINTBUF_MAX    1024
#define PRINTPORT    7
void uart_test(void)
{
    u8 tmp[256];
    s32 i;
    u8 printbuf[PEINTBUF_MAX];
    u8 printcnt;
    u32 printrevtimeid;
    
    dev_com_open(TESTPORT0, 115200, 8, 'n', 0, 0);
    dev_com_open(TESTPORT1, 115200, 8, 'n', 0, 0);
#if 0
    
    while(1)
    {
        for(i=0; i<4096; i++)
        {
            tmp[i] = i;
        }
        //dev_debug_printf("\r\n%s(%d):\r\n", __FUNCTION__, __LINE__);
        //dev_debug_printformat(NULL, tmp, 4096);
        dev_com_write(DEBUG_PORT_NO, tmp, 4096);
    } 
#endif
    printcnt = 0;
    printrevtimeid = dev_user_gettimeID();
    while(1)
    {
        i = dev_com_read(TESTPORT0, tmp, 256);
        dev_com_write(TESTPORT1, tmp, i);
        //i = dev_usbd_print_write(tmp, i); 
//        dev_com_write(TESTPORT0, tmp, i);
//if(i!=0)
//{
//    dev_debug_printf("%s(%d):readlen=%d\r\n", __FUNCTION__, __LINE__, i);
//    dev_debug_printformat(NULL, tmp, i);
//}

        i = dev_com_read(TESTPORT1, tmp, 256);
        dev_com_write(TESTPORT0, tmp, i);

        i = dev_com_read(PRINTPORT, &printbuf[printcnt], PEINTBUF_MAX-printcnt);
        if(i!=0)
        {
            printcnt += i;
            printrevtimeid = dev_user_gettimeID();
        }
        if((printcnt>=PEINTBUF_MAX)
         ||((printcnt!=0)&&(1 == dev_user_querrytimer(printrevtimeid, 100))))
        {
            //解析数据
            if((3 == printcnt) 
             &&(0 == memcmp(printbuf, "\x1d\x72\x01",3)))
            {   //连接测试
                tmp[0] = 0;
                dev_com_write(PRINTPORT, tmp, 1);
            }
            else if((3 == printcnt) 
             &&(0 == memcmp(printbuf, "\x1d\x72\x02",3)))
            {   //打开钱箱
                tmp[0] = 0;
                dev_com_write(PRINTPORT, tmp, 1);
             }
            else if((2 == printcnt) 
             &&(0 == memcmp(printbuf, "\x12\x57",2)))
            {
                //配置信息打印
                
            }
            else if((0 == memcmp(printbuf, "\x1B\x53",2)))
            {
                //打印文本
                
            }
            else if((3 == printcnt) 
             &&(0 == memcmp(printbuf, "\x10\x04\x04",3)))
            {   //打印机状态
                tmp[0] = 0x72;
                dev_com_write(PRINTPORT, tmp, 1);
             }
            else if((3 == printcnt) 
             &&(0 == memcmp(printbuf, "\x10\x04\x01",3)))
            {   //
                tmp[0] = 0x1E;
                dev_com_write(PRINTPORT, tmp, 1);
             }
            else if((3 == printcnt) 
             &&(0 == memcmp(printbuf, "\x10\x04\x02",3)))
            {   //
                tmp[0] = 0x32;
                dev_com_write(PRINTPORT, tmp, 1);
             }
            else if((3 == printcnt) 
             &&(0 == memcmp(printbuf, "\x10\x04\x03",3)))
            {   //
                tmp[0] = 0x12;
                dev_com_write(PRINTPORT, tmp, 1);
             }
            else if((0 == memcmp(printbuf, "\x1c\x71\x01",3)))
            {   //下载图片
                tmp[0] = 0x31;
                dev_com_write(PRINTPORT, tmp, 1);
             }
            else if(printbuf[0] == 0x33)
            {
                //测试dev_usbd_print_write函数
                dev_com_write(PRINTPORT, printbuf, printcnt);
            }
            else
            {
                
            }
            dev_com_write(TESTPORT1, printbuf, printcnt); 
            //if(i!=0)
            {
                dev_debug_printf("%s(%d):readlen=%d\r\n", __FUNCTION__, __LINE__, printcnt);
                dev_debug_printformat(NULL, printbuf, printcnt);
            }
            printcnt = 0;
         }
            
    }
}

void dev_uart_test(void)
{
    u8 tmp[128];
    s32 i;

#if 1
    dev_com_open(PORT_UART3_NO, 115200, 8, 'n', 0, 0);
	dev_debug_printf("uart test:\r\n");

    while(1)
    {
        i = dev_com_read(PORT_UART3_NO, tmp, 100);
        if(i>0)
        {
            dev_com_write(PORT_CDCD_NO, tmp, i);
//dev_user_delay_ms(1000);
        }
        i = dev_com_read(PORT_CDCD_NO, tmp, 100);
        if(tmp[0] == '1')
        {
            
                dev_gpio_direction_output(GPIO_PIN_PTA1, 0);
                dev_user_delay_ms(1000);
                dev_gpio_direction_output(GPIO_PIN_PTA1, 1);
        }
        if(i>0)
        {
            dev_com_write(PORT_UART3_NO, tmp, i);
//dev_user_delay_ms(1000);
        }
    }
#else
//    dev_com_open(DEBUG_PORT_NO, 115200, 8, 'n', 0, 0);
//    dev_com_setbaud(DEBUG_PORT_NO, 115200, 8, 'n', 0, 0);		//3000000
//dev_debug_printf("%s(%d)\r\n", __FUNCTION__, __LINE__);
//dev_user_delay_ms(10);
//    memset(tmp, 0x55, 10);

	dev_debug_printf("uart test:\r\n");

    while(1)
    {
        i = dev_com_read(DEBUG_PORT_NO, tmp, 100);
        if(i>0)
        {
            dev_com_write(DEBUG_PORT_NO, tmp, i);
//dev_user_delay_ms(1000);
        }
    }
#endif
}

#if 0
u8  pciarith_test(void)
{
	u8 flag = 0;
	u8 key[8],plandata[8],encryptdata[8],decryptdata[8];

	memcpy(key,"\x89\x60\x34\x67\x12\x67\x80\x77",8);
	memcpy(plandata,"\x67\x50\x34\x67\x12\x67\x80\x77",8);
	memcpy(encryptdata,plandata,8);
	devarith_desencrypt(ENCRYPT,encryptdata,key);
    memcpy(decryptdata,encryptdata,8);
	devarith_desencrypt(DECRYPT,decryptdata,key);

	if(memcmp(decryptdata,plandata,8) != 0)
	{
		flag = 1;
	}
    
    
	//RSA
	
	
	
	
	
	
	
	
	
	return flag;
		
}

void dev_mag_test(void)
{
    u8 tk1[128];
    u8 tk2[128];
    u8 tk3[128];
    u8 tkstatus[3];
    s32 ret=0;
    u32 timerid;
    u32 key;
    s32 flg = 1;
    u32 success_times=0;
    u32 fail_times=0;
    u32 tk1_ok=0;
    u32 tk2_ok=0;
    u32 tk3_ok=0;
    s32 flg1=0;
    
    ret = dev_mag_open();
    if(ret < 0)
    {
        dev_debug_printf("open ERR!\r\n");
        return;
    }
dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
                
    timerid = dev_user_gettimeID();
    while(flg)
    {
        flg1 = 0;
        ret = dev_mag_read(tk1, tk2, tk3, tkstatus);
        if(ret==0)
        {
            if(strlen(tk1))
            {
                flg1 = 1;
                tk1_ok++;
                dev_debug_printf("%s(%d):tk1(%s)\r\n",__FUNCTION__, __LINE__, tk1);
            }
            if(strlen(tk2))
            {
                flg1 = 1;
                tk2_ok++;
                dev_debug_printf("%s(%d):tk2(%s)\r\n",__FUNCTION__, __LINE__, tk2);
            }
            if(strlen(tk3))
            {
                flg1 = 1;
                tk3_ok++;
                dev_debug_printf("%s(%d):tk3(%s)\r\n",__FUNCTION__, __LINE__, tk3);
            }
            if(flg1 == 1)
            {
                success_times++;
                dev_debug_printf("ok:%d,fail:%d,tk1=%d,tk2=%d,tk3=%d\r\n\r\n",success_times, fail_times,
                              tk1_ok, tk2_ok, tk3_ok);
            }
        }
        else
        {
		    dev_debug_printf("swipecard error!\r\n");
            fail_times++;
            dev_debug_printf("ok:%d,fail:%d,tk1=%d,tk2=%d,tk3=%d\r\n\r\n",success_times, fail_times,
                              tk1_ok, tk2_ok, tk3_ok);
        }
        if(keypad_interface_read_beep(&key)>0)
        {
            switch(key)
            {
            case ESC:
                flg = 0;
                break;
            }
        }
    }
    dev_mag_close();
}

//u8 const icc_test_PSETabl[]={0x00,0xA4,0x04,0x00,0x0E,0x32,0x50,0x41,0x59,0x2E,0x53,0x59,0x53,
//                    0x2E,0x44,0x44,0x46,0x30,0x31,0x00};
u8 const icc_test_PSETabl[]={0x00,0xA4,0x04,0x00,0x0E,0x31,0x50,0x41,0x59,0x2E,0x53,0x59,0x53,
                    0x2E,0x44,0x44,0x46,0x30,0x31,0x00};
void dev_icc_test(s32 nslot)
{
    u8 flg;
    u32 key;
    s32 ret;
    u8 rbuf[128];
    u32 rlen;

    dev_icc_open(nslot);
    flg = 1;
    while(1)
    {
        if(flg == 1)
        {
            flg = 0;
            dev_debug_printf("================ICC test===============\n");
			dev_debug_printf("1.Detect ICC\n");
			dev_debug_printf("2.Reset\n");
			dev_debug_printf("3.Apdu\n");
			dev_debug_printf("4.Poweroff\n");
            
        }
        if(keypad_interface_read_beep(&key)>0)
        {
            switch(key)
            {
            case DIGITAL1:
                ret = dev_icc_getstatus(nslot);
dev_debug_printf("ret=%d\r\n", ret);  
                break;
                
            case DIGITAL2:
                ret = dev_icc_reset(nslot, rbuf);
dev_debug_printf("ret=%d\r\n", ret);  
if(ret>0)
{
    dev_debug_printformat(NULL, rbuf, ret);
}
                break;
                
            case DIGITAL3:
                ret = dev_icc_exchange_apdu(nslot, icc_test_PSETabl, 20, rbuf, &rlen, 64);
dev_debug_printf("ret=%d\r\n", ret);  
if(ret==0)
{
    dev_debug_printformat(NULL, rbuf, rlen);
}
                
                break;
                
            case DIGITAL4:
                ret = dev_icc_poweroff(nslot);
dev_debug_printf("ret=%d\r\n", ret);  
                break;
                
            case ESC:
                flg = 0;
                return ;
                break;
            }
        }
    }
}
#endif

void dev_led_test(void)
{
    u8 flg;
    u32 key;
    s32 ret;
    u8 rbuf[128];
    u32 rlen;
    strLedGleamPara l_ledgleamparam;

//    dev_icc_close(ICC_SLOT_ICCARD);
    //dev_pcd_open();
    dev_led_open();
    flg = 1;
    while(1)
    {
        if(flg == 1)
        {
            flg = 0;
            dev_debug_printf("================LED test===============\n");
			dev_debug_printf("1.Light Off\n");
			dev_debug_printf("2.Light On\n");
			dev_debug_printf("3.Light flash\n");
            
        }
        if(keypad_interface_read_beep(&key)>0)
        {
            switch(key)
            {
            case DIGITAL1:
dev_debug_printf("%s(%d):\r\n",__FUNCTION__, __LINE__);
                dev_led_sta_set(LED_STATUS_B, 0);
                dev_led_sta_set(LED_STATUS_R, 0);
                dev_led_sta_set(LED_SIGNAL_Y, 0);
                dev_led_sta_set(LED_SIGNAL_B, 0);
                break;
                
            case DIGITAL2:
dev_debug_printf("%s(%d):\r\n",__FUNCTION__, __LINE__);
                dev_led_sta_set(LED_STATUS_B, 1);
                dev_led_sta_set(LED_STATUS_R, 1);
                dev_led_sta_set(LED_SIGNAL_Y, 1);
                dev_led_sta_set(LED_SIGNAL_B, 1);
                break;
                
            case DIGITAL3:
dev_debug_printf("%s(%d):\r\n",__FUNCTION__, __LINE__);
                l_ledgleamparam.m_led = LED_STATUS_B;
                l_ledgleamparam.m_ontime = 300;
                l_ledgleamparam.m_offtime = 400;
                l_ledgleamparam.m_duration = 60000;
                dev_led_ioctl(DDI_LED_CTL_GLEAM, (u32)&l_ledgleamparam, 0);
                break;
                
            case DIGITAL4: 
dev_debug_printf("%s(%d):\r\n",__FUNCTION__, __LINE__);
                l_ledgleamparam.m_led = LED_SIGNAL_Y;
                l_ledgleamparam.m_ontime = 400;
                l_ledgleamparam.m_offtime = 300;
                l_ledgleamparam.m_duration = 60000;
                dev_led_ioctl(DDI_LED_CTL_GLEAM, (u32)&l_ledgleamparam, 0);
                break;
            case DIGITAL5: 
dev_debug_printf("%s(%d):\r\n",__FUNCTION__, __LINE__);
                l_ledgleamparam.m_led = LED_SIGNAL_B;
                l_ledgleamparam.m_ontime = 500;
                l_ledgleamparam.m_offtime = 200;
                l_ledgleamparam.m_duration = 60000;
                dev_led_ioctl(DDI_LED_CTL_GLEAM, (u32)&l_ledgleamparam, 0);
                break;
            case DIGITAL6: 
dev_debug_printf("%s(%d):\r\n",__FUNCTION__, __LINE__);
                l_ledgleamparam.m_led = LED_STATUS_R;
                l_ledgleamparam.m_ontime = 500;
                l_ledgleamparam.m_offtime = 400;
                l_ledgleamparam.m_duration = 60000;
                dev_led_ioctl(DDI_LED_CTL_GLEAM, (u32)&l_ledgleamparam, 0);
                break;
            case DIGITAL7:
dev_debug_printf("%s(%d):\r\n",__FUNCTION__, __LINE__);
                //dev_pcd_close();
                break;
            case DIGITAL0:
dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);                
                dev_misc_updateflg_set(1);
                break;
            case ESC:
                flg = 0;
                return;
            }
        }
    }
    
}

void dev_misc_test(void)
{
    
    u8 flg;
    u32 key;
    s32 ret;
    u8 rbuf[128];
    u32 rlen;
//    strLedGleamPara l_ledgleamparam;

//    dev_icc_close(ICC_SLOT_ICCARD);    dev_lcd_open();
    flg = 1;
//    dev_lcd_open();
dev_debug_printf("%s(%d)\r\n", __FUNCTION__, __LINE__);  
    while(1)
    {
        if(flg == 1)
        {
            flg = 0;
            lcd_interface_clear_ram();
            lcd_interface_fill_rowram(0, 0, "MISC TEST", NOFDISP|CDISP);
            lcd_interface_fill_rowram(1, 0, "1.updateflg read", FDISP);
            lcd_interface_fill_rowram(2, 0, "2.updateflg set", FDISP);
            lcd_interface_fill_rowram(3, 0, "3.updateflg clr", FDISP);
            lcd_interface_brush_screen();
            keypad_interface_clear();
        }
        if(keypad_interface_read_beep(&key)>0)
        {
dev_debug_printf("key=%d\r\n", key);            
            switch(key)
            {
            case DIGITAL1:            
                ret = dev_misc_updateflg_check();
dev_debug_printf("%s(%d):ret=%d\r\n",__FUNCTION__, __LINE__, ret); 
                break;
                
            case DIGITAL2:
dev_debug_printf("%s(%d):\r\n",__FUNCTION__, __LINE__);               
                dev_misc_updateflg_set(1);
                break;
                
            case DIGITAL3:
dev_debug_printf("%s(%d):\r\n",__FUNCTION__, __LINE__);               
                dev_misc_updateflg_set(0);
                break;
                
            case ESC:
                flg = 1;
                return;
                break;
            }
        }
    }
    
}
#if 0
//add by cgj test key 20190325
#define FILE_TEST_MAX 1024
void dev_lfs_test(void)
{
    
    u8 flg;
	u32 key;
    u8 rbuf[FILE_TEST_MAX];
	u8 writedata[FILE_TEST_MAX];
    u32 rlen;
	DQFILE fp;
	int result;
	int ret;
	s32 i;

	ret = 0;
    flg = 1;
	fp = 0;

dev_debug_printf("%s(%d)\r\n", __FUNCTION__, __LINE__);  
    while(1)
    {
        if(flg == 1)
        {
            flg = 0;
            lcd_interface_clear_ram();
            lcd_interface_fill_rowram(0, 0, "LFS TEST", NOFDISP|CDISP);
            lcd_interface_fill_rowram(1, 0, "1.vfs open", FDISP);
            lcd_interface_fill_rowram(2, 0, "2.vfs write", FDISP);
            lcd_interface_fill_rowram(3, 0, "3.vfs read", FDISP);
			lcd_interface_fill_rowram(4, 0, "4.vfs close", FDISP);
            lcd_interface_brush_screen();
            keypad_interface_clear();
        }
        if(keypad_interface_read_beep(&key)>0)
        {
dev_debug_printf("key=%d\r\n", key);            
            switch(key)
            {
            case DIGITAL1:            
                fp = sys_vfs_open("/mtd0/testfile","w+",&result);
dev_debug_printf("sys_vfs_open:%d\r\n",fp); 
                break;
                
            case DIGITAL2:
				memset(writedata, 0, FILE_TEST_MAX);
                for(i=0; i<FILE_TEST_MAX; i++)
                {
                    writedata[i] = i;
                }
                ret = sys_vfs_write("/mtd0/testfile",writedata,FILE_TEST_MAX,0,fp);
				dev_debug_printf("vfs write:%d\r\n",ret);
                break;
                
            case DIGITAL3:
                memset(rbuf, 0, FILE_TEST_MAX);
                ret = sys_vfs_read("/mtd0/testfile",rbuf,rlen,0,fp);

				if(ret >= 0){

					dev_debug_printformat(NULL,rbuf,ret);
				}
				

				dev_debug_printf("vfs read:%d\r\n",ret);
                break;
            case DIGITAL4:
dev_debug_printf("sys_vfs_open close\r\n");               
                sys_vfs_close("/mtd0/testfile",fp);
                break;                
            case ESC:
                flg = 1;
                return;
                break;
            }
        }
    }
    
}
//add by cgj test key 20190326
void dev_key_test(void){
    
    u8 flg;
	u32 key;


    flg = 1;

	dev_debug_printf("dev_key_test\r\n"); 

    while(1)
    {
        if(flg == 1)
        {
            flg = 0;
            lcd_interface_clear_ram();
            lcd_interface_fill_rowram(0, 0, "KEYPAD TEST", NOFDISP|CDISP);
            lcd_interface_brush_screen();
            keypad_interface_clear();
        }

        if(keypad_interface_read_beep(&key)>0)
        {
dev_debug_printf("key=%d\r\n", key);

            switch(key)
            {
           	case POWER:
dev_debug_printf("POWER KEY\r\n"); 
				break;
			case F1:			  
dev_debug_printf("KEY F1\r\n"); 
				break;
				
			case F2:
dev_debug_printf("KEY F2\r\n");
				break;
				
			case F3:
dev_debug_printf("KEY F3\r\n");
				break;

            case DIGITAL1:            
               
dev_debug_printf("KEY 1\r\n"); 
                break;
                
            case DIGITAL2:
dev_debug_printf("KEY 2\r\n");
                break;
                
            case DIGITAL3:
dev_debug_printf("KEY 3\r\n");
                break;
            case DIGITAL4:
dev_debug_printf("KEY 4\r\n");
			break;
			case DIGITAL5:
dev_debug_printf("KEY 5\r\n");
			break;
			case DIGITAL6:
dev_debug_printf("KEY 6\r\n");
			break;
			case DIGITAL7:
dev_debug_printf("KEY 7\r\n");
			break;
			case DIGITAL8:
dev_debug_printf("KEY 8\r\n");
			break;    
			case DIGITAL9:
dev_debug_printf("KEY 9\r\n");
			break;
			case DIGITAL0:
dev_debug_printf("KEY 0\r\n");
			break;

            case ESC:
dev_debug_printf("KEY ESC\r\n");

                flg = 0;
                break;
			case CLEAR:
dev_debug_printf("KEY CLEAR\r\n");

				break;
			case ENTER:
dev_debug_printf("KEY ENTER\r\n");

				break;
			case DOWN_OR_11:
dev_debug_printf("KEY NUMBER\r\n");

				break;
			case FUNCTION:
dev_debug_printf("KEY FUNCTION\r\n");
				break;

            }
        }
    }
    
}


void dev_audio_test(void)
{
    u8 flg;
    u32 key;
    s32 ret;
    u8 rbuf[128];
    u32 rlen;

//    dev_icc_close(ICC_SLOT_ICCARD);
    dev_audio_open();
    flg = 1;
    while(1)
    {
        if(flg == 1)
        {
            flg = 0;
            printf("================Audio test===============\n");
			printf("1.Beep\n");
            
        }
        if(keypad_interface_read_beep(&key)>0)
        {
            switch(key)
            {
            case DIGITAL1:
dev_debug_printf("%s(%d):\r\n",__FUNCTION__, __LINE__);
                dev_audio_ioctl(DDI_AUDIO_CTL_BUZZER, 0, 200);
                break;
                
            case DIGITAL2:
dev_debug_printf("%s(%d):\r\n",__FUNCTION__, __LINE__);
                break;
                
            case DIGITAL3:
dev_debug_printf("%s(%d):\r\n",__FUNCTION__, __LINE__);
                break;
                
            case ESC:
                flg = 0;
                break;
            }
        }
        dev_audio_ioctl(DDI_AUDIO_CTL_BUZZER, 0, 200);
        dev_user_delay_ms(300);
    }
    
}

void dev_lcd_test(void)
{
    
    u8 flg;
    u32 key;
    s32 ret;
    u8 rbuf[128];
    u32 rlen;
    u8 regval = 0x28;
//    strLedGleamPara l_ledgleamparam;

//    dev_icc_close(ICC_SLOT_ICCARD);    dev_lcd_open();
    flg = 1;
    dev_lcd_open();
dev_debug_printf("%s(%d)\r\n", __FUNCTION__, __LINE__);  
    while(1)
    {
        if(flg == 1)
        {
            flg = 0;
            lcd_interface_clear_ram();
            lcd_interface_fill_rowram(0, 0, "LCD TEST", NOFDISP|CDISP);
            lcd_interface_fill_rowram(1, 0, "1.LCD clear", FDISP);
            lcd_interface_fill_rowram(2, 0, "2.LCD Black", FDISP);
            lcd_interface_fill_rowram(3, 0, "3.BL ON ", FDISP);
            lcd_interface_fill_rowram(3,64, "4.BL OFF", FDISP);
            sprintf(rbuf, "(0x%02X)", regval);
//            drv_lcd_test_set(regval);
            lcd_interface_fill_rowram(4,0, rbuf, CDISP);
            lcd_interface_brush_screen();
            keypad_interface_clear();
        }
        if(keypad_interface_read_beep(&key)>0)
        {
dev_debug_printf("key=%d\r\n", key);            
            switch(key)
            {
            case DIGITAL1:
dev_debug_printf("%s(%d):\r\n",__FUNCTION__, __LINE__);
                lcd_interface_fill_all(1);
                break;
                
            case DIGITAL2:
dev_debug_printf("%s(%d):\r\n",__FUNCTION__, __LINE__);
                lcd_interface_fill_all(0);
                break;
                
            case DIGITAL3:
dev_debug_printf("%s(%d):\r\n",__FUNCTION__, __LINE__);
                //drv_lcd_bl_ctl(1);
                dev_lcd_ioctl(DDI_LCD_CTL_BKLIGHT_CTRL, 1, 0);
                flg = 1;
                break;
                
            case DIGITAL4: 
dev_debug_printf("%s(%d):\r\n",__FUNCTION__, __LINE__);
                //drv_lcd_bl_ctl(0);
                dev_lcd_ioctl(DDI_LCD_CTL_BKLIGHT_CTRL, 0, 0);
                flg = 1;
                break;
#if 1                
            case DIGITAL5: 
                if(regval>2)
                {
                    regval-=2;
                }
                else
                {
                    regval = 0;
                }
                flg = 1;
dev_debug_printf("%s(%d):reg=%02X\r\n",__FUNCTION__, __LINE__, regval);
                break;
            case DIGITAL6: 
                regval+=2;
                if(regval>0x3F)
                {
                    regval = 0x3f;
                }
                flg = 1;
dev_debug_printf("%s(%d):reg=%2X\r\n",__FUNCTION__, __LINE__, regval);
                break;
#endif                
            case ESC:
                flg = 1;
                return;
                break;
            }
        }
    }
    
    
        dev_lcd_full(1);

}
#endif
extern void esc_set_data(uint8_t *buf, int num);
extern void F25L_Read(uint32_t addr, uint8_t  *buf, uint16_t bytes);
extern int esc_fifo_isEmpty(void);

void dev_key_test(void)
{
    
    u8 flg;
	u32 key;
    uint8_t  atr[65];

    u8 key_code;
    u8 key_event;
    s32 ret;
    //unsigned char buf[] = "\n啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui\n\n啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui\n\n啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui\n\n啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui\n";
    unsigned char buf[] = "国国国国国国国国\r\n";

    flg = 1;

	dev_debug_printf("dev_key_test\r\n"); 

    //SYSCTRL->PHER_CTRL &= ~BIT(20);//ic 上电 
    //SYSCTRL->PHER_CTRL |= BIT(20);
    #if 0
    if(iso7816_init(0, VCC_3000mV | SPD_1X, atr) == 0)
    {
        dev_debug_printf("ic power on success\r\n"); 
    }
    else
    {
        dev_debug_printf("ic power on fail\r\n"); 
    }
    #endif
    while(1)
    {
        if(flg == 1)
        {
            flg = 0;
            lcd_interface_clear_ram();
            lcd_interface_fill_rowram(0, 0, "KEYPAD TEST", NOFDISP|CDISP);
            lcd_interface_brush_screen();
            keypad_interface_clear();
        }

        //if(keypad_interface_read_beep(&key)>0)
        if(dev_keypad_read(&key) > 0)
        {

            key_code = key&0xff;
            key_event = (key&0xff00)>>8;

            #ifdef TRENDIT_CORE
            if((key_code == KEY_CFG) && (key_event == KEY_EVENT_DOWN))
            {
                    int num = 60;
                    ddi_thmprn_esc_init();
                    dev_debug_printf("prt 200 line000\r\n");
                    //PrtFeedStart(200);
                    ddi_thmprn_open();
                    //ddi_thmprn_feed_paper(200);
                    ddi_thmprn_esc_p(buf, sizeof(buf));
                    while(1)
                    {
                        if(ddi_thmprn_esc_loop() != DDI_OK)
                        {
                            break;
                        }
                    
                        ddi_watchdog_feed();
                    }
                    
                    while(1)
                    {
                        ret = ddi_thmprn_get_status();
                        TRACE_DBG("printer status:%d", ret);
                        dev_debug_printf("printer status:%d", ret);
                        if((ret == DDI_OK) || (ret == DDI_EACCES))
                        {   
                            if(ret == DDI_EACCES) //缺纸
                            {
                                TRACE_DBG("printer no paper");
                            }
                            ddi_thmprn_close();
                            break;
                        }
                    
                        ddi_watchdog_feed();
                        ddi_misc_msleep(200);
                    }  

            }
            else if(((key_code == KEY_PAPER) && (key_event == KEY_EVENT_DOWN)))
            {
                //TPHTemperature();
                //TPSelfTest_1();
                TPHTemperatureADTest();
            }
            #else
            switch(key_code)
            {
                case KEY_MINUS:
                    dev_debug_printf("KEY -\r\n");
                    break;
                case KEY_PLUS:
                    dev_debug_printf("KEY +\r\n");
                    break;
                case KEY_CFG:
                    dev_debug_printf("KEY CFG\r\n");
                    break;
                case KEY_PAPER:

                    dev_debug_printf("KEY PAPER\r\n");
                    break;
                case KEY_TBD:

                    dev_debug_printf("KEY TBD\r\n");
                    break;
                case POWER:

                    dev_debug_printf("KEY POWER\r\n");
                    break;
                case KEY_CFG_PLUS:
                    dev_debug_printf("KEY_CFG KEY_+\r\n");
                    break;
                case KEY_CFG_MINUS:
                    dev_debug_printf("KEY_CFG KEY_-\r\n");
                    break;
                case KEY_CFG_PAPER:

                    dev_debug_printf("KEY_CFG KEY_PAPER\r\n");
                    break;
                case KEY_PAPER_PLUS:

                    dev_debug_printf("KEY_PAPER KEY_+\r\n");
                    break;
                case KEY_PAPER_MINUS:

                    dev_debug_printf("KEY_PAPER KEY_-\r\n");
                    break;
                case KEY_PLUS_MINUS:

                    dev_debug_printf("KEY_+ KEY_-\r\n");
                    break;

                case ESC:
                    return ;

            }
            switch(key_event)
            {
                case KEY_EVENT_DOWN:
                    #ifdef TRENDIT_CORE
                    int num = 60;
                    ddi_thmprn_esc_init();
                    dev_debug_printf("prt 200 line\r\n");
                    //PrtFeedStart(200);
                    ddi_thmprn_open();
                    //ddi_thmprn_feed_paper(200);
                    esc_set_data(buf, sizeof(buf));
                    while(num--)
                    {
                        if(esc_fifo_isEmpty())
                        {
                            break;
                        }
                        
                        esc_p();
                        dev_debug_printf("printer44\n");
                    }
                    #endif
                    dev_debug_printf("KEY_DOWN\r\n");
                    break;
                case KEY_EVENT_UP:

                    dev_debug_printf("KEY_UP\r\n");
                    break;
                case KEY_LONG_PRESS:

                    dev_debug_printf("KEY_LONG\r\n");
                    break;
            }
            #endif
        }
    }
    
}
#define SPIFLASH_TEST_MAX    1024
s32 drv_spiflash_check_busy(void);
void dev_flash_test(u32 addrsta)
{
    
    u8 flg;
    u32 key;
    s32 ret;
    u8 tmp[SPIFLASH_TEST_MAX];
    u32 rlen;
    s32 i;

    flg = 1;
    while(1)
    {
        if(flg == 1)
        {
            flg = 0;
            lcd_interface_clear_ram();
            lcd_interface_fill_rowram(0, 0, "FLASH TEST", NOFDISP|CDISP);
            lcd_interface_fill_rowram(1, 0, "1.Read flash", FDISP);
            lcd_interface_fill_rowram(2, 0, "2.write1", FDISP);
            lcd_interface_fill_rowram(3, 0, "3.write2", FDISP);
            lcd_interface_fill_rowram(4, 0, "4.write3", FDISP);
            lcd_interface_brush_screen();
            keypad_interface_clear();
        }
        if(keypad_interface_read_beep(&key)>0)
        {
            switch(key)
            {
            case DIGITAL1:
                memset(tmp, 0, SPIFLASH_TEST_MAX);
                ret = dev_flash_read(addrsta, tmp, SPIFLASH_TEST_MAX);
dev_debug_printf("%s(%d):ret=%d\r\n",__FUNCTION__, __LINE__, ret); 
                if(ret > 0)
                {
                    dev_debug_printformat(NULL, tmp, ret);
                }
                break;
                
            case DIGITAL2:
                memset(tmp, 0, SPIFLASH_TEST_MAX);
                for(i=0; i<SPIFLASH_TEST_MAX; i++)
                {
                    tmp[i] = i;
                }
                //ret = dev_flash_write(addrsta, tmp, SPIFLASH_TEST_MAX);
                drv_spiflash_check_ID();
                //test_spiflash();
//dev_debug_printf("%s(%d):ret = %d\r\n",__FUNCTION__, __LINE__, ret);
            
                break;
                
            case DIGITAL3:
                memset(tmp, 0, SPIFLASH_TEST_MAX);
                for(i=0; i<SPIFLASH_TEST_MAX; i++)
                {
                    tmp[i] = (i&0x0f)+0x30;
                }
                ret = dev_flash_write(addrsta, tmp, SPIFLASH_TEST_MAX);
dev_debug_printf("%s(%d):ret = %d\r\n",__FUNCTION__, __LINE__, ret);
                break;
                
            case DIGITAL4: 
                memset(tmp, 0, SPIFLASH_TEST_MAX);
                for(i=0; i<SPIFLASH_TEST_MAX; i++)
                {
                    tmp[i] = (i&0x0f)+0x40;
                }
                ret = dev_flash_write(addrsta, tmp, SPIFLASH_TEST_MAX);
dev_debug_printf("%s(%d):ret = %d\r\n",__FUNCTION__, __LINE__, ret);
                break;
            case DIGITAL5: 
                ret = 0;//drv_spiflash_check_busy();
dev_debug_printf("%s(%d):ret=%d\r\n",__FUNCTION__, __LINE__, ret);
                break;
            case DIGITAL6: 
//dev_debug_printf("%s(%d):\r\n",__FUNCTION__, __LINE__);
                //dev_com_test();
 //               ret = ddi_misc_bat_status();
dev_debug_printf("%s(%d):ret=%08X\r\n",__FUNCTION__, __LINE__, ret);
                break;
            case ESC:
                flg = 1;
                return;
                break;
            }
        }
    }
}
#if 0
extern void drv_bt_init(void);
//extern void drv_bt_hdrst(void);
extern void drv_bt_wakeup_ctl(u8 flg);
extern s32 drv_bt_boot_swrst(void);
extern s32 drv_bt_boot_patch(void);
#if (BT_EXIST==1) 
#define BT_FILENAME    "/mtd2/apupdate"
#define BT_FILELEN     0x33286
s32 dev_bt_update(void) 
{
    PARAM_BTFIRMWAREUPDATE btfireware;
    u32 len;

    strcpy(btfireware.filename, BT_FILENAME);
    btfireware.offset = 0;
    len = BT_FILELEN;
    return dev_bt_ioctl(DDI_BT_CTL_FIRMWAREUPDATE, (u32)&btfireware, len);
}
#endif

void dev_bt_test(void)
{

#if (BT_EXIST==1)     
    u8 flg=1;
    u32 key;
    u8 tmp[256];
    s32 i;
    s32 ret;

    while(1)
    {
        if(flg==1)
        {
            lcd_interface_clear_ram();
            lcd_interface_fill_rowram(0, 0, "BT TEST", NOFDISP|CDISP);
            lcd_interface_fill_rowram(1, 0, "1.BT open", FDISP);
            lcd_interface_fill_rowram(2, 0, "2.BT close", FDISP);
            lcd_interface_fill_rowram(3, 0, "3.BT getstatus", FDISP);
            lcd_interface_fill_rowram(4, 0, "4.BT write", FDISP);
            lcd_interface_brush_screen();
            keypad_interface_clear();
			flg = 0;
        }
        if(keypad_interface_read_beep(&key)>0)
        {
//printf("key=%02X\r\n", key);  
            switch(key)
            {
            case DIGITAL1:
                ret = dev_bt_open();
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);
                break;
            case DIGITAL2:
//                drv_bt_wakeup_ctl(0);
                dev_bt_close();
                break;
            case DIGITAL3:
//                drv_bt_wakeup_ctl(1);
                ret = dev_bt_get_status();
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);
                break;
            case DIGITAL4:
              #if 0  
                ret = drv_bt_boot_patch();
              #endif
                for(i=0; i<256; i++)
                {
                    tmp[i] = (i&0x0f)+0x30;
                }
                ret = dev_bt_write(tmp, 100);
dev_debug_printf("%s(%d): ret=%d\r\n", __FUNCTION__, __LINE__, ret);                
                break;
            case DIGITAL5:
//                ret = drv_bt_boot_swrst();
//                ret = drv_bt_bk3231s_poweron();
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);                
                break;
            case DIGITAL6:
//                ret = drv_bt_bk3231s_poweroff();
//                ret = drv_bt_boot_setbaud(115200);
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);                
                break;
            case DIGITAL7:
              #if 0  
                dev_com_open(1, 115200, 8, 'n', 0, 0);
                dev_com_write(1, "1234567890\r\n", 12);  
                flg = 1;
              #endif
                ret = dev_misc_updateflg_set(1);
                dev_debug_printf("%s(%d):set updateflg  ret=%d\r\n", __FUNCTION__, __LINE__, ret);
                break;
            case DIGITAL8: 
                ret = dev_bt_ioctl(DDI_BT_CTL_MVER, 0, (u32)tmp);
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);  
                if(ret == 0)
                {
dev_debug_printf("%s(%d):ver=%s\r\n", __FUNCTION__, __LINE__, tmp);  
                }
                
//                drv_bt_bk3231s_hdrst();
 //               dev_com_test();
                flg = 1;
                break;
            case DIGITAL9:
              #if 0  
                lcd_interface_clear_ram();
                lcd_interface_fill_rowram(0, 0, "BT TEST", NOFDISP|CDISP);
                lcd_interface_fill_rowram(2, 0, "BT Update...", CDISP);
                lcd_interface_brush_screen();
                ret = dev_bt_update();
                lcd_interface_clear_rowram(2);
                if(ret < 0)
                {
                    lcd_interface_fill_rowram(2, 0, "Update Err!", CDISP);
                }
                else
                {
                    lcd_interface_fill_rowram(2, 0, "Update Ok!", CDISP);
                }
                lcd_interface_brush_screen();
                dev_user_delay_ms(2000);
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);   
                flg = 1;
              #endif  
                break;
            case ESC:
                return;
            case POWER:
                break;
            }
        }
      #if 0  
        i = dev_com_read(DEBUG_PORT_NO, tmp, 256);
        if(i > 0)
        {
            dev_com_write(0, tmp, i);
        }
        i = dev_com_read(0, tmp, 256);
        if(i > 0)
        {
            dev_com_write(DEBUG_PORT_NO, tmp, i);
        }
      #endif
        i = dev_bt_read(tmp, 256);
        if(i>0)
        {
            dev_debug_printf("READ:i=%d\r\n", i);
            dev_debug_printformat(NULL, tmp, i);
        }
        
    }
#endif    
}
s32 dev_adc_test(void)
{
    
    u8 flg=1;
    u32 key;
    u8 tmp[256];
    s32 i;
    s32 ret;

    while(1)
    {
        if(flg==1)
        {
            dev_debug_printf("================ADC TEST===============\n");
            dev_debug_printf("1.ADC open\n");
			dev_debug_printf("2.ADC close\n");
            dev_debug_printf("3.GET ADCVal\n");
			flg = 0;
        }
        if(keypad_interface_read_beep(&key)>0)
        {
//printf("key=%02X\r\n", key);  
            switch(key)
            {
            case DIGITAL1:
//                drv_bt_hdrst();
                dev_adc_open(DEV_ADC_HWVER);
            #if(MACHINETYPE==MACHINE_S1)
                dev_adc_open(DEV_ADC_PTR_TM);
            #else
                dev_adc_open(DEV_ADC_VBAT);
            #endif
 dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
               break;
            case DIGITAL2:
//                drv_bt_wakeup_ctl(0);
                dev_adc_close(DEV_ADC_HWVER);
 dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
                break;
            case DIGITAL3:
//                drv_bt_wakeup_ctl(1);
                //ret = dev_adc_get_value(DEV_ADC_HWVER);
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);
                break;
            case DIGITAL4:
              #if(MACHINETYPE==MACHINE_S1)
                //ret = dev_adc_get_value(DEV_ADC_PTR_TM);
              #else
                //ret = dev_adc_get_value(DEV_ADC_VBAT);
              #endif
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);
                break;
            case ESC:
                return;
            case POWER:
                break;
            }
        }
    }
}
s32 delay_ms(u32 ms)
{
    s32 i,j;

    for(i=0; i<ms; i++)
    {
        for(j=10200; j>0; j--); //sxl?
    }
}

s32 dev_power_test(void)
{
    
    u8 flg=1;
    u32 key;

    while(1)
    {
        if(flg==1)
        {
            dev_debug_printf("================POWER TEST===============\r\n");
            dev_debug_printf("1.sleep\r\n");
			dev_debug_printf("2.deepsleep\r\n");
            dev_com_tcdrain(DEBUG_PORT_NO);
			flg = 0;
        }
        if(keypad_interface_read_beep(&key)>0)
        {
dev_debug_printf("key=%02X\r\n", key);  
            switch(key)
            {
            case DIGITAL1:
                dev_com_tcdrain(0);
//                dev_user_delay_ms(100);
                delay_ms(10);
                dev_power_sleep(0);
 dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
               break;
            case DIGITAL2:
                dev_com_tcdrain(0);
//                dev_user_delay_ms(100);
                delay_ms(10);
                dev_power_sleep(1);
 dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
                break;
            case DIGITAL3:
 dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
                break;
            case DIGITAL4:
 dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
                break;
            case ESC:
                return;
            case POWER:
                break;
            }
        }
        
    }
    
}
s32 dev_watchdog_test(void)
{
    
    u8 flg=1;
    u32 key;
    u8 tmp[256];
    s32 i;
    s32 ret;

    while(1)
    {
        if(flg==1)
        {
            dev_debug_printf("================BT TEST===============\n");
            dev_debug_printf("1.watchdog open\n");
			dev_debug_printf("2.watchdog close\n");
			dev_debug_printf("3.watchdog feed\n");
			flg = 0;
        }
        if(keypad_interface_read_beep(&key)>0)
        {
//printf("key=%02X\r\n", key);  
            switch(key)
            {
            case DIGITAL1:
                dev_watchdog_open();
 dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
               break;
            case DIGITAL2:
                //dev_watchdog_close();
 dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
                break;
            case DIGITAL3:
                dev_watchdog_feed();
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);
                break;
            case DIGITAL4:
                dev_watchdog_set_time(5);
                dev_watchdog_feed();
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);
                break;
            case DIGITAL5:
                dev_watchdog_set_time(30);
                dev_watchdog_feed();
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);
                break;
            case ESC:
                return;
            case POWER:
                break;
            }
        }
        
    }
    
}
#endif
#if 1//def TRENDIT_CORE
extern void dev_dryicd_task(void);
#if 0
void dev_dryice_test(void)
{
    u8 dispflg=1;
    s32 ret;
    u32 key;
    

//	dev_rtc_open();   //需要打开RTC，tamper 才能工作  不开RTC
//	dev_dryice_poweron();
//	pcitamper_manage_poweronreadporstatus();
	pci_core_close();
    while(1)
    {
        if(dispflg==1)
        {
            dev_debug_printf("========DryiceTest==========\r\n");
            dev_debug_printf("1.Open Sensor\r\n");
            dev_debug_printf("2.Read status\r\n");
            dev_debug_printf("3.Close Sensor\r\n");
            dispflg = 0;
        }
        if(keypad_interface_read_beep(&key)>0)
        {
dev_debug_printf("key=%02X\r\n", key);            
            switch(key)
            {
            case DIGITAL1:
                pci_core_open();
                break;
            case DIGITAL2:
	            pci_core_close();
                break;
            case DIGITAL3:
                dev_dryicd_task();
                break;
            case ESC:
                return;
            case POWER:
                break;
            }
        }
        
    }
}
#endif
#define WIFI_PORT_NO  3
void dev_test_wifi_download(void)
{
#if 0
    s32 ret;
    u32 key;
    u8 disflg=1;
    u8 tmp[32];
    
    lcd_interface_clear_ram();
    lcd_interface_fill_rowram(LINE1, 0, "Wifi DOWNLOAD", CDISP);    
    lcd_interface_brush_screen();
    dev_user_delay_ms(50);
    
	dev_com_open(DEBUG_PORT_NO, 115200, 8, 'n', 0, 0); 
	dev_com_open(WIFI_PORT_NO, 115200, 8, 'n', 0, 0); 
    dev_com_setbaud(DEBUG_PORT_NO, 115200, 8, 'n', 0, 0); 
    dev_com_setbaud(WIFI_PORT_NO, 115200, 8, 'n', 0, 0);
    dev_wifi_ctl_downloadmod();
    while(1)
    {
      #if 1
        if(keypad_interface_read_beep(&key)>0)
        {
//dev_debug_printf("key=%02X\r\n", key);            
            switch(key)
            {
            case ESC:
                return;
            }
        }
      #endif  
      #if 0
        ret = dev_com_read(DEBUG_PORT_NO, tmp, 32);
        if(ret>0)
        {
            dev_com_write(WIFI_PORT_NO, tmp, ret);
        }
        
        ret = dev_com_read(WIFI_PORT_NO, tmp, 32);
        if(ret>0)
        {
            dev_com_write(DEBUG_PORT_NO, tmp, ret);
        }
      #endif  
    }
	dev_com_setbaud(DEBUG_PORT_NO, MCU_UART_BAUD, 8, 'n', 0, 0);
#endif
}

s32 test_wifi_signle(u8* target_ap, u8* target_pwd)
{
#if 0
    u8 flg;
	s32 err;
	u32 ap_num;
	u32 index;
	wifi_apinfo_t aps[20];
	s32 select_ap_index;
	wifi_apinfo_t *select_ap = NULL;
//	TrenditInputText* pwd_input = NULL;
	u8 pwd[20] = {0};
	u8 ap_name[64] = {0};
	u32 ap_found = 0;
	u8 ap_name_with_quote[64] = {0};
	u8 error_info[128] = {0};
//	target_ap = "Trendit-Guest";
//	target_pwd = "trenditguest001";
    u32 key;
    u32 timeid;
    
    

    do
    {
        lcd_interface_clear_ram();
        lcd_interface_fill_rowram(LINE1, 0, "WIFI测试", CDISP);   
        lcd_interface_fill_rowram(LINE3, 0, "正在打开...", CDISP);   
        lcd_interface_brush_screen();
        err = dev_wifi_open();
        lcd_interface_clear_rowram(LINE3);
        if(err!=0)
        {
            lcd_interface_fill_rowram(LINE3, 0, "正在打开...", CDISP);   
            //lcd_interface_brush_screen();
            //dev_user_delay_ms(1000);
            break;
        }
        ap_num = sizeof(aps)/sizeof(aps[0]);
        lcd_interface_fill_rowram(LINE3, 0, "正在搜网...", CDISP);   
        lcd_interface_brush_screen();
        err = dev_wifi_scanap_start(aps, ap_num);
        if(err == 0)
        {
            while(1)
            {
                err = dev_wifi_scanap_status(&ap_num);
                if(err == 0)
                {
                    //正在搜索
                }
                else if(err == 1)
                {
                    //搜索成功
                    err = 0;
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        if(err != 0)
        {
            lcd_interface_clear_rowram(LINE3);
            lcd_interface_fill_rowram(LINE3, 0, "搜网失败", CDISP);   
            break;
        }
        //if((target_ap != NULL) && (target_pwd != NULL))
        {
            for(index=0; index<ap_num; index++)
            {
				sprintf(ap_name_with_quote, "\"%s\"", target_ap);
				if(strcmp(ap_name_with_quote, aps[index].m_ssid) == 0)
				{
						strcpy(ap_name, target_ap);
						strcpy(pwd, target_pwd);
						ap_found = 1;
				}
				//Trace("debug", "wifi: %s \r\n", aps[index].m_ssid);
//             dev_debug_printf("%s(%d):aps[%d]=%s\r\n", __FUNCTION__, __LINE__, index, aps[index].m_ssid);
    		}
    		if(!ap_found)
    		{
				sprintf(error_info, "没有找到目标热点:%s", target_ap);
                lcd_interface_clear_rowram(LINE3);
                lcd_interface_fill_rowram(LINE3, 0, error_info, CDISP);
                err = -1;
                break;
		    }
        }
      #if 0
    	else
    	{	
    		//用户选择热点
    		select_ap_index = select_wifi_ap(aps, ap_num);
    		if(select_ap_index < 0)
    		{
    			clear_and_show_row(SDK_DISP_LINE3, "没有选择热点...");
    			goto error;
    		}
    		
    		select_ap = &aps[select_ap_index];
    		
    		get_wifi_ap_name(select_ap, ap_name, sizeof(ap_name));
    		
    		if(strlen(ap_name) == 0) {
    			sprintf(ap_name, "%s", select_ap->m_mac);
    		} 

    		err = input_wifi_pwd(ap_name, pwd, sizeof(pwd));
    		
    		if(err < 0)
    		{	
    			clear_and_show_row(SDK_DISP_LINE3, "输入密码错误...");
    			goto error;
    		}	
            
    	}
      #endif
        lcd_interface_clear_rowram(LINE3);
        lcd_interface_fill_rowram(LINE3, 0, "正在连接AP...", CDISP);
        lcd_interface_brush_screen();
        err = dev_wifi_connetap(ap_name, pwd, NULL); //ddi_wifi_connectap_start
        if(err == 0)
        {
            while(1)
            {
                err = dev_wifi_get_connectap_status(); //ddi_wifi_connectap_status
                if(err == 0)
                {
                    
                }
                else if(err == 2)
                {
                    //连接成功
                    err = 0;
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        if(err != 0)
        {
            lcd_interface_clear_rowram(LINE3);
            lcd_interface_fill_rowram(LINE3, 0, "连接AP失败..", CDISP);
            break;
        }
        timeid = 0;
        flg = 1;
        while(1)
        {
            if(dev_user_querrytimer(timeid, 1000))
            {
                timeid = dev_user_gettimeID();
                flg = 1;
            }
            if(flg==1)
            {
                err = dev_wifi_get_signal(&ap_num);
                if(err<=0)
                {
                    ap_num = 0;
                }
                sprintf(error_info, "信号强度:%d", ap_num);
                err = dev_wifi_get_connectap_status(); //ddi_wifi_connectap_status();
                lcd_interface_clear_rowram(LINE3);
                lcd_interface_clear_rowram(LINE4);
                lcd_interface_fill_rowram(LINE4, 0, "连接成功！", CDISP);
                if(err == 2)
                {
                    lcd_interface_fill_rowram(LINE3, 0, "连接成功！", CDISP);
                }
                else
                {
                    lcd_interface_fill_rowram(LINE3, 0, "连接失败！", CDISP);
                }
                lcd_interface_fill_rowram(LINE4, 0, error_info, CDISP);
                lcd_interface_brush_screen();
                flg = 0;
            }
            
            if(keypad_interface_read_beep(&key)>0)
            {           
                switch(key)
                {
                case DIGITAL1:
                    break;
                case DIGITAL2:
                    break;
                case DIGITAL3:
                    break;
                case DIGITAL4:
                    break;
                case ESC:
                    return -1;
                case POWER:
                    //flg = 1;
                    break;
                }
            }
        }
        
        
    }while(0);

        
    
    lcd_interface_brush_screen();
    dev_user_delay_ms(1000);
    return err;
#endif
}

void dev_test_wifi(void)
{
#if 0
//    TRENDITDEV_TEST_INFO devtestinfo;
        u8 dispflg=1;
        s32 ret;
        u32 key;
        u8 tmp[32];
        
    
    //  dev_rtc_open();   //需要打开RTC，tamper 才能工作  不开RTC
    //  dev_dryice_poweron();
    //  pcitamper_manage_poweronreadporstatus();

        dev_gpio_config_mux(GPIO_PIN_PTA1, MUX_CONFIG_GPIO);
        dev_gpio_set_pad(GPIO_PIN_PTA1, PAD_CTL_PULL_NONE);
        dev_gpio_direction_output(GPIO_PIN_PTA1, 0);
        while(1)
        {
            if(dispflg==1)
            {
                lcd_interface_clear_ram();
                lcd_interface_fill_rowram(LINE1, 0, "Wifi Test", CDISP);
                lcd_interface_fill_rowram(LINE2, 0, "1.Open", LDISP);
                lcd_interface_fill_rowram(LINE3, 0, "2.Close", LDISP);
                lcd_interface_fill_rowram(LINE4, 0, "3.测试信号", LDISP);
                lcd_interface_fill_rowram(LINE5, 0, "4.Wifi DownLoad", LDISP);
                lcd_interface_brush_screen();
                //dev_debug_printf("========Wifi Test==========\r\n");
                //dev_debug_printf("1.Open \r\n");
                //dev_debug_printf("2.Close \r\n");
                //dev_debug_printf("3.\r\n");
                dispflg = 0;
            }
            if(keypad_interface_read_beep(&key)>0)
            {
    dev_debug_printf("key=%02X\r\n", key);            
                switch(key)
                {
                case DIGITAL1:
                    ret = dev_wifi_open();
                    /*dev_gpio_direction_output(GPIO_PIN_PTA1, 0);
                    dev_user_delay_ms(1000);
                    dev_gpio_direction_output(GPIO_PIN_PTA1, 1);*/
    dev_debug_printf("ret=%d\r\n", ret);  
                    
                    break;
                case DIGITAL2:
                    ret = dev_wifi_close();
                    break;
                case DIGITAL3:
                    //test_wifi(&devtestinfo, DEVICE_TEST_MODE_SINGLE); 
                    test_wifi_signle("Trendit", "trendit123");
                    //dev_debug_printf("devtestinfo->wifi=%d\r\n", devtestinfo.wifi);
                    dispflg = 1;
                    break;
                case DIGITAL4:
                    dev_test_wifi_download();
                    dispflg = 1;
                    break;
                case ESC:
                    return;
                case POWER:
                    break;
                }
            }
            
        }
#endif
}
#define GPRS_PORT    3
void dev_test_gprs(void)
{
}

#define GPRSTEST_STEP_VBATOFF       0
#define GPRSTEST_STEP_VBATON        1
#define GPRSTEST_STEP_POWERON       2
#define GPRSTEST_STEP_WAIT_ATREADY  3
#define GPRSTEST_STEP_WAIT_SIMREADY 4
#define GPRSTEST_STEP_DAIL          5
#define GPRSTEST_STEP_HUNG          6
void dev_test_gprs1(u8 *phonenum)
{

}
#if 0
#define GPIO_TEST_MAX    2
iomux_pin_name_t g_testpin[GPIO_TEST_MAX] = 
{
    GPIO_PIN_PTF4, 
    GPIO_PIN_PTF5, 
    GPIO_PIN_PTF6, 
    GPIO_PIN_PTF7, 
}

GPIO_PIN_PTE7

void dev_test_key(void)
{
    u32 i;
    for(i=0; i<GPIO_TEST_MAX; i++)
    {
        dev_gpio_config_mux(g_testpin[i], iomux_pin_cfg_t cfg)
    }
}
#endif
#if 0
void dev_test_battery(void)
{
        s32 ret;
        u32 key;
        u8 tmp[32];
        u32 timeid=0;
        u8 i;
        u32 adcvalue;
        
    
        while(1)
        {
            if(dev_user_querrytimer(timeid, 1000))
            {
                timeid = dev_user_gettimeID();
                ret = dev_power_get_bat_status();
                //batteryflg = (ret>>8)&0xFF;     //记录充电状态
                i = ret&0xff;
                adcvalue = dev_power_get_adcvalue();
                lcd_interface_clear_ram();
                lcd_interface_fill_rowram(LINE1, 0, "Battery Test", CDISP);
                sprintf(tmp, "1.adc= %d", adcvalue);
                lcd_interface_fill_rowram(LINE2, 0, tmp, LDISP);
                sprintf(tmp, "2.V1 = %d", (20*60*adcvalue)>>10);
                lcd_interface_fill_rowram(LINE3, 0, tmp, LDISP);
                sprintf(tmp, "3.V2 = %d", ((20+82)*60*adcvalue)>>10);
                lcd_interface_fill_rowram(LINE4, 0, tmp, LDISP);
                sprintf(tmp, "4.NUM=%d", i);
                lcd_interface_fill_rowram(LINE5, 0, tmp, LDISP);
                lcd_interface_brush_screen();
            }
            if(keypad_interface_read_beep(&key)>0)
            {           
                switch(key)
                {
                case ESC:
                    return;
                case POWER:
                    break;
                }
            }
            
        }

}
#endif
#endif
#if 0
void dev_test_key(void)
{
        u8 dispflg=1;
        s32 ret;
        u32 key;
        u32 key1;
        
    
    //  dev_rtc_open();   //需要打开RTC，tamper 才能工作  不开RTC
    //  dev_dryice_poweron();
    //  pcitamper_manage_poweronreadporstatus();
        while(1)
        {
            if(dispflg==1)
            {
                dev_debug_printf("========Key Test==========\r\n");
                dispflg = 0;
            }
            if(keypad_interface_read_beep(&key)>0)
            {
    dev_debug_printf("key=%02X\r\n", key); 
#if 1    
                switch(key)
                {
                case DIGITAL1:
                    ret = drv_keypad_get_powerkey();
    dev_debug_printf("ret=%02X\r\n", ret); 
                    
                    break;
                case DIGITAL2: 
                    break;
                case ESC:
                    return;
                case POWER:
                    break;
                }
#endif                
            }
            
            
        }

}
#ifdef TRENDIT_CORE

#include "mh_crypt.h"
#include "mh_rsa.h"
#include "mh_rand.h"
#include "mh_des.h"
#include "mh_aes.h"
#include "mh_sha.h"


#define KEY_LEN_2048        2048 / 32
#define RSA_N_2048          "75A18418B8EB30CD990303626314215772406FB53C90BC34F8963643CB7C86655174E82AC581EA9D93FBCD8B9B9623E9FE8AD5E2F9311C8FEEFB709D78CC961DC49A3CA5F55733E68A10563695782DE1C8BE5F031E84540E14AA369FA8E03D03CCC7E02B2A030C44AF4B989DBC99640BB4925B9C3AAE287C241C1AF133502AD61593C62FAB42604680FBE1EC77C98B68008C15E52C24035A8F220C1B944C547F1E0FA4C6475395839536FD286F23063E802D657C6D176B46E3B1862616D32EADB0DC1C1AF83EAF33640224658BB46052ACD8D8876A7F35A9232ED4AD2E15FD958F294A5BFFBA11E2E18F7D3377136F6C91B93C9656E289E19FE3E928FDF7FB7F"
#define RSA_E_2048          "10001"
#define RSA_D_2048          "562B23211914F0832B7B5197590CFDFF98EA2AEA909B55C357F67179E82E2825402E6B84297AA9EB14E47805B4CB9F33620B41F32AA8174C38BC681A72CB2B8C52E0C95CA9046038991AAE8CFDA133BB19DA9AAF2643B96F3724192F8BFDF7B438E692080C967EFE318B27192577181A9B043BA78E34D5E35A705C2C2472AD6D762273180C7DA3107658259BC5BF554B3CC2CF33F264DDB3C45406A047A1B3720A655CAFF677FA7DCFDB9F125A8C1D333E07F93C884AFC7E9444D6033AAB3975DED23BC71BCA3EC83CCF000D6586020B1BF483762BFE6F1C64CE021CB85AD538A18B7539D42619C6901364ED6B14D07EC9EFECA16AC59D5BAB0F8840A5D86281"
#define RSA_P_2048          "9AA3212A0A6E3158E399844D2F44C9CC3D25D5DDF1C18B2F49E70BB2326F1E738C33DFB87A4DAC5EED914F72724C670D393D1B7368D2006DA1A649E5D03930C239F0A304C04F3222787436A271D544F8BD3DD3A3C2032B84C68B9080B600AE57395146C6F46C18DA1DB9129544D8B9C65F9DD624164A64AB500EB83AFDBDF279"
#define RSA_Q_2048          "C2BC8CFDC7F5C23DE47E95D39BD0EE16B27957280655ED4F0D0C8793AACD279B374BA0C3CFF78781DA7D5EF8DE83FDDB83A04386E599FC5A20E113571852FBFE88275A684C3BF6A1F187E3497DD8EC6B7ACE419F471C615DA1FD48A36EF4DBC9CBC692A258BB67D6C213FD7D8FC513FE307E2A2943D7739227D7FD2EC4AB1FB7"
#define RSA_DP_2048         "598E6E4F8DB8E1A56C32E5775A0FC99B1A32C6588281A79D1C7CCDBE470D29A29BA9134120BBC93F34F0FA04FB031A8F82FDCBDF1BEEBB46C2DEAE5F85270A01025345E62385FA301278A38792CB83E5569BEEED21187AE4ABA3FA6D407AC67D4779B5B6D5A9A0FEF945CCE85BF5F4295A08EC12FC41DB6987B4EEA7637AD569"
#define RSA_DQ_2048         "9B8767BBD096317AA921889C271828EFABBF49A2C7DC9CD9C3FC3BD3EE5847C7A9D417240CD70F3CE0B9BCBE5B4963D88C39D574C9AFC062635C1FCC31BB2553D67E47B081226B0A50134DAE067AD7E7C77F2FDDFC37687415CA6AD74CE8F719264D0413F1764BAAB3FAAF3C37351990AC3E37821992DBD4E1499A065E40A5E5"
#define RSA_QP_2048         "906598444A1664A4F3FD0E67DA92E0BCBB445101DC09ED567ACE2D49DBEC61B2E06ECD64544B0C35D26E68DBE3048B83A601285245AF4FFC94E3EF5F7602E3EC9B8AD6D9FBC518F95D3A638D7A3FC34E72307F67A48C0696753664D589BE2EC861EB659C7AB06C5A9095E4D8794D49504E651EAEE26C76091175C5B7082B4016"
#define RSA_N_C_2048            "65B37280142A48FCED5C720FC0A17DB2A6B6C1901CD1FECA252FEB61E5CF58E4068800EE4F54D2D9E35566BC5CB976B497E65FFA529F398E562BB38AE891A03B212D88E98073DE85A5A00D9B2CA11D0B14AF22CD2C832FD8DCD5E4ADC6BA95FFC7FBEBC80FE9D32ACB4EB6F52C742B11F11D83655E30B5FC786E15CC8C8D1C9F9EF66284D337B8A697E4F8485064D28823FD0DB4E75F5618437D776D6910405AB88B467F2E5DD4C206C670807E25227E61D02EE32B37C731DA1DF141D36CC48B636867C37C6A08C17EE8416A0A33A0791EE2233C3915EC38B2DA1984DE95F00BEAA1575DBD1ACCF4F8DB37500656BE4CD7A874F6247A127C2F32A046197D4D2A"
#define RSA_P_C_2048            "0224D929A1397374039926557BCA9FC74DD9E7AB3DBAF1A216DC1E57ED4F57B1A7D6E922F3CB5FE825FA14264BBFA864FD6129885EABC7C21680C2390511AC7189EF4F7F7BDEA690843353C18235E9E56BA985F712D44B377AE1578321A61DFAB7240E12597A5513B055E455BBAA66BCD25AAB72129D830B6EFAFFBBA5D30132"
#define RSA_Q_C_2048            "5AD49E385E01DBA437ED91437ED4C9C17E1EE4F5EACABF9624C3C8AF4B4712D35900C84522FADC41D5723AA652D33F2F25DB60112F8DE2D9F907E8EE7B4A84129AA9FFCEF7FA1FFB4E4E879C5D55CFD45757AAC16739C495D5C756BFA4C3DEF9A591962919C298495A9727252B345D9470C4BFA33F5121AA861952B0D4A3A3D9"
#define RSA_N_Q_2048            "FAEC3B81"
#define RSA_P_Q_2048            "7D3E2837"
#define RSA_Q_Q_2048            "552511F9"
#define CIPHER_2048         "1A489B964931E96ECBED9DB0A357B0912C203B3020BA1427353AD06CAD229A77F9E864A11BF0AD6A4FE2CB88D3C0F4AE0B964C6122DA581E2E509D9B7828E41BDDC6A179F43EF2B1DAB09C4D49ACCE3CD55FBFA256E264189C365A2E8BE9640B98A73267E9A5ACBD9C93838AD76ED57DCD8C680F248CAC97CDA034CC348A5754EBFAAA3A855FF618CE6BCA51BCBAC33D105D0F6BF58FE1C0E7F8DF4CB293D48A4EC5FEA617DFA00F1E6688893FCF5DC7B45DE3C511E14DF2D6D072511664BF397D322DD61952882CE88E8F1D770B14340B52AAA2D2A388BF2A1F6A5D7A13EF149FF9AEDFE0A2657DCB9568A06FBFFB800D1CFB7CA625630D8DA39174008F93F9"




static int bn_get_digit( uint32_t *d, char c )
{
    *d = 255;

    if( c >= 0x30 && c <= 0x39 ) *d = c - 0x30;
    if( c >= 0x41 && c <= 0x46 ) *d = c - 0x37;
    if( c >= 0x61 && c <= 0x66 ) *d = c - 0x57;

    if( *d >= (uint32_t) 16 )
        return( -1 );

    return( 0 );
}


int bn_read_string_to_bytes( uint8_t *r, uint32_t len, const char *s )
{
    uint32_t i, j, slen;
    uint32_t d;
	
    slen = strlen( s );

	if ((len * 2) < slen)
		return(-1);

	memset(r, 0, len);
	
	len = len - 1;
	
	for( i = slen, j = 0; i > 0; i--, j++)
	{
		if( -1 == bn_get_digit( &d, s[i - 1] ) )
			return (-1);
		r[len - j / 2] |= d << ( (j % 2) << 2 );
	}

    return( 0 );
}

int bn_read_string_from_head( uint8_t *r, uint32_t len, const char *s )
{
    int32_t i, j, slen;
    uint32_t d;
	
    if((slen = strlen( s )) < 0)
    {
        return -1;
    }

	if ((len * 2) < slen)
		return(-1);

    if (slen & 1)
    {
        slen += 1;
    }
    
	memset(r, 0, len);
	
	len = slen >> 1;
	
	for( i = slen, j = 0; i > 0; i--, j++)
	{
		if( -1 == bn_get_digit( &d, s[i - 1] ) )
			return (-1);
		r[len - 1 - j / 2] |= d << ( (j % 2) << 2 );
	}

    return( len );
}

static void rsa_read_key(mh_rsa_private_key* key, uint32_t key_size)
{
    
    key->bytes = 2048 / 8;
    bn_read_string_to_bytes(key->n, key->bytes, RSA_N_2048);
    bn_read_string_to_bytes(key->e, 4, RSA_E_2048);
    bn_read_string_to_bytes(key->d, key->bytes, RSA_D_2048);
    bn_read_string_to_bytes(key->p, key->bytes/2, RSA_P_2048);
    bn_read_string_to_bytes(key->q, key->bytes/2, RSA_Q_2048);
    bn_read_string_to_bytes(key->dp, key->bytes/2, RSA_DP_2048);
    bn_read_string_to_bytes(key->dq, key->bytes/2, RSA_DQ_2048);
    bn_read_string_to_bytes(key->qp, key->bytes/2, RSA_QP_2048);
    bn_read_string_to_bytes(key->n_c, key->bytes, RSA_N_C_2048);
    bn_read_string_to_bytes(key->p_c, key->bytes/2, RSA_P_C_2048);
    bn_read_string_to_bytes(key->q_c, key->bytes/2, RSA_Q_C_2048);
    bn_read_string_to_bytes(key->n_q, 4, RSA_N_Q_2048);
    bn_read_string_to_bytes(key->p_q, 4, RSA_P_Q_2048);
    bn_read_string_to_bytes(key->q_q, 4, RSA_Q_Q_2048);    
}


void r_printf(uint32_t b, char *s)
{
	if (0 != b)
	{
		dev_debug_printf("pass: ");dev_debug_printf("%s", s);
	}
	else
	{
		dev_debug_printf("fail: ");dev_debug_printf("%s", s);
		while(1);
	}
}

extern uint32_t mh_trrand_p(void *rand, uint32_t bytes, void *p_rng);
#define PLAIN "1234567890"

void rsa_func_test()
{
    #if 0
    uint32_t i, u32First;
    
    mh_rsa_private_key pri_key;
    mh_rsa_public_key pub_key;

    rsa_pub_key_t publickey;
    rsa_pri_key_t privatekey;
    
    uint8_t plain[2048 / 8];
    uint8_t cipher[2048 / 8];
    uint8_t tmp0[2048 / 8];
    uint32_t cipherlenth;
    uint32_t plainlenth;
    
	
    dev_debug_printf("\r\nrsa 2048 test\r\n");
    rsa_read_key(&pri_key, 2048);
    
    mh_rsa_complete_key(&pri_key, mh_trrand_p, NULL);
    
    pub_key.bytes = pri_key.bytes;
    memcpy(pub_key.e, pri_key.e, sizeof(pri_key.e));
    memcpy(pub_key.n, pri_key.n, sizeof(pri_key.n));
    memcpy(pub_key.n_c, pri_key.n_c, sizeof(pri_key.n_c));
    memcpy(pub_key.n_q, pri_key.n_q, sizeof(pri_key.n_q));

    bn_read_string_to_bytes(plain, pri_key.bytes, PLAIN);
    bn_read_string_to_bytes(cipher, pri_key.bytes, CIPHER_2048);

    memset(tmp0, 0, pri_key.bytes);
    mh_rsa_private(tmp0, cipher, &pri_key, mh_trrand_p, NULL);
    r_printf((0 == memcmp(tmp0, plain, pri_key.bytes)), "mh_rsa_private test\n");

    
    memset(tmp0, 0, pri_key.bytes);
    mh_rsa_private_crt(tmp0, cipher, &pri_key, mh_trrand_p, NULL);
    r_printf((0 == memcmp(tmp0, plain, pri_key.bytes)), "mh_rsa_private_crt test\n");
    
    memset(tmp0, 0, pri_key.bytes);
    mh_rsa_public(tmp0, plain, &pub_key, mh_trrand_p, NULL);
    r_printf((0 == memcmp(tmp0, cipher, pri_key.bytes)), "mh_rsa_public test\n");
    #endif
	
}



void test_aes(void)
{
    #if 0
	uint32_t t;
	mh_pack_mode_def modes[2] = { ECB, CBC};
    mh_rng_callback f_rng = mh_trrand_p;

	uint8_t plain[] = {0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40, 0x9F, 0x96, 0xE9, 0x3D, 0x7E, 0x11, 0x73, 0x93, 0x17, 0x2A, 
										 0xAE, 0x2D, 0x8A, 0x57, 0x1E, 0x03, 0xAC, 0x9C, 0x9E, 0xB7, 0x6F, 0xAC, 0x45, 0xAF, 0x8E, 0x51};
		 
	uint8_t key128[] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C};
	uint8_t key192[] = {0x8E, 0x73, 0xB0, 0xF7, 0xDA, 0x0E, 0x64, 0x52, 0xC8, 0x10, 0xF3, 0x2B, 0x80, 0x90, 0x79, 0xE5, 
											0X62, 0xF8, 0xEA, 0xD2, 0x52, 0x2C, 0x6B, 0x7B};
	uint8_t key256[] = {0x60, 0x3D, 0xEB, 0x10, 0x15, 0xCA, 0x71, 0xBE, 0x2B, 0x73, 0xAE, 0xF0, 0x85, 0x7D, 0x77, 0x81, 
											0x1F, 0x35, 0x2C, 0x07, 0x3B, 0x61, 0x08, 0xD7, 0x2D, 0x98, 0x10, 0xA3, 0x09, 0x14, 0xDF, 0xF4};
		
	uint8_t iv[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

	uint8_t eCipher[3][32] = {{0x3A, 0xD7, 0x7B, 0xB4, 0x0D, 0x7A, 0x36, 0x60, 0xA8, 0x9E, 0xCA, 0xF3, 0x24, 0x66, 0xEF, 0x97, 
												0xF5, 0xD3, 0xD5, 0x85, 0x03, 0xB9, 0x69, 0x9D, 0xE7, 0x85, 0x89, 0x5A, 0x96, 0xFD, 0xBA, 0xAF},
											 {0xBD, 0x33, 0x4F, 0x1D, 0x6E, 0x45, 0xF2, 0x5F, 0xF7, 0x12, 0xA2, 0x14, 0x57, 0x1F, 0xA5, 0xCC,
												0x97, 0x41, 0x04, 0x84, 0x6D, 0x0A, 0xD3, 0xAD, 0x77, 0x34, 0xEC, 0xB3, 0xEC, 0xEE, 0x4E, 0xEF},
											 {0xF3, 0xEE, 0xD1, 0xBD, 0xB5, 0xD2, 0xA0, 0x3C, 0x06, 0x4B, 0x5A, 0x7E, 0x3D, 0xB1, 0x81, 0xF8,												 
												0x59, 0x1C, 0xCB, 0x10, 0xD4, 0x10, 0xED, 0x26, 0xDC, 0x5B, 0xA7, 0x4A, 0x31, 0x36, 0x28, 0x70}};	

	uint8_t cCipher[3][32] = {{0x76, 0x49, 0xAB, 0xAC, 0x81, 0x19, 0xB2, 0x46, 0xCE, 0xE9, 0x8E, 0x9B, 0x12, 0xE9, 0x19, 0x7D, 
														 0x50, 0x86, 0xCB, 0x9B, 0x50, 0x72, 0x19, 0xEE, 0x95, 0xDB, 0x11, 0x3A, 0x91, 0x76, 0x78, 0xB2},
														{0x4F, 0x02, 0x1D, 0xB2, 0x43, 0xBC, 0x63, 0x3D, 0x71, 0x78, 0x18, 0x3A, 0x9F, 0xA0, 0x71, 0xE8, 
														 0xB4, 0xD9, 0xAD, 0xA9, 0xAD, 0x7D, 0xED, 0xF4, 0xE5, 0xE7, 0x38, 0x76, 0x3F, 0x69, 0x14, 0x5A},
														{0xF5, 0x8C, 0x4C, 0x04, 0xD6, 0xE5, 0xF1, 0xBA, 0x77, 0x9E, 0xAB, 0xFB, 0x5F, 0x7B, 0xFB, 0xD6, 
														 0x9C, 0xFC, 0x4E, 0x96, 0x7E, 0xDB, 0x80, 0x8D, 0x67, 0x9F, 0x77, 0x7B, 0xC6, 0x70, 0x2C, 0x7D}}; 

	uint8_t cipher[32];
	uint8_t mplain[32];

	dev_debug_printf("\nAES Test In\n");
														 
	memset(cipher, 0, sizeof(cipher));
	memset(mplain, 0, sizeof(mplain));
 	mh_aes_enc(modes[0], cipher, sizeof(cipher), plain, sizeof(plain), key128, MH_AES_128, iv, f_rng, NULL);
	mh_aes_dec(modes[0], mplain, sizeof(mplain), cipher, sizeof(cipher), key128, MH_AES_128, iv, f_rng, NULL);
	t = (!memcmp(&eCipher[0][0], cipher, sizeof(cipher)) && !memcmp(mplain, plain, sizeof(plain)));
	r_printf(t, "AES ECB key128 test\n");

	
	memset(cipher, 0, sizeof(cipher));
	memset(mplain, 0, sizeof(mplain));
 	mh_aes_enc(modes[0], cipher, sizeof(cipher), plain, sizeof(plain), key192, MH_AES_192, iv, f_rng, NULL);
	mh_aes_dec(modes[0], mplain, sizeof(mplain), cipher, sizeof(cipher), key192, MH_AES_192, iv, f_rng, NULL);
	t = (!memcmp(&eCipher[1][0], cipher, sizeof(cipher)) && !memcmp(mplain, plain, sizeof(plain)));
	r_printf(t, "AES ECB key192 test\n");
												 

	memset(cipher, 0, sizeof(cipher));
	memset(mplain, 0, sizeof(mplain));
 	mh_aes_enc(modes[0], cipher, sizeof(cipher), plain, sizeof(plain), key256, MH_AES_256, iv, f_rng, NULL);
	mh_aes_dec(modes[0], mplain, sizeof(mplain), cipher, sizeof(cipher), key256, MH_AES_256, iv, f_rng, NULL);
	t = (!memcmp(&eCipher[2][0], cipher, sizeof(cipher)) && !memcmp(mplain, plain, sizeof(plain)));
	r_printf(t, "AES ECB key256 test\n");

	
 	memset(cipher, 0, sizeof(cipher));
 	memset(mplain, 0, sizeof(mplain));
  	mh_aes_enc(modes[1], cipher, sizeof(cipher), plain, sizeof(plain), key128, MH_AES_128, iv, f_rng, NULL);
 	mh_aes_dec(modes[1], mplain, sizeof(mplain), cipher, sizeof(cipher), key128, MH_AES_128, iv, f_rng, NULL);
 	t = (!memcmp(&cCipher[0][0], cipher, sizeof(cipher)) && !memcmp(mplain, plain, sizeof(plain)));
 	r_printf(t, "AES CBC key128 test\n");

 	memset(cipher, 0, sizeof(cipher));
 	memset(mplain, 0, sizeof(mplain));
  	mh_aes_enc(modes[1], cipher, sizeof(cipher), plain, sizeof(plain), key192, MH_AES_192, iv, f_rng, NULL);
 	mh_aes_dec(modes[1], mplain, sizeof(mplain), cipher, sizeof(cipher), key192, MH_AES_192, iv, f_rng, NULL);
 	
 	t = (!memcmp(&cCipher[1][0], cipher, sizeof(cipher)) && !memcmp(mplain, plain, sizeof(plain)));
 	r_printf(t, "AES CBC key192 test\n");

 	memset(cipher, 0, sizeof(cipher));
 	memset(mplain, 0, sizeof(mplain));
  	mh_aes_enc(modes[1], cipher, sizeof(cipher), plain, sizeof(plain), key256, MH_AES_256, iv, f_rng, NULL);
 	mh_aes_dec(modes[1], mplain, sizeof(mplain), cipher, sizeof(cipher), key256, MH_AES_256, iv, f_rng, NULL);
 	t = (!memcmp(&cCipher[2][0], cipher, sizeof(cipher)) && !memcmp(mplain, plain, sizeof(plain)));
 	r_printf(t, "AES CBC key256 test\n");
	#endif
}


void test_des(void)
{
	mh_rng_callback f_rng = mh_trrand_p;
	uint32_t t;
	mh_pack_mode_def modes[2] = {ECB, CBC};
	char *cmodes[2] = {"ECB", "CBC"};

	uint8_t PLAIN1[8] = {0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40, 0x9F, 0x96};

	mh_des_key_def KEY = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};				
	uint8_t IV[8] = {0xF6, 0x9F, 0x24, 0x45, 0xDF, 0x4F, 0x9B, 0x17};
	
	uint8_t cCrypt[8] = {0x2D, 0x12, 0x1F, 0x90, 0xFC, 0xF6, 0x86, 0x31};			 
	uint8_t eCrypt[8] = {0x72, 0x77, 0xA0, 0x0D, 0xC1, 0xC1, 0xC3, 0x6B};
	
	uint8_t plain[8] ={0};
	uint8_t cipher[8] = {0};

	uint8_t m = 0;	

	dev_debug_printf("\nDES Test In\n");
	
	memset(plain, 0, sizeof(plain));
	memset(cipher, 0, sizeof(cipher));
	
	mh_des_enc(modes[0],  cipher, sizeof(cipher), PLAIN1, sizeof(PLAIN1), KEY, NULL, NULL, NULL);
	t = (!memcmp(eCrypt, cipher, sizeof(cipher)));
	r_printf(t, "DES_Encrypt ECB test\n");
	
	mh_des_dec(modes[0],  plain, sizeof(plain), cipher, sizeof(cipher), KEY, NULL, NULL, NULL);
	t = (!memcmp(plain, PLAIN1, sizeof(PLAIN1)));
	r_printf(t, "DES_Decrypt ECB test\n");
	
	
	//CBC
	memset(plain, 0, sizeof(plain));
	memset(cipher, 0, sizeof(cipher));
	
	mh_des_enc(modes[1], cipher, sizeof(cipher), PLAIN1, sizeof(PLAIN1), KEY, IV, f_rng, NULL);
	t = (!memcmp(cCrypt, cipher, sizeof(cipher)));
	r_printf(t, "DES_Encrypt CBC test\n");
	
	mh_des_dec(modes[1], plain, sizeof(plain), cipher, sizeof(cipher), KEY, IV, f_rng, NULL);
	t = (!memcmp(plain, PLAIN1, sizeof(PLAIN1)));
	r_printf(t, "DES_Decrypt CBC test\n");

	
	
	
}



void test_tdes(void)
{
	
	mh_rng_callback f_rng = mh_trrand_p;
	uint32_t t;
	mh_pack_mode_def modes[2] = { ECB, CBC};
	char *cmodes[2] = {"ECB", "CBC"};

	uint8_t PLAIN1[8] = {0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40, 0x9F, 0x96};

	mh_tdes_key_def KEY = {{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
						{0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01},
						{0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01, 0x23}};
										
	uint8_t IV[8] = {0xF6, 0x9F, 0x24, 0x45, 0xDF, 0x4F, 0x9B, 0x17};
	

	uint8_t eCrypt[2][8] = {{0x06, 0xED, 0xE3, 0xD8, 0x28, 0x84, 0x09, 0x0A},
							{0x71, 0x47, 0x72, 0xF3, 0x39, 0x84, 0x1D, 0x34}};

	uint8_t cCrypt[2][8] = {{0x74, 0x01, 0xCE, 0x1E, 0xAB, 0x6D, 0x00, 0x3C},
							 {0x20, 0x79, 0xC3, 0xD5, 0x3A, 0xA7, 0x63, 0xE1}};
												 
	uint8_t plain[8];
	uint8_t cipher[8];

	
	uint32_t m;

												 
	dev_debug_printf("\nTDES Test In\n");

	memset(plain, 0, sizeof(plain));
	memset(cipher, 0, sizeof(cipher));
	//===================ECB======================
	mh_tdes_enc(modes[0], cipher, sizeof(cipher), PLAIN1, sizeof(PLAIN1), &KEY, NULL, NULL, NULL);
	t = (!memcmp(&eCrypt[1][0], cipher, sizeof(cipher)));
	r_printf(t, "TDES_Encrypt ECB test\n");
	
	mh_tdes_dec(modes[0], plain, sizeof(plain), cipher, sizeof(cipher), &KEY, NULL, NULL, NULL);
	t = (!memcmp(plain, PLAIN1, sizeof(PLAIN1)));
	r_printf(t, "TDES_Decrypt ECB test\n");
	
	//===================CBC======================
	memset(plain, 0, sizeof(plain));
	memset(cipher, 0, sizeof(cipher));
	mh_tdes_enc(modes[1], cipher, sizeof(cipher), PLAIN1, sizeof(PLAIN1), &KEY, IV, NULL, NULL);
	//cmp cipher
	t = (!memcmp(&cCrypt[1][0], cipher, sizeof(cipher)));
	r_printf(t, "TDES_Encrypt CBC test\n");
	
	mh_tdes_dec(modes[1], plain, sizeof(plain), cipher, sizeof(cipher), &KEY, IV, NULL, NULL);

	//cmp cipher
	t = (!memcmp(plain, PLAIN1, sizeof(PLAIN1)));
	r_printf(t, "TDES_Decrypt CBC test\n");

	
	
}



void test_SHA(void)
{
    mh_rng_callback f_rng = mh_trrand_p;
    
    const char *cc1_160 = "39F37D135FA66858516859BEF7E32F6C40D23640";
    const char *cc1_256 = "346A578A87CFFC251AE7B0FE9ED08607C3537B691CFE7F521F6AD7D5AF015D90";
 

    
    const int inputlen = 1024 * 10;
    int i = 0;
    
 
    unsigned char TestSHA[30] = "012345678901234567890123456789";

    unsigned char c1_160[160 /8] = {0};

    unsigned char c1_256[256 /8] = {0};

    unsigned char exp_c1_160[160 /8] = {0};
    unsigned char exp_c1_256[256 /8] = {0};
    unsigned char *c_512;
    
    dev_debug_printf("\nSHA Test In\n");
    
    bn_read_string_to_bytes(exp_c1_160, sizeof(exp_c1_160), cc1_160);
    bn_read_string_to_bytes(exp_c1_256, sizeof(exp_c1_256), cc1_256);
    
    c_512 = (unsigned char *)k_malloc(10241);
	for(i = 0; i < inputlen; i++)
        c_512[inputlen - i - 1] = i;
	
    
    memset(c1_160, 0, sizeof(c1_160));
    memset(c1_256, 0, sizeof(c1_256));

    mh_sha(SHA_160, c1_160, c_512, inputlen);
	r_printf(0==(memcmp(c1_160, exp_c1_160, sizeof(exp_c1_160))), "SHA c1_160 test\n");
    mh_sha(SHA_256, c1_256, c_512, inputlen);
	r_printf(0==(memcmp(c1_256, exp_c1_256, sizeof(exp_c1_256))), "SHA c1_256 test\n");
  	
    k_free(c_512);
  
    
}



void test_RAND(void)
{
    u8 i,rand[256];
    for(i = 0;i < 5;i++)
    {
		dev_trng_read(rand,256);
		dev_debug_printformat("get rand", rand, 256);
    }
	
}


//arith test
void arith_test(void)
{
	rsa_func_test();
	test_aes();
	test_des();
	test_tdes();
	test_SHA();
	test_RAND();
}

#if 0
void testrf(void)
{
    s32 ret;

	while(1)
	{
	    dev_pcd_powerdown();
		dev_user_delay_ms(20);
		
		dev_pcd_powerup(0x03);

		
		dev_pcd_check_picc(&rf_rxtmp[2], &rf_rxtmp[3], &rlen);

	
}
#endif

/*void test_power(void)
{
    while(1)
    {
        dev_power_switch_ctl(0);
        dev_user_delay_ms(10);
        dev_power_switch_ctl(1);
        dev_user_delay_ms(10);
        
    }
}*/

void testfile(void)
{
	s32 ret;
	u8 *tempdata;
	
	
	tempdata = (u8 *)k_malloc(1024);
	memset(tempdata,0,1024);


	ret = ddi_file_write("/mtd0/test",tempdata, 1024);

	dev_debug_printf("\r\n write file ret = %d\r\n",ret);
	

	ret = ddi_file_getlength("/mtd0/test");
	dev_debug_printf("\r\n file len = %d\r\n",ret);

	
}


void tp_test(void)
{
	strLine lpstrLine;
	strRect lpstrRect;
  	struct ts_sample ts, old_ts;
	s32 ret;
	s32 LcdX, LcdY;
	s32 width,height;


	dev_lcd_full(0);

	old_ts.type = TP_TOUCH_NULL;

	if(dev_if_tp_calibrate() != 1)
	{
		ts_calibrate();
	}
	dev_lcd_full(0);

	
	ddi_elec_sign_close();
//	ddi_elec_sign_get_screen_size(&width,&height);
    ret = ddi_elec_sign_open(0, 0, 128, 64);
	ddi_elec_sign_start();

    while(1)
	{
		dev_user_delay_ms(20);
		ddi_elec_sign_ioctl(1, 0, 0);

	}
}
void dev_test_sleep(void)
{
    u8 dispflg=1;
    s32 ret;
    u32 key;

    
	dev_keypad_pwr_enable(1);
dev_mag_open();
    while(1)
    {
        if(dispflg == 1)
        {
            lcd_interface_clear_ram();
            lcd_interface_fill_rowram(LINE1, 0, "Sleep Test", CDISP);
            lcd_interface_fill_rowram(LINE2, 0, "1.Idle", LDISP);
            lcd_interface_fill_rowram(LINE3, 0, "2.Deepsleep", LDISP);
            lcd_interface_brush_screen();
            dispflg=0;
        }
        if(keypad_interface_read_beep(&key)>0)
        {
dev_debug_printf("%s(%d):key=%02X\r\n", __FUNCTION__, __LINE__, key);            
            switch(key)
            {
            case DIGITAL1:
                ddi_misc_msleep(100);
    			ddi_misc_sleep(0);
                dispflg=1;
                break;
            case DIGITAL2:
                ddi_misc_msleep(100);
    			ddi_misc_sleep(1);
                dispflg=1;
                break;
            case DIGITAL3:
                ret = dev_keypad_read_power();
dev_debug_printf("%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);
                dispflg = 1;
                break;
            case IDLESLEEP:
                dispflg = 1;
                break;
            case ESC:
	            dev_keypad_pwr_enable(1);
                dev_mag_close();
                return;
                break;
            case POWER:
                break;
            }
        }
    }
    
}
#endif
#endif
extern void drv_spi_test(void);
extern void dev_rf_test(void);
extern void emvcl_test(void);
extern s32 mifare_test(u8 block);
extern s32 drv_tample_close(void);

#if 1
uint16_t sinTable[256] = {0};

void GetSinTable(uint32_t max, uint16_t *table, uint32_t size)
{
	uint32_t i = 0;
	float radian, angle;
	
	angle = 360.000 / size;
	
	for (i = 0; i < size; i++)
	{
		radian = angle * i;
		radian = radian * 0.01744;   ///< 弧度=角度*(π/180)
		
		table[i] = (max/2) * sin(radian) + (max/2);
	}
}

void test_adc_value()
{
    s32 i = 0;
    uint16_t adcret;
    uint32_t delay = 0x5FFFFF;


    ADC_InitTypeDef ADC_InitStruct;

    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_ADC, ENABLE);
    SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_ADC, ENABLE);
    
	GPIO_PinRemapConfig(GPIOC, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5, GPIO_Remap_2);
    GPIO_PullUpCmd(GPIOC,GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5,DISABLE);
    
    //ADC channel ChargeVbat是Charge_Vbat管脚；channel1对应PC0，channel2对应PC1，以此类推
    ADC_InitStruct.ADC_Channel = ADC_CHANNEL_2;
    ADC_InitStruct.ADC_SampSpeed = ADC_SpeedPrescaler_2;
    ADC_InitStruct.ADC_IRQ_EN = DISABLE;
    ADC_InitStruct.ADC_FIFO_EN = DISABLE;

    ADC_Init(&ADC_InitStruct);
    ADC_StartCmd(ENABLE);
    ADC_BuffCmd(ENABLE);
    ADC_DivResistorCmd(DISABLE);

    //Delay();


    
    while(delay--);
    for(i = 0; i < 10; i++)
    {
        adcret = ADC_GetResult();

        dev_debug_printf("adc data=%d,vol:%d\r\n", adcret,(adcret*1880)/0xFFF);
    }
}

void dev_adc_test(void)
{
    
    u8 flg=1;
    u32 key;
    u8 tmp[256];
    u16 adcbuff[1];
    s32 adclen = 1;
    u32 alladcval = 0;
    s32 i;
    s32 ret;

    while(1)
    {
        if(flg==1)
        {
            dev_debug_printf("================ADC TEST===============\n");
            dev_debug_printf("1.ADC open\n");
			dev_debug_printf("2.GET ADCVal\n");
            dev_debug_printf("3.ADC close\n");
			flg = 0;
        }
        if(keypad_interface_read_beep(&key)>0)
        {
//printf("key=%02X\r\n", key);  
            switch(key)
            {
            case DIGITAL1:

                //test_adc_value();
                ret = dev_adc_open(DEV_ADC_HWVER);
                dev_debug_printf("dev_adc_open ret:%d\r\n", ret);
               break;
            case DIGITAL2:

                #if 1
                #if 0
                alladcval = 0;
                for(i = 0; i < 10; i++)
                {
                    memset(adcbuff, 0x0, adclen);
                    ret = dev_adc_get_value(DEV_ADC_HWVER,adcbuff,adclen);
                    //adcbuff[0] = ADC_GetResult();
                    dev_debug_printf("adc hw data=%d,vol:%d\r\n", adcbuff[0],(adcbuff[0]*1880)/0xFFF);
                    alladcval += adcbuff[0];
                }

                dev_debug_printf("aver hw vol=%d\r\n", (alladcval/10)*1880/0xFFF);
                #endif
                alladcval = 0;
                //dev_adc_open(DEV_ADC_OVP);
                for(i = 0; i < 10; i++)
                {
                    memset(adcbuff, 0x0, adclen);
                    ret = dev_adc_get_value(DEV_ADC_HWVER,adcbuff,adclen);
                    //adcbuff[0] = ADC_GetResult();
                    dev_debug_printf("adc ovp data=%d,vol:%d\r\n", adcbuff[0],(adcbuff[0]*1880*2)/0xFFF);
                    alladcval += adcbuff[0];
                }

                dev_debug_printf("aver ovp vol=%d\r\n", (alladcval/10)*1880/0xFFF);
                //dev_adc_close(DEV_ADC_OVP);
                #else
                memset(adcbuff, 0x0, adclen);
                ret = dev_adc_get_value(DEV_ADC_HWVER,adcbuff,adclen);

                dev_debug_printf("adc data=%d\r\n", (adcbuff[0]*1880)/0xFFF);
                #endif
                break;
            case DIGITAL3:
                ret = dev_adc_close(DEV_ADC_HWVER);
                dev_debug_printf("dev_adc_close ret:%d\r\n", ret);

                break;
            case ESC:
                return;
            case POWER:
                break;
            }
        }
    }
}

#define MAX_TTS_NUM 12

static s8 tts_text[MAX_TTS_NUM][100] = 
{
    "设备已联网成功",
    "你有新订单了",
    "设备升级中,请勿拔掉电源",
    "升级成功",
    "已进入wifi配网模式",
    "已进入wifi网页配置模式",
    "设备配置超时,退出配置模式",
    "音量加",
    "音量最大",
    "音量减",
    "正在恢复出厂设置,请勿拔掉电源",
    "恢复出厂成功",
};

void dac_test(void)
{
    s32 ret = -1;
    u8 flg = 1;
    u32 key;
    s32 vol_level = 0;
    str_audio_tts audio_tts;

    u8 test_buf[462] = 0;
    u8 vol_cur;
    s32 i = 0;
    u8  is_playing = 0;


    while(1)
    {
        if(flg == 1)
        {
            dev_debug_printf("================DAC Test===============\n");
            dev_debug_printf("1.DAC open\n");
            dev_debug_printf("2.DAC play\n");
            dev_debug_printf("3.DAC close\n");
            dev_debug_printf("4.DAC pause\n");
            dev_debug_printf("5.DAC replay\n");
            dev_debug_printf("6.DAC KEY +\n");
            dev_debug_printf("7.DAC KEY -\n");

            dev_debug_printf("================DAC End=================\n");
            flg = 0;

        }
        if(keypad_interface_read_beep(&key)>0)
        {
            switch(key)
            {
                case DIGITAL1:
                    ret = dev_audio_open();
                    
                //audio_tts.m_text = "好的 请输入金额圆";
                //ret = dev_audio_playcharacters(audio_tts.m_text);
                    break;
                case DIGITAL2:
                    //audio_tts.m_text = "请输入密码撤销成功 好的 ";
                    //audio_tts.m_text = "设备配置超时，退出配置模式";
                    audio_tts.m_text = tts_text[i];
                    i++;
                    if(i>=MAX_TTS_NUM)
                    {
                        i = 0;
                    }
                    ret = dev_audio_playcharacters(1, audio_tts.m_text);

                    dev_debug_printf("dev_dac_play ret:%d\r\n",ret);
                    break;
                case DIGITAL3:
                    ret = dev_audio_close();
                    dev_debug_printf("dev_dac_close ret:%d\r\n",ret);
                    break;
                case DIGITAL4:
                    ret = dev_audio_pause();
                    dev_debug_printf("dev_dac_pause ret:%d\r\n",ret);
                    break;
                case DIGITAL5:
                    ret = dev_audio_replay();
                    dev_debug_printf("dev_dac_replay ret:%d\r\n",ret);
                    break;
                case DIGITAL6:
                    
                    vol_level = dev_audio_get_volume();
                    ret = dev_dac_set_volume_level(vol_level + 1);
                    dev_debug_printf("dev_dac_set_volume_level ret:%d\r\n",ret);
                    break;
                case DIGITAL7:
                    vol_level = dev_audio_get_volume();
                    ret = dev_audio_set_volume(vol_level - 1);
                    dev_debug_printf("dev_dac_set_volume_level ret:%d\r\n",ret);
                    break;
                case DIGITAL8:
                    dev_audio_ioctl(DDI_AUDIO_CTL_GET_STATUS, (u32)&is_playing, 0);
                    dev_debug_printf("is_playing:%d\r\n",is_playing);
                    //dev_flash_read(0x00101000+55360, test_buf, sizeof(test_buf));
                    //dev_debug_printf("data:addr=%08X, len=%d\r\n",0x00101000+55360, sizeof(test_buf));
                    //dev_debug_printformat(NULL, test_buf, sizeof(test_buf));
                    break;

                case ESC:
                    return ;

            }
        
        }

    }


}
#endif
const u8 test_tab[] = { 0x21, 0x00,0x02,0x00,0x00,0x1B,0x00,0x02,0xD1,
                        0xD1,0x07,0x00,0x1E,0x00,0x06,0x00,0x16,
                        0x9F,0x81,0xD4,0x75,0x83,0x10,0x72,0x95,
                        0x60,0x7E,0x2B,0x8A,0x7C,0xEF,0x8D,0x6A,
                        0x03
                        };
const u8 test_tab1[] = { 0x1D, 0x00,0x00,0x94,0x00,0x17,0x00,0x03,0xA1,
                        0xD8,0x01,0x01,0x01,0x31,0x14,0xC7,0xEB,
                        0xCB,0xA2,0xBF,0xA8,0xB2,0xE5,0xBF,0xA8,
                        0xBB,0xD3,0xBF,0xA8,0x03
                        };
void test_crc16_buypass(void)
{
    u16 crc;
    u8 crcret[2];
    
    crc = dev_maths_calcCRC16_buypass(0x0000, &test_tab[1], test_tab[0], crcret);
dev_debug_printf("%s(%d):crc=%04X, crc=%02X,%02X\r\n", __FUNCTION__, __LINE__, crc, crcret[0], crcret[1]);
//    crc = dev_maths_Crc16CCITT(&test_tab[1], test_tab[0], crcret);
//dev_debug_printf("%s(%d):crc=%04X, crc=%02X,%02X\r\n", __FUNCTION__, __LINE__, crc, crcret[0], crcret[1]);
    
    crc = dev_maths_calcCRC16_buypass(0x0000, &test_tab1[1], test_tab1[0], crcret);
dev_debug_printf("%s(%d):crc=%04X, crc=%02X,%02X\r\n", __FUNCTION__, __LINE__, crc, crcret[0], crcret[1]);
//    crc = dev_maths_Crc16CCITT(&test_tab1[1], test_tab1[0], crcret);
//dev_debug_printf("%s(%d):crc=%04X, crc=%02X,%02X\r\n", __FUNCTION__, __LINE__, crc, crcret[0], crcret[1]);
}
s32 testmain(void)
{   
    u8 flg=1;
    u32 key;
	SYSCTRL_ClocksTypeDef  clocks;
    u32 timeid;
	unsigned char buf[] = "\n啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui\n\n啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui\n\n啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui\n\n啊啊啊啊啊啊啊啊啊啊国国国   1234567  aqweertyuyuui\n";
	unsigned char buf1[] = {0x1B, 0x2D, 0x01};
    
    drv_gprs_io_init();
    dev_user_delay_ms(1000);
//    drv_gprs_io_VBAT_ctl(1);
//    dev_test_gprs1("112");
    #if 0
    while(1)
    {
        #if 1
        test_spiflash();
        //dev_user_delay_ms(1000);
        #else
        dev_gpio_direction_output(GPIO_PIN_PTA7, 1);
        dev_user_delay_ms(10);
        dev_gpio_direction_output(GPIO_PIN_PTA7, 0);
        dev_user_delay_ms(10);
        dev_gpio_direction_output(GPIO_PIN_PTA7, 1);
        #endif
    }
    #endif
    while(1)
    {
        if(flg==1)
        {
            flg = 0;
            dev_debug_printf("================Test MainMenu===============\n");
            dev_debug_printf("0.GPIO Test\n");
            dev_debug_printf("1.UART Test\n");
            dev_debug_printf("2.FLASH Test\n");
            dev_debug_printf("3.DAC Test\n");
            dev_debug_printf("4.ADC Test\n");
            dev_debug_printf("5.WIFI Test\n");
            dev_debug_printf("6.GPRS Test\n");
            dev_debug_printf("7.KEYTEST Test\n");
            dev_debug_printf("8.printer Test\n");
            dev_debug_printf("9.RTC Test\n");
            
            dev_debug_printf("p.print Menu\n");
            dev_debug_printf("====================Menu End=================\n");
            test_crc16_buypass();
            timeid = dev_user_gettimeID();
            dev_user_delay_ms(200);
        }
      #if 0
        if(dev_user_querrytimer(timeid, 1000))
        {
            timeid = dev_user_gettimeID();
            dev_debug_printf("%s(%d):\n", __FUNCTION__, __LINE__);
            dev_misc_cashbox_open();
        }
      #endif
     #if 1  
        if(keypad_interface_read_beep(&key)>0)
        {
dev_debug_printf("key=%02X\r\n", key);   
            switch(key)
            {     
            case DIGITAL0:
                gpio_test();
                //dev_gpio_test();
              #if(PRINTER_EXIST==1)  
               // TPSelfTest_4();
              #endif
                break;
            case DIGITAL1:
                //uart_test();
                dev_uart_test();
                break;
            case DIGITAL2:
                //dev_flash_test(0x10000);
                dev_flash_test(SPIFLASH_ADDRSTA);
                break;

            case DIGITAL3:
                dac_test();
                //dev_rf_test();
                //dev_flash_test(SPIFLASH_ADDRSTA);
                break;
            case DIGITAL4:
                dev_adc_test();
                break;
            case DIGITAL5:
                dev_test_wifi();
                flg = 1;
                break;
            case DIGITAL6:
                dev_test_gprs();
                flg = 1;
                break;
            case DIGITAL7:
                dev_key_test();
                //dev_test_key();
                //dev_test_gprs1("13632651039");
                flg = 1;
                break;

            case DIGITAL8:
                //dev_test_key();
                #ifdef TRENDIT_CORE
                int num = 60;
                #if 1
                esc_init();
                #else
                pt48d_dev_init();
                PrtSetSpeed(3);
                dev_debug_printf("printer11\n");

                
                esc_p_init();
                dev_debug_printf("printer22\n");
               // set_data_test(buf1, sizeof(buf1));
                //set_data_test(buf2, sizeof(buf2));
                #endif
                esc_set_data(buf, sizeof(buf));
                dev_debug_printf("printer33\n");
                #if 1
                    dev_debug_printf("prt 200 line\r\n");
                    PrtFeedStart(200);
                #else
                while(num--)
                {
                    if(esc_fifo_isEmpty())
                    {
                        break;
                    }
                    
                    esc_p();
                    dev_debug_printf("printer44\n");
                }
             //   TPSelfTest_1();
                //TPSelfTest_2();
                #endif
                #endif
                flg = 1;

                break;

      #if 0  
            case DIGITAL8:
                dev_led_test();
                flg = 1;
                break;
            case DIGITAL9:
                //dev_lcd_test();
                //dev_flash_test(SPIFLASH_ADDRSTA);
                dev_bt_test();
                flg = 1;
                break;
         #endif       
            case DIGITAL9:
                dev_led_test();
                #ifdef TRENDIT_CORE
                Test_rtc();
                #endif
                flg = 1;
                break;
            case ESC:
                return 0;
            case POWER:
                //dev_power_switch_ctl(0);
                break;
            }
        }
     #endif
    }
}





