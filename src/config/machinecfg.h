#ifndef __MACHINECFG_H
#define __MACHINECFG_H
//CUP�źŶ���
#define MH1903_121  0
#define MH1903_88   1
#define MH1903_TYPE   MH1903_88

//�������
#if 0
typedef enum _machinetype
{
    MACHINE_M2 = 0,     //mpos
    MACHINE_S1,         //����POS
}machinetype_t;
#endif

#define MACHINE_M2  0     //mpos
#define MACHINE_S1  1         //����POS
#if 1
#define MACHINETYPE    MACHINE_S1   
#else
#define MACHINETYPE    MACHINE_M2  //M2PCI
#endif

#define CAT_AB(a, b)       a##b
#define CODE_HEX(a)        CAT_AB (0x, a)
#define STR(a)             #a

#define MACHINE_P6_VALUE   56
#define MACHINE_P6         "56"

#define MACHINE_P7_VALUE   57
#define MACHINE_P7         "57"


//#define MACHINE_P6_NEW      1

//#define MACHINE_P6_WIFI_WITH_CTS

//�����������ַ
#define MCU_BASE_ADDR   0x1000000
#define BOOT_START_ADDR    0x1000

#define BOOT_SIZE_MAX      0x16000
#define CORE_START_ADDR    0x18000
#define CORE_SIZE_MAX      0x60000
#define CORE_DDI_ADDR       (CORE_START_ADDR+0x1000)
#define APP_START_ADDR     (CORE_START_ADDR+CORE_SIZE_MAX) //0x48000
#define APP_SIZE_MAX       0x48000
#define APP_CORE_START     (APP_START_ADDR+0x200)

#define MACHINEPARAM_START_ADDR     0xFD000//??Ŀǰ���԰Ѳ�����������//0x17000
#define MACHINEPARAM_SIZE_MAX       0x1000
#define MACHINEPARAMBAK_START_ADDR  0xFE000
#define MACHINEPARAMBAK_SIZE_MAX    0x1000
#define KEY_START_ADDR              0xFF000
#define KEY_SIZE_MAX                0x1000


#define GBK2424_START_ADDR          0x0
#define GBK2424_SIZE_MAX            0x1A6000
#define GBK1616_START_ADDR          0x1A6000
#define GBK1616_SIZE_MAX            0xBC000
#define TTS_START_ADDR              0x262000
#define TTS_SIZE_MAX                0x100000
#define APUPDATE_SPACE_EXIST        1         
#if(APUPDATE_SPACE_EXIST==1)
#define APUPDATE_START_ADDR         0x370000  // apupdate
#define APUPDATE_SIZE_MAX           0xB0000   //704K
#else
#define APUPDATE_START_ADDR         0x370000  // apupdate
#define APUPDATE_SIZE_MAX           0x0  
#endif
#define LFS_START_ADDR              (APUPDATE_START_ADDR+APUPDATE_SIZE_MAX)
#define LFS_SIZE_MAX                (0x390000-APUPDATE_SIZE_MAX)


#if(MACHINETYPE==MACHINE_S1)
#define MCU_EXT12M_EN   0       //�ⲿ����ʹ��
#define SPI_SLAVE_EN    0       //SPI��ģʽ����
#define SPIFLASH_EXIST  1       
#define QSPIFLASH_EXIST 1       
#define EXTERNAL_SPIFLASH_ENABLE  //�ⲿFLASHʹ��
#define KEYPAD_EXIST    1       //����������
#define AUDIO_EXIST     1       
#define LCD_EXIST       0       //LCD�Ƿ����
#define PSAM_EXIST      0       //PSAM������      //��ʱ����ͻ
#define BT_EXIST        0       //����������
#define LITTLEFS_EXIST  0       //littlefs�ļ�ϵͳ�Ƿ����
#ifdef TRENDIT_CORE
#define WIFI_EXIST      1       //WIFI����
#define WIRELESS_EXIST  1       //����ģ�����
#define PRINTER_EXIST   1       //��ӡ������
#define MAG_EXIST       0       //�ſ��Ƿ����
#define USBD_CDC_EXIST  1     //
#define DEBUG_PORT_NO  PORT_CDCD_NO//PORT_CDCD_NO //PORT_UART0_NO //
#else  //BOOT
#define WIFI_EXIST      0       //WIFI����
#define WIRELESS_EXIST  0       //����ģ�����
#define PRINTER_EXIST   0       //��ӡ������
#define MAG_EXIST       0       //�ſ��Ƿ����
#define USBD_CDC_EXIST  1       //
#define DEBUG_PORT_NO   PORT_UART0_NO  //PORT_CDCD_NO //PORT_UART0_NO //
#endif
#if 1
#define MCU_UART_BAUD   115200 //1000000//750000//
#define MCU_FLOWCTL_FLG  0  
#else
#define MCU_UART_BAUD  3000000 
#define MCU_FLOWCTL_FLG  1   
#endif  
#endif
//==========================================
//debug����
#define DEBUG_ALL_EN
#undef  DEBUG_ALL_EN

#ifdef DEBUG_ALL_EN
#define SYS_APP_DEBUG_EN
#define DEBUG_BOOT_EN   
#define DEBUG_MISC_EN               //����豸����ʹ��
#define DEBUG_TIMER_EN              //��ʱ���豸����ʹ��
#define DEBUG_SPI_EN                //SPI�豸����ʹ��
#define DEBUG_SPIFLASH_EN           //SPIFLASH��������ʹ��
//#define DEBUG_INNERFLASH_EN         //�ڲ�FLASH��������ʹ��
#define DEBUG_QSPIFLASH_EN          //SPIFLASH��������ʹ��
#define DEBUG_RF_EN                 //�ǽ���������ʹ��
#define DEBUG_MAG_EN                //�ſ���������ʹ��
#define DEBUG_ICC_EN                //ICC��������ʹ��
#define DEBUG_PSAM_EN               //PSAM��������ʹ��
#define DEBUG_AUDIO_EN              //��Ƶ����ʹ��
#define DEBUG_KEYPAD_EN             //��������ʹ��
#define DEBUG_LCD_EN                //LCD����ʹ��
#define DEBUG_BT_EN                 //BT����ʹ��
#define DEBUG_POWER_EN              //��Դ����ʹ��
#define DEBUG_WIFI_EN               //WIFI����ʹ��

#endif
//==========================================
//#define PCI_TAMPLE_DISABLE          //��������

//

#endif
