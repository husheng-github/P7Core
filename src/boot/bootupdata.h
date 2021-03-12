#ifndef __BOOTUPDATA_H
#define __BOOTUPDATA_H

//#define UPDATA

#ifdef UPDATA
#define updata_printf dev_debug_printf
#else
#define updata_printf(a,...)
#endif



#define FNLEN 30
#define DIRLEN 10

typedef struct _updata_infor
{
    u8 filedir[DIRLEN];
    u8 filename[FNLEN];
    u32 filelen;
    u32 filecrc;
    u32 fileadr;
    u8 zipflag;
    u8 cfgvalidflg;
    u8 updata_state;
}updata_info;


void bootupdata_clear_flg(void);

u32 bootupdata_deal(void);
u8 bootupdata_find_fileversion(u32 addr,u32 length,u8 *version,u8 type);
u8 bootupdata_check_ifnewver(u8 *currver,u8 *updatever);
u32 bootupdata_deal_withoutdisplay(void);
int get_updatainfo(s8 *text, u16 len,updata_info *updata_information);


#endif

