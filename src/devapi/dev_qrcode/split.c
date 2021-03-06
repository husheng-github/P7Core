
#if HAVE_CONFIG_H
	#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "qrencode.h"
#include "qrinput.h"
#include "qrspec.h"
#include "split.h"

#define isdigit(__c__) ((unsigned char)((signed char)(__c__) - '0') < 10)	//判断是不是数字
#define isalnum(__c__) (QRinput_lookAnTable(__c__) >= 0)	//判断__c__的最高位是不是1，是返回-1，否则查表QRinput_anTable[128]

#if !HAVE_STRDUP
#undef strdup

/*拷贝数据*/
char *strdup(const char *s)
{
	size_t len = strlen(s) + 1;	//计算字符串的长度
	
	void *new = malloc(len);	//分配一个新的空间给new
	
	if(new == NULL) 
		return NULL;
	
	return (char *)memcpy(new, s, len);	//拷贝s数据到new中
}

#endif

static QRencodeMode Split_identifyMode(const char *string, QRencodeMode hint)
{
	unsigned char c, d;
	unsigned int word;

	c = string[0];

	if(c == '\0') 
		return QR_MODE_NUL;		//如果第一个字符为空，返回错误
		
	if(isdigit(c)) 
	{
		return QR_MODE_NUM;		//数字模式
	} 
	else if(isalnum(c))
	{
		return QR_MODE_AN;		//字母数字模式
	} 
	else if(hint == QR_MODE_KANJI)
	{
		d = string[1];
		
		if(d != '\0') 
		{
			word = ((unsigned int)c << 8) | d;	//合并2个字节的数据成一个字
			
			if((word >= 0x8140 && word <= 0x9ffc) || (word >= 0xe040 && word <= 0xebbf)) 
			{
				return QR_MODE_KANJI;
			}
		}
	}

	return QR_MODE_8;	//如果都不是8bit模式
}

static int Split_eatNum(const char *string, QRinput *input, QRencodeMode hint);
static int Split_eatAn(const char *string, QRinput *input, QRencodeMode hint);
static int Split_eat8(const char *string, QRinput *input, QRencodeMode hint);
static int Split_eatKanji(const char *string, QRinput *input, QRencodeMode hint);

static int Split_eatNum(const char *string, QRinput *input,QRencodeMode hint)
{
	const char *p;
	int ret;
	int run;
	int dif;
	int ln;
	QRencodeMode mode;

	ln = QRspec_lengthIndicator(QR_MODE_NUM, input->version);

	p = string;
	
	while(isdigit(*p)) 
	{
		p++;
	}
	
	run = p - string;
	
	mode = Split_identifyMode(p, hint);
	
	if(mode == QR_MODE_8) 
	{
		dif = QRinput_estimateBitsModeNum(run) + 4 + ln
			+ QRinput_estimateBitsMode8(1) /* + 4 + l8 */
			- QRinput_estimateBitsMode8(run + 1) /* - 4 - l8 */;
		
		if(dif > 0) 
		{
			return Split_eat8(string, input, hint);
		}
	}
	
	if(mode == QR_MODE_AN) 
	{
		dif = QRinput_estimateBitsModeNum(run) + 4 + ln
			+ QRinput_estimateBitsModeAn(1) /* + 4 + la */
			- QRinput_estimateBitsModeAn(run + 1) /* - 4 - la */;
		
		if(dif > 0) 
		{
			return Split_eatAn(string, input, hint);
		}
	}

	ret = QRinput_append(input, QR_MODE_NUM, run, (unsigned char *)string);
	
	if(ret < 0) 
		return -1;

	return run;
}

static int Split_eatAn(const char *string, QRinput *input, QRencodeMode hint)
{
	const char *p, *q;
	int ret;
	int run;
	int dif;
	int la, ln;

	la = QRspec_lengthIndicator(QR_MODE_AN, input->version);
	ln = QRspec_lengthIndicator(QR_MODE_NUM, input->version);

	p = string;
	while(isalnum(*p)) 
	{
		if(isdigit(*p)) 
		{
			q = p;
			
			while(isdigit(*q))
			{
				q++;
			}
			
			dif = QRinput_estimateBitsModeAn(p - string) /* + 4 + la */
				+ QRinput_estimateBitsModeNum(q - p) + 4 + ln
				+ (isalnum(*q)?(4 + ln):0)
				- QRinput_estimateBitsModeAn(q - string) /* - 4 - la */;
			
			if(dif < 0) 
			{
				break;
			} 
			else 
			{
				p = q;
			}
		}		
		else 
		{
			p++;
		}
	}

	run = p - string;

	if(*p && !isalnum(*p)) 
	{
		dif = QRinput_estimateBitsModeAn(run) + 4 + la
			+ QRinput_estimateBitsMode8(1) /* + 4 + l8 */
			- QRinput_estimateBitsMode8(run + 1) /* - 4 - l8 */;
		
		if(dif > 0) 
		{
			return Split_eat8(string, input, hint);
		}
	}

	ret = QRinput_append(input, QR_MODE_AN, run, (unsigned char *)string);
	
	if(ret < 0) 
		return -1;

	return run;
}

static int Split_eatKanji(const char *string, QRinput *input, QRencodeMode hint)
{
	const char *p;
	int ret;
	int run;

	p = string;
	
	while(Split_identifyMode(p, hint) == QR_MODE_KANJI) 
	{
		p += 2;
	}
	
	run = p - string;
	
	ret = QRinput_append(input, QR_MODE_KANJI, run, (unsigned char *)string);
	
	if(ret < 0) 
		return -1;

	return run;
}

static int Split_eat8(const char *string, QRinput *input, QRencodeMode hint)
{
	const char *p, *q;
	QRencodeMode mode;
	int ret;
	int run;
	int dif;
	int la, ln, l8;
	int swcost;

	la = QRspec_lengthIndicator(QR_MODE_AN, input->version);
	ln = QRspec_lengthIndicator(QR_MODE_NUM, input->version);
	l8 = QRspec_lengthIndicator(QR_MODE_8, input->version);

	p = string + 1;
	
	while(*p != '\0') 
	{
		mode = Split_identifyMode(p, hint);
		
		if(mode == QR_MODE_KANJI) 
		{
			break;
		}
		
		if(mode == QR_MODE_NUM) 
		{
			q = p;
			
			while(isdigit(*q)) 
			{
				q++;
			}
			
			if(Split_identifyMode(q, hint) == QR_MODE_8) 
			{
				swcost = 4 + l8;
			} 
			else 
			{
				swcost = 0;
			}
			
			dif = QRinput_estimateBitsMode8(p - string) /* + 4 + l8 */
				+ QRinput_estimateBitsModeNum(q - p) + 4 + ln
				+ swcost
				- QRinput_estimateBitsMode8(q - string) /* - 4 - l8 */;
			
			if(dif < 0)
			{
				break;
			} 
			else 
			{
				p = q;
			}
		} 
		else if(mode == QR_MODE_AN)
		{
			q = p;
			
			while(isalnum(*q)) 
			{
				q++;
			}
			
			if(Split_identifyMode(q, hint) == QR_MODE_8) 
			{
				swcost = 4 + l8;
			} 
			else
			{
				swcost = 0;
			}
			
			dif = QRinput_estimateBitsMode8(p - string) /* + 4 + l8 */
				+ QRinput_estimateBitsModeAn(q - p) + 4 + la
				+ swcost
				- QRinput_estimateBitsMode8(q - string) /* - 4 - l8 */;
			
			if(dif < 0)
			{
				break;
			} 
			else
			{
				p = q;
			}
		}
		else 
		{
			p++;
		}
	}

	run = p - string;
	
	ret = QRinput_append(input, QR_MODE_8, run, (unsigned char *)string);
	
	if(ret < 0) 
		return -1;

	return run;
}

//分割字符串
static int Split_splitString(const char *string, QRinput *input,
		QRencodeMode hint)
{
	int length;
	QRencodeMode mode;

	if(*string == '\0') 
		return 0;

	mode = Split_identifyMode(string, hint);	//拆分识别模式
	
	if(mode == QR_MODE_NUM) 
	{
		length = Split_eatNum(string, input, hint);
	}
	else if(mode == QR_MODE_AN) 
	{
		length = Split_eatAn(string, input, hint);
	} 
	else if(mode == QR_MODE_KANJI && hint == QR_MODE_KANJI)
	{
		length = Split_eatKanji(string, input, hint);
	} 
	else 
	{
		length = Split_eat8(string, input, hint);
	}
	
	if(length == 0) 
		return 0;
	
	if(length < 0) 
		return -1;
	
	return Split_splitString(&string[length], input, hint);
}

static char *dupAndToUpper(const char *str, QRencodeMode hint)//QRencodeMode为编码模式
{
	char *newstr, *p;
	QRencodeMode mode;

	newstr = strdup(str);	//给newstr重新分配缓存
	
	if(newstr == NULL) 
		return NULL;

	p = newstr;
	
	while(*p != '\0') 
	{
		mode = Split_identifyMode(p, hint);//拆分识别模式
		
		if(mode == QR_MODE_KANJI) 
		{
			p += 2;
		} 
		else 
		{
			if (*p >= 'a' && *p <= 'z') 
			{
				*p = (char)((int)*p - 32);
			}
			
			p++;
		}
	}

	return newstr;
}

//分割二位字符串输入
//string：编码的数据
//input：编码数据的输入信息
//hint：编码模式
//casesensitive：是否区分大小写 0不区分(都是小写)
int Split_splitStringToQRinput(const char *string, QRinput *input,
		QRencodeMode hint, int casesensitive)
{
	char *newstr;
	int ret;

	if(string == NULL || *string == '\0') 
	{
		errno = EINVAL;
		return -1;
	}
	
	if(!casesensitive) 
	{
		newstr = dupAndToUpper(string, hint);
		
		if(newstr == NULL) 
			return -1;
		
		ret = Split_splitString(newstr, input, hint);
		
		free(newstr);	//dupAndToUpper申请了内存，必须释放
	} 
	else 
	{
		ret = Split_splitString(string, input, hint);
	}

	return ret;
}
