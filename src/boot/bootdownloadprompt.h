

#ifndef _BOOTDOWNLOADPROMPT_H_
#define _BOOTDOWNLOADPROMPT_H_



#define PROMPT_DOWNLOADING         "DOWNLOADING..."



#define PROMPT_EXE_OTHER           "Other fault"
#define EXECUTION_MEMORY           1
#define PROMPT_EXE_MEMORY          "Memory not enough"
#define EXECUTION_HARDWAREVERSION  2
#define PROMPT_EXE_HARDWAREVERSION "Hardware not match!"
#define EXECUTION_DOWNFILENUM      3
#define PROMPT_EXE_DOWNFILENUM     "file number overflow!"
#define EXECUTION_DOWNFILEOPEN     4
#define PROMPT_EXE_DOWNFILEOPEN    "file open fail!"
#define EXECUTION_DOWNFILEOPS      5
#define PROMPT_EXE_DOWNFILEOPS     "operate file fail!"
#define EXECUTION_MACHINEMODE      6
#define PROMPT_EXE_MACHINEMODE     "model  not match!"
#define EXECUTION_DISKNOTENOUGH    7
#define PROMPT_EXE_DISKNOTENOUGH   "space not enough"
#define EXECUTION_DOWNFILENAME     8
#define PROMPT_EXE_DOWNFILENAME    "file name err!"
#define EXECUTION_CRCERR            9
#define PROMPT_EXE_CRCERR          "CRC err!"
#define DOWNLOADFILESUCCESS         10
#define PROMPT_DOWNLOADFILESUCCESS  "Download success!"
#define DOWNLOADAUTHENTICATE        11
#define PROMPT_DOWNLOADAUTHENTICATE "Authenticate..."
#define AUTHENTICATEFAIL            12
#define PROMPT_AUTHENTICATEFAIL     "Authenticate fail"
#define EXECUTION_SAVEINFOERROR     13
#define PROMPT_SAVEINFOERROR        "save file err"
#define EXECUTION_TRENDITPKNOTEXIST    14
#define PROMPT_TRENDITPKNOTEXIST        "TREN PK not exist"
#define EXECUTION_ACQUIRERPKAUTHENFAIL       15
#define PROMPT_ACQUIRERPKAUTHENFAIL          "acquirer pk fail"
#define EXECUTION_APPVERIFYFAIL     16
#define PROMPT_APPVERIFYFAIL        "app verify fail!"
#define EXECUTION_PKNOTSUPPORT      17
#define PROMPT_PKNOTSUPPORT         "PK file not support"


#define EXECUTION_PREVENTTHELFTFAIL            18
#define EXECUTION_PREVENTTHELFTFAIL_CPUID      19
#define EXECUTION_PREVENTTHELFTFAIL_VERSION    20
#define EXECUTION_HANDSHAKEKEYNOTEXIST         21



extern void bootdownload_prompt(u8 prompttype,u8 *promptlen ,u8 *prompt);
extern void bootdownload_prompt_downloading(u8 *filename);





#define PROMPT_LCDDISPLAY_SUPPORT
//#undef PROMPT_LCDDISPLAY_SUPPORT




#ifdef PROMPT_LCDDISPLAY_SUPPORT
#define BIOSSECLINE1 0            //sxl?
#define BIOSSECLINE2 2


#define BIOSSECFDISP 0
#define BIOSSECCDISP 1
#define BIOSSECLDISP 2
#define BIOSSECRDISP 4


extern void bootdownload_prompt_cleardisprow(u8 rowno);
extern void bootdownload_prompt_fill_row(u8 pageid, u32 colid, u8 *str, u8 atr);
#endif




#endif

