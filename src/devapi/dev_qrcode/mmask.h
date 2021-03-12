
#ifndef __MMASK_H__
#define __MMASK_H__

extern unsigned char *MMask_makeMask(int version, unsigned char *frame, int mask, QRecLevel level);
extern unsigned char *MMask_mask(int version, unsigned char *frame, QRecLevel level);

#ifdef WITH_TESTS
extern int MMask_evaluateSymbol(int width, unsigned char *frame);
extern void MMask_writeFormatInformation(int version, int width, unsigned char *frame, int mask, QRecLevel level);
extern unsigned char *MMask_makeMaskedFrame(int width, unsigned char *frame, int mask);
#endif

#endif 
