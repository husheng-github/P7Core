#ifndef __TYPEDEFINE_H
#define __TYPEDEFINE_H

#include "machinecfg.h"

#undef  NULL
#define NULL        0

#define REG8(addr)          (*(volatile UINT8 *) (addr))
#define REG16(addr)          (*(volatile UINT16 *)(addr))
#define REG32(addr)          (*(volatile UINT32 *)(addr))

//------------------------------------------------------------------
//                        TypeDefs
//------------------------------------------------------------------
typedef    unsigned char            UINT8;    ///<unsigned char
typedef    signed char             INT8;    ///< char

typedef    unsigned short            UINT16;    ///<unsigned char
typedef    signed short            INT16;    ///<short

typedef unsigned int            UINT32;    ///<unsigned int
typedef    signed int              INT32;    ///<int

typedef unsigned char            BOOL;    ///<BOOL

typedef unsigned int            uint32_t;
typedef unsigned short          uint16_t;
typedef unsigned char           uint8_t;

typedef signed short             int16_t;
typedef signed int                 int32_t;
typedef signed char             int8_t;

typedef unsigned int             U32;
typedef unsigned short             U16;
typedef unsigned char             U8;
typedef signed short             S16;
typedef signed int                 S32;
typedef signed char             S8;
typedef unsigned long long         U64;

typedef unsigned int             u32;
typedef unsigned short             u16;
typedef unsigned char             u8;
typedef signed short             s16;
typedef signed int                 s32;
typedef signed char             s8;
typedef unsigned long long         s64;



#endif
