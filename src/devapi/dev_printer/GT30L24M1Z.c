
#if 0
#define DZ_Data 0
/***************************************
ASCII 调用
ASCIICode：表示ASCII 码（8bits）
BaseAdd：说明该套字库在芯片中的起始地址。
r_dat_bat： 是读点阵数据函数。
DZ_Data：是保存读出的点阵数据的数组。
****************************************/
u8  ASCII_GetData(u8 ASCIICode,u32 BaseAdd)
{
	if  ((ASCIICode >= 0x20)&&(ASCIICode<=0x7e))
	{
	  switch(BaseAdd)
	  {

	  case 0x1F6500: F25L_Read((ASCIICode-0x20)*48+BaseAdd,48,DZ_Data); //12X24
	                 break ;
	  case 0x1F88D0: F25L_Read((ASCIICode-0x20)*74+BaseAdd+2,72,DZ_Data); //24X24 A
	                 break ;
	  case 0x1FA490: F25L_Read((ASCIICode-0x20)*74+BaseAdd+2,72,DZ_Data); //24X24 T
	                 break ;
	  default:       break ;
	  }
	 return 1;
	}
    else  return 0;
}

/***************************************************************************************************
24x24点阵GB18030汉字&字符
函数：u8 GB18030_24_GetData(u8 c1, u8 c2, u8 c3, u8 c4)
功能：计算汉字点阵在芯片中的地址,读取点阵数据到指定数组。
参数：c1,c2,c3,c4：
4字节汉字内码通过参数c1,c2,c3,c4传入，双字节内码通过参数c1,c2 传入，c3=0,c4=0
返回：汉字点阵的字节地址(byte address)。
如果用户是按 word mode 读取点阵数据，则其地址(word address)为字节地址除以2，
即：word address = byte address / 2 . 例如：
BaseAdd: 说明汉字点阵数据在字库芯片中的起始地址，即BaseAdd＝0;
 “啊”字的内码为0xb0a1,则byte address = GB18030_24_GetData(0xb0,0xa1,0x00,0x00) *72+BaseAdd;
word address = byte address / 2
“.”字的内码为0x8139ee39,则byte address = GB18030_24_GetData(0x81,0x39,0xee,0x39) *72+ BaseAdd
word address = byte address / 2
****************************************************************************************************/
u32 GB18030_24_GetData(u8 c1, u8 c2, u8 c3, u8 c4)
{
    u32 h=0;
    u32 BaseAdd=0;
    if(c2==0x7f)
    {
        r_dat_bat(BaseAdd,72,DZ_Data);
        return (BaseAdd);
    }
    if(c1>=0xA1 && c1 <= 0xa9 && c2>=0xa1) //Section 1
        h= (c1 - 0xA1) * 94 + (c2 - 0xA1);
    else if(c1>=0xa8 && c1 <= 0xa9 && c2<0xa1) //Section 5
    {
    if(c2>0x7f) c2--;
        h=(c1-0xa8)*96 + (c2-0x40)+846;
    }
    if(c1>=0xb0 && c1 <= 0xf7 && c2>=0xa1) //Section 2
        h= (c1 - 0xB0) * 94 + (c2 - 0xA1)+1038;
    else if(c1<0xa1 && c1>=0x81 && c2>=0x40 ) //Section 3
    {
        if(c2>0x7f)
           c2--;
        h=(c1-0x81)*190 + (c2-0x40) + 1038 +6768;
    }
    else if(c1>=0xaa && c2<0xa1) //Section 4
    {
        if(c2>0x7f)
           c2--;
        h=(c1-0xaa)*96 + (c2-0x40) + 1038 +12848;
    }
    else if(c1==0x81 && c2>=0x39) //四字节区1
    {
        h =1038 + 21008+(c3-0xEE)*10+c4-0x39;
    }
    else if(c1==0x82)//四字节区2
    {
        h =1038 + 21008+161+(c2-0x30)*1260+(c3-0x81)*10+c4-0x30;
    }
    r_dat_bat(h*72+BaseAdd,72,DZ_Data);
    return(h*72+BaseAdd);
}

//12x24点阵国标扩展字符
/***************************************************************
BaseAdd：说明本套字库在字库芯片中的起始字节地址。
FontCode：表示字符内码（16bits）
ByteAddress：表示字符点阵在芯片中的字节地址。
****************************************************************/
u32 GB_EXT_12X24_GetData(u16 FontCode)
{
 u32 Address,BaseAdd=0x1F6530;
 if (FontCode>= 0xAAA1 && FontCode<=0xAAFE )
 Address = (FontCode-0xAAA1 ) * 48+BaseAdd;
 else if(FontCode>= 0xABA1 && FontCode<=0xABC0 )
 Address = (FontCode-0xABA1 + 95) * 48+BaseAdd;
 r_dat_bat(Address,48,DZ_Data);
 return Address;
}
#if 0
/****************************************************
从字库中读数据函数
说明：
Address  ： 表示字符点阵在芯片中的字节地址。
byte_long： 是读点阵数据字节数。
*p_arr   ： 是保存读出的点阵数据的数组。
*****************************************************/
u8 r_dat_bat(u32 address,u8 byte_long,u8 *p_arr)
{
	unsigned int j=0;
	CS_L;
	SendByte(address);
	for(j=0;j<byte_long;j++)
	{
	 p_arr[j]=ReadByte();
	}
	CS_H;
	return p_arr[0];
}

void SendByte(u32 cmd)
{
	u8 i;
	cmd=cmd|0x03000000;
	for(i=0;i<32;i++)
	{
		CLK_L;
		if(cmd&0x80000000)
			SI_H;
		else
			SI_L;
		CLK_H;
		cmd=cmd<<1;
	}
}

u8 ReadByte(void)
{
	u8 i;
	u8 dat=0;
	CLK_H;
	for(i=0;i<8;i++)
	{
		CLK_L;
		dat=dat<<1;
		if(SO)
			dat=dat|0x01;
		else
			dat&=0xfe;
		CLK_H	;
	}
	return dat;
}
#endif
#endif
