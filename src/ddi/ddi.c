


#include "pciglobal.h"
#include "ddi.h"
//#include "ddi_manage.h"

#define FILL_NULL_POINTER_32 NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, \
                              NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
#define FILL_NULL_POINTER_31 NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, \
                              NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
#define FILL_NULL_POINTER_16 NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
#define FILL_NULL_POINTER_15 NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
#define FILL_NULL_POINTER_10 NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
#define FILL_NULL_POINTER_8 NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
#define FILL_NULL_POINTER_7 NULL,NULL,NULL,NULL,NULL,NULL,NULL
#define FILL_NULL_POINTER_5 NULL,NULL,NULL,NULL,NULL
#define FILL_NULL_POINTER_4 NULL,NULL,NULL,NULL
#define FILL_NULL_POINTER_3 NULL,NULL,NULL
#define FILL_NULL_POINTER_2 NULL,NULL
#define FILL_NULL_POINTER_1 NULL



extern void *k_mallocapp(unsigned nbytes);
extern void k_freeapp (void *ap);


#if (defined(TRENDIT_COS) ||defined(TRENDIT_CORE))
const struct __core core __attribute__((at(CORE_DDI_ADDR+MCU_BASE_ADDR)))  =
{
    
    ddi_misc_appprinf,//调试信息
    ddi_misc_msleep,
    ddi_misc_set_time,
    ddi_misc_get_time,
    ddi_misc_get_tick,  //系统滴答
    ddi_misc_read_dsn,
    ddi_misc_sleep,
    ddi_misc_get_firmwareversion,
    ddi_misc_bat_status,
    ddi_misc_poweroff,
    k_malloc,  
    k_free,
    ddi_misc_sleep_with_alarm,
    ddi_misc_reboot,
    
 #if 0  //20190428 pengxuebin删除 
    ddi_manage_download,
    ddi_misc_bat,
    ddi_misc_rtcbatterybat_check,//私加
    ddi_misc_key_check_f_3,
 #endif   
    ddi_misc_probe_dev,
    ddi_user_timer_open,
    ddi_user_timer_close,
    ddi_pci_readtamperstatus,
    ddi_pci_getcurrenttamperstatus,
    ddi_pci_unlock,
    ddi_pci_opendryice,
    ddi_pci_getdryiceconfig,
  #if 0  //20190428 pengxuebin删除 
    dev_flash_read,
//    ddi_misc_get_bootver,
//    ddi_misc_get_corever,
    ddi_misc_get_ver,
    ddi_misc_get_batvol,
//    ddi_misc_get_hardwarever,
  #endif  
    ddi_pci_getspstatuspointer,
    ddi_misc_ioctl,
    FILL_NULL_POINTER_8,//_1,

    //2 串口
    ddi_com_open,
    ddi_com_close,
    ddi_com_clear,
    ddi_com_read,
    ddi_com_write,
    ddi_com_ioctl,
    ddi_com_tcdrain,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_5,
//    FILL_NULL_POINTER_1,

    //3 磁卡
    ddi_mag_open,
    ddi_mag_close,
    ddi_mag_clear,
    ddi_mag_read,
    ddi_mag_ioctl,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_5,
    FILL_NULL_POINTER_2,
    
    //4 IC PSAM
    ddi_iccpsam_open,
    ddi_iccpsam_close,
    ddi_iccpsam_poweron,
    ddi_iccpsam_poweroff,
    ddi_iccpsam_get_status,
    ddi_iccpsam_exchange_apdu,
    ddi_iccpsam_ioctl, 
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_5,

    //5 非接卡操作
    ddi_rf_open,
    ddi_rf_close,
    ddi_rf_poweron,
    ddi_rf_poweroff,
    ddi_rf_get_status,
    ddi_rf_activate,
    ddi_rf_exchange_apdu,
    ddi_rf_remove,
    ddi_rf_ioctl,
    FILL_NULL_POINTER_5,
    FILL_NULL_POINTER_8,
    FILL_NULL_POINTER_10,

    //6 热敏打印
    ddi_thmprn_open,
    ddi_thmprn_close,
    ddi_thmprn_feed_paper,
    ddi_thmprn_print_image,
    ddi_thmprn_print_image_file,
    ddi_thmprn_print_text,
    ddi_thmprn_print_comb_text,
    ddi_thmprn_get_status,
    ddi_thmprn_ioctl,
    ddi_thmprn_esc_p,
    ddi_thmprn_esc_loop,
    ddi_thmprn_esc_init,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_10,
        
    //7 按键
    ddi_key_open,
    ddi_key_close,
    ddi_key_clear,
    ddi_key_read,
    ddi_key_read_withoutdelay,
    ddi_key_ioctl,
//    ddi_powerkey_check,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_5,
    FILL_NULL_POINTER_1,  
    
    //8 显示
    ddi_lcd_open,
    ddi_lcd_close,
    ddi_lcd_fill_rect,
    ddi_lcd_clear_rect,
    ddi_lcd_show_text,
    ddi_lcd_show_picture,
    ddi_lcd_monochrome,
    ddi_lcd_show_picture_file,//显示图形文件
    ddi_lcd_show_pixel,
    ddi_lcd_show_line,//划线
    ddi_lcd_show_rect,
    ddi_lcd_extract_rect,
    ddi_lcd_ioctl,
    ddi_lcd_brush_screen,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_8,
//    FILL_NULL_POINTER_1,
    
    //9 音频
    ddi_audio_open,
    ddi_audio_close,
    ddi_audio_play,
    ddi_audio_playcharacters,
    ddi_audio_stop,
    ddi_audio_pause,
    ddi_audio_resumeplay,
    ddi_audio_ioctl,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_4,//5,
  #if 0
    //10 触摸屏设备
    ddi_tp_open,
    ddi_tp_close,
    ddi_tp_read_pixels,
    ddi_tp_clear,
    ddi_tp_ioctl,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_5,
    FILL_NULL_POINTER_2, 
    
    
    //11 无线通信
    ddi_wireless_open,
    ddi_wireless_close,
    ddi_wireless_destroy,
    ddi_wireless_dial,
    ddi_wireless_hangup,
    ddi_wireless_get_status ,
    ddi_wireless_sleep ,//内部保留使用
    ddi_wireless_wakeup,
    ddi_wireless_ioctl,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_2,
    FILL_NULL_POINTER_1,
    
    //12 modem
    ddi_modem_open ,
    ddi_modem_close,
    ddi_modem_dial,
    ddi_modem_hangup,
    ddi_modem_get_status,
    ddi_modem_read,
    ddi_modem_write,
    ddi_modem_ioctl,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_2,
    FILL_NULL_POINTER_2,
  #endif  
    //内置密钥
    ddi_innerkey_open,
    ddi_innerkey_close,
    ddi_innerkey_inject,
    ddi_innerkey_delete,
    ddi_innerkey_encrypt,
    ddi_innerkey_decrypt,
    ddi_innerkey_getHW21SNinfo,
    ddi_innerkey_ioctl,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_4,
    
    //14 虚拟文件系统
    ddi_file_create,  
    ddi_file_write,
    ddi_file_read,
    ddi_file_insert,
    ddi_file_delete,
    ddi_file_rename,
    ddi_file_getlength,
    ddi_file_veritysignature,
    static_vfs_getarea,  //9
    vfs_open,
    vfs_close,
    vfs_seek,
    vfs_tell,
    vfs_read,
    vfs_write,
    vfs_sync,
    vfs_deletefile,
    vfs_renamefile,
    vfs_free_space,   
    vfs_factory_initial,//20
    ddi_flash_read,
    ddi_flash_write,
    FILL_NULL_POINTER_10,
//    FILL_NULL_POINTER_2,

    //蓝牙
    ddi_bt_open,
    ddi_bt_close,
    ddi_bt_disconnect,
    ddi_bt_write,
    ddi_bt_read,
    ddi_bt_get_status,
    ddi_bt_ioctl,
    //FILL_NULL_POINTER_7,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_5,

    //GPRS
    ddi_gprs_open,
    ddi_gprs_close,
    ddi_gprs_get_signalquality,
    ddi_gprs_telephony_dial,
    ddi_gprs_telephony_hangup,
    ddi_gprs_telephony_answer,
    ddi_gprs_get_pdpstatus,
    ddi_gprs_set_apn,
    ddi_gprs_socket_create,
    ddi_gprs_socket_send,
    ddi_gprs_socket_recv,
    ddi_gprs_socket_close,
    ddi_gprs_socket_get_status,
    ddi_gprs_get_siminfo,
    ddi_gprs_get_imei,
    ddi_gprs_get_basestation_info,
    ddi_gprs_ioctl,
    //FILL_NULL_POINTER_7,
    //FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_5,

    //wifi
  #if 0 
	ddi_wifi_get_switchcfg,
	ddi_wifi_set_switchcfg,
	ddi_wifi_open,
	ddi_wifi_close,
	ddi_wifi_scanap,
	ddi_wifi_connetap,
	ddi_wifi_disconnetap,
	ddi_wifi_socket_create,
	ddi_wifi_socket_send,
	ddi_wifi_socket_recv,
	ddi_wifi_socket_close,
	ddi_wifi_get_signal,
	ddi_wifi_get_status,
	FILL_NULL_POINTER_15,
	FILL_NULL_POINTER_4, 
  #endif    
    ddi_wifi_open,
    ddi_wifi_close,
    ddi_wifi_scanap_start,
    ddi_wifi_scanap_status,
    ddi_wifi_connectap_start,
    ddi_wifi_connectap_status,
    ddi_wifi_disconnectap,
    ddi_wifi_socket_create_start,
    ddi_wifi_socket_get_status,
    ddi_wifi_socket_send,
    ddi_wifi_socket_recv,
    ddi_wifi_socket_close,
    ddi_wifi_get_signal,
    ddi_wifi_ioctl,
    ddi_wifi_start_airkiss,
    ddi_wifi_get_airkiss_status,
    ddi_wifi_get_airkiss_config,
    ddi_wifi_stop_airkiss,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_4,
    
    //16 LED
    ddi_led_open,
    ddi_led_close,
    ddi_led_sta_set,
    ddi_led_ioctl,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_5,
    FILL_NULL_POINTER_3,

#if 0
    //17 usb
    ddi_usb_open,
    ddi_usb_close,
    ddi_usb_read,
    ddi_usb_write,
    ddi_usb_ioctl,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_5,
    FILL_NULL_POINTER_2,
 #endif   

    //18 dukpt
    ddi_dukpt_open,     
    ddi_dukpt_close,    
    ddi_dukpt_inject,   
    ddi_dukpt_encrypt,  
    ddi_dukpt_decrypt,  
    ddi_dukpt_getksn,   
    ddi_dukpt_ioctl,  
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_5,

    //MBEDTLS SSL
    ddi_mbedtls_ssl_open,
    ddi_mbedtls_ssl_close,
    ddi_mbedtls_ssl_set_ca_cert,
    ddi_mbedtls_ssl_set_client_private_key,
    ddi_mbedtls_ssl_set_client_cert,
    ddi_mbedtls_ssl_handshake,
    ddi_mbedtls_ssl_send,
    ddi_mbedtls_ssl_recv,
    ddi_mbedtls_ssl_set_host_name,
    ddi_mbedtls_ioctl,
    ddi_mbedtls_verify_certchain,         
    ddi_mbedtls_parse_cert_info,   
    ddi_mbedtls_pk_verify_sign,
    ddi_mbedtls_sk_verify_sign,
    ddi_mbedtls_pk_sign,
    ddi_mbedtls_sk_sign,
    FILL_NULL_POINTER_1,
//    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_5,

    //电子签名
  #if 0
    ddi_elec_sign_open,
    ddi_elec_sign_get_data_size,                                       
    ddi_elec_sign_get_data,    
    ddi_elec_sign_process,                                                      
    ddi_elec_sign_start,                                                         
    ddi_elec_sign_stop,                                                         
    ddi_elec_sign_close,                                                       
    ddi_elec_sign_draw_text, 
    ddi_elec_sign_ioctl,
  #else
    FILL_NULL_POINTER_4,
    FILL_NULL_POINTER_5,
  #endif
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_3,

	// hardware arith
	ddi_arith_hash,
	ddi_arith_des,
	ddi_arith_getrand,
	ddi_arith_rsaencrypt,
	ddi_arith_rsadecrypt,
	ddi_arith_rsaskencrypt,
	ddi_arith_rsaskdecrypt,
	ddi_arith_rsarecover,
	ddi_arith_sm2veritysignature,
	ddi_arith_sm3,
	ddi_arith_sm4,
	ddi_arith_file_hash,
	ddi_arith_md5,
	ddi_arith_sm2encrypt,
	ddi_arith_sm2decrypt,
	ddi_arith_sm2skencrypt,
	ddi_arith_sm2skdecrypt,
	ddi_arith_sm2_digital_sign,
	ddi_arith_sm2_gen_PubAndPri_keys,
    FILL_NULL_POINTER_10,
	FILL_NULL_POINTER_3,    

	//21 watchdog
	ddi_watchdog_open,
    ddi_watchdog_close,
    ddi_watchdog_feed,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_5,
    FILL_NULL_POINTER_4, 
	
    //扫码及二维码生成
    ddi_QRcode_encodeString,
    ddi_QRcode_free,
    ddi_scanner_open,
    ddi_scanner_start,
    ddi_scanner_abort,
    ddi_scanner_getdata,
    ddi_scanner_close,
    ddi_scanner_ioctl,
    ddi_Barcode_encodeString,
    ddi_Barcode_free,
    //FILL_NULL_POINTER_7,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_10,
    FILL_NULL_POINTER_2,

    //APSP通讯接口    
    ddi_spnormalmanage_reset,
	ddi_spnormalmanage_hardwarereset,
	ddi_spnormalmanage_setsleeptime,
	ddi_spnormalmanage_getsleeptime,
	ddi_spnormalmanage_intosleep,
	ddi_spsecuremanage_getspstatus,
	ddi_spfirmwaremanage_intobootmode,
	ddi_spfirmwaremanage_exitbootmode,
	ddi_spfirmwaremanage_update,
	ddi_spfirmwaremanage_intocdcdatatospimode,
	ddi_spfirmwaremanage_spireadcdcdata,
	ddi_spfirmwaremanage_getspcertinfo,
	ddi_spnormalmanage_setdevicesn,
	ddi_spnormalmanage_getdevicesn,
	ddi_spnormalmanage_getversion,
	ddi_spnormalmanage_randnum,
	ddi_spsecuremanage_gettamperinfo,
	ddi_spsecuremanage_requestsecuredata,
	ddi_spsecuremanage_setsecuredata,
	ddi_spnormalmanage_beep,
	ddi_spsecuremanage_getdebugflag,
	ddi_spsecuremanage_opendryice,
	ddi_dev_apspcomm_command,
	ddi_secureprevent_getspinfo,
	ddi_secureprevent_sendbinddatatosp,
	ddi_secureprevent_saveappcertinfo,
	ddi_secureprevent_readappcertinfo,
	ddi_secureprevent_getsnkeycheckvalue,
    ddi_secureprevent_downloadsnkey,
    ddi_spsecuremanage_getencryptedranddata,
    ddi_spsecuremanage_dump_memoryinfo,
    FILL_NULL_POINTER_1,
};
#endif


