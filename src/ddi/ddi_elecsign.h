#ifndef __DDI_ELECSIGN_H__
#define __DDI_ELECSIGN_H__


#include "ddi_common.h"


//-------------------------------------------------
#define DDI_ELEC_SIGN_LOOP 	1
#define DDI_ELEC_SIGN_CALIBRATE_STATE	2
#define DDI_ELEC_SIGN_CALIBRATE	3 

//对外函数声明
s32 ddi_elec_sign_open(s32 canvas_x, s32 canvas_y, s32 canvas_width, s32 canvas_height); 
s32 ddi_elec_sign_get_data_size(s32 with_header);                                        
s32 ddi_elec_sign_get_data(unsigned char* buffer, s32 *buffer_size, s32 with_header);    
s32 ddi_elec_sign_process(void);                                                        
s32 ddi_elec_sign_start(void);                                                          
s32 ddi_elec_sign_stop(void);                                                           
s32 ddi_elec_sign_close(void);                                                          
s32 ddi_elec_sign_draw_text(u32 x, u32 y, u8 *text);  
s32 ddi_elec_sign_ioctl(u32 nCmd, u32 lParam, u32 wParam);
//====================================================
typedef s32 (*core_ddi_elec_sign_open)(s32 canvas_x, s32 canvas_y, s32 canvas_width, s32 canvas_height);
typedef s32 (*core_ddi_elec_sign_get_data_size)(s32 with_header);
typedef s32 (*core_ddi_elec_sign_get_data)(unsigned char* buffer, s32 *buffer_size, s32 with_header);
typedef s32 (*core_ddi_elec_sign_process)(void);
typedef s32 (*core_ddi_elec_sign_start)(void);
typedef s32 (*core_ddi_elec_sign_stop)(void);
typedef s32 (*core_ddi_elec_sign_close)(void);
typedef s32 (*core_ddi_elec_sign_draw_text)(u32 x, u32 y, u8 *text);
typedef s32 (*core_ddi_elec_sign_ioctl)(u32 nCmd, u32 lParam, u32 wParam);



#endif



