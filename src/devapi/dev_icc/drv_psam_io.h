#ifndef __DRV_PSAM_IO_H
#define __DRV_PSAM_IO_H



#ifdef DEBUG_PSAM_EN
#define PSAM_DEBUG(format,...)    dev_debug_printf("%s(%d):"format, __func__, __LINE__, ##__VA_ARGS__)
#define PSAM_DEBUGHEX             dev_debug_printformat
#else
#define PSAM_DEBUG(...) 
#define PSAM_DEBUGHEX(...)
#endif

#define PSAM_NUM_MAX     1//2


#define PSAM_CLK_Frequency     4000000    
#define PSAM_LowLevelPeriod     1
#define PSAM_HighLevelPeriod    1


s32 drv_psam_io_init(void);
s32 drv_psam_io_irq_request(s32 psamno, void (*handler)(void* ));
void drv_psam_io_irq_enable(s32 psamno, s32 mod);
void drv_psam_power_ctl(s32 psamno, s32 flg);
void drv_psam_rst_ctl(s32 psamno, s32 flg);
void drv_psam_clk_ctl(s32 psamno, s32 flg);
void drv_psam_io_direction_set(s32 psamno, s32 mod, s32 value);
void drv_psam_io_set_value(s32 psamno, s32 value);
s32 drv_psam_io_get_value(s32 psamno);

void drv_psam_testpin_set(s32 n, s32 flg);
void drv_psam_testpin_xor(s32 n);    
#endif
