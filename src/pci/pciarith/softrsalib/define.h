#ifndef _define_H
#define _define_H

//#if defined(_WIN32) || defined(_WINDOWS) || defined(WIN32) || defined(_WINDOWS_)
//#include <windows.h>
//#define WINAPI __stdcall
//#else
#define WINAPI
//#endif

#undef BUILD_DLL
#ifdef  BUILD_DLL
#define DLL_EXPORT  extern "C" __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#define DW2P(p, x)      ( p[0] = (unsigned char)( x & 0x000000FF), \
                          p[1] = (unsigned char)((x & 0x0000FF00) >>  8), \
                          p[2] = (unsigned char)((x & 0x00FF0000) >> 16), \
                          p[3] = (unsigned char)((x & 0xFF000000) >> 24) )
#define P2DW(x, p)      ( x = (unsigned long)((p[3] << 24) & 0xFF000000) | \
                              (unsigned long)((p[2] << 16) & 0x00FF0000) | \
                              (unsigned long)((p[1] <<  8) & 0x0000FF00) | \
                              (unsigned long)( p[0] & 0x000000FF) )


//20120104
#define DW3P(p, x)      ( p[0] = (unsigned char)( x & 0x000000FF), \
                          p[1] = (unsigned char)((x & 0x0000FF00) >>  8), \
                          p[2] = (unsigned char)((x & 0x00FF0000) >> 16))


#endif // _define_H
