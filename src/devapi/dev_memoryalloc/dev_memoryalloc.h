#ifndef __DEV_MEMORYALLOC_H
#define __DEV_MEMORYALLOC_H
#include "ddi_misc.h"

void *k_malloc(u32 nbytes);
void k_free(void *ap);
#if 1//def TRENDIT_CORE
void *k_calloc(unsigned int n, unsigned int size);
void *k_realloc(void *ap, unsigned int size);
void ddi_mem_printfallinfo(u8 type);
s32 ddi_misc_get_mem_info(mem_info_t *mem_info);
#endif

#endif
