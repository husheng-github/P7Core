/********************************Copyright ( c ) ********************************
**                  DAQU 
** 						
**                        
**
**Created By:		 
** Created Date:	 
** Version:		
** Description:	该文件包含LCD的驱动相关接口
  
****************************************************************************/
#include "devglobal.h"
//#include "sdk/mhscpu_sdk.h"
#include "drv_lcd.h"

#define LCD_PIN_MAX   3
const str_lcd_pin_t str_lcd_pin_tab[LCD_PIN_MAX] = 
{
  #ifdef MACHINE_M6  
    //A0
    {GPIO_PIN_PTB12, 0},
    //RST
   #if 0 
    {GPIO_PIN_PTD5, 0},
//     {GPIO_PIN_PTB3, 0},
   #else
    {GPIO_PIN_NONE, 0},              //sxl
   #endif
    //BACKLIGHT
    {GPIO_PIN_PTC12, 0},
  #else
    //A0
    {GPIO_PIN_PTC9, 0},//{GPIO_PIN_PTD4, 0}, 
    //RST
   #if 0 
    {GPIO_PIN_PTD5, 0},
//     {GPIO_PIN_PTB3, 0},
   #else
    {GPIO_PIN_NONE, 0},              //sxl
   #endif
    //BACKLIGHT
    {GPIO_PIN_PTB1, 0},
  #endif
};
#define LCD_PIN_A0    0
#define LCD_PIN_RST   1
#define LCD_PIN_BL    2

/****************************************************************************
**Description:	    LCD管脚初始化
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170526
**---------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void drv_lcd_pininit(void)
{
    s32 i;
    static u8 flg =0;

    if(0 == flg)
    {
        flg = 1;
        for(i=0; i<LCD_PIN_MAX; i++)
        {
            if(str_lcd_pin_tab[i].m_gpiopin != GPIO_PIN_NONE)
            {
                dev_gpio_config_mux(str_lcd_pin_tab[i].m_gpiopin, MUX_CONFIG_GPIO);
                dev_gpio_set_pad(str_lcd_pin_tab[i].m_gpiopin, PAD_CTL_PULL_NONE);
                dev_gpio_direction_output(str_lcd_pin_tab[i].m_gpiopin, str_lcd_pin_tab[i].m_rstval);
            }
        }
        dev_spi_open(SPI_DEV_LCD);
    }
}

/****************************************************************************
**Description:	    控制A0
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170526
**--------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void drv_lcd_A0_ctl(u8 flg)
{
    dev_gpio_set_value(str_lcd_pin_tab[LCD_PIN_A0].m_gpiopin, flg);
}


#if(LCD_EXIST==1)




/****************************************************************************
**Description:	    LCD Reset
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170526
**--------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static void drv_lcd_reset(void)
{
    if(str_lcd_pin_tab[LCD_PIN_RST].m_gpiopin != GPIO_PIN_NONE)
    {
        dev_gpio_set_value(str_lcd_pin_tab[LCD_PIN_RST].m_gpiopin, 0);
        dev_user_delay_us(10000);
        dev_gpio_set_value(str_lcd_pin_tab[LCD_PIN_RST].m_gpiopin, 1);
        dev_user_delay_us(10000);
    }
}
/****************************************************************************
**Description:	    向LCD发关一个命令
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170526
**--------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static void drv_lcd_send_cmd(u8 cmd)
{
//    drv_lcd_A0_ctl(0);
    dev_spi_master_transceive_polling(SPI_DEV_LCD, &cmd, NULL, 1);
//    drv_lcd_A0_ctl(1);
}
/****************************************************************************
**Description:	    软件复位LCD
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170526
**--------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static void drv_lcd_softreset(void)
{
    drv_lcd_send_cmd(__DISPLAY_RESET); 
}
/****************************************************************************
**Description:	    LCD进入省电模式
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170526
**--------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static void drv_lcd_pwr_sv(void)
{
    drv_lcd_send_cmd(__DISPLAY_OFF);
    drv_lcd_send_cmd(__DISPLAY_ALL_PIXEL);
}
/****************************************************************************
**Description:	    LCD退出省电模式
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170526
**-------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
static void drv_lcd_pwr_nrml(void)
{
    drv_lcd_send_cmd(__DISPLAY_NOR_PIXEL);
    drv_lcd_send_cmd(__DISPLAY_ON);
}

/****************************************************************************
**Description:	    向LCD发关一个数据
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170526
**--------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void drv_lcd_send_byte(u8 data)
{
    drv_lcd_A0_ctl(1);
    dev_spi_master_transceive_polling(SPI_DEV_LCD, &data, NULL, 1);
    drv_lcd_A0_ctl(0);
}
/****************************************************************************
**Description:	    设置LCD显示地址
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170526
**-------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void drv_lcd_set_ram_addr(u8 page, u8 col)
{
    u8 tmpval;


    if( (page<LCD_MAX_ROW) && (col<LCD_MAX_COL))
    {
     
      //发送页地址
      tmpval = 0x0f & page;
      tmpval |= __PAGE_ADDR;
      drv_lcd_send_cmd(tmpval);
      
      //发送列地址高字节
      tmpval = 0x0f & (col >> 4);
      tmpval |= __COL_ADDR_HI;
      drv_lcd_send_cmd(tmpval);
      
      //发送列地址低字节
      tmpval = 0x0f & col;
      tmpval |= __COL_ADDR_LO;
      drv_lcd_send_cmd(tmpval);
    }
}

/****************************************************************************
**Description:	    设置LCD对比度
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170526
**--------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void drv_lcd_set_contrast(u8 contrast)
{
    drv_lcd_send_cmd(0x81);
    drv_lcd_send_cmd(contrast);
}
/****************************************************************************
**Description:	    LCD初始化
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170526
**--------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void drv_lcd_init_12864(void)
{
	#if 0
	drv_lcd_send_cmd(0xE2);//Software reset
	drv_lcd_send_cmd(0xA2); //Bias select a2
	drv_lcd_send_cmd(0xC8); //COM deriction
	drv_lcd_send_cmd(0xA0); //SEG deriction
	drv_lcd_send_cmd(0x2C); //Power control
	drv_lcd_send_cmd(0x2E);
	drv_lcd_send_cmd(0x2F);

	//wcomspi(0xF8); //Set booster
	//wcomspi(0x00);

	drv_lcd_send_cmd(0x24);//0x24); //V0 control
	drv_lcd_send_cmd(0x81);
	drv_lcd_send_cmd(0x28);//0x28);	 //28:9.0V

	drv_lcd_send_cmd(0x40); //Set start line
	drv_lcd_send_cmd(0xAF);	//display on
	#else
    drv_lcd_send_cmd(__DISPLAY_OFF);              //
    drv_lcd_send_cmd(__DISPLAY_RESET);
    dev_user_delay_us(10);
    
    drv_lcd_send_cmd(0x2f);
    dev_user_delay_us(10);
    drv_lcd_send_cmd(0xf8);
    drv_lcd_send_cmd(0x00);
    drv_lcd_send_cmd(__CONT_CTRL);  //0x81
    drv_lcd_send_cmd(0x26);
    drv_lcd_send_cmd(0xa2);
  #ifdef LCD_OVERTURN_EN   
    drv_lcd_send_cmd(0xa1);		//显示屏左右翻转
  #else
    drv_lcd_send_cmd(0xa0);
  #endif
    drv_lcd_send_cmd(0xa6);
    drv_lcd_send_cmd(0xc8);
    drv_lcd_send_cmd(0xa4);
    drv_lcd_send_cmd(__DISPLAY_ON);
    drv_lcd_set_contrast(0x2A);
    #endif
	
}
#if 0
void drv_lcd_test_set(u8 val)
{
	drv_lcd_send_cmd(0x81);
	drv_lcd_send_cmd(val);	 //28:9.0V
}
#endif
void drv_lcd_init_12832(void)
{
    //M5的屏的参数
    drv_lcd_send_cmd(0xe2);
    drv_lcd_send_cmd(0xa2);	/* 1/6 bias */
    drv_lcd_send_cmd(0xa0);	/* ADC select , Normal */
    drv_lcd_send_cmd(0xc8);	/* Common output reverse */
    drv_lcd_send_cmd(0xa6);	/* normal display 1=on */
    drv_lcd_send_cmd(0x2c);
    drv_lcd_send_cmd(0x2e);
    drv_lcd_send_cmd(0x2f);	/* V/C off, V/R off, V/F on */

    drv_lcd_send_cmd(0xf8);	/***5 booster***/
    drv_lcd_send_cmd(0x00);
    drv_lcd_send_cmd(0x24);	/* internal resistor ratio */
    drv_lcd_send_cmd(0x81);	/* electronic volume mode set */
    drv_lcd_send_cmd(0x12);	/* electronic volume */
    drv_lcd_send_cmd(0x40);	/* display start first line */
    drv_lcd_send_cmd(0xa4);
    drv_lcd_send_cmd(0xaf);
    dev_user_delay_us(10);
    
}
void drv_lcd_init(void)
{
	
    drv_lcd_pininit();
//    drv_lcd_A0_ctl(1);

    drv_lcd_reset();
    drv_lcd_init_12864();
    
}

/****************************************************************************
**Description:	    
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170526
**--------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void drv_lcd_disp_on(void)
{
    drv_lcd_send_cmd(0xa4); //2f
    drv_lcd_send_cmd(0xaf); 
}

void drv_lcd_disp_off(void)
{
    drv_lcd_send_cmd(0xae);
    drv_lcd_send_cmd(0xa5); //28
    drv_lcd_A0_ctl(0);
}
/****************************************************************************
**Description:	    控制LCD背光
**Input parameters:	
**Output parameters: 
**Returned value:
**Created by:		pengxuebin,20170526
**--------------------------------------------------------------------------
** Modified by:	
****************************************************************************/
void drv_lcd_bl_ctl(u8 status)
{
    dev_gpio_set_value(str_lcd_pin_tab[LCD_PIN_BL].m_gpiopin, status);
}
#else
void drv_lcd_bl_ctl(u8 status)
{
    
}
#endif
