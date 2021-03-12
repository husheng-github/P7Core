

#ifndef _DDI_H_
#define _DDI_H_
#include "ddi_common.h"
#include "ddi_misc.h"
#include "ddi_com.h"
#include "ddi_mag.h"
#include "ddi_iccpsam.h"
#include "ddi_rf.h"
#include "ddi_thmprn.h"
#include "ddi_key.h"
#include "ddi_lcd.h"
#include "ddi_audio.h"
#include "ddi_innerkey.h"
#include "ddi_file.h"
#include "ddi_bt.h"
#include "ddi_gprs.h"
#include "ddi_wifi.h"
#include "ddi_led.h"
#include "ddi_dukpt.h"
#include "ddi_mbedtls_ssl.h"
#include "ddi_elecsign.h"
#include "ddi_arith.h"
#include "ddi_watchdog.h"
#include "ddi_scanner.h"
#include "ddi_sp.h"


/*********************************************************************************
***
***   define the struct contains all the function as follow
***
*********************************************************************************/

struct  __core
{
	//系统
	core_ddi_misc_appprinf    CoreApi_ddi_misc_appprinf ;
	core_ddi_misc_msleep     CoreApi_ddi_misc_msleep;
	core_ddi_misc_set_time   CoreApi_ddi_misc_set_time;
	core_ddi_misc_get_time   CoreApi_ddi_misc_get_time;
	core_ddi_misc_get_tick   CoreApi_ddi_misc_get_tick; //系统滴答
	core_ddi_misc_read_dsn   CoreApi_ddi_misc_read_dsn;
	core_ddi_misc_sleep      CoreApi_ddi_misc_sleep;
	core_ddi_misc_get_firmwareversion CoreApi_ddi_misc_get_firmwareversion;
	core_ddi_misc_bat_status CoreApi_ddi_misc_bat_status;
	core_ddi_misc_poweroff   CoreApi_ddi_misc_poweroff ;
	core_k_mallocapp       CoreApi_k_mallocapp;
	core_k_freeapp         CoreApi_k_freeapp;
	core_ddi_misc_sleep_with_alarm CoreApi_ddi_misc_sleep_with_alarm;
	core_ddi_misc_reboot     CoreApi_ddi_misc_reboot;
	core_ddi_misc_probe_dev  CoreApi_ddi_misc_probe_dev;
	core_ddi_user_timer_open CoreApi_ddi_user_timer_open;
	core_ddi_user_timer_close CoreApi_ddi_user_timer_close ;
	core_ddi_pci_readtamperstatus CoreApi_ddi_pci_readtamperstatus;
	core_ddi_pci_getcurrenttamperstatus CoreApi_ddi_pci_getcurrenttamperstatus;
	core_ddi_pci_unlock    CoreApi_ddi_pci_unlock;
	core_ddi_pci_opendryice    CoreApi_ddi_pci_opendryice;
	core_ddi_pci_getdryiceconfig CoreApi_ddi_pci_getdryiceconfig;
    core_ddi_pci_getspstatuspointer CoreApi_ddi_pci_getspstatuspointer;
    core_ddi_misc_ioctl CoreApi_ddi_misc_ioctl;
    char *Reservedmisc[(32-24)];//31)];
	
	
    //串口
    core_ddi_com_open       CoreApi_ddi_com_open;
    core_ddi_com_close      CoreApi_ddi_com_close;
    core_ddi_com_clear      CoreApi_ddi_com_clear;
    core_ddi_com_read       CoreApi_ddi_com_read;
    core_ddi_com_write      CoreApi_ddi_com_write;
    core_ddi_com_ioctl      CoreApi_ddi_com_ioctl; 
    core_ddi_com_tcdrain    CoreApi_ddi_com_tcdrain; 
    char *Reservedcom[(32-7)];

    //磁卡
    core_ddi_mag_open       CoreApi_ddi_mag_open;
    core_ddi_mag_close      CoreApi_ddi_mag_close;
    core_ddi_mag_clear      CoreApi_ddi_mag_clear;
    core_ddi_mag_read       CoreApi_ddi_mag_read;
    core_ddi_mag_ioctl      CoreApi_ddi_mag_ioctl;
    char *Reservedmag[(32-5)];
    
    //IC PSAM
    core_ddi_iccpsam_open       CoreApi_ddi_iccpsam_open;
    core_ddi_iccpsam_close      CoreApi_ddi_iccpsam_close;
    core_ddi_iccpsam_poweron    CoreApi_ddi_iccpsam_poweron;
    core_ddi_iccpsam_poweroff   CoreApi_ddi_iccpsam_poweroff;
    core_ddi_iccpsam_get_status CoreApi_ddi_iccpsam_get_status;
    core_ddi_iccpsam_exchange_apdu  CoreApi_ddi_iccpsam_exchange_apdu;
    core_ddi_iccpsam_ioctl      CoreApi_ddi_iccpsam_ioctl;
    char *Reservediccpsam[(32-7)];


    //非接卡操作
    core_ddi_rf_open            CoreApi_ddi_rf_open;
    core_ddi_rf_close           CoreApi_ddi_rf_close;
    core_ddi_rf_poweron         CoreApi_ddi_rf_poweron;
    core_ddi_rf_poweroff        CoreApi_ddi_rf_poweroff;
    core_ddi_rf_get_status      CoreApi_ddi_rf_get_status;
    core_ddi_rf_activate        CoreApi_ddi_rf_activate;
    core_ddi_rf_exchange_apdu   CoreApi_ddi_rf_exchange_apdu;
    core_ddi_rf_remove          CoreApi_ddi_rf_remove;
    core_ddi_rf_ioctl           CoreApi_ddi_rf_ioctl;
	
    char *Reservedrf[(32-9)];

    //热敏打印
    core_ddi_thmprn_open            CoreApi_ddi_thmprn_open;
    core_ddi_thmprn_close           CoreApi_ddi_thmprn_close;
    core_ddi_thmprn_feed_paper      CoreApi_ddi_thmprn_feed_paper;
    core_ddi_thmprn_print_image     CoreApi_ddi_thmprn_print_image;
    core_ddi_thmprn_print_image_file     CoreApi_ddi_thmprn_print_image_file;
    core_ddi_thmprn_print_text      CoreApi_ddi_thmprn_print_text;
    core_ddi_thmprn_print_comb_text CoreApi_ddi_thmprn_print_comb_text;
    core_ddi_thmprn_get_status      CoreApi_ddi_thmprn_get_status;
    core_ddi_thmprn_ioctl           CoreApi_ddi_thmprn_ioctl;
    core_ddi_thmprn_esc_p           CoreApi_ddi_thmprn_esc_p;
    core_ddi_thmprn_esc_loop        CoreApi_ddi_thmprn_esc_loop;
    core_ddi_thmprn_esc_init        CoreApi_ddi_thmprn_esc_init;
    char *Reservedthmprn[(32-12)];
    
    //按键
    core_ddi_key_open   CoreApi_ddi_key_open;
    core_ddi_key_close  CoreApi_ddi_key_close;
    core_ddi_key_clear  CoreApi_ddi_key_clear;
    core_ddi_key_read   CoreApi_ddi_key_read;
    core_ddi_key_read_withoutdelay CoreApi_ddi_key_read_withoutdelay;
    core_ddi_key_ioctl  CoreApi_ddi_key_ioctl;
    char *Reservedkey[(32-6)];
        
    //显示
    core_ddi_lcd_open               CoreApi_ddi_lcd_open;
    core_ddi_lcd_close              CoreApi_ddi_lcd_close;
    core_ddi_lcd_fill_rect          CoreApi_ddi_lcd_fill_rect;
    core_ddi_lcd_clear_rect         CoreApi_ddi_lcd_clear_rect;
    core_ddi_lcd_show_text          CoreApi_ddi_lcd_show_text;
    core_ddi_lcd_show_picture       CoreApi_ddi_lcd_show_picture;
    core_ddi_lcd_monochrome         CoreApi_ddi_lcd_monochrome;
    core_ddi_lcd_show_picture_file  CoreApi_ddi_lcd_show_picture_file;
    core_ddi_lcd_show_pixel         CoreApi_ddi_lcd_show_pixel;
    core_ddi_lcd_show_line          CoreApi_ddi_lcd_show_line;//划线
    core_ddi_lcd_show_rect          CoreApi_ddi_lcd_show_rect;
    core_ddi_lcd_extract_rect       CoreApi_ddi_lcd_extract_rect;
    core_ddi_lcd_ioctl              CoreApi_ddi_lcd_ioctl;
	core_ddi_lcd_brush_screen       CoreApi_ddi_lcd_brush_screen;
    char *Reservedlcd[(32-14)];

    //音频
    core_ddi_audio_open     CoreApi_ddi_audio_open;
    core_ddi_audio_close    CoreApi_ddi_audio_close;
    core_ddi_audio_play     CoreApi_ddi_audio_play;
    core_ddi_audio_playcharacters CoreApi_ddi_audio_playcharacters;
    core_ddi_audio_stop     CoreApi_ddi_audio_stop;
    core_ddi_audio_pause    CoreApi_ddi_audio_pause;
    core_ddi_audio_resumeplay   CoreApi_ddi_audio_resumeplay;
    core_ddi_audio_ioctl    CoreApi_ddi_audio_ioctl;
    char *Reservedaudio[(32-8)];
    //内置密钥
    core_ddi_innerkey_open      CoreApi_ddi_innerkey_open;
    core_ddi_innerkey_close     CoreApi_ddi_innerkey_close;
    core_ddi_innerkey_inject    CoreApi_ddi_innerkey_inject;
	core_ddi_innerkey_delete    CoreApi_ddi_innerkey_delete;
    core_ddi_innerkey_encrypt   CoreApi_ddi_innerkey_encrypt;
    core_ddi_innerkey_decrypt   CoreApi_ddi_innerkey_decrypt;
    core_ddi_innerkey_getHW21SNinfo CoreApi_ddi_innerkey_getHW21SNinfo;
    core_ddi_innerkey_ioctl     CoreApi_ddi_innerkey_ioctl;
	char *Reserved1innerkey[(32-8)];

    //虚拟文件系统
    core_ddi_file_create     CoreApi_ddi_file_create;
    core_ddi_file_write      CoreApi_ddi_file_write ;
    core_ddi_file_read       CoreApi_ddi_file_read;
    core_ddi_file_insert     CoreApi_ddi_file_insert;
    core_ddi_file_delete     CoreApi_ddi_file_delete;
    core_ddi_file_rename     CoreApi_ddi_file_rename;
    core_ddi_file_getlength  CoreApi_ddi_file_getlength;
	core_ddi_file_veritysignature CoreApi_ddi_file_veritysignature;
    core_ddi_static_vfs_getarea CoreApi_ddi_static_vfs_getarea;
	core_ddi_vfs_open CoreApi_ddi_vfs_open;
	core_ddi_vfs_close CoreApi_ddi_vfs_close;
	core_ddi_vfs_seek CoreApi_ddi_vfs_seek;
	core_ddi_vfs_tell CoreApi_ddi_vfs_tell;
	core_ddi_vfs_read CoreApi_ddi_vfs_read;
	core_ddi_vfs_write CoreApi_ddi_vfs_write;
	core_ddi_vfs_sync CoreApi_ddi_vfs_sync;
	core_ddi_vfs_deletefile CoreApi_ddi_vfs_deletefile;
	core_ddi_vfs_renamefile CoreApi_ddi_vfs_renamefile;
	core_ddi_vfs_free_space CoreApi_ddi_vfs_free_space;	
	core_ddi_vfs_factory_initial CoreApi_ddi_vfs_factory_initial;
    core_ddi_flash_read CoreApi_ddi_flash_read;
    core_ddi_flash_write CoreApi_ddi_flash_write;
    char *Reservedfile[(32-22)];
	
	
    //蓝牙
    core_ddi_bt_open        CoreApi_ddi_bt_open;
    core_ddi_bt_close       CoreApi_ddi_bt_close;
    core_ddi_bt_disconnect  CoreApi_ddi_bt_disconnect;
    core_ddi_bt_write       CoreApi_ddi_bt_write;
    core_ddi_bt_read        CoreApi_ddi_bt_read;
    core_ddi_bt_get_status  CoreApi_ddi_bt_get_status;
    core_ddi_bt_ioctl       CoreApi_ddi_bt_ioctl;
    char *Reservedbt[(32-7)];

    //gprs
    core_ddi_gprs_open                  CoreApi_ddi_gprs_open                        ;
    core_ddi_gprs_close                 CoreApi_ddi_gprs_close                       ;
    core_ddi_gprs_get_signalquality     CoreApi_ddi_gprs_get_signalquality           ;
    core_ddi_gprs_telephony_dial        CoreApi_ddi_gprs_telephony_dial              ;
    core_ddi_gprs_telephony_hangup      CoreApi_ddi_gprs_telephony_hangup            ;
    core_ddi_gprs_telephony_answer      CoreApi_ddi_gprs_telephony_answer        ;
    core_ddi_gprs_get_pdpstatus         CoreApi_ddi_gprs_get_pdpstatus               ;
    core_ddi_gprs_set_apn               CoreApi_ddi_gprs_set_apn                     ;
    core_ddi_gprs_socket_create         CoreApi_ddi_gprs_socket_create               ;
    core_ddi_gprs_socket_send           CoreApi_ddi_gprs_socket_send                 ;
    core_ddi_gprs_socket_recv           CoreApi_ddi_gprs_socket_recv                 ;
    core_ddi_gprs_socket_close          CoreApi_ddi_gprs_socket_close                ;
    core_ddi_gprs_socket_get_status     CoreApi_ddi_gprs_socket_get_status           ;
    core_ddi_gprs_get_siminfo           CoreApi_ddi_gprs_get_siminfo                 ;
    core_ddi_gprs_get_imei              CoreApi_ddi_gprs_get_imei                    ;
    core_ddi_gprs_get_basestation_info  CoreApi_ddi_gprs_get_basestation_info        ;
    core_ddi_gprs_ioctl                 CoreApi_ddi_gprs_ioctl                       ;
    char *Reservedgprs[32-17];

    //wifi
    core_ddi_wifi_open CoreApi_ddi_wifi_open;
    core_ddi_wifi_close CoreApi_ddi_wifi_close;
    core_ddi_wifi_scanap_start CoreApi_ddi_wifi_scanap_start;
    core_ddi_wifi_scanap_status CoreApi_ddi_wifi_scanap_status;
    core_ddi_wifi_connectap_start CoreApi_ddi_wifi_connectap_start;
    core_ddi_wifi_connectap_status CoreApi_ddi_wifi_connectap_status;
    core_ddi_wifi_disconnectap CoreApi_ddi_wifi_disconnectap;
    core_ddi_wifi_socket_create_start CoreApi_ddi_wifi_socket_create_start;
    core_ddi_wifi_socket_get_status CoreApi_ddi_wifi_socket_get_status;
    core_ddi_wifi_socket_send CoreApi_ddi_wifi_socket_send;
    core_ddi_wifi_socket_recv CoreApi_ddi_wifi_socket_recv;
    core_ddi_wifi_socket_close CoreApi_ddi_wifi_socket_close;
    core_ddi_wifi_get_signal CoreApi_ddi_wifi_get_signal;
    core_ddi_wifi_ioctl CoreApi_ddi_wifi_ioctl;
    core_ddi_wifi_start_airkiss CoreApi_ddi_wifi_start_airkiss;
    core_ddi_wifi_get_airkiss_status CoreApi_ddi_wifi_get_airkiss_status;
    core_ddi_wifi_get_airkiss_config CoreApi_ddi_wifi_get_airkiss_config;
    core_ddi_wifi_stop_airkiss CoreApi_ddi_wifi_stop_airkiss;
    char *Reservedwifi[(32-18)];

    //LED
    core_ddi_led_open       CoreApi_ddi_led_open;
    core_ddi_led_close      CoreApi_ddi_led_close;
    core_ddi_led_sta_set    CoreApi_ddi_led_sta_set;
    core_ddi_led_ioctl      CoreApi_ddi_led_ioctl;    
    char *Reservedled[(32-4)];

    //dukpt
    core_ddi_dukpt_open     CoreApi_ddi_dukpt_open;
    core_ddi_dukpt_close    CoreApi_ddi_dukpt_close;
    core_ddi_dukpt_inject   CoreApi_ddi_dukpt_inject;
    core_ddi_dukpt_encrypt  CoreApi_ddi_dukpt_encrypt;
    core_ddi_dukpt_decrypt  CoreApi_ddi_dukpt_decrypt;
    core_ddi_dukpt_getksn   CoreApi_ddi_dukpt_getksn;
    core_ddi_dukpt_ioctl    CoreApi_ddi_dukpt_ioctl;
    char *Reserveddukpt[(32-7)];

    	//ssl socket
    core_ddi_mbedtls_ssl_open CoreApi_ddi_mbedtls_ssl_open;
    core_ddi_mbedtls_ssl_close CoreApi_ddi_mbedtls_ssl_close;
    core_ddi_mbedtls_ssl_set_ca_cert CoreApi_ddi_mbedtls_ssl_set_ca_cert;
    core_ddi_mbedtls_ssl_set_client_private_key CoreApi_ddi_mbedtls_ssl_set_client_private_key;
    core_ddi_mbedtls_ssl_set_client_cert CoreApi_ddi_mbedtls_ssl_set_client_cert;
    core_ddi_mbedtls_ssl_handshake CoreApi_ddi_mbedtls_ssl_handshake;
    core_ddi_mbedtls_ssl_send CoreApi_ddi_mbedtls_ssl_send;
    core_ddi_mbedtls_ssl_recv CoreApi_ddi_mbedtls_ssl_recv;
    core_ddi_mbedtls_ssl_set_host_name	CoreApi_ddi_mbedtls_ssl_set_host_name;
    core_ddi_mbedtls_ioctl    CoreApi_ddi_mbedtls_ioctl;
    core_ddi_mbedtls_verify_certchain  CoreApi_ddi_mbedtls_verify_certchain;
    core_ddi_mbedtls_parse_cert_info   CoreApi_ddi_mbedtls_parse_cert_info;
    core_ddi_mbedtls_pk_verify_sign  CoreApi_ddi_mbedtls_pk_verify_sign;
    core_ddi_mbedtls_sk_verify_sign  CoreApi_ddi_mbedtls_sk_verify_sign;
    core_ddi_mbedtls_pk_sign    CoreApi_ddi_mbedtls_pk_sign;
    core_ddi_mbedtls_sk_sign    CoreApi_ddi_mbedtls_sk_sign;
    char *Reservedmbedtls[(32-16)];

    //电子签名
    core_ddi_elec_sign_open          CoreApi_ddi_elec_sign_open         ;
    core_ddi_elec_sign_get_data_size CoreApi_ddi_elec_sign_get_data_size;
    core_ddi_elec_sign_get_data      CoreApi_ddi_elec_sign_get_data     ;
    core_ddi_elec_sign_process       CoreApi_ddi_elec_sign_process      ;
    core_ddi_elec_sign_start         CoreApi_ddi_elec_sign_start        ;
    core_ddi_elec_sign_stop          CoreApi_ddi_elec_sign_stop         ;
    core_ddi_elec_sign_close          CoreApi_ddi_elec_sign_close         ;
    core_ddi_elec_sign_draw_text    CoreApi_ddi_elec_sign_draw_text    ;
	core_ddi_elec_sign_ioctl		CoreApi_ddi_elec_sign_ioctl;
   char *Reserveelecsign[32-9];

   
	// hardware arith
	core_ddi_arith_hash CoreApi_ddi_arith_hash;
	core_ddi_arith_des  CoreApi_ddi_arith_des;
	core_ddi_arith_getrand CoreApi_ddi_arith_getrand;
	core_ddi_arith_rsaencrypt CoreApi_ddi_arith_rsaencrypt;
    core_ddi_arith_rsadecrypt CoreApi_ddi_arith_rsadecrypt;
	core_ddi_arith_rsaskencrypt CoreApi_ddi_arith_rsaskencrypt;
	core_ddi_arith_rsaskdecrypt CoreApi_ddi_arith_rsaskdecrypt;
	core_ddi_arith_rsarecover CoreApi_ddi_arith_rsarecover;
	core_ddi_arith_sm2veritysignature CoreApi_ddi_arith_sm2veritysignature;
	core_ddi_arith_sm3 CoreApi_ddi_arith_sm3;
	core_ddi_arith_sm4 CoreApi_ddi_arith_sm4;
    core_ddi_arith_file_hash CoreApi_ddi_arith_file_hash;
    core_ddi_arith_md5 CoreApi_ddi_arith_md5;
	core_ddi_arith_sm2encrypt CoreApi_ddi_arith_sm2encrypt;
	core_ddi_arith_sm2decrypt CoreApi_ddi_arith_sm2decrypt;
	core_ddi_arith_sm2skencrypt CoreApi_ddi_arith_sm2skencrypt;
	core_ddi_arith_sm2skdecrypt CoreApi_ddi_arith_sm2skdecrypt;
	core_ddi_arith_sm2_digital_sign CoreApi_ddi_arith_sm2_digital_sign;
	core_ddi_arith_sm2_gen_PubAndPri_keys CoreApi_ddi_arith_sm2_gen_PubAndPri_keys;	
	char *Reservedarith[(32-19)];

	//watchdog
	core_ddi_watchdog_open     CoreApi_ddi_watchdog_open;
    core_ddi_watchdog_close    CoreApi_ddi_watchdog_close;
    core_ddi_watchdog_feed     CoreApi_ddi_watchdog_feed;
	char *Reservedwatchdog[(32-3)];

    //扫码及二维码生成
    core_ddi_QRcode_encodeString  CoreApi_ddi_QRcode_encodeString;
    core_ddi_QRcode_free          CoreApi_ddi_QRcode_free        ;
    core_ddi_scanner_open         CoreApi_ddi_scanner_open       ;
    core_ddi_scanner_start        CoreApi_ddi_scanner_start      ;
    core_ddi_scanner_abort        CoreApi_ddi_scanner_abort      ;
    core_ddi_scanner_getdata      CoreApi_ddi_scanner_getdata    ;
    core_ddi_scanner_close        CoreApi_ddi_scanner_close      ;
	core_ddi_scanner_ioctl		  CoreApi_ddi_scanner_ioctl;
    core_ddi_Barcode_encodeString CoreApi_ddi_Barcode_encodeString;
    core_ddi_Barcode_free         CoreApi_ddi_Barcode_free;
	char *Reservedscanner[(32-10)];   

    //SP normal manage
	core_ddi_spnormalmanage_reset CoreApi_ddi_spnormalmanage_reset;
	core_ddi_spnormalmanage_hardwarereset CoreApi_ddi_spnormalmanage_hardwarereset;
	core_ddi_spnormalmanage_setsleeptime CoreApi_ddi_spnormalmanage_setsleeptime;
	core_ddi_spnormalmanage_getsleeptime CoreApi_ddi_spnormalmanage_getsleeptime;
	core_ddi_spnormalmanage_intosleep CoreApi_ddi_spnormalmanage_intosleep;
	core_ddi_spsecuremanage_getspstatus CoreApi_ddi_spsecuremanage_getspstatus;
	core_ddi_spfirmwaremanage_intobootmode CoreApi_ddi_spfirmwaremanage_intobootmode;
	core_ddi_spfirmwaremanage_exitbootmode CoreApi_ddi_spfirmwaremanage_exitbootmode;
	core_ddi_spfirmwaremanage_update CoreApi_ddi_spfirmwaremanage_update;
	core_ddi_spfirmwaremanage_intocdcdatatospimode CoreApi_ddi_spfirmwaremanage_intocdcdatatospimode;
	core_ddi_spfirmwaremanage_spireadcdcdata CoreApi_ddi_spfirmwaremanage_spireadcdcdata;
	core_ddi_spfirmwaremanage_getspcertinfo CoreApi_ddi_spfirmwaremanage_getspcertinfo;
	core_ddi_spnormalmanage_setdevicesn CoreApi_ddi_spnormalmanage_setdevicesn;
	core_ddi_spnormalmanage_getdevicesn CoreApi_ddi_spnormalmanage_getdevicesn;
	core_ddi_spnormalmanage_getversion CoreApi_ddi_spnormalmanage_getversion;
	core_ddi_spnormalmanage_randnum CoreApi_ddi_spnormalmanage_randnum;
	core_ddi_spsecuremanage_gettamperinfo CoreApi_ddi_spsecuremanage_gettamperinfo;
	core_ddi_spsecuremanage_requestsecuredata CoreApi_ddi_spsecuremanage_requestsecuredata;
	core_ddi_spsecuremanage_setsecuredata CoreApi_ddi_spsecuremanage_setsecuredata;
	core_ddi_spnormalmanage_beep CoreApi_ddi_spnormalmanage_beep;
	core_ddi_spsecuremanage_getdebugflag CoreApi_ddi_spsecuremanage_getdebugflag;
	core_ddi_spsecuremanage_opendryice CoreApi_ddi_spsecuremanage_opendryice;
	core_ddi_dev_apspcomm_command CoreApi_ddi_dev_apspcomm_command;
	core_ddi_secureprevent_getspinfo CoreApi_ddi_secureprevent_getspinfo;
	core_ddi_secureprevent_sendbinddatatosp CoreApi_ddi_secureprevent_sendbinddatatosp;
	core_ddi_secureprevent_saveappcertinfo CoreApi_ddi_secureprevent_saveappcertinfo;
	core_ddi_secureprevent_readappcertinfo CoreApi_ddi_secureprevent_readappcertinfo;
	core_ddi_secureprevent_getsnkeycheckvalue CoreApi_ddi_secureprevent_getsnkeycheckvalue;

    core_ddi_secureprevent_downloadsnkey CoreApi_ddi_secureprevent_downloadsnkey;
    core_ddi_spsecuremanage_getencryptedranddata CoreApi_ddi_spsecuremanage_getencryptedranddata;
    core_ddi_spsecuremanage_dump_memoryinfo CoreApi_ddi_spsecuremanage_dump_memoryinfo;
    char *Reservedsp[32-31];
	
};




#endif






