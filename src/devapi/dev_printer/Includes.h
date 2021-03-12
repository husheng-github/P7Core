#ifndef  _INCLUDES_H_
#define __INCLUDES_H__


#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "config.h"

#include "dev_pt48d.h"
#include "config-param.h"

#include "Type.h"
#include "Event.h"
#include "Esc_p.h"


#include "dotfill.h"
#include "font.h"
#include "Barcode1D.h"


#if defined(SUPPORT_QRCode)
#include "qr.h"
#endif
//======================================================================================================

//======================================================================================================
#define ARRAY_SIZE(arr)		(sizeof(arr) / sizeof(arr[0]))
//======================================================================================================


#endif

