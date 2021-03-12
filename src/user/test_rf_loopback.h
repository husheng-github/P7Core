#ifndef __LOOPBACK_H
#define __LOOPBACK_H

#define EMV_CL_TEST
//#undef  EMV_CL_TEST

#define EMV_DEBUG_EN
#undef  EMV_DEBUG_EN


#define EMV_FUNCTION   DIGITAL1//0x14

#define EMVRF_BMP_ICO
#undef  EMVRF_BMP_ICO 
#define EMVRF_SEARCH_ROW     DISPAGE2
#define EMVRF_PCD_BMP_XSTA   27 
#define EMVRF_PCD_BMP_YSTA   11

#define EMVRF_PCD_BMP_XLEN   LCD_MAX_COL
#define EMVRF_PCD_BMP_YLEN   LCD_MAX_LINE
#define EMVRF_PCD_ICO_PATH   "/mtd1/pcd.bmp"


#define LOOPBACK_START_COMM         0x40
#define LOOPBACK_END_COMM           0x41
#define LOOPBACK_REPORT_COMM        0x42
#define LOOPBACK_ACTIVE_COMM        0x43
#define LOOPBACK_OPERATE_COMM       0x44
#define LOOPBACK_REMOVE_COMM        0x45
#define LOOPBACK_SETPARAM_COMM      0x46    
#define LOOPBACK_READPARAM_COMM     0x47
#define LOOPBACK_CONTROL_CARRIER    0x48     //¿ØÖÆÔØ²¨
#define LOOPBACK_RESET              0x49     //RESET
#define LOOPBACK_WUP_R              0x4A     //Ñ­»·A/B¿¨,ËÑÑ°µ½A¿¨(WUPA->ANTICOLL->SELECT->RATS),ËÑÑ°B¿¨(WUPB->ATTRIBE)
                                             //          ËÑÑ°µ½B¿¨(WUPB->ATTRIB)

#define EMVRF_DISPLAY_F     
#undef  EMVRF_DISPLAY_F     


void loopback_deal_command(u8 *wbuf, u16 wlen);

void emvcl_test(void);
#endif

