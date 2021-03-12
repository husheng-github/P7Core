

#ifndef _BOOTGLOBAL_H_
#define _BOOTGLOBAL_H_


#ifdef AFX_BOOT_VARIABLES
    #define EXTERN
#else
    #define EXTERN extern
#endif


#define BOOTDOWNLOAD_DEBUG 
#undef  BOOTDOWNLOAD_DEBUG


#ifndef SXL_DEBUG
//#define SXL_DEBUG
#endif

#define APPPROGRAM_DEBUG
#undef  APPPROGRAM_DEBUG





#include <string.h>
#include <stdio.h>
//#include "mhscpu.h"
#ifndef TRENDIT_CORE
#include "sdk\mhscpu_sdk.h"
#endif
#include "devglobal.h"
#include "pciglobal.h"



#include "./bootdownload.h"
#include "./bootdownload_parsedata.h"
#include "./bootdownloadprompt.h"
#include "./bootdownloadauthen.h"
#include "./bootdownloadappinfo.h"
#include "./bootdownloadparseadrfile.h"
#include "./bootdownloadspi.h"
#include "./bootupdata.h"
#include "./bootdownload_processcmd.h"
#include "ddi/ddi_key.h"
#include "ddi/ddi_led.h"

//#define DEBUG_BOOT_EN

#ifdef DEBUG_BOOT_EN
#define BOOT_DEBUG(format,...)    dev_debug_printf("%s(%d):"format, __func__, __LINE__, ##__VA_ARGS__)
#define BOOT_DEBUGHEX             dev_debug_printformat 
#else
#define BOOT_DEBUG(...) 
#define BOOT_DEBUGHEX(...)
#endif

extern SPSTATUS  gSpStatus;
EXTERN DOWNFILE *gCurFileIndex;
EXTERN BOOTDOWNLOAD_PARSEDATA gParseData;
EXTERN u8 gCurcandownfilenum;
EXTERN CANDOWNFILEINFO gCanDownFileInfo[MAXFILE_NUM];
EXTERN DOWNFILE *gDownFileInfo;
EXTERN FILEBAGINFO *gFileBagInfo;
EXTERN u8 gRecFileNum;
EXTERN APPINFO gAppInfo;


#endif


