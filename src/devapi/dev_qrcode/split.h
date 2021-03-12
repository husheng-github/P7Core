
#ifndef __SPLIT_H__
#define __SPLIT_H__

#include "qrencode.h"

extern int Split_splitStringToQRinput(const char *string, QRinput *input,
		QRencodeMode hint, int casesensitive);

#endif 
