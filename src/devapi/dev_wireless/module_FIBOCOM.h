#ifndef __MODULE_FIBOCOM_H
#define __MODULE_FIBOCOM_H


typedef enum 
{
    SIMCARDTYPE_NONE,  //SIM卡不存在,这种情况默认发送LTE指令
    SIMCARDTYPE_GSM,   
    SIMCARDTYPE_LTE    
}SIMCARDTYPE;



#define FIBOCOM_G500_MOULE_KEY_WORDS                       "35050.1000"     //纯modem关键字
#define FIBOCOM_G500_OPEN_CPU_MOULE_KEY_WORDS              "35000.1000"     //open cpu关键字

#define FIBOCOM_L610_MOULE_KEY_WORDS                       "16000"



#endif
