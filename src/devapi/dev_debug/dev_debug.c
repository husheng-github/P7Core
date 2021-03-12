/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:         
** Created Date:     
** Version:        
** Description:    调试程序接口
****************************************************************************/
//#include <string.h>
//#include <stdio.h>
//#include "sdk\mhscpu_sdk.h"
#include "devglobal.h"
#if 0
int fputc(int c, FILE *f)
{
#ifdef DEBUG_ALL_EN
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART */
    u8 tmp[2];
    s32 i;
    i = 0;
    if(c =='\n')
    {
        tmp[i++] = '\r';
    }
    tmp[i++] = (u8)c;
    dev_com_write(DEBUG_PORT_NO, tmp, i);
#endif
    return c;
}
#endif

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif


#ifdef DEBUG_ALL_EN
void dev_debug_printf(const s8 *fmt,...)
{
    s32 length = 0;
    va_list ap;
    s8 *pt;
    s8 string[1024];

    
    va_start(ap,fmt);
    vsprintf((s8 *)string,(const s8 *)fmt,ap);
    pt = &string[0];
    while(*pt!='\0')
    {
        length++;
        pt++;
    }
    
    
    //UartSend((u8 *)string,length);  //sxl?
  #if 0    
    dev_com_write(0, (u8 *)string,length);
  #else
    dev_com_write(PORT_CDCD_NO, (u8 *)string,length); 
  #endif
    va_end(ap);
    
    
}
#else

void dev_debug_printf(const s8 *fmt,...)
{
}
#endif



void dev_debug_printformat(u8 *str, u8 *wbuf, s32 wlen)
{
#ifdef DEBUG_ALL_EN
    s32 i;

    if(str!=NULL)
    {
        dev_debug_printf("%s\r\n", str);
    }

    for(i=0; i<wlen; i++)
    {
        if(0 == (i&0x0f))
        {
            dev_debug_printf("\r\n");
        }
        dev_debug_printf("%02X ", wbuf[i]);
    }
    dev_debug_printf("\r\n");
#endif
    
}

#include "sdk/mhscpu_sdk.h"

void dev_debug_printf_hardfault(const s8 *fmt,...)
{
    s32 length = 0;
    va_list ap;
    s8 *pt;
    s8 string[1024];
    s32 i;
    
    va_start(ap,fmt);
    vsprintf((s8 *)string,(const s8 *)fmt,ap);
    pt = &string[0];
    while(*pt!='\0')
    {
        length++;
        pt++;
    }
    
    
    for(i = 0;i < length;i++)
    {
        UART_SendData(UART0, string[i]);
    }
    
    va_end(ap);
    
    
}

