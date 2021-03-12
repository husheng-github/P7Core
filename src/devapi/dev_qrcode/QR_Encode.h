
#ifndef __QRENCODE_H
#define __QRENCODE_H

#include "data_type.h"

#define min(a,b)	(((a) < (b)) ? (a) : (b))

//4�־���ȼ�
#define QR_LEVEL_L	0		//7%������ɱ�����
#define QR_LEVEL_M	1		//15%������ɱ�����
#define QR_LEVEL_Q	2		//25%������ɱ�����
#define QR_LEVEL_H	3		//30%������ɱ�����

//���ݱ���ģʽ
#define QR_MODE_NUMERAL		0
#define QR_MODE_ALPHABET	1
#define QR_MODE_8BIT		2
#define QR_MODE_KANJI		3
#define QR_MODE_CHINESE		4	

//�汾��
#define QR_VRESION_S	0 
#define QR_VRESION_M	1 
#define QR_VRESION_L	2 

#define QR_MARGIN	4
#define	QR_VER1_SIZE	21		//�汾1��������

#define MAX_ALLCODEWORD		400		//3706//400// //P14,P35 ��������[����]* (E) (VER:40), ��������Ϊ8λ   400

#define MAX_DATACODEWORD	400		//2956//400// //P27     �����Ϣ����(Ver��40-L)����������Ϊ8λ

#define MAX_CODEBLOCK		153		//���������� Ver��36.37.38_L_�ڶ���

#define MAX_VERSION         13		//128*160 v=27 125	13

#define MAX_MODULESIZE		(17+MAX_VERSION*4)	// 21:Version=1,����ַ�=17(8.5������)
												// 25:Version=2,����ַ�=32(16������)
												// 29:Version=3,����ַ�=49(24.5������)
												// 33:Version=4,����ַ�=78(39������)
												// 37:Version=5,����ַ�=106(53������) 
												// 41:Version=6,����ַ�=134(67������)
											 	// 45:Version=7,����ַ�=154(77������)
											 	// 49:Version=8,����ַ�=192(96������)
																				
extern int m_nSymbleSize;	//�����εı߳�

extern unsigned char  m_byModuleData[MAX_MODULESIZE][MAX_MODULESIZE];	//���������(125)���������
//extern char **m_byModuleData;

typedef struct 
{
	unsigned short  int	 ncRSBlock;			//����Ŀ���
	unsigned short  int	 ncAllCodeWord;		//��������
	unsigned short  int	 ncDataCodeWord;	//ָ������ȼ��µ����������
}RS_BLOCKINFO;

typedef struct 
{
	unsigned short  int	 nVersionNo;			//�汾 1~40
	unsigned short  int	 ncAllCodeWord; 		//��������=�������ָ���+�������ָ���

	unsigned short  int	 ncDataCodeWord[4];		//4�־���ȼ��µ��������ָ���(0=L,1=M,2=Q,3=H)
	unsigned short  int	 ncAlignPoint;			//У��ͼ�θ���
	unsigned short  int	 nAlignPoint[6];		//У��ͼ����������
	
	RS_BLOCKINFO RS_BlockInfo1[4];	//�����1
	RS_BLOCKINFO RS_BlockInfo2[4];	//�����2
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
