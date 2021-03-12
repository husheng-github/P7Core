#ifndef __DRV_KEYPAD_H
#define __DRV_KEYPAD_H


#ifdef DEBUG_KEYPAD_EN
#define KEYPAD_DEBUG(format,...)    dev_debug_printf("%s(%d):"format, __func__, __LINE__, ##__VA_ARGS__)
#define KEYPAD_DEBUGHEX             dev_debug_printformat
#else
#define KEYPAD_DEBUG(...) 
#define KEYPAD_DEBUGHEX(...)
#endif

typedef struct _str_KEYPAD_PIN
{
    iomux_pin_name_t m_gpiopin;     //GPIO¹Ü½Å×öÆ¬Ñ¡
    u32 m_pinmask;                  //pinÑÚÂë
}str_keypad_pin_t;
#define KEYPAD_ROW_MAX   4
#define KEYPAD_COL_MAX   4

void drv_keypad_irq_open(void);
void drv_keypad_irq_close(void);
u8 drv_keypad_get_intflg(void);
u8 drv_keypad_clear_intflg(void);

void drv_keypad_init(void);
void drv_keypad_rowin(void);
void drv_keypad_rowout(u8 value);
u32 drv_keypad_rowread(void);
void drv_keypad_colin(void);
void drv_keypad_colout(u8 value);
u32 drv_keypad_colread(void);
u32 drv_keypad_get_powerkey(void);
void drv_power_set_latch(u8 flg);
s32 drv_keypad_row_ismultiplekey(u32 key_value);

#endif
