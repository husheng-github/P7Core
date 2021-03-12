
#ifndef __QRENCODE_H
#define __QRENCODE_H

#include "data_type.h"

#define min(a,b)	(((a) < (b)) ? (a) : (b))

//4种纠错等级
#define QR_LEVEL_L	0		//7%的字码可被修正
#define QR_LEVEL_M	1		//15%的字码可被修正
#define QR_LEVEL_Q	2		//25%的字码可被修正
#define QR_LEVEL_H	3		//30%的字码可被修正

//数据编码模式
#define QR_MODE_NUMERAL		0
#define QR_MODE_ALPHABET	1
#define QR_MODE_8BIT		2
#define QR_MODE_KANJI		3
#define QR_MODE_CHINESE		4	

//版本组
#define QR_VRESION_S	0 
#define QR_VRESION_M	1 
#define QR_VRESION_L	2 

#define QR_MARGIN	4
#define	QR_VER1_SIZE	21		//版本1的行列数

#define MAX_ALLCODEWORD		400		//3706//400// //P14,P35 数据容量[码字]* (E) (VER:40), 所有码字为8位   400

#define MAX_DATACODEWORD	400		//2956//400// //P27     最大信息码子(Ver：40-L)，所有码字为8位

#define MAX_CODEBLOCK		153		//最大纠错码字 Ver：36.37.38_L_第二块

#define MAX_VERSION         13		//128*160 v=27 125	13

#define MAX_MODULESIZE		(17+MAX_VERSION*4)	// 21:Version=1,最大字符=17(8.5个汉字)
												// 25:Version=2,最大字符=32(16个汉字)
												// 29:Version=3,最大字符=49(24.5个汉字)
												// 33:Version=4,最大字符=78(39个汉字)
												// 37:Version=5,最大字符=106(53个汉字) 
												// 41:Version=6,最大字符=134(67个汉字)
											 	// 45:Version=7,最大字符=154(77个汉字)
											 	// 49:Version=8,最大字符=192(96个汉字)
																				
extern int m_nSymbleSize;	//正方形的边长

extern unsigned char  m_byModuleData[MAX_MODULESIZE][MAX_MODULESIZE];	//最大正方形(125)的数据填充
//extern char **m_byModuleData;

typedef struct 
{
	unsigned short  int	 ncRSBlock;			//纠错的块数
	unsigned short  int	 ncAllCodeWord;		//码字总数
	unsigned short  int	 ncDataCodeWord;	//指定纠错等级下的数据码个数
}RS_BLOCKINFO;

typedef struct 
{
	unsigned short  int	 nVersionNo;			//版本 1~40
	unsigned short  int	 ncAllCodeWord; 		//码字总数=数据码字个数+纠错码字个数

	unsigned short  int	 ncDataCodeWord[4];		//4种纠错等级下的数据码字个数(0=L,1=M,2=Q,3=H)
	unsigned short  int	 ncAlignPoint;			//校正图形个数
	unsigned short  int	 nAlignPoint[6];		//校正图形行列坐标
	
	RS_BLOCKINFO RS_BlockInfo1[4];	//纠错块1
	RS_BLOCKINFO RS_BlockInfo2[4];	//纠错块2
}QR_VERSIONINFO;

bool EncodeData(char *lpsSource, int RSLevel, int *SymbleSize);
int GetEncodeVersion(int nVersion, char *lpsSource, int ncLength);
int EncodeSourceData(char *lpsSource, int ncLength, int nVerGroup);
int GetBitLength(unsigned char	 nMode, int ncData, int nVerGroup);
int SetBitStream(int nIndex, unsigned short wData, int ncData);
bool IsNumeralData(unsigned char c);
bool IsAlphabetData(unsigned char c);
bool IsKanjiData(unsigned char c1, unsigned char c2);
bool IsChineseData(unsigned char c1, unsigned char c2);	
unsigned char	 AlphabetToBinaly(unsigned char c);
unsigned short KanjiToBinaly(unsigned short wc);
unsigned short ChineseToBinaly(unsigned short wc);
void GetRSCodeWord(unsigned char	 *lpbyRSWork, int ncDataCodeWord, int ncRSCodeWord);
void FormatModule(void);
void SetFunctionModule(void);
void SetFinderPattern(int x, int y);
void SetAlignmentPattern(int x, int y);
void SetVersionPattern(void);
void SetCodeWordPattern(void);
void SetMaskingPattern(int nPatternNo);
void SetFormatInfoPattern(int nPatternNo);
int CountPenalty(void);
void Print_2DCode(void);
void test_qrcode(void);
int dev_Generate_QRencode(char *lpsSource, int RSLevel, u8 x, u8 y);
char ** malloc2D(void); 
void free2D(u8 **a);

#endif
