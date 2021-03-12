

#ifndef _DEV_DRYICE_H_
#define _DEV_DRYICE_H_


typedef enum _DRYICE_SET_TYPE
{    
    
    DRYICE_SET_POWERON = 0,  //…œµÁ≈‰÷√
    DRYICE_SET_NORMAL,
    DRYICE_SET_QUERY
    
}DRYICE_SET_TYPE;



#define EXTTAMPER_STATIC_LOW     0
#define EXTTAMPER_STATIC_HIGH    1
#define EXTTAMPER_DYNAMIC_OUT    2
#define EXTTAMPER_DYNAMIC_IN     3

//sxl?2017
//#define DRYICE_DEBUG



typedef struct{
    
    unsigned int dryice_tampervgdetected:1;  //µÁ—π√´¥Ã
    unsigned int dryice_tamperxtal32k:1;    // 32.768K
    unsigned int dryice_tampertoe:1; // Time Overflow Enable
    unsigned int dryice_tampermoe:1; //Monotonic Overflow Enable
    unsigned int dryice_tampervte:1;  //Voltage Tamper Flag
    unsigned int dryice_tampercte:1;  //CLOCK tampter flag
    unsigned int dryice_tampertte:1;  //Temperature Tamper
    unsigned int dryice_tamperste:1;  //Security Tamper Enable
    
    unsigned int dryice_tamperfse:1;  //Flash Security Enable
    unsigned int dryice_tampertme:1;  //Test Mode Enable
    unsigned int dryice_tampermesh:1; // mesh  tamper enable
    unsigned int dryice_reserved11:1;
    unsigned int dryice_reserved12:1;
    unsigned int dryice_reserved13:1;
    unsigned int dryice_reserved14:1;
    unsigned int dryice_reserved15:1;
    
    unsigned int dryice_tamperpin0:1;
    unsigned int dryice_tamperpin1:1;
    unsigned int dryice_tamperpin2:1;
    unsigned int dryice_tamperpin3:1;
    unsigned int dryice_tamperpin4:1;
    unsigned int dryice_tamperpin5:1;
    unsigned int dryice_tamperpin6:1;
    unsigned int dryice_tamperpin7:1;
    
    unsigned int dryice_reserved24:1;
    unsigned int dryice_reserved25:1;
    unsigned int dryice_reserved26:1;
    unsigned int dryice_reserved27:1;
    unsigned int dryice_reserved28:1;
    unsigned int dryice_reserved29:1;
    unsigned int dryice_reserved30:1;
    unsigned int dryice_reserved31:1;
    
    
    unsigned int dryice_tamperpin0_config:2;
    unsigned int dryice_tamperpin1_config:2;
    unsigned int dryice_tamperpin2_config:2;
    unsigned int dryice_tamperpin3_config:2;
    unsigned int dryice_tamperpin4_config:2;
    unsigned int dryice_tamperpin5_config:2;
    unsigned int dryice_tamperpin6_config:2;
    unsigned int dryice_tamperpin7_config:2;
    unsigned int dryice_reserved_config:16;
    
    }DRYICETAMPERCONFIG;



typedef union{
    DRYICETAMPERCONFIG dryicetamperconfig;
    uint32_t dryiceconfigregister[2];
}UNION_DRYICECONFIG;



extern void SENSOR_IRQHandler(void);
extern void SSC_IRQHandler(void);


extern s32 dev_dryice_open (DRYICETAMPERCONFIG *dryicetamperconfig,u8 type);
extern s32 dev_dryice_writesecurekey(u8 *securekey,u32 securekeylen);
extern s32 dev_dryice_readsecurekey(u8 *securekey);
extern u32 dev_dryice_gettamperstatus(void);
extern u32 dev_dryice_gettamperstatusonly(void);


extern s32 pcitamper_manage_dryice_isrhandler(u32 drysrreg);
extern s32 dev_dryice_checkresetstatus(void);
extern void dev_dryice_poweron(void);
extern u32 dev_dryice_poweronreadstatus(void);
extern void dev_dryice_reset(void);

extern void testdryice(void);

#endif

