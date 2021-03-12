
#if 0
#define DZ_Data 0
/***************************************
ASCII ����
ASCIICode����ʾASCII �루8bits��
BaseAdd��˵�������ֿ���оƬ�е���ʼ��ַ��
r_dat_bat�� �Ƕ��������ݺ�����
DZ_Data���Ǳ�������ĵ������ݵ����顣
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
24x24����GB18030����&�ַ�
������u8 GB18030_24_GetData(u8 c1, u8 c2, u8 c3, u8 c4)
���ܣ����㺺�ֵ�����оƬ�еĵ�ַ,��ȡ�������ݵ�ָ�����顣
������c1,c2,c3,c4��
4�ֽں�������ͨ������c1,c2,c3,c4���룬˫�ֽ�����ͨ������c1,c2 ���룬c3=0,c4=0
���أ����ֵ�����ֽڵ�ַ(byte address)��
����û��ǰ� word mode ��ȡ�������ݣ������ַ(word address)Ϊ�ֽڵ�ַ����2��
����word address = byte address / 2 . ���磺
BaseAdd: ˵�����ֵ����������ֿ�оƬ�е���ʼ��ַ����BaseAdd��0;
 �������ֵ�����Ϊ0xb0a1,��byte address = GB18030_24_GetData(0xb0,0xa1,0x00,0x00) *72+BaseAdd;
word address = byte address / 2
��.���ֵ�����Ϊ0x8139ee39,��byte address = GB18030_24_GetData(0x81,0x39,0xee,0x39) *72+ BaseAdd
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
    else if(c1==0x81 && c2>=0x39) //���ֽ���1
    {
        h =1038 + 21008+(c3-0xEE)*10+c4-0x39;
    }
    else if(c1==0x82)//���ֽ���2
    {
        h =1038 + 21008+161+(c2-0x30)*1260+(c3-0x81)*10+c4-0x30;
    }
    r_dat_bat(h*72+BaseAdd,72,DZ_Data);
    return(h*72+BaseAdd);
}

//12x24���������չ�ַ�
/***************************************************************
BaseAdd��˵�������ֿ����ֿ�оƬ�е���ʼ�ֽڵ�ַ��
FontCode����ʾ�ַ����루16bits��
ByteAddress����ʾ�ַ�������оƬ�е��ֽڵ�ַ��
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
���ֿ��ж����ݺ���
˵����
Address  �� ��ʾ�ַ�������оƬ�е��ֽڵ�ַ��
byte_long�� �Ƕ����������ֽ�����
*p_arr   �� �Ǳ�������ĵ������ݵ����顣
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
