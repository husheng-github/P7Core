
#ifndef __RSCODE_H__
#define __RSCODE_H__

/*
 * General purpose RS codec, 8-bit symbols.
 */

typedef struct _RS RS;

extern RS *init_rs(int symsize, int gfpoly, int fcr, int prim, int nroots, int pad);
extern void encode_rs_char(RS *rs, const unsigned char *data, unsigned char *parity);
extern void free_rs_char(RS *rs);
extern void free_rs_cache(void);

#endif 
