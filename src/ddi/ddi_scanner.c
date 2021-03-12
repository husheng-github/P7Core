

#include "ddi_scanner.h"

#ifdef DDI_SCANNER_ENABLE

QRcode *ddi_QRcode_encodeString(const char *string, int version, QRecLevel level, QRencodeMode hint, int casesensitive)
{
	return NULL;
}
void ddi_QRcode_free(QRcode *qrcode)
{
//	return DDI_ENODEV;
}
s32 ddi_scanner_open(void)
{
	return DDI_ENODEV;
}
s32 ddi_scanner_start(SCANNER_MOD_t mod)
{
	return DDI_ENODEV;
}
s32 ddi_scanner_abort(void)
{
	return DDI_ENODEV;
}
s32 ddi_scanner_getdata(void* data, u32 datalen)
{
	return DDI_ENODEV;
}
s32 ddi_scanner_close(void)
{
	return DDI_ENODEV;
}
s32 ddi_scanner_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	return DDI_ENODEV;
}
Barcode*  ddi_Barcode_encodeString(const char *string, BARCODE_TYPE barcode_type)
{
    return DDI_ENODEV;
}

void  ddi_Barcode_free(Barcode* barcode)
{
    return DDI_ENODEV;
}

#else
QRcode *ddi_QRcode_encodeString(const char *string, int version, QRecLevel level, QRencodeMode hint, int casesensitive)
{
	return NULL;
}
void ddi_QRcode_free(QRcode *qrcode)
{
	//return DDI_ENODEV;
}
s32 ddi_scanner_open(void)
{
	return DDI_ENODEV;
}
s32 ddi_scanner_start(SCANNER_MOD_t mod)
{
	return DDI_ENODEV;
}
s32 ddi_scanner_abort(void)
{
	return DDI_ENODEV;
}
s32 ddi_scanner_getdata(void* data, u32 datalen)
{
	return DDI_ENODEV;
}
s32 ddi_scanner_close(void)
{
	return DDI_ENODEV;
}

s32 ddi_scanner_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	return DDI_ENODEV;
}

Barcode*  ddi_Barcode_encodeString(const char *string, BARCODE_TYPE barcode_type)
{
    return NULL;
}
void  ddi_Barcode_free(Barcode* barcode)
{
    return;//DDI_ENODEV;
}


#endif


