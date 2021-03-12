#ifndef __DEV_DEBUG_H
#define __DEV_DEBUG_H
#include <stdio.h>
#include "typedefine.h"

#ifdef DEBUG_ALL_EN
extern void dev_debug_printf(const s8 *fmt,...);
#else
//#define dev_debug_printf(...)
#endif


int fputc(int c, FILE *f);
void dev_debug_printformat(u8 *str, u8 *wbuf, s32 wlen);
void dev_debug_printf_special(const s8 *fmt,...);

#endif

