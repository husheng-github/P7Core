

#ifndef _PCITASK_PROTOCOL_KEYDOWN_H_
#define _PCITASK_PROTOCOL_KEYDOWN_H_

#define UARTPORT_PCI 0

typedef struct _COMMUNICATEDATA
{
    unsigned int serialno;
	unsigned int XGD_RFU[4];		// 20110214 hhl adds according to the new specification
	unsigned int command;
	unsigned int datalen;
	unsigned char  data[1024+1024];    //reserve 1024 bytes for future use
	unsigned int RxCNT;
	unsigned int Rx_Valid;
}COMMUNICATEDATA;

extern COMMUNICATEDATA *gCommData;
#define COMMUNICATEDATAPOS 11


//#define PORT_PC  

#define COMMDATABUFMAXLEN   1024


extern void pcitask_protocol_communicationinit(void);
extern void pcitask_protocol_communicationexit(void);
extern void pcitask_protocol_parse(void);
extern void pcitask_protocol_keydownload(void);

#endif

