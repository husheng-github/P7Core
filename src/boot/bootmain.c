
#define AFX_BOOT_VARIABLES
#include "bootglobal.h"
#undef AFX_BOOT_VARIABLES


#ifdef TRENDIT_BOOT

typedef void (*function)(void);


void jump_core_main(void)
{
    dev_com_tcdrain(DEBUG_PORT_NO);
    //关所有中断
	__disable_irq();
    __disable_fault_irq(); 
    dev_sys_NVIC_CloseAllIrq();//sxl?2018
	
     __set_MSP(*(volatile u32*)(COREBINADDR+MCU_BASE_ADDR));     //file is right,then go to run it; 
    (*(function *)(COREBINADDR+MCU_BASE_ADDR+0x04))();//跳到CORE的入口地址
    
}
#if 0
void dev_test_app_file(void)
{
    u8 tmp[512];
    s32 i;
    u8 *ap;

    //ap = COREBINADDR;
    dev_flash_read(COREBINADDR, tmp, 512);
    dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
    dev_debug_printformat(NULL, tmp, 512);
    
    ap = (u8*)(COREBINADDR+MCU_BASE_ADDR);
    memset(tmp, 0, 512);
    for(i=0; i<512; i++)
    {
        tmp[i] = ap[i];
    }
    dev_debug_printf("%s(%d):\r\n", __FUNCTION__, __LINE__);
    dev_debug_printformat(NULL, tmp, 512);
    
}
#endif
s32 boot_main_checknecessaryfile(void)
{  
  #ifdef SXL_DEBUG
    u8 *gSpStatusdata;
	#endif
	
    
	//校验ACQUIRE
    if(pcipkmanage_readpkdata(PK_ACQUIRER,NULL) == 0)
    {
    	gSpStatus.spsecurestatus_acquirerpkstatus = 0;
    }
	else
	{
		gSpStatus.spsecurestatus_acquirerpkstatus = 1;
	}
    if((gSpStatus.spsecurestatus_trenditpkstatus!=0)
     ||(gSpStatus.spsecurestatus_corestatus !=0)
     ||(gSpStatus.spsecurestatus_appstatus !=0)
     ||(gSpStatus.spsecurestatus_acquirerpkstatus !=0))
    {
        BOOT_DEBUG("Check err!(%d, %d, %d, %d)\r\n", gSpStatus.spsecurestatus_trenditpkstatus, 
            gSpStatus.spsecurestatus_corestatus, gSpStatus.spsecurestatus_appstatus, gSpStatus.spsecurestatus_acquirerpkstatus);
        return -1;      //检验不通过
    }
    else
    {
        return 0;       //检验通过
    }
}
#if 0
void boottest(void)
{
    u8 tmp[2048];
    s32 i;
    u32 addr;
    s32 ret;

    //firewareinfo
    BOOT_DEBUG("\r\n", ret);
    dev_user_delay_ms(3000);
    addr = 0x70000;
    ret = dev_flash_read(addr, tmp, 1024);
    BOOT_DEBUG("firewareinfo.ret=%d\r\n", ret);
    if(ret >= 0)
    {
        BOOT_DEBUGHEX(NULL, tmp, 1024);
    }
    //core
    addr = 0x18000;
    ret = dev_flash_read(addr, tmp, 256);
    BOOT_DEBUG("core.ret=%d\r\n", ret);
    if(ret >= 0)
    {
        BOOT_DEBUGHEX(NULL, tmp, 256);
    }
    //app
    addr = 0x48000;
    ret = dev_flash_read(addr, tmp, 256);
    BOOT_DEBUG("app.ret=%d\r\n", ret);
    if(ret >= 0)
    {
        BOOT_DEBUGHEX(NULL, tmp, 256);
    }
    //trendpk
    addr = 0x74000;
    ret = dev_flash_read(addr, tmp, 0x680);
    BOOT_DEBUG("trendpk.ret=%d\r\n", ret);
    if(ret >= 0)
    {
        BOOT_DEBUGHEX(NULL, tmp, 0x680);
    }
    //acquirerpk
    addr = 0x76000;
    ret = dev_flash_read(addr, tmp, 0x680);
    BOOT_DEBUG("acquirerpk.ret=%d\r\n", ret);
    if(ret >= 0)
    {
        BOOT_DEBUGHEX(NULL, tmp, 0x680);
    } 
}
#endif



#if 0
void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
    
    GPIO_Init(GPIOD, &GPIO_InitStruct);
    
}
void test_gpio_pin(void)
{
    s32 i;

    while(1)
    {
        GPIO_SetBits(GPIOD, GPIO_Pin_9);
        for(i=0; i<10000; i++);
        GPIO_ResetBits(GPIOD, GPIO_Pin_9);
        for(i=0; i<10000; i++);
    }
}
#endif
void boot_main_set_latch_power(void)
{
    s32 timecnt = 0;
    s32 ret = 0;

    ret = dev_misc_reset_get();
    if(ret > 0)
    {
        dev_misc_reset_set_to_flash(ret);
        
        //复位, 不需要等待按键
        dev_misc_reset_set(0);
    }
    else
    {
        while(1)
        {
            dev_user_delay_ms(10);
            if(drv_keypad_get_powerkey() == 1)
            {
                break;
                timecnt++;
                if(timecnt > 0)
                {
                    break;
                }
            }
            else
            {
                timecnt = 0;
            }
        }
    }

    dev_power_switch_ctl(1);//锁电

    dev_led_sta_set(LED_STATUS_B, 1);//亮蓝灯
    
}

#include "sdk/mhscpu_sdk.h"

#ifndef TRENDIT_CORE
// hard fault handler in C,  
// with stack frame location as input parameter  
void hard_fault_handler_c(unsigned int * hardfault_args)  
{  
    unsigned int stacked_r0;  
    unsigned int stacked_r1;  
    unsigned int stacked_r2;  
    unsigned int stacked_r3;  
    unsigned int stacked_r12;  
    unsigned int stacked_lr;  
    unsigned int stacked_pc;  
    unsigned int stacked_psr;  

    //dev_led_ctl(0,1);
    //dev_led_ctl(1,1);
    //dev_led_ctl(2,1);
    //dev_led_ctl(3,1);
    __disable_fiq();
    __disable_irq(); //sxl2020604 关闭中断
    stacked_r0 = ((unsigned long) hardfault_args[0]);  
    stacked_r1 = ((unsigned long) hardfault_args[1]);  
    stacked_r2 = ((unsigned long) hardfault_args[2]);  
    stacked_r3 = ((unsigned long) hardfault_args[3]);  
     
    stacked_r12 = ((unsigned long) hardfault_args[4]);  
    stacked_lr = ((unsigned long) hardfault_args[5]);  
    stacked_pc = ((unsigned long) hardfault_args[6]);  
    stacked_psr = ((unsigned long) hardfault_args[7]);  

    dev_misc_reboot(ST_HARD_FAULT);
#if 0
    //UART0->OFFSET_0.THR = 0x55;
    //UART0->OFFSET_0.THR = 0x55;
    //UART0->OFFSET_0.THR = 0x55;
    //UART0->OFFSET_0.THR = 0x55;

    //UART0->OFFSET_0.THR = (u8)(stacked_r0>>24);
    //UART0->OFFSET_0.THR = (u8)(stacked_r0>>16);
    //UART0->OFFSET_0.THR = (u8)(stacked_r0>>8);
    //UART0->OFFSET_0.THR = (u8)(stacked_r0);


    //UART0->OFFSET_0.THR = (u8)(stacked_r1>>24);
    //UART0->OFFSET_0.THR = (u8)(stacked_r1>>16);
    //UART0->OFFSET_0.THR = (u8)(stacked_r1>>8);
    //UART0->OFFSET_0.THR = (u8)(stacked_r1);


    //UART0->OFFSET_0.THR = (u8)(stacked_r2>>24);
    //UART0->OFFSET_0.THR = (u8)(stacked_r2>>16);
    //UART0->OFFSET_0.THR = (u8)(stacked_r2>>8);
    //UART0->OFFSET_0.THR = (u8)(stacked_r2);


    //UART0->OFFSET_0.THR = (u8)(stacked_r3>>24);
    //UART0->OFFSET_0.THR = (u8)(stacked_r3>>16);
    //UART0->OFFSET_0.THR = (u8)(stacked_r3>>8);
    //UART0->OFFSET_0.THR = (u8)(stacked_r3);

    //UART0->OFFSET_0.THR = (u8)(stacked_r12>>24);
    //UART0->OFFSET_0.THR = (u8)(stacked_r12>>16);
    //UART0->OFFSET_0.THR = (u8)(stacked_r12>>8);
    //UART0->OFFSET_0.THR = (u8)(stacked_r12);
    

    UART0->OFFSET_0.THR = (u8)(stacked_lr>>24);
    UART0->OFFSET_0.THR = (u8)(stacked_lr>>16);
    UART0->OFFSET_0.THR = (u8)(stacked_lr>>8);
    UART0->OFFSET_0.THR = (u8)(stacked_lr);

    UART0->OFFSET_0.THR = (u8)(stacked_pc>>24);
    UART0->OFFSET_0.THR = (u8)(stacked_pc>>16);
    UART0->OFFSET_0.THR = (u8)(stacked_pc>>8);
    UART0->OFFSET_0.THR = (u8)(stacked_pc);


    UART0->OFFSET_0.THR = (u8)(stacked_psr>>24);
    UART0->OFFSET_0.THR = (u8)(stacked_psr>>16);
    UART0->OFFSET_0.THR = (u8)(stacked_psr>>8);
    UART0->OFFSET_0.THR = (u8)(stacked_psr);
    
    //dev_debug_printf_hardfault ("LR = %x\n", stacked_lr);  
    //dev_debug_printf_hardfault ("PC = %x\n", stacked_pc);  
    dev_debug_printf_hardfault("[Hard fault handler]\n");  
    dev_debug_printf_hardfault("R0 = %x\n", stacked_r0);  
    dev_debug_printf_hardfault("R1 = %x\n", stacked_r1);  
    dev_debug_printf_hardfault ("R2 = %x\n", stacked_r2);  
    dev_debug_printf_hardfault ("R3 = %x\n", stacked_r3);  
    dev_debug_printf_hardfault ("R12 = %x\n", stacked_r12);  
    dev_debug_printf_hardfault ("LR = %x\n", stacked_lr);  
    dev_debug_printf_hardfault ("PC = %x\n", stacked_pc);  
    dev_debug_printf_hardfault ("PSR = %x\n", stacked_psr);  
    dev_debug_printf_hardfault ("BFAR = %x\n", (*((volatile unsigned long *)(0xE000ED38))));  
    dev_debug_printf_hardfault ("CFSR = %x\n", (*((volatile unsigned long *)(0xE000ED28))));  
    dev_debug_printf_hardfault ("HFSR = %x\n", (*((volatile unsigned long *)(0xE000ED2C))));  
    dev_debug_printf_hardfault ("DFSR = %x\n", (*((volatile unsigned long *)(0xE000ED30))));  
    dev_debug_printf_hardfault ("AFSR = %x\n", (*((volatile unsigned long *)(0xE000ED3C))));  
    
    while(1)
    {
            ;;
    } 
#endif
}  
#endif

int main(void)
{
    u8 downloadflag = 0;
    u32 boottimer;
	u8 data;
	u8 cnt = 0;
    u32 key;
    s32 ret;
	
    NVIC_SetVectorTable(MCU_BASE_ADDR, BOOT_START_ADDR);

    dev_sys_init();
    dev_led_sta_set(0, 1);
    //testmain();
    //boot_main_set_latch_power();

    //判断更新(updata_deal)
    bootupdata_deal_withoutdisplay();
#if 0    
    fs_system_init();   //文件系统要打印调试信息等，要移到后面来
    pci_core_spstatusinit();
    gSpStatus.spstatusmachine = SPSTATUSMACHINE_BOOTSTART;
    pcipkmanage_checkmainpk();
    bootdownload_readappinfo();

    if(bootupdata_deal())
    {
        gSpStatus.spsecurestatus_updateflag = 1;
        gSpStatus.spstatusmachine = SPSTATUSMACHINE_BOOTDOWNLOAD;
    }
    #if 1//def SXL_DEBUG
    BOOT_DEBUG("check if need download\r\n");
    #endif
    while(gSpStatus.spstatusmachine == SPSTATUSMACHINE_BOOTSTART
        ||gSpStatus.spstatusmachine == SPSTATUSMACHINE_BOOTDOWNLOAD)
    {
        boot_main_checknecessaryfile();
        BOOT_DEBUG("spstatus=(%d,%d,%d,%d)\r\n", gSpStatus.spstatusmachine,gSpStatus.spsecurestatus_trenditpkstatus,
                         gSpStatus.spsecurestatus_appstatus,gSpStatus.spsecurestatus_corestatus);        
        //检查是否进入下载
        if(gSpStatus.spstatusmachine != SPSTATUSMACHINE_BOOTDOWNLOAD)
        {
            //acquirer.pk不存在也可以启动,acquirer.pk用作防切机
            if(gSpStatus.spsecurestatus_trenditpkstatus||gSpStatus.spsecurestatus_appstatus||gSpStatus.spsecurestatus_corestatus)
            {
                gSpStatus.spstatusmachine = SPSTATUSMACHINE_BOOTDOWNLOAD;
            }
            
        }
        
        //机型号不能识别
        if(gSpStatus.spstatusmachine == SPSTATUSMACHINE_BOOTDOWNLOAD)  //进入下载状态的话不
        {
//BOOT_DEBUG("DOWNLOAD\r\n");
//dev_user_delay_ms(1000);

            bootdownload_init();
            bootdownload_task();
            bootdownload_exit();
            
            //重启
            dev_smc_systemsoftreset();
                
        }
        
        
        if(gSpStatus.spstatusmachine == SPSTATUSMACHINE_BOOTSTART)
        {
            break;
        }
     } 
#endif
//testmain();
//dev_test_app_file();

//dev_user_delay_ms(300);

    //跳转到应用
    jump_core_main();
    while(1);
}

#endif



