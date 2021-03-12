#include "includes.h"

#define BARCODE_BAR_WIDTH	(2)		// (3) 宽条码所占的点数

static uint8_t PrintBarcode_GetBarcodeInput(uint8_t *barcode, uint8_t max, uint8_t m)
{
	uint8_t i, c, n;

	if(m < 64)
	{
		n = 255;
	}
	else
	{
		n = Getchar();
	}
	i = 0;
	do
	{
		c = Getchar();
		if((m < 64) && (c == 0))
		{
			break;
		}
		if(i < max)
		{
			*barcode++ = c;
		}
	}while((++i) < n);
	return i;
}

static uint8_t PrintBarcode_CalcEanUpcChecksum(uint8_t *barcode, uint8_t len)
{
	uint8_t c1, c2, cc;
	uint8_t i;

	c1 = c2 = 0;
	i = len;
	while(1)
	{
		c1 += (*barcode-'0');
		if((--i) == 0) break;
		barcode++;
		c2 += (*barcode-'0');
		if((--i) == 0) break;
		barcode++;
	}
	if(len & 0x01)
	{
		cc = (c1*3+c2)%10;
	}
	else
	{
		cc = (c1+c2*3)%10;
	}
	if(cc == 0)
	{
		return '0';
	}
	else
	{
		return (('0'+10) - cc);
	}
}

static uint16_t PrintBarcodePixel(uint32_t pixel, uint8_t cnt, uint8_t *dot, uint16_t col)
{
	uint8_t j;
	if(col == esc_sts.leftspace)//set leftspace and relative print position ---zxq add
	{
		col += esc_sts.start_dot;
	}
	do
	{
		cnt--;
		if(pixel & (1UL << cnt))
		{
			for(j=0; j<esc_sts.barcode_width; j++)
			{
				if(col < LineDot)
				{
					dot[col >> 3] |= (1 << (7 - (col & 0x07)));
				}
				col++;
			}
		}
		else
		{
			col += esc_sts.barcode_width;
		}
	}while(cnt);
	return col;
}

static void PrintBarcode_HRI_Characters(uint8_t *barcode, uint8_t n, uint16_t col)
{
	for(; n; n--)
	{
		GetEnglishHRIFont(*barcode++);
	}
	PrintCurrentBuffer(0);
}

static void Barcode_Print_Align(uint8_t *dot, uint16_t col)
{
	uint8_t tmp;
	uint8_t dot_tmp[LineDot/8];
	int8_t bits;
	uint16_t i,start_col=0;
	uint16_t bit = 0;

//===============条码打印位置处理===================================================
	memset(dot_tmp,0,sizeof(dot_tmp));


	switch(esc_sts.align)
	{
		default:
		case 0:
			start_col = 0;
			break;
		case 1:
			start_col = (LineDot - col)/2;
			break;
		case 2:
			start_col = LineDot - col;
			break;
	}
	if(esc_sts.upside_down)
	{
		bits = 7;
		bit = (LineDot -1) - start_col;
		for(i=0;i<col;i++)
		{
			tmp = dot[i>>3];
			if(tmp&(1<<bits))
			{
				dot_tmp[bit>>3] |= (1<<(7-(bit & 0x07)));
			}
			else
			{
				dot_tmp[bit>>3] &= ~(1<<(7-(bit & 0x07)));
			}
			bit --;
			bits --;
			if(bits<0)
			{
				bits = 7;
			}
		}
	}
	else
	{
		bits = 7;
		bit = start_col;
		for(i=0;i<col;i++)
		{
			tmp = dot[i>>3];
			if(tmp&(1<<bits))
			{
				dot_tmp[bit>>3] |= (1<<(7-(bit & 0x07)));
			}
			else
			{
				dot_tmp[bit>>3] &= ~(1<<(7-(bit & 0x07)));
			}
			bit ++;
			bits --;
			if(bits<0)
			{
				bits = 7;
			}
		}
	}
	for(i=0; i<esc_sts.barcode_height; i++)
	{
		TPPrintRasterLine(dot_tmp);
	}

}

static void PrintBarcode(uint8_t *barcode, uint8_t n, uint8_t *dot, uint16_t col)
{

	if(col > LineDot)
	{
		TPFeedLine(esc_sts.barcode_height);
		return;
	}
	#if 0
	if(esc_sts.barcode_char_pos & (1 << 0))
	{
		PrintBarcode_HRI_Characters(barcode, n, col);
	}

	// 打印条码
	/*for(i=0; i<esc_sts.barcode_height; i++)
	{
		TPPrintLine(dot);
	}*/

	Barcode_Print_Align(dot,col);

	if(esc_sts.barcode_char_pos & (1 << 1))
	{
		PrintBarcode_HRI_Characters(barcode, n, col);
	}
	#endif

	if(esc_sts.upside_down)
	{
		if(esc_sts.barcode_char_pos & (1 << 1))
		{
			PrintBarcode_HRI_Characters(barcode, n, col);
		}

		// 打印条码
		/*for(i=0; i<esc_sts.barcode_height; i++)
		{
			TPPrintLine(dot);
		}*/
		Barcode_Print_Align(dot,col);
		TPFeedLine(4);  //设置条码和HRI字符之间的间距
		if(esc_sts.barcode_char_pos & (1 << 0))
		{
			PrintBarcode_HRI_Characters(barcode, n, col);
		}
	}
	else
	{
		if(esc_sts.barcode_char_pos & (1 << 0))
		{
			PrintBarcode_HRI_Characters(barcode, n, col);
		}

		// 打印条码
		/*for(i=0; i<esc_sts.barcode_height; i++)
		{
			TPPrintLine(dot);
		}*/
		Barcode_Print_Align(dot,col);
		TPFeedLine(4);
		if(esc_sts.barcode_char_pos & (1 << 1))
		{
			PrintBarcode_HRI_Characters(barcode, n, col);
		}
	}
 //   printf("打印完成\n");
}
/*
extern void PrintBarcode_UPC_A(uint8_t m)
{
	uint8_t dot[LineDot/8];
	uint16_t col;
	uint8_t barcode[12];
	uint8_t i, n;

	uint8_t const left_encode[] =
	{
		0x05,		// 101
	};

	uint8_t const a_encode[] =
	{
		0x0d,		// 0001101
		0x19,		// 0011001
		0x13,		// 0010011
		0x3d,		// 0111101
		0x23,		// 0100011
		0x31,		// 0110001
		0x2f,		// 0101111
		0x3b,		// 0111011
		0x37,		// 0110111
		0x0b,		// 0001011
	};

	uint8_t const s_encode[] =
	{
		0x0a,		// 01010
	};

	uint8_t const b_encode[] =
	{
		0x72,		// 1110010
		0x66,		// 1100110
		0x6c,		// 1101100
		0x42,		// 1000010
		0x5c,		// 1011100
		0x4e,		// 1001110
		0x50,		// 1010000
		0x44,		// 1000100
		0x48,		// 1001000
		0x74,		// 1110100
	};

	uint8_t const right_encode[] =
	{
		0x05,		// 101
	};

	// 接收条码命令
	n = PrintBarcode_GetBarcodeInput(barcode, sizeof(barcode), m);
	// 检查条码格式的合法性
	if((n < 11) || (n > 12))
	{
		return;
	}
	for(i=0; i<n; i++)
	{
		if(isdigit(barcode[i]) == 0)
		{
			return;
		}
	}
	// 计算条码校验位
	if(n == 11)
	{
		barcode[11] = PrintBarcode_CalcEanUpcChecksum(barcode, 11);
	}

	memset(dot, 0, sizeof(dot));
	//col = 0;
    col = esc_sts.leftspace;
	// 打印左护码
	col = PrintBarcodePixel(left_encode[0], 3, dot, col);
	// 打印左资料码
	for(i=0; i<=5; i++)
	{
		col = PrintBarcodePixel(a_encode[(barcode[i]-'0')], 7, dot, col);
	}
	// 打印中线
	col = PrintBarcodePixel(s_encode[0], 5, dot, col);
	// 打印右资料码
	for(; i<= 11; i++)
	{
		col = PrintBarcodePixel(b_encode[(barcode[i]-'0')], 7, dot, col);
	}
	// 打印右护码
	col = PrintBarcodePixel(right_encode[0], 3, dot, col);

	// 打印条码
	PrintBarcode(barcode, sizeof(barcode), dot, col);
}

extern void PrintBarcode_UPC_E(uint8_t m)
{
	uint8_t dot[LineDot/8];
	uint16_t col;
	uint8_t barcode[12];
	uint8_t barcode_e[8];
	uint8_t i, n;
	uint8_t c;

	uint8_t const ab_encode[] =
	{
		0x38,		// 111000
		0x34,		// 110100
		0x32,		// 110010
		0x31,		// 110001
		0x2c,		// 101100
		0x26,		// 100110
		0x23,		// 100011
		0x2a,		// 101010
		0x29,		// 101001
		0x25,		// 100101
	};

	uint8_t const left_encode[] =
	{
		0x05,		// 101
	};

	uint8_t const odd_encode[] =
	{
		0x0d,		// 0001101
		0x19,		// 0011001
		0x13,		// 0010011
		0x3d,		// 0111101
		0x23,		// 0100011
		0x31,		// 0110001
		0x2f,		// 0101111
		0x3b,		// 0111011
		0x37,		// 0110111
		0x0b,		// 0001011
	};

	uint8_t const even_encode[] =
	{
		0x27,		// 0100111
		0x33,		// 0110011
		0x1b,		// 0011011
		0x21,		// 0100001
		0x1d,		// 0011101
		0x39,		// 0111001
		0x05,		// 0000101
		0x11,		// 0010001
		0x09,		// 0001001
		0x17,		// 0010111
	};

	uint8_t const right_encode[] =
	{
		0x15,		// 010101
	};


	// 接收条码命令
	n = PrintBarcode_GetBarcodeInput(barcode, sizeof(barcode), m);
	// 检查UPC-A条码格式的合法性
	if((n < 11) || (n > 12))
	{
		return;
	}

	for(i=0; i<n; i++)
	{

        if(!((barcode[i] >= '0') && (barcode[i] <= '9')))
//		if(isdigit(barcode[i]) == 0)
		{
			return;
		}
	}
	// 计算条码校验位
	if(n == 11)
	{
		barcode[11] = PrintBarcode_CalcEanUpcChecksum(barcode, 11);
	}
	// 检查是否符合UPC-E编码规则
	if(barcode[0] != '0')
	{
		return;
	}
	barcode_e[0] = '0';
	if((barcode[3] <= '2') && (barcode[4] == '0') && (barcode[5] == '0') && (barcode[6] == '0') && (barcode[7] == '0'))
	{
		barcode_e[1] = barcode[1];
		barcode_e[2] = barcode[2];
		barcode_e[3] = barcode[8];
		barcode_e[4] = barcode[9];
		barcode_e[5] = barcode[10];
		barcode_e[6] = barcode[3];
		barcode_e[7] = barcode[11];
	}
	else if((barcode[4] == '0') && (barcode[5] == '0') && (barcode[6] == '0') && (barcode[7] == '0') && (barcode[8] == '0'))
	{
		barcode_e[1] = barcode[1];
		barcode_e[2] = barcode[2];
		barcode_e[3] = barcode[3];
		barcode_e[4] = barcode[9];
		barcode_e[5] = barcode[10];
		barcode_e[6] = '3';
		barcode_e[7] = barcode[11];
	}
	else if((barcode[5] == '0') && (barcode[6] == '0') && (barcode[7] == '0') && (barcode[8] == '0') && (barcode[9] == '0'))
	{
		barcode_e[1] = barcode[1];
		barcode_e[2] = barcode[2];
		barcode_e[3] = barcode[3];
		barcode_e[4] = barcode[4];
		barcode_e[5] = barcode[10];
		barcode_e[6] = '4';
		barcode_e[7] = barcode[11];
	}
	else if((barcode[6] == '0') && (barcode[7] == '0') && (barcode[8] == '0') && (barcode[9] == '0') && (barcode[10] >= '5'))
	{
		barcode_e[1] = barcode[1];
		barcode_e[2] = barcode[2];
		barcode_e[3] = barcode[3];
		barcode_e[4] = barcode[4];
		barcode_e[5] = barcode[5];
		barcode_e[6] = barcode[10];
		barcode_e[7] = barcode[11];
	}
	else
	{
		return;
	}

	memset(dot, 0, sizeof(dot));
	//col = 0;
	col = esc_sts.leftspace;
	// 打印左护码
	col = PrintBarcodePixel(left_encode[0], 5, dot, col);
	// 打印资料码
	c = ab_encode[(barcode_e[7]-'0')];
	for(i=1; i<=6; i++)
	{
		if(c & (1 << (6-i)))
		{
			col = PrintBarcodePixel(even_encode[(barcode_e[i]-'0')], 7, dot, col);
		}
		else
		{
			col = PrintBarcodePixel(odd_encode[(barcode_e[i]-'0')], 7, dot, col);
		}
	}
	// 打印右护码
	col = PrintBarcodePixel(right_encode[0], 6, dot, col);

	// 打印条码
	PrintBarcode(barcode_e, sizeof(barcode_e), dot, col);
}
*/
extern void PrintBarcode_EAN13(uint8_t m)
{
	uint8_t dot[LineDot/8];
	uint16_t col;
	uint8_t barcode[13];
	uint8_t i, n;


	uint8_t const ab_encode[] =
	{
		0x00,		// AAAAAA
		0x0b,		// AABABB
		0x0d,		// AABBAB
		0x0e,		// AABBBA
		0x13,		// ABAABB
		0x19,		// ABBAAB
		0x1c,		// ABBBAA
		0x15,		// ABABAB
		0x16,		// ABABBA
		0x1a,		// ABBABA
	};

	uint8_t const left_encode[] =
	{
		0x05,		// 101
	};

	uint8_t const a_encode[] =
	{
		0x0d,		// 0001101
		0x19,		// 0011001
		0x13,		// 0010011
		0x3d,		// 0111101
		0x23,		// 0100011
		0x31,		// 0110001
		0x2f,		// 0101111
		0x3b,		// 0111011
		0x37,		// 0110111
		0x0b,		// 0001011
	};

	uint8_t const b_encode[] =
	{
		0x27,		// 0100111
		0x33,		// 0110011
		0x1b,		// 0011011
		0x21,		// 0100001
		0x1d,		// 0011101
		0x39,		// 0111001
		0x05,		// 0000101
		0x11,		// 0010001
		0x09,		// 0001001
		0x17,		// 0010111
	};

	uint8_t const s_encode[] =
	{
		0x0a,		// 01010
	};

	uint8_t const c_encode[] =
	{
		0x72,		// 1110010
		0x66,		// 1100110
		0x6c,		// 1101100
		0x42,		// 1000010
		0x5c,		// 1011100
		0x4e,		// 1001110
		0x50,		// 1010000
		0x44,		// 1000100
		0x48,		// 1001000
		0x74,		// 1110100
	};

	uint8_t const right_encode[] =
	{
		0x05,		// 101
	};

	// 接收条码命令
	n = PrintBarcode_GetBarcodeInput(barcode, sizeof(barcode), m);
	// 检查条码格式的合法性
	if((n < 12) || (n > 13))
	{
		return;
	}
	for(i=0; i<n; i++)
	{
  //      printf("0x%X ",barcode[i]);
 //      printf("%d ",isdigit(barcode[i]));
	//	if(isdigit(barcode[i]) == 0)//程序出错:运行几次后barcode[i]==0
        if(!((barcode[i] >= '0') && (barcode[i] <= '9')))
		{
 //           printf("Return\n");
			return;
		}

	}
	// 计算条码校验位
	if(n == 12)
	{
		barcode[12] = PrintBarcode_CalcEanUpcChecksum(barcode, 12);
	}
	memset(dot, 0, sizeof(dot));
	//col = 0;
	col = esc_sts.leftspace;

	// 打印左护码
	col = PrintBarcodePixel(left_encode[0], 3, dot, col);
	// 打印左资料码
	for(i=1; i<=6; i++)
	{
		if(ab_encode[(barcode[0]-'0')] & (1 << (6-i)))
		{
			col = PrintBarcodePixel(b_encode[(barcode[i]-'0')], 7, dot, col);
		}
		else
		{
			col = PrintBarcodePixel(a_encode[(barcode[i]-'0')], 7, dot, col);
		}
	}
	// 打印中线
	col = PrintBarcodePixel(s_encode[0], 5, dot, col);
	// 打印右资料码
	for(; i<= 12; i++)
	{
		col = PrintBarcodePixel(c_encode[(barcode[i]-'0')], 7, dot, col);
	}
	// 打印右护码
	col = PrintBarcodePixel(right_encode[0], 3, dot, col);

	// 打印条码
//	printf("打印条码...\n");
	PrintBarcode(barcode, sizeof(barcode), dot, col);
}
/*
extern void PrintBarcode_EAN8(uint8_t m)
{
	uint8_t dot[LineDot/8];
	uint16_t col;
	uint8_t barcode[8];
	uint8_t i, n;

	uint8_t const left_encode[] =
	{
		0x05,		// 101
	};

	uint8_t const a_encode[] =
	{
		0x0d,		// 0001101
		0x19,		// 0011001
		0x13,		// 0010011
		0x3d,		// 0111101
		0x23,		// 0100011
		0x31,		// 0110001
		0x2f,		// 0101111
		0x3b,		// 0111011
		0x37,		// 0110111
		0x0b,		// 0001011
	};

	uint8_t const s_encode[] =
	{
		0x0a,		// 01010
	};

	uint8_t const c_encode[] =
	{
		0x72,		// 1110010
		0x66,		// 1100110
		0x6c,		// 1101100
		0x42,		// 1000010
		0x5c,		// 1011100
		0x4e,		// 1001110
		0x50,		// 1010000
		0x44,		// 1000100
		0x48,		// 1001000
		0x74,		// 1110100
	};

	uint8_t const right_encode[] =
	{
		0x05,		// 101
	};

	// 接收条码命令
	n = PrintBarcode_GetBarcodeInput(barcode, sizeof(barcode), m);
	// 检查条码格式的合法性
	if((n < 7) || (n > 8))
	{
		return;
	}
	for(i=0; i<n; i++)
	{
	//	if(isdigit(barcode[i]) == 0)

    if(!((barcode[i] >= '0') && (barcode[i] <= '9')))
		{
			return;
		}
	}
	// 计算条码校验位
	if(n == 7)
	{
		barcode[7] = PrintBarcode_CalcEanUpcChecksum(barcode, 7);
	}
	memset(dot, 0, sizeof(dot));
    //col = 0;
	col = esc_sts.leftspace;
	// 打印左护码
	col = PrintBarcodePixel(left_encode[0], 3, dot, col);
	// 打印左资料码
	for(i=0; i<=3; i++)
	{
		col = PrintBarcodePixel(a_encode[(barcode[i]-'0')], 7, dot, col);
	}
	// 打印中线
	col = PrintBarcodePixel(s_encode[0], 5, dot, col);
	// 打印右资料码
	for(; i<= 7; i++)
	{
		col = PrintBarcodePixel(c_encode[(barcode[i]-'0')], 7, dot, col);
	}
	// 打印右护码
	col = PrintBarcodePixel(right_encode[0], 3, dot, col);

	// 打印条码
	PrintBarcode(barcode, sizeof(barcode), dot, col);
}*/
static uint8_t PrintBarcode_CODE39_ASCII_to_Hex(uint8_t ascii)
{
	if((ascii >= '0') && (ascii <= '9'))
	{
		return (ascii-'0');
	}
	if((ascii >= 'A') && (ascii <= 'Z'))
	{
		return (ascii-'A'+10);
	}
	if(ascii == '-')
	{
		return 36;
	}
	if(ascii == '.')
	{
		return 37;
	}
	if(ascii == ' ')
	{
		return 38;
	}
	if(ascii == '$')
	{
		return 39;
	}
	if(ascii == '/')
	{
		return 40;
	}
	if(ascii == '+')
	{
		return 41;
	}
	if(ascii == '%')
	{
		return 42;
	}
	if(ascii == '*')
	{
		return 43;
	}
	return 0xff;
}

extern void PrintBarcode_CODE39(uint8_t m)
{
#if (BARCODE_BAR_WIDTH == 2)
	#define CODE39_BAR_WIDTH	(12)
#else
	#define CODE39_BAR_WIDTH	(15)
#endif

	uint8_t dot[LineDot/8];
	uint16_t col;
	uint8_t barcode[255];
	uint8_t i, n;

#if (CODE39_BAR_WIDTH == 12)
	uint16_t const code39_encode[] =
	{
		// 0
		0xa6d,	// 1010,0110,1101
		// 1
	 	0xd2b,	// 1101,0010,1011
		// 2
		0xb2b,	// 1011,0010,1011
		// 3
		0xd95,	// 1101,1001,0101
		// 4
		0xa6b,	// 1010,0110,1011
		// 5
	 	0xd35,	// 1101,0011,0101
		// 6
		0xb35,	// 1011,0011,0101
		// 7
		0xa5b,	// 1010,0101,1011
		// 8
		0xd2d,	// 1101,0010,1101
		// 9
		0xb2d,	// 1011,0010,1101
		// A
		0xd4b,	// 1101,0100,1011
		// B
		0xb4b,	// 1011,0100,1011
		// C
		0xda5,	// 1101,1010,0101
		// D
		0xacb,	// 1010,1100,1011
		// E
		0xd65,	// 1101,0110,0101
		// F
		0xb65,	// 1011,0110,0101
		// G
		0xa9b,	// 1010,1001,1011
		// H
		0xd4d,	// 1101,0100,1101
		// I
		0xb4d,	// 1011,0100,1101
		// J
		0xacd,	// 1010,1100,1101
		// K
		0xd43,	// 1101,0101,0011
		// L
		0xb53,	// 1011,0101,0011
		// M
		0xda9,	// 1101,1010,1001
		// N
		0xad3,	// 1010,1101,0011
		// O
		0xd69,	// 1101,0110,1001
		// P
		0xb69,	// 1011,0110,1001
		// Q
		0xab3,	// 1010,1011,0011
		// R
		0xd59,	// 1101,0101,1001
		// S
		0xb59,	// 1011,0101,1001
		// T
		0xad9,	// 1010,1101,1001
		// U
		0xcab,	// 1100,1010,1011
		// V
		0x9ab,	// 1001,1010,1011
		// W
		0xcd5,	// 1100,1101,0101
		// X
		0x96b,	// 1001,0110,1011
		// Y
		0xcb5,	// 1100,1011,0101
		// Z
		0x9b5,	// 1001,1011,0101
		// -
		0x95b,	// 1001,0101,1011
		// .
		0xcad,	// 1100,1010,1101
	   	// SP          ,    ,
		0x9ad,	// 1001,1010,1101
	   	// $
		0x925,	// 1001,0010,0101
		// ／          ,    ,
		0x929,	// 1001,0010,1001
		// +
		0x949,	// 1001,0100,1001
		// %
		0xa49,	// 1010,0100,1001
		// *
		0x96d,	// 1001,0110,1101
	};
#else
	uint16_t const code39_encode[] =
	{
		// 0
		0x51dd,	// 101,0001,1101,1101
		// 1
	 	0x7457,	// 111,0100,0101,0111
		// 2
		0x5c57,	// 101,1100,0101,0111
		// 3
		0x7715,	// 111,0111,0001,0101
		// 4
		0x51d7,	// 101,0001,1101,0111
		// 5
	 	0x7475,	// 111,0100,0111,0101
		// 6
		0x5c75,	// 101,1100,0111,0101
		// 7
		0x5177,	// 101,0001,0111,0111
		// 8
		0x745d,	// 111,0100,0101,1101
		// 9
		0x5c5d,	// 101,1100,0101,1101
		// A
		0x7517,	// 111,0101,0001,0111
		// B
		0x5d17,	// 101,1101,0001,0111
		// C
		0x7745,	// 111,0111,0100,0101
		// D
		0x5717,	// 101,0111,0001,0111
		// E
		0x75c5,	// 111,0101,1100,0101
		// F
		0x5dc5,	// 101,1101,1100,0101
		// G
		0x5477,	// 101,0100,0111,0111
		// H
		0x751d,	// 111,0101,0001,1101
		// I
		0x5d1d,	// 101,1101,0001,1101
		// J
		0x571d,	// 101,0111,0001,1101
		// K
		0x7547,	// 111,0101,0100,0111
		// L
		0x5d47,	// 101,1101,0100,0111
		// M
		0x7751,	// 111,0111,0101,0001
		// N
		0x5747,	// 101,0111,0100,0111
		// O
		0x75d1,	// 111,0101,1101,0001
		// P
		0x5dd1,	// 101,1101,1101,0001
		// Q
		0x55c7,	// 101,0101,1100,0111
		// R
		0x7571,	// 111,0101,0111,0001
		// S
		0x5d71,	// 101,1101,0111,0001
		// T
		0x5771,	// 101,0111,0111,0001
		// U
		0x7157,	// 111,0001,0101,0111
		// V
		0x4757,	// 100,0111,0101,0111
		// W
		0x71d5,	// 111,0001,1101,0101
		// X
		0x45d7,	// 100,0101,1101,0111
		// Y
		0x7175,	// 111,0001,0111,0101
		// Z
		0x4775,	// 100,0111,0111,0101
		// -
		0x4577,	// 100,0101,0111,0111
		// .
		0x715d,	// 111,0001,0101,1101
	   	// SP
		0x475d,	// 100,0111,0101,1101
	   	// $
		0x4445,	// 100,0100,0100,0101
		// ／
		0x4451,	// 100,0100,0101,0001
		// +
		0x4511,	// 100,0101,0001,0001
		// %
		0x5111,	// 101,0001,0001,0001
		// *
		0x45dd,	// 100,0101,1101,1101
	};
#endif

	// 接收条码命令
	n = PrintBarcode_GetBarcodeInput(barcode, sizeof(barcode), m);
	if((n < 1))
	{
		return;
	}
	// 检查条码格式的合法性
	for(i=0; i<n; i++)
	{
		if(PrintBarcode_CODE39_ASCII_to_Hex(barcode[i]) > 42)
		{
			return;
		}
	}
	memset(dot, 0, sizeof(dot));
	//col = 0;
	col = esc_sts.leftspace;
	// 打印起始码
	col = PrintBarcodePixel(code39_encode[43], CODE39_BAR_WIDTH, dot, col);	// '*'
	col = PrintBarcodePixel(0, 1, dot, col);	// Narrow Space
	// 打印资料码
	for(i=0; i<n; i++)
	{
		col = PrintBarcodePixel(code39_encode[PrintBarcode_CODE39_ASCII_to_Hex(barcode[i])], CODE39_BAR_WIDTH, dot, col);
		col = PrintBarcodePixel(0, 1, dot, col);	// Narrow Space
	}
	// 打印终止码
	col = PrintBarcodePixel(code39_encode[43], CODE39_BAR_WIDTH, dot, col);	// '*'

	// 打印条码
	PrintBarcode(barcode, n, dot, col);
}

static uint16_t PrintBarcode_ITF_DrawPattern(uint16_t pattern, uint8_t bits, uint8_t *dot, uint16_t col)
{
	uint8_t i;
	uint8_t width;
	uint32_t mark;

	for(i=0; i<bits; i++)
	{
		if(pattern & (1 << (bits-i-1)))		// wide
		{
			width = BARCODE_BAR_WIDTH;	// wide = 3
		}
		else
		{
			width = 1;
		}

		if(i & 0x01)
		{
			mark = 0x00000000ul;
		}
		else
		{
			mark = 0xfffffffful;
		}
		col = PrintBarcodePixel(mark, width, dot, col);
	}
	return col;
}

extern void PrintBarcode_ITF(uint8_t m)
{
	uint8_t dot[LineDot/8];
	uint16_t col;
	uint8_t barcode[255];
	uint8_t i, n;

	uint16_t const itf_encode[]=
	{
		// 0
		0x014,		// nnwwn
		// 1
		0x101,		//wnnnw
		// 2
		0x041,		// nwnnw
		// 3
		0x140,		//wwnnn
		// 4
		0x011,		// nnwnw
		// 5
		0x110,		// wnwnn
		// 6
		0x050,		// nwwnn
		// 7
		0x005,		// nnnww
		// 8
		0x104,		// wnnwn
		// 9
		0x044,		// nwnwn
	};
	// 接收条码命令
	n = PrintBarcode_GetBarcodeInput(barcode, sizeof(barcode), m);
	// 检查条码格式的合法性
	if(n & 0x01)	// When an odd number of data is input, the printer ignores the last received data
	{
		n--;
	}
	if(n == 0)
	{
		return;
	}
	for(i=0; i<n; i++)
	{
//		if(isdigit(barcode[i]) == 0)
        if(!((barcode[i] >= '0') && (barcode[i] <= '9')))

		{
			return;
		}
	}

	memset(dot, 0, sizeof(dot));
	//col = 0;
	col = esc_sts.leftspace;

	// 打印起始码
	col = PrintBarcode_ITF_DrawPattern(0x00, 4, dot, col);		// 0000
	// 打印资料码
	for(i=0; i<n; i+=2)
	{
		col = PrintBarcode_ITF_DrawPattern((itf_encode[(barcode[i]-'0')] << 1) | itf_encode[(barcode[i+1]-'0')], 10, dot, col);
	}
	// 打印终止码
	col = PrintBarcode_ITF_DrawPattern(0x04, 3, dot, col);		// 100

	// 打印条码
	PrintBarcode(barcode, n, dot, col);
}
/*
static uint8_t PrintBarcode_CODABAR_ASCII_to_Hex(uint8_t ascii)
{
	if((ascii >= '0') && (ascii <= '9'))
	{
		return (ascii-'0');
	}
	if(ascii == '$')
	{
		return 10;
	}
	if(ascii == '-')
	{
		return 11;
	}
	if(ascii == '+')
	{
		return 12;
	}
	if(ascii == '.')
	{
		return 13;
	}
	if(ascii == '/')
	{
		return 14;
	}
	if(ascii == ':')
	{
		return 15;
	}
	if((ascii >= 'A') && (ascii <= 'D'))
	{
		return (ascii-'A'+16);
	}
	return 0xff;
}


extern void PrintBarcode_CODABAR(uint8_t m)
{
	uint8_t dot[LineDot/8];
	uint16_t col;
	uint8_t barcode[255];
	uint8_t i, n;
	uint8_t c;

#if (BARCODE_BAR_WIDTH == 2)
	uint16_t const codabar_encode[]=
	{
		// 0
		0x2a6,	// 10,1010,0110
		// 1
		0x2b2,	// 10,1011,0010
		// 2
		0x296,	// 10,1001,0110
		// 3
		0x32a,	// 11,0010,1010
		// 4
		0x2d2,	// 10,1101,0010
		// 5
		0x352,	// 11,0101,0010
		// 6
		0x256,	// 10,0101,0110
		// 7
		0x25a,	// 10,0101,1010
		// 8
		0x26a,	// 10,0110,1010
		// 9
		0x34a,	// 11,0100,1010
		// $
		0x2ca,	// 10,1100,1010
		// -
		0x29a,	// 10,1001,1010
		// +
		0x5b6,	// 101,1011,0110
		// .
		0x6da,	// 110,1101,1010
		// /
		0x6d6,	// 110,1101,0110
		// :
		0x6b6,	// 110,1011,0110
		// A
		0x592,	// 101,1001,0010
		// B
		0x496,	// 100,1001,0110
		// C
		0x526,	// 101,0010,0110
		// D
		0x532,	// 101,0011,0010
	};
#else
	uint16_t const codabar_encode[]=
	{
		// 0
		0xa8e,	// 1010,1000,1110
		// 1
		0xae2,	// 1010,1110,0010
		// 2
		0xa2e,	// 1010,0010,1110
		// 3
		0xe2a,	// 1110,0010,1010
		// 4
		0xba2,	// 1011,1010,0010
		// 5
		0xea2,	// 1110,1010,0010
		// 6
		0x8ae,	// 1000,1010,1110
		// 7
		0x8ba,	// 1000,1011,1010
		// 8
		0x8ea,	// 1000,1110,1010
		// 9
		0xe8a,	// 1110,1000,1010
		// $
		0xb8a,	// 1011,1000,1010
		// -
		0xa3a,	// 1010,0011,1010
		// +
		0x2eee,	// 10,1110,1110,1110
		// .
		0x3bba,	// 11,1011,1011,1010
		// /
		0x3bae,	// 11,1011,1010,1110
		// :
		0x3aee,	// 11,1010,1110,1110
		// A
		0x2e22,	// 10,1110,0010,0010
		// B
		0x222e,	// 10,0010,0010,1110
		// C
		0x288e,	// 10,1000,1000,1110
		// D
		0x28e2,	// 10,1000,1110,0010
	};
#endif

	// 接收条码命令
	n = PrintBarcode_GetBarcodeInput(barcode, sizeof(barcode), m);
	// 检查条码格式的合法性
	if(n < 1)
	{
		return;
	}
	if((barcode[0] < 'A') || (barcode[0] > 'D'))	// A~D
	{
		return;
	}
	for(i=1; i<n-1; i++)
	{
		if(PrintBarcode_CODABAR_ASCII_to_Hex(barcode[i]) > 15)
		{
			return;
		}
	}
	if(n >= 2)
	{
		c = PrintBarcode_CODABAR_ASCII_to_Hex(barcode[n-1]);
		if((barcode[n-1] < 'A') || (barcode[n-1] > 'D'))	// A~D
		{
			return;
		}
	}

	memset(dot, 0, sizeof(dot));
	//col = 0;
	col = esc_sts.leftspace;
	// 打印起始码
	col = PrintBarcodePixel(codabar_encode[PrintBarcode_CODABAR_ASCII_to_Hex(barcode[0])], (BARCODE_BAR_WIDTH*3+5), dot, col);
	// 打印资料码
	for(i=1; i<n-1; i++)
	{
		c = PrintBarcode_CODABAR_ASCII_to_Hex(barcode[i]);
		if(c <= 11)
		{
			col = PrintBarcodePixel(codabar_encode[c], (BARCODE_BAR_WIDTH*2+6), dot, col);
		}
		else
		{
			col = PrintBarcodePixel(codabar_encode[c], (BARCODE_BAR_WIDTH*3+5), dot, col);
		}
	}
	// 打印终止码
	if(n >= 2)
	{
		col = PrintBarcodePixel(codabar_encode[PrintBarcode_CODABAR_ASCII_to_Hex(barcode[n-1])] >> 1, (BARCODE_BAR_WIDTH*3+5-1), dot, col);
	}
	// 打印条码
	PrintBarcode(barcode, n, dot, col);
}

extern void PrintBarcode_CODE93(uint8_t m)
{
	uint8_t dot[LineDot/8];
	uint16_t col;
	uint8_t barcode[255];
	uint8_t i, n;
	uint8_t c1, c2;
	uint32_t nWidth, nSum;

	uint8_t const code93_decode[][2]=
	{
		{ // 0
			44,
			30,
		},
		{ // 1
			43,
			10,
		},
		{ // 2
			43,
			11,
		},
		{ // 3
			43,
			12,
		},
		{ // 4
			43,
			13,
		},
		{ // 5
			43,
			14,
		},
		{ // 6
			43,
			15,
		},
		{ // 7
			43,
			16,
		},
		{ // 8
			43,
			17,
		},
		{ // 9
			43,
			18,
		},
		{ // 10
			43,
			19,
		},
		{ // 11
			43,
			20,
		},
		{ // 12
			43,
			21,
		},
		{ // 13
			43,
			22,
		},
		{ // 14
			43,
			23,
		},
		{ // 15
			43,
			24,
		},
		{ // 16
			43,
			25,
		},
		{ // 17
			43,
			26,
		},
		{ // 18
			43,
			27,
		},
		{ // 19
			43,
			28,
		},
		{ // 20
			43,
			29,
		},
		{ // 21
			43,
			30,
		},
		{ // 22
			43,
			31,
		},
		{ // 23
			43,
			32,
		},
		{ // 24
			43,
			33,
		},
		{ // 25
			43,
			34,
		},
		{ // 26
			43,
			35,
		},
		{ // 27
			44,
			10,
		},
		{ // 28
			44,
			11,
		},
		{ // 29
			44,
			12,
		},
		{ // 30
			44,
			13,
		},
		{ // 31
			44,
			14,
		},
		{ // 32
			38,
			255,
		},
		{ // 33
			45,
			10,
		},
		{ // 34
			45,
			11,
		},
		{ // 35
			45,
			12,
		},
		{ // 36
			39,
			255,
		},
		{ // 37
			42,
			255,
		},
		{ // 38
			45,
			15,
		},
		{ // 39
			45,
			16,
		},
		{ // 40
			45,
			17,
		},
		{ // 41
			45,
			18,
		},
		{ // 42
			45,
			19,
		},
		{ // 43
			41,
			255,
		},
		{ // 44
			45,
			21,
		},
		{ // 45
			36,
			255,
		},
		{ // 46
			37,
			255,
		},
		{ // 47
			40,
			255,
		},
		{ // 48
			0,
			255,
		},
		{ // 49
			1,
			255,
		},
		{ // 50
			2,
			255,
		},
		{ // 51
			3,
			255,
		},
		{ // 52
			4,
			255,
		},
		{ // 53
			5,
			255,
		},
		{ // 54
			6,
			255,
		},
		{ // 55
			7,
			255,
		},
		{ // 56
			8,
			255,
		},
		{ // 57
			9,
			255,
		},
		{ // 58
			45,
			35,
		},
		{ // 59
			44,
			15,
		},
		{ // 60
			44,
			16,
		},
		{ // 61
			44,
			17,
		},
		{ // 62
			44,
			18,
		},
		{ // 63
			44,
			19,
		},
		{ // 64
			44,
			31,
		},
		{ // 65
			10,
			255,
		},
		{ // 66
			11,
			255,
		},
		{ // 67
			12,
			255,
		},
		{ // 68
			13,
			255,
		},
		{ // 69
			14,
			255,
		},
		{ // 70
			15,
			255,
		},
		{ // 71
			16,
			255,
		},
		{ // 72
			17,
			255,
		},
		{ // 73
			18,
			255,
		},
		{ // 74
			19,
			255,
		},
		{ // 75
			20,
			255,
		},
		{ // 76
			21,
			255,
		},
		{ // 77
			22,
			255,
		},
		{ // 78
			23,
			255,
		},
		{ // 79
			24,
			255,
		},
		{ // 80
			25,
			255,
		},
		{ // 81
			26,
			255,
		},
		{ // 82
			27,
			255,
		},
		{ // 83
			28,
			255,
		},
		{ // 84
			29,
			255,
		},
		{ // 85
			30,
			255,
		},
		{ // 86
			31,
			255,
		},
		{ // 87
			32,
			255,
		},
		{ // 88
			33,
			255,
		},
		{ // 89
			34,
			255,
		},
		{ // 90
			35,
			255,
		},
		{ // 91
			44,
			20,
		},
		{ // 92
			44,
			21,
		},
		{ // 93
			44,
			22,
		},
		{ // 94
			44,
			23,
		},
		{ // 95
			44,
			24,
		},
		{ // 96
			44,
			32,
		},
		{ // 97
			46,
			10,
		},
		{ // 98
			46,
			11,
		},
		{ // 99
			46,
			12,
		},
		{ // 100
			46,
			13,
		},
		{ // 101
			46,
			14,
		},
		{ // 102
			46,
			15,
		},
		{ // 103
			46,
			16,
		},
		{ // 104
			46,
			17,
		},
		{ // 105
			46,
			18,
		},
		{ // 106
			46,
			19,
		},
		{ // 107
			46,
			20,
		},
		{ // 108
			46,
			21,
		},
		{ // 109
			46,
			22,
		},
		{ // 110
			46,
			23,
		},
		{ // 111
			46,
			24,
		},
		{ // 112
			46,
			25,
		},
		{ // 113
			46,
			26,
		},
		{ // 114
			46,
			27,
		},
		{ // 115
			46,
			28,
		},
		{ // 116
			46,
			29,
		},
		{ // 117
			46,
			30,
		},
		{ // 118
			46,
			31,
		},
		{ // 119
			46,
			32,
		},
		{ // 120
			46,
			33,
		},
		{ // 121
			46,
			34,
		},
		{ // 122
			46,
			35,
		},
		{ // 123
			44,
			25,
		},
		{ // 124
			44,
			26,
		},
		{ // 125
			44,
			27,
		},
		{ // 126
			44,
			28,
		},
		{ // 127
			44,
			29,
		},
	};

	uint16_t const code93_encode[] =
	{
		// 0
		0x114,	// 1,0001,0100
		// 1
		0x148,	// 1,0100,1000
		// 2
		0x144,	// 1,0100,0100
		// 3
		0x142,	// 1,0100,0010
		// 4
		0x128,	// 1,0010,1000
		// 5
		0x124,	// 1,0010,0100
		// 6
		0x122,	// 1,0010,0010
		// 7
		0x150,	// 1,0101,0000
		// 8
		0x112,	// 1,0001,0010
		// 9
		0x10a,	// 1,0000,1010
		// 10
		0x1a8,	// 1,1010,1000
		// 11
		0x1a4,	// 1,1010,0100
		// 12
		0x1a2,	// 1,1010,0010
		// 13
		0x194,	// 1,1001,0100
		// 14
		0x192,	// 1,1001,0010
		// 15
		0x18a,	// 1,1000,1010
		// 16
		0x168,	// 1,0110,1000
		// 17
		0x164,	// 1,0110,0100
		// 18
		0x162,	// 1,0110,0010
		// 19
		0x134,	// 1,0011,0100
		// 20
		0x11a,	// 1,0001,1010
		// 21
		0x158,	// 1,0101,1000
		// 22
		0x14c,	// 1,0100,1100
		// 23
		0x146,	// 1,0100,0110
		// 24
		0x12c,	// 1,0010,1100
		// 25
		0x116,	// 1,0001,0110
		// 26
		0x1b4,	// 1,1011,0100
		// 27
		0x1b2,	// 1,1011,0010
		// 28
		0x1ac,	// 1,1010,1100
		// 29
		0x1a6,	// 1,1010,0110
		// 30
		0x196,	// 1,1001,0110
		// 31
		0x19a,	// 1,1001,1010
		// 32
		0x16c,	// 1,0110,1100
		// 33
		0x166,	// 1,0110,0110
		// 34
		0x136,	// 1,0011,0110
		// 35
		0x13a,	// 1,0011,1010
		// 36
		0x12e,	// 1,0010,1110
		// 37
		0x1d4,	// 1,1101,0100
		// 38
		0x1d2,	// 1,1101,0010
		// 39
		0x1ca,	// 1,1100,1010
		// 40
		0x16e,	// 1,0110,1110
		// 41
		0x176,	// 1,0111,0110
		// 42
		0x1ae,	// 1,1010,1110
		// 43
		0x126,	// 1,0010,0110
		// 44
		0x1da,	// 1,1101,1010
		// 45
		0x1d6,	// 1,1101,0110
		// 46
		0x132,	// 1,0011,0010
		// 47
		0x15e,	// 1,0101,1110
		// 48
		0x2bd,	// 10,1011,1101
	};

	// 接收条码命令
	n = PrintBarcode_GetBarcodeInput(barcode, sizeof(barcode), m);
	// 检查条码格式的合法性
	if(n == 0)
	{
		return;
	}
	for(i=0; i<n; i++)
	{
		if(barcode[i] >= 128)
		{
			return;
		}
	}

	memset(dot, 0, sizeof(dot));
	//col = 0;
    col = esc_sts.leftspace;

	// 打印起始码
	col = PrintBarcodePixel(code93_encode[47], 9, dot, col);
	// 打印资料码
	for(i=0; i<n; i++)
	{
		c1 = code93_decode[(barcode[i])][0];
		c2 = code93_decode[(barcode[i])][1];
		col = PrintBarcodePixel(code93_encode[c1], 9, dot, col);
		if(c2 < ARRAY_SIZE(code93_encode))
		{
			col = PrintBarcodePixel(code93_encode[c2], 9, dot, col);
		}
	}
	// 打印校验和
	// "C" check digit character
	nWidth = 1;
	nSum = 0;
	i = n-1;
	do
	{
		c1 = code93_decode[(barcode[i])][0];
		c2 = code93_decode[(barcode[i])][1];

		// add to the sum
		nSum += (nWidth * c1);
		if (nWidth < 20)
		{
			nWidth++;
		}
		else
		{
			nWidth = 1;
		}
		// if its a two sequence character
		if(c2 < ARRAY_SIZE(code93_encode))
		{
			nSum += (nWidth * c2);
			if (nWidth < 20)
			{
				nWidth++;
			}
			else
			{
				nWidth = 1;
			}
		}
	}while(i--);

	// draw the "C" check digit character
	col = PrintBarcodePixel(code93_encode[nSum%47], 9, dot, col);

	// "K" check digit character - include the "C" check digit character in calculations
	nWidth = 2;
	nSum = nSum%47;
	i = n-1;
	do
	{
		c1 = code93_decode[(barcode[i])][0];
		c2 = code93_decode[(barcode[i])][1];

		// add to the sum
		nSum += (nWidth * c1);
		if (nWidth < 15)
		{
			nWidth++;
		}
		else
		{
			nWidth = 1;
		}
		// if its a two sequence character
		if(c2 < ARRAY_SIZE(code93_encode))
		{
			nSum += (nWidth * c2);
			if (nWidth < 15)
			{
				nWidth++;
			}
			else
			{
				nWidth = 1;
			}
		}
	}while(i--);
	// draw the "K" check digit character
	col = PrintBarcodePixel(code93_encode[nSum%47], 9, dot, col);

	// 打印终止码
	col = PrintBarcodePixel(code93_encode[48], 10, dot, col);
	// 打印条码
	PrintBarcode(barcode, n, dot, col);
}

extern void PrintBarcode_CODE128(uint8_t m)
{
	uint8_t dot[LineDot/8];
	uint16_t col;
	uint8_t barcode[255];
	uint8_t barcode_HRI[255];
	uint8_t barcode_HRI_pt;
	uint8_t i, n;
	uint8_t currMode, offset, digit;
	uint32_t cksum;

	uint16_t const code128_encode[] =
	{
		// 0
		0x6cc,	// 110,1100,1100
		// 1
		0x66c,	// 110,0110,1100
		// 2
		0x666,	// 110,0110,0110
		// 3
		0x498,	// 100,1001,1000
		// 4
		0x48c,	// 100,1000,1100
		// 5
		0x44c,	// 100,0100,1100
		// 6
		0x4c8,	// 100,1100,1000
		// 7
		0x4c4,	// 100,1100,0100
		// 8
		0x464,	// 100,0110,0100
		// 9
		0x648,	// 110,0100,1000
		// 10
		0x644,	// 110,0100,0100
		// 11
		0x624,	// 110,0010,0100
		// 12
		0x59c,	// 101,1001,1100
		// 13
		0x4dc,	// 100,1101,1100
		// 14
		0x4ce,	// 100,1100,1110
		// 15
		0x5cc,	// 101,1100,1100
		// 16
		0x4ec,	// 100,1110,1100
		// 17
		0x4e6,	// 100,1110,0110
		// 18
		0x672,	// 110,0111,0010
		// 19
		0x65c,	// 110,0101,1100
		// 20
		0x64e,	// 110,0100,1110
		// 21
		0x6e4,	// 110,1110,0100
		// 22
		0x674,	// 110,0111,0100
		// 23
		0x76e,	// 111,0110,1110
		// 24
		0x74c,	// 111,0100,1100
		// 25
		0x72c,	// 111,0010,1100
		// 26
		0x726,	// 111,0010,0110
		// 27
		0x764,	// 111,0110,0100
		// 28
		0x734,	// 111,0011,0100
		// 29
		0x732,	// 111,0011,0010
		// 30
		0x6d8,	// 110,1101,1000
		// 31
		0x6c6,	// 110,1100,0110
		// 32
		0x636,	// 110,0011,0110
		// 33
		0x518,	// 101,0001,1000
		// 34
		0x458,	// 100,0101,1000
		// 35
		0x446,	// 100,0100,0110
		// 36
		0x588,	// 101,1000,1000
		// 37
		0x468,	// 100,0110,1000
		// 38
		0x462,	// 100,0110,0010
		// 39
		0x688,	// 110,1000,1000
		// 40
		0x628,	// 110,0010,1000
		// 41
		0x622,	// 110,0010,0010
		// 42
		0x5b8,	// 101,1011,1000
		// 43
		0x58e,	// 101,1000,1110
		// 44
		0x46e,	// 100,0110,1110
		// 45
		0x5d8,	// 101,1101,1000
		// 46
		0x5c6,	// 101,1100,0110
		// 47
		0x476,	// 100,0111,0110
		// 48
		0x776,	// 111,0111,0110
		// 49
		0x68e,	// 110,1000,1110
		// 50
		0x62e,	// 110,0010,1110
		// 51
		0x6e8,	// 110,1110,1000
		// 52
		0x6e2,	// 110,1110,0010
		// 53
		0x6ee,	// 110,1110,1110
		// 54
		0x758,	// 111,0101,1000
		// 55
		0x746,	// 111,0100,0110
		// 56
		0x716,	// 111,0001,0110
		// 57
		0x768,	// 111,0110,1000
		// 58
		0x762,	// 111,0110,0010
		// 59
		0x71a,	// 111,0001,1010
		// 60
		0x77a,	// 111,0111,1010
		// 61
		0x642,	// 110,0100,0010
		// 62
		0x78a,	// 111,1000,1010
		// 63
		0x530,	// 101,0011,0000
		// 64
		0x50c,	// 101,0000,1100
		// 65
		0x4b0,	// 100,1011,0000
		// 66
		0x486,	// 100,1000,0110
		// 67
		0x42c,	// 100,0010,1100
		// 68
		0x426,	// 100,0010,0110
		// 69
		0x590,	// 101,1001,0000
		// 70
		0x584,	// 101,1000,0100
		// 71
		0x4d0,	// 100,1101,0000
		// 72
		0x4c2,	// 100,1100,0010
		// 73
		0x434,	// 100,0011,0100
		// 74
		0x432,	// 100,0011,0010
		// 75
		0x612,	// 110,0001,0010
		// 76
		0x650,	// 110,0101,0000
		// 77
		0x7ba,	// 111,1011,1010
		// 78
		0x614,	// 110,0001,0100
		// 79
		0x47a,	// 100,0111,1010
		// 80
		0x53c,	// 101,0011,1100
		// 81
		0x4bc,	// 100,1011,1100
		// 82
		0x49e,	// 100,1001,1110
		// 83
		0x5e4,	// 101,1110,0100
		// 84
		0x4f4,	// 100,1111,0100
		// 85
		0x4f2,	// 100,1111,0010
		// 86
		0x7a4,	// 111,1010,0100
		// 87
		0x794,	// 111,1001,0100
		// 88
		0x792,	// 111,1001,0010
		// 89
		0x6de,	// 110,1101,1110
		// 90
		0x6f6,	// 110,1111,0110
		// 91
		0x7b6,	// 111,1011,0110
		// 92
		0x578,	// 101,0111,1000
		// 93
		0x51e,	// 101,0001,1110
		// 94
		0x45e,	// 100,0101,1110
		// 95
		0x5e8,	// 101,1110,1000
		// 96
		0x5e2,	// 101,1110,0010
		// 97
		0x7a8,	// 111,1010,1000
		// 98
		0x7a2,	// 111,1010,0010
		// 99
		0x5de,	// 101,1101,1110
		// 100
		0x5ee,	// 101,1110,1110
		// 101
		0x75e,	// 111,0101,1110
		// 102
		0x7ae,	// 111,1010,1110
		// 103
		0x684,	// 110,1000,0100
		// 104
		0x690,	// 110,1001,0000
		// 105
		0x69c,	// 110,1001,1100
		// 106
		0x18eb,	// 1,1000,1110,1011
	};

	// 接收条码命令
	n = PrintBarcode_GetBarcodeInput(barcode, sizeof(barcode), m);
	// 检查条码格式的合法性
	if(n < 2)
	{
		return;
	}

	for(i=0; i<n; i++)
	{
		if(barcode[i] >= 128)
		{
			return;
		}
	}

	memset(dot, 0, sizeof(dot));
	currMode = 'S';
	i = col = digit = cksum = barcode_HRI_pt = 0;
    col = esc_sts.leftspace;
	do
	{
		if(barcode[i] == '{')	// ESCAPE
		{
			i++;
			if(i >= n) return;
			switch(barcode[i])
			{
			case 'A':
				switch(currMode)
				{
				case 'S':
					col = PrintBarcodePixel(code128_encode[103], 11, dot, col);
					cksum = 103;
					break;
				default:
					col = PrintBarcodePixel(code128_encode[101], 11, dot, col);
					cksum += 101 * digit;
					break;
				}
				currMode = 'A';
				digit++;
				i++;
				break;
			case 'B':
				switch(currMode)
				{
				case 'S':
					col = PrintBarcodePixel(code128_encode[104], 11, dot, col);
					cksum = 104;
					break;
				default:
					col = PrintBarcodePixel(code128_encode[100], 11, dot, col);
					cksum += 100 * digit;
					break;
				}
				currMode = 'B';
				digit++;
				i++;
				break;
			case 'C':
				switch(currMode)
				{
				case 'S':
					col = PrintBarcodePixel(code128_encode[105], 11, dot, col);
					cksum = 105;
					break;
				default:
					col = PrintBarcodePixel(code128_encode[99], 11, dot, col);
					cksum += 99 * digit;
					break;
				}
				currMode = 'C';
				digit++;
				i++;
				break;
			case '{':
				// ESCAPE char
				goto ESC_OUT;
			case 'S':
				switch(currMode)
				{
				case 'A':
				case 'B':
					col = PrintBarcodePixel(code128_encode[98], 11, dot, col);
					cksum += 98 * digit;
					digit++;
					i++;
					break;
				default:
					return; 	// unknown format
				}
				break;
			case '1':
				switch(currMode)
				{
				case 'A':
				case 'B':
				case 'C':
					col = PrintBarcodePixel(code128_encode[102], 11, dot, col);
					cksum += 102 * digit;
					digit++;
					i++;
					break;
				default:
					return; 	// unknown format
				}
				break;
			case '2':
				switch(currMode)
				{
				case 'A':
				case 'B':
					col = PrintBarcodePixel(code128_encode[97], 11, dot, col);
					cksum += 97 * digit;
					digit++;
					i++;
					break;
				default:
					return; 	// unknown format
				}
				break;
			case '3':
				switch(currMode)
				{
				case 'A':
				case 'B':
					col = PrintBarcodePixel(code128_encode[96], 11, dot, col);
					cksum += 96 * digit;
					digit++;
					i++;
					break;
				default:
					return; 	// unknown format
				}
				break;
			case '4':
				switch(currMode)
				{
				case 'A':
					col = PrintBarcodePixel(code128_encode[101], 11, dot, col);
					cksum += 101 * digit;
					digit++;
					i++;
					break;
				case 'B':
					col = PrintBarcodePixel(code128_encode[100], 11, dot, col);
					cksum += 100 * digit;
					digit++;
					i++;
					break;
				default:
					return; 	// unknown format
				}
				break;
			default:
				return;		// unknow format
			}
			continue;
		}
ESC_OUT:
		switch(currMode)
		{
		case 'S':
			return;		// unknown format
		case 'A':	// A (0x00~0x5F)
			if(barcode[i] < 0x20)
			{
				offset = barcode[i]+0x40;
				col = PrintBarcodePixel(code128_encode[offset], 11, dot, col);
				barcode_HRI[barcode_HRI_pt++] = barcode[i];
				i++;
				cksum += offset * digit;
				digit++;
			}
			else if(barcode[i] < 0x60)		// 0x20~0x5F
			{
				offset = barcode[i]-0x20;
				col = PrintBarcodePixel(code128_encode[offset], 11, dot, col);
				barcode_HRI[barcode_HRI_pt++] = barcode[i];
				i++;
				cksum += offset * digit;
				digit++;
			}
			else	// 0x60~0x7F
			{
				return;		// unknown format
			}
			break;
		case 'B':	// B (0x20~0x7F)
			if(barcode[i] < 0x20)
			{
				return;		// unknown format
			}
			else
			{
				offset = barcode[i]-0x20;
				col = PrintBarcodePixel(code128_encode[offset], 11, dot, col);
				barcode_HRI[barcode_HRI_pt++] = barcode[i];
				i++;
				cksum += offset * digit;
				digit++;
			}
			break;
		case 'C':	// C (00~99)
//			if(((i+1) < n) && (isdigit(barcode[i]) && isdigit(barcode[i+1])))
            if(((i+1)<n) && (barcode[i] >= '0' &&  barcode[i] <= '9') && (barcode[i+1] >= '0' &&  barcode[i+1] <= '9'))
			{
				barcode_HRI[barcode_HRI_pt++] = barcode[i];
				barcode_HRI[barcode_HRI_pt++] = barcode[i+1];
				offset = (barcode[i]-'0')*10+(barcode[i+1]-'0');
				col = PrintBarcodePixel(code128_encode[offset], 11, dot, col);
				i += 2;
				cksum += offset * digit;
				digit++;
			}
			else
			{
				return;		// unknown format
			}
			break;
		}
	}while(i<n);
	// 打印校验码
	col = PrintBarcodePixel(code128_encode[cksum % 103], 11, dot, col);

	// 打印终止码
	col = PrintBarcodePixel(code128_encode[106], 13, dot, col);
	// 打印条码
	PrintBarcode(barcode_HRI, barcode_HRI_pt, dot, col);
}
*/

#if defined(SUPPORT_ALL_BARCODE)
extern void PrintBarcode_UPC_A(uint8_t m)
{
    uint8_t dot[LineDot/8];
    uint16_t col;
    uint8_t barcode[12];
    uint8_t i, n;

    uint8_t const left_encode[] =
    {
        0x05,		// 101
    };

    uint8_t const a_encode[] =
    {
        0x0d,		// 0001101
        0x19,		// 0011001
        0x13,		// 0010011
        0x3d,		// 0111101
        0x23,		// 0100011
        0x31,		// 0110001
        0x2f,		// 0101111
        0x3b,		// 0111011
        0x37,		// 0110111
        0x0b,		// 0001011
    };

    uint8_t const s_encode[] =
    {
        0x0a,		// 01010
    };

    uint8_t const b_encode[] =
    {
        0x72,		// 1110010
        0x66,		// 1100110
        0x6c,		// 1101100
        0x42,		// 1000010
        0x5c,		// 1011100
        0x4e,		// 1001110
        0x50,		// 1010000
        0x44,		// 1000100
        0x48,		// 1001000
        0x74,		// 1110100
    };

    uint8_t const right_encode[] =
    {
        0x05,		// 101
    };

    // 接收条码命令
    n = PrintBarcode_GetBarcodeInput(barcode, sizeof(barcode), m);
    // 检查条码格式的合法性
    if ((n < 11) || (n > 12))
    {
        return;
    }
    for (i=0; i<n; i++)
    {
        //if (isdigit(barcode[i]) == 0)
        {
        //    return;
        }
    }
    // 计算条码校验位
    if (n == 11)
    {
        barcode[11] = PrintBarcode_CalcEanUpcChecksum(barcode, 11);
    }

    memset(dot, 0, sizeof(dot));
    //col = 0;
    col = esc_sts.leftspace;
    // 打印左护码
    col = PrintBarcodePixel(left_encode[0], 3, dot, col);
    // 打印左资料码
    for (i=0; i<=5; i++)
    {
        col = PrintBarcodePixel(a_encode[(barcode[i]-'0')], 7, dot, col);
    }
    // 打印中线
    col = PrintBarcodePixel(s_encode[0], 5, dot, col);
    // 打印右资料码
    for (; i<= 11; i++)
    {
        col = PrintBarcodePixel(b_encode[(barcode[i]-'0')], 7, dot, col);
    }
    // 打印右护码
    col = PrintBarcodePixel(right_encode[0], 3, dot, col);
    // 打印条码
    PrintBarcode(barcode, sizeof(barcode), dot, col);
}

extern void PrintBarcode_UPC_E(uint8_t m)
{
    uint8_t dot[LineDot/8];
    uint16_t col;
    uint8_t barcode[12];
    uint8_t barcode_e[8];
    uint8_t i, n;
    uint8_t c;

    uint8_t const ab_encode[] =
    {
        0x38,		// 111000
        0x34,		// 110100
        0x32,		// 110010
        0x31,		// 110001
        0x2c,		// 101100
        0x26,		// 100110
        0x23,		// 100011
        0x2a,		// 101010
        0x29,		// 101001
        0x25,		// 100101
    };

    uint8_t const left_encode[] =
    {
        0x05,		// 101
    };

    uint8_t const odd_encode[] =
    {
        0x0d,		// 0001101
        0x19,		// 0011001
        0x13,		// 0010011
        0x3d,		// 0111101
        0x23,		// 0100011
        0x31,		// 0110001
        0x2f,		// 0101111
        0x3b,		// 0111011
        0x37,		// 0110111
        0x0b,		// 0001011
    };

    uint8_t const even_encode[] =
    {
        0x27,		// 0100111
        0x33,		// 0110011
        0x1b,		// 0011011
        0x21,		// 0100001
        0x1d,		// 0011101
        0x39,		// 0111001
        0x05,		// 0000101
        0x11,		// 0010001
        0x09,		// 0001001
        0x17,		// 0010111
    };

    uint8_t const right_encode[] =
    {
        0x15,		// 010101
    };


    // 接收条码命令
    n = PrintBarcode_GetBarcodeInput(barcode, sizeof(barcode), m);
    // 检查UPC-A条码格式的合法性
    if ((n < 11) || (n > 12))
    {
        return;
    }

    for (i=0; i<n; i++)
    {
        //if (isdigit(barcode[i]) == 0)
        {
        //    return;
        }
    }
    // 计算条码校验位
    if (n == 11)
    {
        barcode[11] = PrintBarcode_CalcEanUpcChecksum(barcode, 11);
    }
    // 检查是否符合UPC-E编码规则
    if (barcode[0] != '0')
    {
        return;
    }
    barcode_e[0] = '0';
    if ((barcode[3] <= '2') && (barcode[4] == '0') && (barcode[5] == '0') && (barcode[6] == '0') && (barcode[7] == '0'))
    {
        barcode_e[1] = barcode[1];
        barcode_e[2] = barcode[2];
        barcode_e[3] = barcode[8];
        barcode_e[4] = barcode[9];
        barcode_e[5] = barcode[10];
        barcode_e[6] = barcode[3];
        barcode_e[7] = barcode[11];
    }
    else if ((barcode[4] == '0') && (barcode[5] == '0') && (barcode[6] == '0') && (barcode[7] == '0') && (barcode[8] == '0'))
    {
        barcode_e[1] = barcode[1];
        barcode_e[2] = barcode[2];
        barcode_e[3] = barcode[3];
        barcode_e[4] = barcode[9];
        barcode_e[5] = barcode[10];
        barcode_e[6] = '3';
        barcode_e[7] = barcode[11];
    }
    else if ((barcode[5] == '0') && (barcode[6] == '0') && (barcode[7] == '0') && (barcode[8] == '0') && (barcode[9] == '0'))
    {
        barcode_e[1] = barcode[1];
        barcode_e[2] = barcode[2];
        barcode_e[3] = barcode[3];
        barcode_e[4] = barcode[4];
        barcode_e[5] = barcode[10];
        barcode_e[6] = '4';
        barcode_e[7] = barcode[11];
    }
    else if ((barcode[6] == '0') && (barcode[7] == '0') && (barcode[8] == '0') && (barcode[9] == '0') && (barcode[10] >= '5'))
    {
        barcode_e[1] = barcode[1];
        barcode_e[2] = barcode[2];
        barcode_e[3] = barcode[3];
        barcode_e[4] = barcode[4];
        barcode_e[5] = barcode[5];
        barcode_e[6] = barcode[10];
        barcode_e[7] = barcode[11];
    }
    else
    {
        return;
    }

    memset(dot, 0, sizeof(dot));
    //col = 0;
    col = esc_sts.leftspace;
    // 打印左护码
    col = PrintBarcodePixel(left_encode[0], 5, dot, col);
    // 打印资料码
    c = ab_encode[(barcode_e[7]-'0')];
    for (i=1; i<=6; i++)
    {
        if (c & (1 << (6-i)))
        {
            col = PrintBarcodePixel(even_encode[(barcode_e[i]-'0')], 7, dot, col);
        }
        else
        {
            col = PrintBarcodePixel(odd_encode[(barcode_e[i]-'0')], 7, dot, col);
        }
    }
    // 打印右护码
    col = PrintBarcodePixel(right_encode[0], 6, dot, col);
    //----ccc
#if defined(CODEPAGE)||defined(ARABIC)
    //if(esc_sts.character_code_page==0x2a)
    if (esc_sts.from_right_print!=0)
        MoveBarcodeDot(col,dot);
#endif
    // 打印条码
    PrintBarcode(barcode_e, sizeof(barcode_e), dot, col);
}


extern void PrintBarcode_EAN8(uint8_t m)
{
    uint8_t dot[LineDot/8];
    uint16_t col;
    uint8_t barcode[8];
    uint8_t i, n;

    uint8_t const left_encode[] =
    {
        0x05,		// 101
    };

    uint8_t const a_encode[] =
    {
        0x0d,		// 0001101
        0x19,		// 0011001
        0x13,		// 0010011
        0x3d,		// 0111101
        0x23,		// 0100011
        0x31,		// 0110001
        0x2f,		// 0101111
        0x3b,		// 0111011
        0x37,		// 0110111
        0x0b,		// 0001011
    };

    uint8_t const s_encode[] =
    {
        0x0a,		// 01010
    };

    uint8_t const c_encode[] =
    {
        0x72,		// 1110010
        0x66,		// 1100110
        0x6c,		// 1101100
        0x42,		// 1000010
        0x5c,		// 1011100
        0x4e,		// 1001110
        0x50,		// 1010000
        0x44,		// 1000100
        0x48,		// 1001000
        0x74,		// 1110100
    };

    uint8_t const right_encode[] =
    {
        0x05,		// 101
    };

    // 接收条码命令
    n = PrintBarcode_GetBarcodeInput(barcode, sizeof(barcode), m);
    // 检查条码格式的合法性
    if ((n < 7) || (n > 8))
    {
        return;
    }
    for (i=0; i<n; i++)
    {
        //if (isdigit(barcode[i]) == 0)
        {
        //    return;
        }
    }
    // 计算条码校验位
    if (n == 7)
    {
        barcode[7] = PrintBarcode_CalcEanUpcChecksum(barcode, 7);
    }
    memset(dot, 0, sizeof(dot));
    //col = 0;
    col = esc_sts.leftspace;
    // 打印左护码
    col = PrintBarcodePixel(left_encode[0], 3, dot, col);
    // 打印左资料码
    for (i=0; i<=3; i++)
    {
        col = PrintBarcodePixel(a_encode[(barcode[i]-'0')], 7, dot, col);
    }
    // 打印中线
    col = PrintBarcodePixel(s_encode[0], 5, dot, col);
    // 打印右资料码
    for (; i<= 7; i++)
    {
        col = PrintBarcodePixel(c_encode[(barcode[i]-'0')], 7, dot, col);
    }
    // 打印右护码
    col = PrintBarcodePixel(right_encode[0], 3, dot, col);
    //----ccc
#if defined(CODEPAGE)||defined(ARABIC)
    //if(esc_sts.character_code_page==0x2a)
    if (esc_sts.from_right_print!=0)
        MoveBarcodeDot(col,dot);
#endif
    // 打印条码
    PrintBarcode(barcode, sizeof(barcode), dot, col);
}

static uint8_t PrintBarcode_CODABAR_ASCII_to_Hex(uint8_t ascii)
{
    if ((ascii >= '0') && (ascii <= '9'))
    {
        return (ascii-'0');
    }
    if (ascii == '$')
    {
        return 10;
    }
    if (ascii == '-')
    {
        return 11;
    }
    if (ascii == '+')
    {
        return 12;
    }
    if (ascii == '.')
    {
        return 13;
    }
    if (ascii == '/')
    {
        return 14;
    }
    if (ascii == ':')
    {
        return 15;
    }
    if ((ascii >= 'A') && (ascii <= 'D'))
    {
        return (ascii-'A'+16);
    }
    return 0xff;
}


extern void PrintBarcode_CODABAR(uint8_t m)
{
    uint8_t dot[LineDot/8];
    uint16_t col;
    uint8_t barcode[255];
    uint8_t i, n;
    uint8_t c;

#if (BARCODE_BAR_WIDTH == 2)
    uint16_t const codabar_encode[]=
    {
        // 0
        0x2a6,	// 10,1010,0110
        // 1
        0x2b2,	// 10,1011,0010
        // 2
        0x296,	// 10,1001,0110
        // 3
        0x32a,	// 11,0010,1010
        // 4
        0x2d2,	// 10,1101,0010
        // 5
        0x352,	// 11,0101,0010
        // 6
        0x256,	// 10,0101,0110
        // 7
        0x25a,	// 10,0101,1010
        // 8
        0x26a,	// 10,0110,1010
        // 9
        0x34a,	// 11,0100,1010
        // $
        0x2ca,	// 10,1100,1010
        // -
        0x29a,	// 10,1001,1010
        // +
        0x5b6,	// 101,1011,0110
        // .
        0x6da,	// 110,1101,1010
        // /
        0x6d6,	// 110,1101,0110
        // :
        0x6b6,	// 110,1011,0110
        // A
        0x592,	// 101,1001,0010
        // B
        0x496,	// 100,1001,0110
        // C
        0x526,	// 101,0010,0110
        // D
        0x532,	// 101,0011,0010
    };
#else
    uint16_t const codabar_encode[]=
    {
        // 0
        0xa8e,	// 1010,1000,1110
        // 1
        0xae2,	// 1010,1110,0010
        // 2
        0xa2e,	// 1010,0010,1110
        // 3
        0xe2a,	// 1110,0010,1010
        // 4
        0xba2,	// 1011,1010,0010
        // 5
        0xea2,	// 1110,1010,0010
        // 6
        0x8ae,	// 1000,1010,1110
        // 7
        0x8ba,	// 1000,1011,1010
        // 8
        0x8ea,	// 1000,1110,1010
        // 9
        0xe8a,	// 1110,1000,1010
        // $
        0xb8a,	// 1011,1000,1010
        // -
        0xa3a,	// 1010,0011,1010
        // +
        0x2eee,	// 10,1110,1110,1110
        // .
        0x3bba,	// 11,1011,1011,1010
        // /
        0x3bae,	// 11,1011,1010,1110
        // :
        0x3aee,	// 11,1010,1110,1110
        // A
        0x2e22,	// 10,1110,0010,0010
        // B
        0x222e,	// 10,0010,0010,1110
        // C
        0x288e,	// 10,1000,1000,1110
        // D
        0x28e2,	// 10,1000,1110,0010
    };
#endif

    // 接收条码命令
    n = PrintBarcode_GetBarcodeInput(barcode, sizeof(barcode), m);
    // 检查条码格式的合法性
    if (n < 1)
    {
        return;
    }
    if ((barcode[0] < 'A') || (barcode[0] > 'D'))	// A~D
    {
        return;
    }
    for (i=1; i<n-1; i++)
    {
        if (PrintBarcode_CODABAR_ASCII_to_Hex(barcode[i]) > 15)
        {
            return;
        }
    }
    if (n >= 2)
    {
        c = PrintBarcode_CODABAR_ASCII_to_Hex(barcode[n-1]);
        if ((barcode[n-1] < 'A') || (barcode[n-1] > 'D'))	// A~D
        {
            return;
        }
    }

    memset(dot, 0, sizeof(dot));
    //col = 0;
    col = esc_sts.leftspace;
    // 打印起始码
    col = PrintBarcodePixel(codabar_encode[PrintBarcode_CODABAR_ASCII_to_Hex(barcode[0])], (BARCODE_BAR_WIDTH*3+5), dot, col);
    // 打印资料码
    for (i=1; i<n-1; i++)
    {
        c = PrintBarcode_CODABAR_ASCII_to_Hex(barcode[i]);
        if (c <= 11)
        {
            col = PrintBarcodePixel(codabar_encode[c], (BARCODE_BAR_WIDTH*2+6), dot, col);
        }
        else
        {
            col = PrintBarcodePixel(codabar_encode[c], (BARCODE_BAR_WIDTH*3+5), dot, col);
        }
    }
    // 打印终止码
    if (n >= 2)
    {
        col = PrintBarcodePixel(codabar_encode[PrintBarcode_CODABAR_ASCII_to_Hex(barcode[n-1])] >> 1, (BARCODE_BAR_WIDTH*3+5-1), dot, col);
    }
    //----ccc
#if defined(CODEPAGE)||defined(ARABIC)
    //if(esc_sts.character_code_page==0x2a)
    if (esc_sts.from_right_print!=0)
        MoveBarcodeDot(col,dot);
#endif
    // 打印条码
    PrintBarcode(barcode, n, dot, col);
}

extern void PrintBarcode_CODE93(uint8_t m)
{
    uint8_t dot[LineDot/8];
    uint16_t col;
    uint8_t barcode[255];
    uint8_t i, n;
    uint8_t c1, c2;
    uint32_t nWidth, nSum;

    uint8_t const code93_decode[][2]=
    {
        { // 0
            44,
            30,
        },
        { // 1
            43,
            10,
        },
        { // 2
            43,
            11,
        },
        { // 3
            43,
            12,
        },
        { // 4
            43,
            13,
        },
        { // 5
            43,
            14,
        },
        { // 6
            43,
            15,
        },
        { // 7
            43,
            16,
        },
        { // 8
            43,
            17,
        },
        { // 9
            43,
            18,
        },
        { // 10
            43,
            19,
        },
        { // 11
            43,
            20,
        },
        { // 12
            43,
            21,
        },
        { // 13
            43,
            22,
        },
        { // 14
            43,
            23,
        },
        { // 15
            43,
            24,
        },
        { // 16
            43,
            25,
        },
        { // 17
            43,
            26,
        },
        { // 18
            43,
            27,
        },
        { // 19
            43,
            28,
        },
        { // 20
            43,
            29,
        },
        { // 21
            43,
            30,
        },
        { // 22
            43,
            31,
        },
        { // 23
            43,
            32,
        },
        { // 24
            43,
            33,
        },
        { // 25
            43,
            34,
        },
        { // 26
            43,
            35,
        },
        { // 27
            44,
            10,
        },
        { // 28
            44,
            11,
        },
        { // 29
            44,
            12,
        },
        { // 30
            44,
            13,
        },
        { // 31
            44,
            14,
        },
        { // 32
            38,
            255,
        },
        { // 33
            45,
            10,
        },
        { // 34
            45,
            11,
        },
        { // 35
            45,
            12,
        },
        { // 36
            39,
            255,
        },
        { // 37
            42,
            255,
        },
        { // 38
            45,
            15,
        },
        { // 39
            45,
            16,
        },
        { // 40
            45,
            17,
        },
        { // 41
            45,
            18,
        },
        { // 42
            45,
            19,
        },
        { // 43
            41,
            255,
        },
        { // 44
            45,
            21,
        },
        { // 45
            36,
            255,
        },
        { // 46
            37,
            255,
        },
        { // 47
            40,
            255,
        },
        { // 48
            0,
            255,
        },
        { // 49
            1,
            255,
        },
        { // 50
            2,
            255,
        },
        { // 51
            3,
            255,
        },
        { // 52
            4,
            255,
        },
        { // 53
            5,
            255,
        },
        { // 54
            6,
            255,
        },
        { // 55
            7,
            255,
        },
        { // 56
            8,
            255,
        },
        { // 57
            9,
            255,
        },
        { // 58
            45,
            35,
        },
        { // 59
            44,
            15,
        },
        { // 60
            44,
            16,
        },
        { // 61
            44,
            17,
        },
        { // 62
            44,
            18,
        },
        { // 63
            44,
            19,
        },
        { // 64
            44,
            31,
        },
        { // 65
            10,
            255,
        },
        { // 66
            11,
            255,
        },
        { // 67
            12,
            255,
        },
        { // 68
            13,
            255,
        },
        { // 69
            14,
            255,
        },
        { // 70
            15,
            255,
        },
        { // 71
            16,
            255,
        },
        { // 72
            17,
            255,
        },
        { // 73
            18,
            255,
        },
        { // 74
            19,
            255,
        },
        { // 75
            20,
            255,
        },
        { // 76
            21,
            255,
        },
        { // 77
            22,
            255,
        },
        { // 78
            23,
            255,
        },
        { // 79
            24,
            255,
        },
        { // 80
            25,
            255,
        },
        { // 81
            26,
            255,
        },
        { // 82
            27,
            255,
        },
        { // 83
            28,
            255,
        },
        { // 84
            29,
            255,
        },
        { // 85
            30,
            255,
        },
        { // 86
            31,
            255,
        },
        { // 87
            32,
            255,
        },
        { // 88
            33,
            255,
        },
        { // 89
            34,
            255,
        },
        { // 90
            35,
            255,
        },
        { // 91
            44,
            20,
        },
        { // 92
            44,
            21,
        },
        { // 93
            44,
            22,
        },
        { // 94
            44,
            23,
        },
        { // 95
            44,
            24,
        },
        { // 96
            44,
            32,
        },
        { // 97
            46,
            10,
        },
        { // 98
            46,
            11,
        },
        { // 99
            46,
            12,
        },
        { // 100
            46,
            13,
        },
        { // 101
            46,
            14,
        },
        { // 102
            46,
            15,
        },
        { // 103
            46,
            16,
        },
        { // 104
            46,
            17,
        },
        { // 105
            46,
            18,
        },
        { // 106
            46,
            19,
        },
        { // 107
            46,
            20,
        },
        { // 108
            46,
            21,
        },
        { // 109
            46,
            22,
        },
        { // 110
            46,
            23,
        },
        { // 111
            46,
            24,
        },
        { // 112
            46,
            25,
        },
        { // 113
            46,
            26,
        },
        { // 114
            46,
            27,
        },
        { // 115
            46,
            28,
        },
        { // 116
            46,
            29,
        },
        { // 117
            46,
            30,
        },
        { // 118
            46,
            31,
        },
        { // 119
            46,
            32,
        },
        { // 120
            46,
            33,
        },
        { // 121
            46,
            34,
        },
        { // 122
            46,
            35,
        },
        { // 123
            44,
            25,
        },
        { // 124
            44,
            26,
        },
        { // 125
            44,
            27,
        },
        { // 126
            44,
            28,
        },
        { // 127
            44,
            29,
        },
    };

    uint16_t const code93_encode[] =
    {
        // 0
        0x114,	// 1,0001,0100
        // 1
        0x148,	// 1,0100,1000
        // 2
        0x144,	// 1,0100,0100
        // 3
        0x142,	// 1,0100,0010
        // 4
        0x128,	// 1,0010,1000
        // 5
        0x124,	// 1,0010,0100
        // 6
        0x122,	// 1,0010,0010
        // 7
        0x150,	// 1,0101,0000
        // 8
        0x112,	// 1,0001,0010
        // 9
        0x10a,	// 1,0000,1010
        // 10
        0x1a8,	// 1,1010,1000
        // 11
        0x1a4,	// 1,1010,0100
        // 12
        0x1a2,	// 1,1010,0010
        // 13
        0x194,	// 1,1001,0100
        // 14
        0x192,	// 1,1001,0010
        // 15
        0x18a,	// 1,1000,1010
        // 16
        0x168,	// 1,0110,1000
        // 17
        0x164,	// 1,0110,0100
        // 18
        0x162,	// 1,0110,0010
        // 19
        0x134,	// 1,0011,0100
        // 20
        0x11a,	// 1,0001,1010
        // 21
        0x158,	// 1,0101,1000
        // 22
        0x14c,	// 1,0100,1100
        // 23
        0x146,	// 1,0100,0110
        // 24
        0x12c,	// 1,0010,1100
        // 25
        0x116,	// 1,0001,0110
        // 26
        0x1b4,	// 1,1011,0100
        // 27
        0x1b2,	// 1,1011,0010
        // 28
        0x1ac,	// 1,1010,1100
        // 29
        0x1a6,	// 1,1010,0110
        // 30
        0x196,	// 1,1001,0110
        // 31
        0x19a,	// 1,1001,1010
        // 32
        0x16c,	// 1,0110,1100
        // 33
        0x166,	// 1,0110,0110
        // 34
        0x136,	// 1,0011,0110
        // 35
        0x13a,	// 1,0011,1010
        // 36
        0x12e,	// 1,0010,1110
        // 37
        0x1d4,	// 1,1101,0100
        // 38
        0x1d2,	// 1,1101,0010
        // 39
        0x1ca,	// 1,1100,1010
        // 40
        0x16e,	// 1,0110,1110
        // 41
        0x176,	// 1,0111,0110
        // 42
        0x1ae,	// 1,1010,1110
        // 43
        0x126,	// 1,0010,0110
        // 44
        0x1da,	// 1,1101,1010
        // 45
        0x1d6,	// 1,1101,0110
        // 46
        0x132,	// 1,0011,0010
        // 47
        0x15e,	// 1,0101,1110
        // 48
        0x2bd,	// 10,1011,1101
    };

    // 接收条码命令
    n = PrintBarcode_GetBarcodeInput(barcode, sizeof(barcode), m);
    // 检查条码格式的合法性
    if (n == 0)
    {
        return;
    }
    for (i=0; i<n; i++)
    {
        if (barcode[i] >= 128)
        {
            return;
        }
    }

    memset(dot, 0, sizeof(dot));
    //col = 0;
    col = esc_sts.leftspace;

    // 打印起始码
    col = PrintBarcodePixel(code93_encode[47], 9, dot, col);
    // 打印资料码
    for (i=0; i<n; i++)
    {
        c1 = code93_decode[(barcode[i])][0];
        c2 = code93_decode[(barcode[i])][1];
        col = PrintBarcodePixel(code93_encode[c1], 9, dot, col);
        if (c2 < ARRAY_SIZE(code93_encode))
        {
            col = PrintBarcodePixel(code93_encode[c2], 9, dot, col);
        }
    }
    // 打印校验和
    // "C" check digit character
    nWidth = 1;
    nSum = 0;
    i = n-1;
#if 0
    do
    {
        c1 = code93_decode[(barcode[i])][0];
        c2 = code93_decode[(barcode[i])][1];

        // add to the sum
        nSum += (nWidth * c1);
        if (nWidth < 20)
        {
            nWidth++;
        }
        else
        {
            nWidth = 1;
        }
        // if its a two sequence character
        if (c2 < ARRAY_SIZE(code93_encode))
        {
            nSum += (nWidth * c2);
            if (nWidth < 20)
            {
                nWidth++;
            }
            else
            {
                nWidth = 1;
            }
        }
    }
    while (i--);
#else//校验的顺序是从后面算到前面，所以要先算c2的校验值
    do
    {
        c1 = code93_decode[(barcode[i])][0];
        c2 = code93_decode[(barcode[i])][1];

        // if its a two sequence character
        if (c2 < ARRAY_SIZE(code93_encode))
        {
            nSum += (nWidth * c2);
            if (nWidth < 20)
            {
                nWidth++;
            }
            else
            {
                nWidth = 1;
            }
        }


        // add to the sum
        nSum += (nWidth * c1);
        if (nWidth < 20)
        {
            nWidth++;
        }
        else
        {
            nWidth = 1;
        }

    }
    while (i--);

#endif

    // draw the "C" check digit character
    col = PrintBarcodePixel(code93_encode[nSum%47], 9, dot, col);

    // "K" check digit character - include the "C" check digit character in calculations
    nWidth = 2;
    nSum = nSum%47;
    i = n-1;
#if 0
    do
    {
        c1 = code93_decode[(barcode[i])][0];
        c2 = code93_decode[(barcode[i])][1];

        // add to the sum
        nSum += (nWidth * c1);
        if (nWidth < 15)
        {
            nWidth++;
        }
        else
        {
            nWidth = 1;
        }
        // if its a two sequence character
        if (c2 < ARRAY_SIZE(code93_encode))
        {
            nSum += (nWidth * c2);
            if (nWidth < 15)
            {
                nWidth++;
            }
            else
            {
                nWidth = 1;
            }
        }
    }
    while (i--);
#else//校验的顺序是从后面算到前面，所以要先算c2的校验值
    do
    {
        c1 = code93_decode[(barcode[i])][0];
        c2 = code93_decode[(barcode[i])][1];
        // if its a two sequence character
        if (c2 < ARRAY_SIZE(code93_encode))
        {
            nSum += (nWidth * c2);
            if (nWidth < 15)
            {
                nWidth++;
            }
            else
            {
                nWidth = 1;
            }
        }

        // add to the sum
        nSum += (nWidth * c1);
        if (nWidth < 15)
        {
            nWidth++;
        }
        else
        {
            nWidth = 1;
        }

    }
    while (i--);
#endif
    // draw the "K" check digit character
    col = PrintBarcodePixel(code93_encode[nSum%47], 9, dot, col);

    // 打印终止码
    col = PrintBarcodePixel(code93_encode[48], 10, dot, col);
    //----ccc
#if defined(CODEPAGE)||defined(ARABIC)
    //if(esc_sts.character_code_page==0x2a)
    if (esc_sts.from_right_print!=0)
        MoveBarcodeDot(col,dot);
#endif
    // 打印条码
    PrintBarcode(barcode, n, dot, col);
}

extern void PrintBarcode_CODE128(uint8_t m)
{
    uint8_t dot[LineDot/8];
    uint16_t col;
    uint8_t barcode[255];
    uint8_t barcode_HRI[255];
    uint8_t barcode_HRI_pt;
    uint8_t i, n;
    uint8_t currMode, offset, digit;
    uint32_t cksum;

    uint16_t const code128_encode[] =
    {
        // 0
        0x6cc,	// 110,1100,1100
        // 1
        0x66c,	// 110,0110,1100
        // 2
        0x666,	// 110,0110,0110
        // 3
        0x498,	// 100,1001,1000
        // 4
        0x48c,	// 100,1000,1100
        // 5
        0x44c,	// 100,0100,1100
        // 6
        0x4c8,	// 100,1100,1000
        // 7
        0x4c4,	// 100,1100,0100
        // 8
        0x464,	// 100,0110,0100
        // 9
        0x648,	// 110,0100,1000
        // 10
        0x644,	// 110,0100,0100
        // 11
        0x624,	// 110,0010,0100
        // 12
        0x59c,	// 101,1001,1100
        // 13
        0x4dc,	// 100,1101,1100
        // 14
        0x4ce,	// 100,1100,1110
        // 15
        0x5cc,	// 101,1100,1100
        // 16
        0x4ec,	// 100,1110,1100
        // 17
        0x4e6,	// 100,1110,0110
        // 18
        0x672,	// 110,0111,0010
        // 19
        0x65c,	// 110,0101,1100
        // 20
        0x64e,	// 110,0100,1110
        // 21
        0x6e4,	// 110,1110,0100
        // 22
        0x674,	// 110,0111,0100
        // 23
        0x76e,	// 111,0110,1110
        // 24
        0x74c,	// 111,0100,1100
        // 25
        0x72c,	// 111,0010,1100
        // 26
        0x726,	// 111,0010,0110
        // 27
        0x764,	// 111,0110,0100
        // 28
        0x734,	// 111,0011,0100
        // 29
        0x732,	// 111,0011,0010
        // 30
        0x6d8,	// 110,1101,1000
        // 31
        0x6c6,	// 110,1100,0110
        // 32
        0x636,	// 110,0011,0110
        // 33
        0x518,	// 101,0001,1000
        // 34
        0x458,	// 100,0101,1000
        // 35
        0x446,	// 100,0100,0110
        // 36
        0x588,	// 101,1000,1000
        // 37
        0x468,	// 100,0110,1000
        // 38
        0x462,	// 100,0110,0010
        // 39
        0x688,	// 110,1000,1000
        // 40
        0x628,	// 110,0010,1000
        // 41
        0x622,	// 110,0010,0010
        // 42
        0x5b8,	// 101,1011,1000
        // 43
        0x58e,	// 101,1000,1110
        // 44
        0x46e,	// 100,0110,1110
        // 45
        0x5d8,	// 101,1101,1000
        // 46
        0x5c6,	// 101,1100,0110
        // 47
        0x476,	// 100,0111,0110
        // 48
        0x776,	// 111,0111,0110
        // 49
        0x68e,	// 110,1000,1110
        // 50
        0x62e,	// 110,0010,1110
        // 51
        0x6e8,	// 110,1110,1000
        // 52
        0x6e2,	// 110,1110,0010
        // 53
        0x6ee,	// 110,1110,1110
        // 54
        0x758,	// 111,0101,1000
        // 55
        0x746,	// 111,0100,0110
        // 56
        0x716,	// 111,0001,0110
        // 57
        0x768,	// 111,0110,1000
        // 58
        0x762,	// 111,0110,0010
        // 59
        0x71a,	// 111,0001,1010
        // 60
        0x77a,	// 111,0111,1010
        // 61
        0x642,	// 110,0100,0010
        // 62
        0x78a,	// 111,1000,1010
        // 63
        0x530,	// 101,0011,0000
        // 64
        0x50c,	// 101,0000,1100
        // 65
        0x4b0,	// 100,1011,0000
        // 66
        0x486,	// 100,1000,0110
        // 67
        0x42c,	// 100,0010,1100
        // 68
        0x426,	// 100,0010,0110
        // 69
        0x590,	// 101,1001,0000
        // 70
        0x584,	// 101,1000,0100
        // 71
        0x4d0,	// 100,1101,0000
        // 72
        0x4c2,	// 100,1100,0010
        // 73
        0x434,	// 100,0011,0100
        // 74
        0x432,	// 100,0011,0010
        // 75
        0x612,	// 110,0001,0010
        // 76
        0x650,	// 110,0101,0000
        // 77
        0x7ba,	// 111,1011,1010
        // 78
        0x614,	// 110,0001,0100
        // 79
        0x47a,	// 100,0111,1010
        // 80
        0x53c,	// 101,0011,1100
        // 81
        0x4bc,	// 100,1011,1100
        // 82
        0x49e,	// 100,1001,1110
        // 83
        0x5e4,	// 101,1110,0100
        // 84
        0x4f4,	// 100,1111,0100
        // 85
        0x4f2,	// 100,1111,0010
        // 86
        0x7a4,	// 111,1010,0100
        // 87
        0x794,	// 111,1001,0100
        // 88
        0x792,	// 111,1001,0010
        // 89
        0x6de,	// 110,1101,1110
        // 90
        0x6f6,	// 110,1111,0110
        // 91
        0x7b6,	// 111,1011,0110
        // 92
        0x578,	// 101,0111,1000
        // 93
        0x51e,	// 101,0001,1110
        // 94
        0x45e,	// 100,0101,1110
        // 95
        0x5e8,	// 101,1110,1000
        // 96
        0x5e2,	// 101,1110,0010
        // 97
        0x7a8,	// 111,1010,1000
        // 98
        0x7a2,	// 111,1010,0010
        // 99
        0x5de,	// 101,1101,1110
        // 100
        0x5ee,	// 101,1110,1110
        // 101
        0x75e,	// 111,0101,1110
        // 102
        0x7ae,	// 111,1010,1110
        // 103
        0x684,	// 110,1000,0100
        // 104
        0x690,	// 110,1001,0000
        // 105
        0x69c,	// 110,1001,1100
        // 106
        0x18eb,	// 1,1000,1110,1011
    };

    // 接收条码命令
    n = PrintBarcode_GetBarcodeInput(barcode, sizeof(barcode), m);
    // 检查条码格式的合法性
    if (n < 2)
    {
        return;
    }

    for (i=0; i<n; i++)
    {
        if (barcode[i] >= 128)
        {
            return;
        }
    }

    memset(dot, 0, sizeof(dot));
    currMode = 'S';
    i = col = digit = cksum = barcode_HRI_pt = 0;
    col = esc_sts.leftspace;
    do
    {
        if (barcode[i] == '{')	// ESCAPE
        {
            i++;
            if (i >= n) return;
            switch (barcode[i])
            {
            case 'A':
                switch (currMode)
                {
                case 'S':
                    col = PrintBarcodePixel(code128_encode[103], 11, dot, col);
                    cksum = 103;
                    break;
                default:
                    col = PrintBarcodePixel(code128_encode[101], 11, dot, col);
                    cksum += 101 * digit;
                    break;
                }
                currMode = 'A';
                digit++;
                i++;
                break;
            case 'B':
                switch (currMode)
                {
                case 'S':
                    col = PrintBarcodePixel(code128_encode[104], 11, dot, col);
                    cksum = 104;
                    break;
                default:
                    col = PrintBarcodePixel(code128_encode[100], 11, dot, col);
                    cksum += 100 * digit;
                    break;
                }
                currMode = 'B';
                digit++;
                i++;
                break;
            case 'C':
                switch (currMode)
                {
                case 'S':
                    col = PrintBarcodePixel(code128_encode[105], 11, dot, col);
                    cksum = 105;
                    break;
                default:
                    col = PrintBarcodePixel(code128_encode[99], 11, dot, col);
                    cksum += 99 * digit;
                    break;
                }
                currMode = 'C';
                digit++;
                i++;
                break;
            case '{':
                // ESCAPE char
                goto ESC_OUT;
            case 'S':
                switch (currMode)
                {
                case 'A':
                case 'B':
                    col = PrintBarcodePixel(code128_encode[98], 11, dot, col);
                    cksum += 98 * digit;
                    digit++;
                    i++;
                    break;
                default:
                    return; 	// unknown format
                }
                break;
            case '1':
                switch (currMode)
                {
                case 'A':
                case 'B':
                case 'C':
                    col = PrintBarcodePixel(code128_encode[102], 11, dot, col);
                    cksum += 102 * digit;
                    digit++;
                    i++;
                    break;
                default:
                    return; 	// unknown format
                }
                break;
            case '2':
                switch (currMode)
                {
                case 'A':
                case 'B':
                    col = PrintBarcodePixel(code128_encode[97], 11, dot, col);
                    cksum += 97 * digit;
                    digit++;
                    i++;
                    break;
                default:
                    return; 	// unknown format
                }
                break;
            case '3':
                switch (currMode)
                {
                case 'A':
                case 'B':
                    col = PrintBarcodePixel(code128_encode[96], 11, dot, col);
                    cksum += 96 * digit;
                    digit++;
                    i++;
                    break;
                default:
                    return; 	// unknown format
                }
                break;
            case '4':
                switch (currMode)
                {
                case 'A':
                    col = PrintBarcodePixel(code128_encode[101], 11, dot, col);
                    cksum += 101 * digit;
                    digit++;
                    i++;
                    break;
                case 'B':
                    col = PrintBarcodePixel(code128_encode[100], 11, dot, col);
                    cksum += 100 * digit;
                    digit++;
                    i++;
                    break;
                default:
                    return; 	// unknown format
                }
                break;
            default:
                return;		// unknow format
            }
            continue;
        }
ESC_OUT:
        switch (currMode)
        {
        case 'S':
            return;		// unknown format
        case 'A':	// A (0x00~0x5F)
            if (barcode[i] < 0x20)
            {
                offset = barcode[i]+0x40;
                col = PrintBarcodePixel(code128_encode[offset], 11, dot, col);
                barcode_HRI[barcode_HRI_pt++] = barcode[i];
                i++;
                cksum += offset * digit;
                digit++;
            }
            else if (barcode[i] < 0x60)		// 0x20~0x5F
            {
                offset = barcode[i]-0x20;
                col = PrintBarcodePixel(code128_encode[offset], 11, dot, col);
                barcode_HRI[barcode_HRI_pt++] = barcode[i];
                i++;
                cksum += offset * digit;
                digit++;
            }
            else	// 0x60~0x7F
            {
                return;		// unknown format
            }
            break;
        case 'B':	// B (0x20~0x7F)
            if (barcode[i] < 0x20)
            {
                return;		// unknown format
            }
            else
            {
                offset = barcode[i]-0x20;
                col = PrintBarcodePixel(code128_encode[offset], 11, dot, col);
                barcode_HRI[barcode_HRI_pt++] = barcode[i];
                i++;
                cksum += offset * digit;
                digit++;
            }
            break;
        case 'C':	// C (00~99)
            if ((i < n) && (barcode[i]<=0x63))
            {
                barcode_HRI[barcode_HRI_pt++] =barcode[i]/10+'0';
                barcode_HRI[barcode_HRI_pt++] =barcode[i]%10+'0';
                col = PrintBarcodePixel(code128_encode[barcode[i]], 11, dot, col);
                cksum += barcode[i] * digit;
                digit++;
                i ++;
            }
            /*
            if(((i+1) < n) && (isdigit(barcode[i]) && isdigit(barcode[i+1])))
            {
            	barcode_HRI[barcode_HRI_pt++] = barcode[i];
            	barcode_HRI[barcode_HRI_pt++] = barcode[i+1];
            	offset = (barcode[i]-'0')*10+(barcode[i+1]-'0');
            	col = PrintBarcodePixel(code128_encode[offset], 11, dot, col);
            	i += 2;
            	cksum += offset * digit;
            	digit++;
            }*/
            else
            {
                return;		// unknown format
            }
            break;
        }
    }
    while (i<n);
    // 打印校验码
    col = PrintBarcodePixel(code128_encode[cksum % 103], 11, dot, col);

    // 打印终止码
    col = PrintBarcodePixel(code128_encode[106], 13, dot, col);
    //----ccc
#if defined(CODEPAGE)||defined(ARABIC)
    //if(esc_sts.character_code_page==0x2a)
    if (esc_sts.from_right_print!=0)
        MoveBarcodeDot(col,dot);
#endif
    // 打印条码
    PrintBarcode(barcode_HRI, barcode_HRI_pt, dot, col);
}

#endif


