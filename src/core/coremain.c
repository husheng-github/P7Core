#include "devglobal.h"
#ifdef TRENDIT_CORE
typedef void (*function)(void);

void jump_app_main(void)
{
    void (*go)(void);
    u32 addr;
dev_debug_printf("");
    addr = *((unsigned int *)(APP_CORE_START+MCU_BASE_ADDR));
dev_debug_printf("addr=%08X\r\n", (u32)addr);
dev_user_delay_ms(100);

    go = (void (*)(void))(addr);
    go();
}


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


#include "sdk/mhscpu_sdk.h"

#ifdef TRENDIT_CORE
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

    NVIC_SetVectorTable(MCU_BASE_ADDR, CORE_START_ADDR);

    dev_sys_init();

    //dev_power_switch_ctl(1);//锁电

    
//testmain();
    //dev_key_test();

//while(1);
//    ddi_watchdog_open();
    //跳转到应用
    jump_app_main();
    while(1);
}

#endif



