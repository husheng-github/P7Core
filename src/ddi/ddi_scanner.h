

#ifndef _DDI_SCANNER_H_
#define _DDI_SCANNER_H_


#include "ddi_common.h"


//扫码及二维码接口
typedef enum {
	QR_ECLEVEL_L = 0, ///< lowest
	QR_ECLEVEL_M,
	QR_ECLEVEL_Q,
	QR_ECLEVEL_H      ///< highest
} QRecLevel;

typedef enum {
	QR_MODE_NUL = -1,   ///< Terminator (NUL character). Internal use only
	QR_MODE_NUM = 0,    ///< Numeric mode
	QR_MODE_AN,         ///< Alphabet-numeric mode
	QR_MODE_8,          ///< 8-bit data mode
	QR_MODE_KANJI,      ///< Kanji (shift-jis) mode
	QR_MODE_STRUCTURE,  ///< Internal use only
	QR_MODE_ECI,        ///< ECI mode
	QR_MODE_FNC1FIRST,  ///< FNC1, first position
	QR_MODE_FNC1SECOND, ///< FNC1, second position
} QRencodeMode;
typedef struct {
	int version;         ///< version of the symbol
	int width;           ///< width of the symbol
	unsigned char *data; ///< symbol data
} QRcode;
typedef enum SCANNER_MOD
{
    SCANNER_MODE_CONTINUE   = 0,
    SCANNER_MODE_ONCE       = 1,
    SCANNER_MODE_MOTIONDETECT = 2,
    SCANNER_MODE_ONED_ONLY  = 3,
    SCANNER_MODE_MOTIONDETECT_MINI = 4,
    SCANNER_MODE_MAX,
}SCANNER_MOD_t;

typedef enum {
    BARCODE_ANY = 0,     /* choose best-fit */
    BARCODE_EAN,
    BARCODE_UPC,        /* upc == 12-digit ean */
    BARCODE_ISBN,       /* isbn numbers (still EAN13) */
    BARCODE_39,         /* code 39 */
    BARCODE_128,        /* code 128 (a,b,c: autoselection) */
    BARCODE_128C,       /* code 128 (compact form for digits) */
    BARCODE_128B,       /* code 128, full printable ascii */
    BARCODE_I25,        /* interleaved 2 of 5 (only digits) */
    BARCODE_128RAW,     /* Raw code 128 (by Leonid A. Broukhis) */
    BARCODE_CBR,        /* Codabar (by Leonid A. Broukhis) */
    BARCODE_MSI,        /* MSI (by Leonid A. Broukhis) */
    BARCODE_PLS,        /* Plessey (by Leonid A. Broukhis) */
    BARCODE_93          /* code 93 (by Nathan D. Holmes) */
}BARCODE_TYPE;


typedef struct {
    int length;//长度
    char *data;//一行的点的数据
}Barcode;

//=====================================================
//对外函数声明
QRcode *ddi_QRcode_encodeString(const char *string, int version, QRecLevel level, QRencodeMode hint, int casesensitive);
void ddi_QRcode_free(QRcode *qrcode);
s32 ddi_scanner_open(void);
s32 ddi_scanner_start(SCANNER_MOD_t mod);
s32 ddi_scanner_abort(void);
s32 ddi_scanner_getdata(void* data, u32 datalen);
s32 ddi_scanner_close(void);
s32 ddi_scanner_ioctl(u32 nCmd, u32 lParam, u32 wParam);
Barcode*  ddi_Barcode_encodeString(const char *string, BARCODE_TYPE barcode_type);
void  ddi_Barcode_free(Barcode* barcode);

//===============================================
typedef QRcode *(*core_ddi_QRcode_encodeString)(const char *string, int version, QRecLevel level, QRencodeMode hint, int casesensitive);
typedef void (*core_ddi_QRcode_free)(QRcode *qrcode);
typedef s32 (*core_ddi_scanner_open)(void);
typedef s32 (*core_ddi_scanner_start)(SCANNER_MOD_t mod);
typedef s32 (*core_ddi_scanner_abort)(void);
typedef s32 (*core_ddi_scanner_getdata)(void* data, u32 datalen);
typedef s32 (*core_ddi_scanner_close)(void);
typedef s32 (*core_ddi_scanner_ioctl)(u32 nCmd, u32 lParam, u32 wParam);
typedef Barcode* (*core_ddi_Barcode_encodeString)(const char *string, BARCODE_TYPE barcode_type);
typedef void  (*core_ddi_Barcode_free)(Barcode* barcode);
#endif



