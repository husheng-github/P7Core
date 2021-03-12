#include "includes.h"
//#include "devglobal.h"
#include "../dev_qrcode/qrencode.h"
#include "../dev_qrcode/QR_Encode.h"


//======================================================================================================
//======================================================================================================
//======================================================================================================
//#define NULL    (0x00)
#define SOH        (0x01)
#define STX        (0x02)
#define ETX        (0x03)
#define EOT        (0x04)
#define ENQ        (0x05)
#define ACK        (0x06)
#define BEL        (0x07)
#define BS        (0x08)
#define HT        (0x09)
#define LF        (0x0a)
#define VT        (0x0b)
#define FF        (0x0c)
#define CR        (0x0d)
#define SO        (0x0e)
#define SI        (0x0f)
#define DLE        (0x10)
#define DC1        (0x11)
#define DC2        (0x12)
#define DC3        (0x13)
#define DC4        (0x14)
#define NAK        (0x15)
#define SYN        (0x16)
#define ETB        (0x17)
#define CAN        (0x18)
#define EM        (0x19)
#define SUB        (0x1a)
#define ESC        (0x1b)
#define FS        (0x1c)
#define GS        (0x1d)
#define RS        (0x1e)
#define US        (0x1f)
#define SP        (0x20)
//=======================================================================================================
#define RESETFLAG_JMPFROMMAIN   0xa55a5aa5bbcca55a

 ESC_P_STS_T  esc_sts;
// uint32_t  bitmap_addr = 0x200000-0x14000;

#define MAX (10*1024)      //4096 modify by xiaohonghui 2020.12.28

typedef struct{
    uint8_t *queue;  //uint8_t queue[MAX] ;
    unsigned int head ;
    unsigned int tail ; 
}FIFO;

static FIFO Queue;
FIFO * qQueue = NULL;
static s32 g_queue_init_flag = 0;

//-------初始化 
FIFO * Queueinit(void){
    if(0 == g_queue_init_flag)
    {
        Queue.queue = (u8 *)k_malloc(MAX);
        if(Queue.queue)
        {
            g_queue_init_flag = 1;
        }
        else
        {
            TRACE_ERR("malloc queue fail");
        }
    }

    if(g_queue_init_flag)
    {
        memset(Queue.queue, 0, MAX);
        Queue.head = 0;
        Queue.tail = 0;
        qQueue = &Queue;
    }
    else
    {
        qQueue = NULL;
    }
    
//    qQueue = (FIFO *)malloc(sizeof(FIFO)) ;
    //memset(qQueue , 0x00 , sizeof(FIFO)) ;
    
    return qQueue ;
}

//-------满 
int isFull(FIFO * qQueue){
    int ret = 1 ;
    unsigned int hd = qQueue->head ;
    unsigned int tl = qQueue->tail ;
    if((tl + 1) == MAX){
        tl = 0 ;
        if(tl == hd){
            goto OUT ;
        }else{
            ret = 0 ;
        }
    }else if((tl + 1) == hd){
        goto OUT ;
    }else{
        ret = 0 ;
    }
    
OUT :    
    return ret ;
}

//-------空 
int isEmpty(FIFO * qQueue){
    int ret = 1 ;
    unsigned int hd = qQueue->head ;
    unsigned int tl = qQueue->tail ;
    if(hd == tl){
        goto OUT ;
    }else{
        ret = 0 ;
    }
    
OUT :    
    return ret ;
}

int esc_fifo_isEmpty(void)
{
    return isEmpty(qQueue);
}

//-------添加数据 
int insertData(FIFO * qQueue , uint8_t data){
    int ret = 0 ;
    if(isFull(qQueue)){
        ret = -1 ;
        goto OUT ;    
    }
    
    if(qQueue->tail == MAX - 1){
        qQueue->queue[qQueue->tail] = data ;
        qQueue->tail = 0 ;
    }else{
        qQueue->queue[qQueue->tail] = data ;
        qQueue->tail++ ;
    }
OUT :
    return ret ;
}

int esc_set_data(uint8_t *buf, int num)
{
    int i = 0;
    int ret = 0;
    
    for(i = 0; i < num; i++)
    {
        ret =insertData(qQueue,buf[i]);
        if(ret != 0)
        {
            break;
        }
    }
    
    return i;   //返回成功插入了几个data
}

//-------删除数据 
int deleteData(FIFO * qQueue){
    int ret = 0 ;
    if(isEmpty(qQueue)){
        ret = -1 ;
        goto OUT ;
    }
    
    if(qQueue->head == MAX - 1){
        qQueue->queue[qQueue->head] = 0 ;
        qQueue->head = 0 ;
    }else{
        qQueue->queue[qQueue->head] = 0 ;
        qQueue->head++ ;
    }
OUT :    
    return ret ;
}
//-------提取数据 
int getData(FIFO * qQueue, uint8_t *data){
    int ret = 0 ;
    if(isEmpty(qQueue)){
        ret = -1 ;
        goto OUT ;
    }
    
    if(qQueue->head == MAX - 1){
        *data = qQueue->queue[qQueue->head] ;
        qQueue->queue[qQueue->head] = 0 ;
        qQueue->head = 0 ;
    }else{
        *data = qQueue->queue[qQueue->head] ;
        qQueue->queue[qQueue->head] = 0 ;
        qQueue->head++ ;
    }
OUT :
    return ret ;
}
//-------检查队列头 
int checkHead(FIFO * qQueue){
    int ret = 0 ;
    if(isEmpty(qQueue)){
        ret = -1 ;
        goto OUT ;
    }
    ret = qQueue->queue[qQueue->head] ;
OUT :
    return ret ;
}
//-----打印队列 
void prQueue(FIFO * qQueue){
    int i ;
    for(i = 0 ; i < MAX ; i ++){
        printf("%d " , qQueue->queue[i]) ;
    }
    printf("\n") ;
}

extern uint8_t Getchar(void)
{
    uint8_t data;
    int ret;
    
    ret =getData(qQueue, &data);

    return data;
    
}

extern uint8_t Putchar(uint8_t c)
{
    return 0;
}

//==================================================================================


static u8 g_line_buf[64];
static u8 g_line_buf_2[64];
extern ESC_P_STS_T  esc_sts;

char bitmap_path[5][20] = {"/mtd0/nvimage0.bin", "/mtd0/nvimage1.bin", "/mtd0/nvimage2.bin", "/mtd0/nvimage3.bin", "/mtd0/nvimage4.bin"};
/**
* @brief  获取bitmap 模式 
* @param  file_path bitmap路径
* @retval 0 文件不存在， 1 bitmap原图， 2 bitmap nv位图
*/
u8 get_bitmap_mode(const char *file_path)
{
    u8 buf[4];
    short n;

    memset(buf, 0, sizeof(buf));
    n = fs_read_file(file_path, (u8*)buf, 2, 0);
    if(n != 2)
    {
        return 0;
    }
    dev_debug_printf("%s: %02X %02X\r\n",__func__, buf[0], buf[1]);
    if(strcmp(buf,"BM") == 0)
    {
        return 1;
    }
    else
    {
        return 2;
    }
}

static void bitmap_line_set_pixel(u8 color, s32 x)
{

    if(color)
    {
        g_line_buf[x>>3] |= (1<<((x&0x07)));
    }
    else
    {
        g_line_buf[x>>3] &= (~(1<<((x&0x07))));
    }
    
}

/**
*Description:    bitmap图片原图打印，仅支持位深为1的黑白图片打印
*   const char *BmpFileName   文件路径
*   uint8_t msize             放大倍数，目前只支持一倍放大 
*                             1，原图打印
*                             2, 原图放大两倍
*   uint8_t align             排版格式
*                             0，左对齐
*                             1，居中对齐
*                             2，右对齐
**/

#ifdef MACHINE_P7

/**
* @brief  先全部取出bitmap数据
* @param  srcbuf-bitmap数据
* @param  rbuf-读取数据
* @param  rlen-读取长度
* @param  startaddr-读取地址

* @retval len长度
*/
s32 print_read_bitmap_data(u8 *srcbuf,u8 *rbuf, s32 rlen, s32 startaddr)
{
    s32 i = 0;
    for(i = 0; i < rlen; i++)
    {
        rbuf[i] = srcbuf[startaddr+i];
    }
    return rlen;
}

bool print_original_bitmap(const char *BmpFileName, uint8_t mode, uint8_t align)
{
    BITMAPFILEHEADER    bf;
    BITMAPINFOHEADER    bi;
    unsigned long       LineBytes;
    unsigned long       ImgSize;
    unsigned long       k,m;
    unsigned long       sd256_bitmap_len;
    short               n;
    u8 buf[40];
    u8 *sd256_bitmap = NULL;    //包含前8个字节
    u32 c;
    u32  palatte=0;
    s32 i,j, z;
    u32 xlen, ylen;
    uint16_t offset;
    u8 r,g,b;
    uint8_t msize = 1;
    u8 *mbitmap = NULL;//bitmap数据
    s32 bitmap_len = 0;
//    char *BmpFileName;

    //优化从flash中全部读取bitmap
    bitmap_len = fs_access_file(BmpFileName);
    if(bitmap_len <= 0)
    {
        dev_debug_printf("bitmap_len %d\r\n",bitmap_len);
        return -1;
    }
    mbitmap = (u8*)k_malloc(bitmap_len);
    if(mbitmap == NULL)
    {
        return -1;
    }

    n = fs_read_file(BmpFileName, mbitmap, bitmap_len, 0);
    if(n != bitmap_len)
    {
        dev_debug_printf("bitmap_len %d\r\n",bitmap_len);
        k_free(mbitmap);
        return -6;

    }
    
    if(mode == 0 || mode == 2)
    {
        msize = 1;
    } 
    else if(mode == 1 || mode == 3)
    {
        msize = 2;
    }
    
    n = print_read_bitmap_data(mbitmap, (u8*)buf, 14, 0);
    if(n != 14)
    {
        k_free(mbitmap);
        return -6;
    }
//    dev_debug_printformat("", buf, 14);
    
    dev_debug_printformat(NULL, (u8*)buf, 14);
    bf.bfSize      = buf[2]+(((u32)buf[3])<<8)+(((u32)buf[4])<<16)+(((u32)buf[5])<<24);    
    bf.bfOffBits   = (((u32)buf[10]))+(((u32)buf[11])<<8)+(((u32)buf[12])<<16)+(((u32)buf[13])<<24);
    memset (buf, 0, sizeof (buf));

    n = print_read_bitmap_data(mbitmap, (u8*)buf, 40, 14);
    
    if(n>0)dev_debug_printformat(NULL, (u8*)buf, n);
    
    bi.biSize           = (((u32)buf[0]))+(((u32)buf[1])<<8)+(((u32)buf[2])<<16)+(((u32)buf[3])<<24);
    bi.biWidth          = (((u32)buf[4]))+(((u32)buf[5])<<8)+(((u32)buf[6])<<16)+(((u32)buf[7])<<24);
    bi.biHeight         = (((u32)buf[8]))+(((u32)buf[9])<<8)+(((u32)buf[10])<<16)+(((u32)buf[11])<<24);
    bi.biPlanes         = (((u32)buf[12]))+(((u32)buf[13])<<8);
    bi.biBitCount       = (((u32)buf[14]))+(((u32)buf[15])<<8);
    bi.biCompression    = (((u32)buf[16]))+(((u32)buf[17])<<8)+(((u32)buf[18])<<16)+(((u32)buf[19])<<24);
    bi.biSizeImage      = (((u32)buf[20]))+(((u32)buf[21])<<8)+(((u32)buf[22])<<16)+(((u32)buf[23])<<24);
    bi.biXPelsPerMeter  = (((u32)buf[24]))+(((u32)buf[25])<<8)+(((u32)buf[26])<<16)+(((u32)buf[27])<<24);
    bi.biYPelsPerMeter  = (((u32)buf[28]))+(((u32)buf[29])<<8)+(((u32)buf[30])<<16)+(((u32)buf[21])<<24);
    bi.biClrUsed        = (((u32)buf[32]))+(((u32)buf[33])<<8)+(((u32)buf[34])<<16)+(((u32)buf[35])<<24);
    bi.biClrImportant   = (((u32)buf[36]))+(((u32)buf[37])<<8)+(((u32)buf[38])<<16)+(((u32)buf[39])<<24);
    LineBytes = (unsigned long) (((bi.biWidth>>5)+1)<<5) * bi.biBitCount;

    ImgSize   = (unsigned long) LineBytes  * bi.biHeight/8;

    if(bi.biBitCount != 1)  //如果不是单色图片
    {
        k_free(mbitmap);
        return -1;
    }

    n = print_read_bitmap_data(mbitmap, (u8*)buf, 8, 54);

   if(n>0)dev_debug_printformat(NULL, (u8*)buf, n);
   
    palatte = (((u32)buf[0]))+(((u32)buf[1])<<8)+(((u32)buf[2])<<16)+(((u32)buf[3])<<24);;
    dev_debug_printf("palatte %d\r\n",palatte);


    sd256_bitmap_len = ImgSize;

//    if(xlen > bi.biWidth)
        xlen = bi.biWidth;

    //如果长度比图片小，
//    if(ylen > bi.biHeight)
        ylen = bi.biHeight;

    k = bi.biWidth;
    if((k%8) != 0)
    {
        k=((k>>3)+1)<<3;
    }
    
    if((k%32) != 0) //注意:单色图片对齐方式(4字节对齐)
    {
        m = (k>>3);
        m %= 4;
        k = ((k>>3) + 4-m)<<3;
    }

    //ddi_vfs_seek (f, bf.bfOffBits, SEEK_SET);
    sd256_bitmap = (u8*)k_malloc(k>>3);

    dev_debug_printf("msize:%d, xlen:%d\r\n", msize, xlen);

    if(msize*xlen > LineDot)   //大于打印机打印点数，退出
    {
         dev_debug_printf("%s   %d\r\n", __func__, __LINE__);
        k_free(mbitmap);
        k_free(sd256_bitmap);
        return FALSE;
    }

    if(align == 0) //左对齐
    {
        offset = 0;
    }
    else if(align == 1)  //居中对齐
    {
        offset = (LineDot - xlen*msize)/2;
    }
    else if(align == 2)   //右对齐
    {
        offset = LineDot - xlen*msize;
    }

    dev_debug_printf("offset = %d\r\n", offset);
    
//    for(j=0; j<bi.biHeight; j++) //图片取模:横向,左高右低
    for(j= bi.biHeight-1; j >= 0; j--) //图片取模:横向,左高右低
    {
        //n = ddi_vfs_read (sd256_bitmap, k>>3, f);
        n = print_read_bitmap_data(mbitmap, sd256_bitmap, (k>>3), bf.bfOffBits+((k>>3)*j));
        if(j < (bi.biHeight - ylen))
        {
            continue;
        }
        
        memset(g_line_buf, 0, sizeof(g_line_buf));
        for(i=0; i<xlen; i++)
        {           
       //     c = sd256_bitmap[(i>>3)]&(1 << (7-i%8));//320
            c = sd256_bitmap[(i>>3)]&(1 << (i%8));//320
            if(palatte>0)
            {
                if(c)
                {
                    bitmap_line_set_pixel(1, i);
                }
                else
                {
                    bitmap_line_set_pixel(0, i);

                }
            }
            else
            {
                if(c)
                {
                    bitmap_line_set_pixel(0, i);
                }
                else
                {
                    bitmap_line_set_pixel(1, i);
                }
                
            }     
        }
        
/*      if(mode == 0)
        {
            TPPrintLine(g_line_buf);
        }
        else if(mode == 2)
        {
            TPPrintLine(g_line_buf);
            TPPrintLine(g_line_buf);
        }
*/
        memset(g_line_buf_2,0, sizeof(g_line_buf_2));
        if(mode == 0)
        {
            for(r = 0; r < (xlen/8 +1); r++)
            {
                g_line_buf_2[r+ offset/8] = g_line_buf[r];
            }
            TPPrintLine(g_line_buf_2);
        }
        else if(mode == 2)
        {
            for(r = 0; r < (xlen/8 +1); r++)
            {
                g_line_buf_2[r+ offset/8] = g_line_buf[r];
            }
            TPPrintLine(g_line_buf_2);
            TPPrintLine(g_line_buf_2);
        }
        else
        {
            for(r = 0; r < (xlen/8 +1); r++)
            {
                for(g = 0; g < 4; g++)
                {
                    g_line_buf_2[2*r +offset/8] |= ((g_line_buf[r]&0xF0)>>(4+g)&0x1)<<(2*g);
                    g_line_buf_2[2*r + offset/8] |= ((g_line_buf[r]&0xF0)>>(4+g)&0x1)<<(2*g +1);
                }

                for(g = 0; g < 4; g++)
                {
                    g_line_buf_2[2*r+1 +offset/8] |= (((g_line_buf[r]&0x0F)>>g)&0x1)<<(2*g);
                    g_line_buf_2[2*r+1 + offset/8] |= (((g_line_buf[r]&0x0F)>>g)&0x1)<<(2*g +1);
                }
            }
            if(mode == 1)
            {
                TPPrintLine(g_line_buf_2);
            }
            else if(mode == 3)
            {
                TPPrintLine(g_line_buf_2);
                TPPrintLine(g_line_buf_2);
            }
/*
            for(z = 0; z < 2; z++)
            {
                TPPrintLine(g_line_buf_2);
            }
*/            
   //         dev_debug_printformat("1:", g_line_buf, 48);
   //         dev_debug_printformat("2:", g_line_buf_2, 48);
        }
    }

    k_free(mbitmap);
    k_free(sd256_bitmap);
 //   ddi_vfs_close(f);

    return 0;
}

#else
bool print_original_bitmap(const char *BmpFileName, uint8_t mode, uint8_t align)
{
    BITMAPFILEHEADER    bf;
    BITMAPINFOHEADER    bi;
    unsigned long       LineBytes;
    unsigned long       ImgSize;
    unsigned long       k,m;
    unsigned long       sd256_bitmap_len;
    short               n;
    u8 buf[40];
    u8 *sd256_bitmap = NULL;    //包含前8个字节
    u32 c;
    u32  palatte=0;
    s32 i,j, z;
    u32 xlen, ylen;
    uint16_t offset;
    u8 r,g,b;
    uint8_t msize = 1;
//    char *BmpFileName;

    if(mode == 0 || mode == 2)
    {
        msize = 1;
    } 
    else if(mode == 1 || mode == 3)
    {
        msize = 2;
    }
    
    n = fs_read_file(BmpFileName, (u8*)buf, 14, 0);
    if(n != 14)
    {
        return -6;
    }
//    dev_debug_printformat("", buf, 14);
    
    dev_debug_printformat(NULL, (u8*)buf, 14);
    bf.bfSize      = buf[2]+(((u32)buf[3])<<8)+(((u32)buf[4])<<16)+(((u32)buf[5])<<24);    
    bf.bfOffBits   = (((u32)buf[10]))+(((u32)buf[11])<<8)+(((u32)buf[12])<<16)+(((u32)buf[13])<<24);
    memset (buf, 0, sizeof (buf));

    n = fs_read_file(BmpFileName, (u8*)buf, 40, 14);
    if(n>0)dev_debug_printformat(NULL, (u8*)buf, n);
    
    bi.biSize           = (((u32)buf[0]))+(((u32)buf[1])<<8)+(((u32)buf[2])<<16)+(((u32)buf[3])<<24);
    bi.biWidth          = (((u32)buf[4]))+(((u32)buf[5])<<8)+(((u32)buf[6])<<16)+(((u32)buf[7])<<24);
    bi.biHeight         = (((u32)buf[8]))+(((u32)buf[9])<<8)+(((u32)buf[10])<<16)+(((u32)buf[11])<<24);
    bi.biPlanes         = (((u32)buf[12]))+(((u32)buf[13])<<8);
    bi.biBitCount       = (((u32)buf[14]))+(((u32)buf[15])<<8);
    bi.biCompression    = (((u32)buf[16]))+(((u32)buf[17])<<8)+(((u32)buf[18])<<16)+(((u32)buf[19])<<24);
    bi.biSizeImage      = (((u32)buf[20]))+(((u32)buf[21])<<8)+(((u32)buf[22])<<16)+(((u32)buf[23])<<24);
    bi.biXPelsPerMeter  = (((u32)buf[24]))+(((u32)buf[25])<<8)+(((u32)buf[26])<<16)+(((u32)buf[27])<<24);
    bi.biYPelsPerMeter  = (((u32)buf[28]))+(((u32)buf[29])<<8)+(((u32)buf[30])<<16)+(((u32)buf[21])<<24);
    bi.biClrUsed        = (((u32)buf[32]))+(((u32)buf[33])<<8)+(((u32)buf[34])<<16)+(((u32)buf[35])<<24);
    bi.biClrImportant   = (((u32)buf[36]))+(((u32)buf[37])<<8)+(((u32)buf[38])<<16)+(((u32)buf[39])<<24);
    LineBytes = (unsigned long) (((bi.biWidth>>5)+1)<<5) * bi.biBitCount;

    ImgSize   = (unsigned long) LineBytes  * bi.biHeight/8;

    if(bi.biBitCount != 1)  //如果不是单色图片
        return -1;
    
    n = fs_read_file(BmpFileName, (u8*)buf, 8, 54);
   if(n>0)dev_debug_printformat(NULL, (u8*)buf, n);
   
    palatte = (((u32)buf[0]))+(((u32)buf[1])<<8)+(((u32)buf[2])<<16)+(((u32)buf[3])<<24);;
    dev_debug_printf("palatte %d\r\n",palatte);


    sd256_bitmap_len = ImgSize;

//    if(xlen > bi.biWidth)
        xlen = bi.biWidth;

    //如果长度比图片小，
//    if(ylen > bi.biHeight)
        ylen = bi.biHeight;

    k = bi.biWidth;
    if((k%8) != 0)
    {
        k=((k>>3)+1)<<3;
    }
    
    if((k%32) != 0) //注意:单色图片对齐方式(4字节对齐)
    {
        m = (k>>3);
        m %= 4;
        k = ((k>>3) + 4-m)<<3;
    }

    //ddi_vfs_seek (f, bf.bfOffBits, SEEK_SET);
    sd256_bitmap = (u8*)k_malloc(k>>3);

    dev_debug_printf("msize:%d, xlen:%d\r\n", msize, xlen);

    if(msize*xlen > LineDot)   //大于打印机打印点数，退出
    {
         dev_debug_printf("%s   %d\r\n", __func__, __LINE__);
        return FALSE;
    }

    if(align == 0) //左对齐
    {
        offset = 0;
    }
    else if(align == 1)  //居中对齐
    {
        offset = (LineDot - xlen*msize)/2;
    }
    else if(align == 2)   //右对齐
    {
        offset = LineDot - xlen*msize;
    }

    dev_debug_printf("offset = %d\r\n", offset);
    
//    for(j=0; j<bi.biHeight; j++) //图片取模:横向,左高右低
    for(j= bi.biHeight-1; j >= 0; j--) //图片取模:横向,左高右低
    {
        //n = ddi_vfs_read (sd256_bitmap, k>>3, f);
        n = fs_read_file(BmpFileName, sd256_bitmap, (k>>3), bf.bfOffBits+((k>>3)*j));
        
        if(j < (bi.biHeight - ylen))
        {
            continue;
        }
        
        memset(g_line_buf, 0, sizeof(g_line_buf));
        for(i=0; i<xlen; i++)
        {           
       //     c = sd256_bitmap[(i>>3)]&(1 << (7-i%8));//320
            c = sd256_bitmap[(i>>3)]&(1 << (i%8));//320
            if(palatte>0)
            {
                if(c)
                {
                    bitmap_line_set_pixel(1, i);
                }
                else
                {
                    bitmap_line_set_pixel(0, i);

                }
            }
            else
            {
                if(c)
                {
                    bitmap_line_set_pixel(0, i);
                }
                else
                {
                    bitmap_line_set_pixel(1, i);
                }
                
            }     
        }
        
/*      if(mode == 0)
        {
            TPPrintLine(g_line_buf);
        }
        else if(mode == 2)
        {
            TPPrintLine(g_line_buf);
            TPPrintLine(g_line_buf);
        }
*/
        memset(g_line_buf_2,0, sizeof(g_line_buf_2));
        if(mode == 0)
        {
            for(r = 0; r < (xlen/8 +1); r++)
            {
                g_line_buf_2[r+ offset/8] = g_line_buf[r];
            }
            TPPrintLine(g_line_buf_2);
        }
        else if(mode == 2)
        {
            for(r = 0; r < (xlen/8 +1); r++)
            {
                g_line_buf_2[r+ offset/8] = g_line_buf[r];
            }
            TPPrintLine(g_line_buf_2);
            TPPrintLine(g_line_buf_2);
        }
        else
        {
            for(r = 0; r < (xlen/8 +1); r++)
            {
                for(g = 0; g < 4; g++)
                {
                    g_line_buf_2[2*r +offset/8] |= ((g_line_buf[r]&0xF0)>>(4+g)&0x1)<<(2*g);
                    g_line_buf_2[2*r + offset/8] |= ((g_line_buf[r]&0xF0)>>(4+g)&0x1)<<(2*g +1);
                }

                for(g = 0; g < 4; g++)
                {
                    g_line_buf_2[2*r+1 +offset/8] |= (((g_line_buf[r]&0x0F)>>g)&0x1)<<(2*g);
                    g_line_buf_2[2*r+1 + offset/8] |= (((g_line_buf[r]&0x0F)>>g)&0x1)<<(2*g +1);
                }
            }
            if(mode == 1)
            {
                TPPrintLine(g_line_buf_2);
            }
            else if(mode == 3)
            {
                TPPrintLine(g_line_buf_2);
                TPPrintLine(g_line_buf_2);
            }
/*
            for(z = 0; z < 2; z++)
            {
                TPPrintLine(g_line_buf_2);
            }
*/            
   //         dev_debug_printformat("1:", g_line_buf, 48);
   //         dev_debug_printformat("2:", g_line_buf_2, 48);
        }
    }

    k_free(sd256_bitmap);
 //   ddi_vfs_close(f);

    return 0;
}

#endif

//#if defined(SUPPORT_NVBITMAP)
extern void Printf_Bitmap (uint8_t n,uint8_t mode)
   {
      uint16_t height_byte,width,height,p_w_bak,l_s_bak,i,j,act_width,height_byte_offset,start_dot;
      uint32_t  k,addr,offset;

      uint8_t tmpbuf[LineDot];    //开一个毫米高的缓冲区
      uint8_t dotbuf[LineDot];    //开一个毫米高的缓冲区
      struct
      {
         uint8_t  xL,xH,yL,yH;
      }                    BitmapWH;
      //当多条命令一起发送时，这个判断会出错
      /*if(IsPrintBufEmpty() == 0)
      {
            return;// break;   //在标准模式下，只有打印缓冲区为空时才处理此命令
      }*/
 //     dev_debug_printf("--- 1 \r\n");

      if(esc_sts.bitmap_flag)
      {
              return;
      }
//      dev_debug_printf("--- %d \r\n", __LINE__);
      
      memset(tmpbuf,0,sizeof(tmpbuf));
      if(mode >= 0x30)
      {
         mode -= 0x30;
      }
      addr = 0;
      while(n--)
      {
         //F25L_Read_nv("/mtd2/nvimage.bin",addr,(uint8_t *)&BitmapWH,4);
         F25L_Read_nv(bitmap_path[n-1],addr,(uint8_t *)&BitmapWH,4);
         dev_debug_printf("%02X %02X %02X %02X \r\n",BitmapWH.xH,BitmapWH.xL,BitmapWH.yH,BitmapWH.yL);
    /*     BitmapWH.xH = 0x0;
         BitmapWH.xL = 014;
         BitmapWH.yH = 0x0;
         BitmapWH.yL = 0x0A;
      */   
//         dev_debug_printf("--- %d \r\n", __LINE__);
         addr += 4;
         if ((BitmapWH.xH > 0x0f)||(BitmapWH.yH > 0x0f))
         {
             break;
         }

  //       dev_debug_printf("--- %d \r\n", __LINE__);


         width = ((BitmapWH.xH<<8) + BitmapWH.xL)*8;
         height = ((BitmapWH.yH<<8) + BitmapWH.yL) * 8;
         height_byte = (height>>3);
         k = width * height / 8 ;
         if(k>0xA000)    //计算得到的图片大小超过40k，则为异常。
             break;
         if(n == 0)
         {
             p_w_bak = esc_sts.print_width;
             l_s_bak = esc_sts.leftspace;
             if((esc_sts.leftspace + width) > esc_sts.print_width)//首先需要对左边距和当前打印宽度做处理
             {
                 if(esc_sts.leftspace + width < LineDot)
                 {
                         esc_sts.print_width = esc_sts.leftspace + width;
                 }
                 else if(LineDot > width)
                 {
                         esc_sts.leftspace = LineDot - width;
                         esc_sts.print_width = LineDot;
                 }
                 else
                 {
                         esc_sts.leftspace = 0;
                         esc_sts.print_width = LineDot;
                         width=LineDot;//位图图片超过打印宽度，只打印前384点
                 }
             }
        //    printf("%d  ",esc_sts.leftspace);
            start_dot = esc_sts.start_dot;
             for(height_byte_offset=0;height_byte_offset<height_byte;height_byte_offset++)
             {
                 offset = addr + height_byte_offset;
                 memset(dotbuf,0,sizeof(dotbuf));
                 for(i=0;i<width;i++)
                 {
                     F25L_Read_nv("/mtd2/nvimage.bin",offset,&tmpbuf[i],1);
                     offset += (height+7)/8;
                 //    dev_debug_printf("%02X ", tmpbuf[i]);
                 }
                 switch(mode)
                 {
                     case 0:
                         act_width = (width>LineDot)?LineDot:width;
                         memcpy(dotbuf,tmpbuf,width);
                         esc_sts.start_dot = start_dot;
                         PictureDotFillToBuf(dotbuf,act_width,8);
                         break;
                     case 1:
                         act_width = ((width*2)>LineDot)?(LineDot):(width*2);
                         for(j=0;j<act_width/2;j++)
                         {
                             dotbuf[2*j] = tmpbuf[j];
                             dotbuf[2*j+1] = tmpbuf[j];
                         }
                         esc_sts.start_dot = start_dot;
                         PictureDotFillToBuf(dotbuf,act_width,8);
                         break;
                     case 2:
                         act_width = (width>LineDot)?LineDot:width;
                         //高半字节放大成一个字节
                         for(j=0;j<act_width;j++)
                         {
                             for(i=4;i<8;i++)
                             {
                                 if(tmpbuf[j] &(1<<i) )
                                 {
                                     dotbuf[j] |= (3<< ((i-4)*2));
                                 }
                             }
                         }
                         esc_sts.start_dot = start_dot;
                         PictureDotFillToBuf(dotbuf,act_width,8);
                         //低半字节放大成一个字节
                         memset(dotbuf,0,sizeof(dotbuf));
                         for(j=0;j<act_width;j++)
                         {
                             for(i=0;i<4;i++)
                             {
                                 if(tmpbuf[j] &(1<<i) )
                                 {
                                     dotbuf[j] |= (3<<i*2);
                                 }
                             }

                         }
                         esc_sts.start_dot = start_dot;
                         PictureDotFillToBuf(dotbuf,act_width,8);

                         break;
                      case 3:
                         act_width = ((width*2)>LineDot)?(LineDot):(width*2);
                           //高半字节放大成一个字节
                         for(j=0;j<act_width/2;j++)
                         {
                             for(i=4;i<8;i++)
                             {
                                 if(tmpbuf[j] &(1<<i) )
                                 {
                                     dotbuf[2*j] |= (3<< ((i-4)*2));
                                     dotbuf[2*j+1] |= (3<< ((i-4)*2));
                                 }
                             }
                         }
                         esc_sts.start_dot = start_dot;
                         PictureDotFillToBuf(dotbuf,act_width,8);
                         //低半字节放大成一个字节
                         memset(dotbuf,0,sizeof(dotbuf));
                         for(j=0;j<act_width/2;j++)
                         {
                             for(i=0;i<4;i++)
                             {
                                 if(tmpbuf[j] &(1<<i) )
                                 {
                                     dotbuf[2*j] |= (3<<i*2);
                                     dotbuf[2*j+1] |= (3<<i*2);
                                 }
                             }

                         }
                         esc_sts.start_dot = start_dot;
                         PictureDotFillToBuf(dotbuf,act_width,8);

                         break;
                 }
             }
             esc_sts.print_width = p_w_bak;
             esc_sts.leftspace = l_s_bak;
         }
         addr += k;
      }
    //  printf("位图打印完成\n");
      return;
      }
//#endif
static void RasterBitmapPos(uint8_t *dst, uint8_t *src, uint16_t x, uint16_t buf_len, uint8_t wr)
{
    uint8_t i;//, temp1, temp2;
    int8_t j;
    uint16_t k, bit, start_col, right_space;

    if(x > LineDot/8)x = LineDot/8;

    if(esc_sts.start_dot > esc_sts.leftspace)
        start_col = esc_sts.start_dot;
    else
        start_col = esc_sts.leftspace;

    right_space = esc_sts.print_width + esc_sts.leftspace;
    if(right_space > LineDot)
        right_space = LineDot;
    memset(dst, 0, buf_len);

    if((start_col == 0) && (wr == 1) && (esc_sts.align == 0))
    {
        memcpy(dst, src, x);
        goto OUT;
    }

    switch (esc_sts.align)
    {
    case 0:        // left align
    default:

        break;
    case 1:        // middle align
        {
            if(esc_sts.print_width > x * 8 * wr)
                start_col = esc_sts.leftspace + ((esc_sts.print_width - x * 8 * wr) >> 1);
        }
        break;
    case 2:        // right align
        {
            if(esc_sts.print_width > x * 8 * wr)
                start_col = (right_space - x * 8 * wr);
        }
        break;
    }

    bit = start_col;       //起始纵向位置+纵行数*宽放大倍数
    for (k=0; k<x; k++)    //实现横向放大
    {
        for(j = 7; j >=0; j--)
        {
            if(src[k]& (1 << j))
            {
                for(i = 0; i < wr; i++)
                {
                    dst[bit >> 3] |= 1<<(7-(bit & 0x07));
                    bit++;
                    if(bit >= (right_space))
                        goto OUT;
                }
            }
            else
            {
                bit += wr;
                if(bit >= (right_space))
                    goto OUT;
            }
        }
    }
OUT:
    return;
}
//=======================================================================================================
static void RasterPrint(uint8_t cmd, uint16_t x, uint16_t y)
{
    uint16_t i, k;
    uint8_t  dot[LineDot/8];
    uint8_t  dot_print[LineDot/8];
    uint8_t  bit_2x[]={0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f, 0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff};

    switch(cmd)     // m
    {
    case 0:     // 180x180
    case 48:
        for(i=0; i<y; i++)
        {
            for(k=0; k<x; k++)
            {
                cmd = Getchar();
                if(k < ARRAY_SIZE(dot))
                {
                    dot[k] = cmd;
                }
            }
            while(k < ARRAY_SIZE(dot))
            {
                dot[k++] = 0;
            }
            RasterBitmapPos(dot_print, dot, x, LineDot/8, 1);
            TPPrintRasterLine(dot_print);
        }
        break;
    case 1:     // 90x180
    case 49:
        for(i=0; i<y; i++)
        {
            for(k=0; k<x; k++)
            {
                cmd = Getchar();
                if(k < ARRAY_SIZE(dot)/2)
                {
                    dot[k<<1] = bit_2x[cmd >> 4];
                    dot[(k<<1) + 0x01] = bit_2x[cmd & 0x0f];
                }
            }
            while(k < ARRAY_SIZE(dot)/2)
            {
                dot[k<<1] = 0;
                dot[(k<<1) + 0x01] = 0;
                k++;
            }
            RasterBitmapPos(dot_print, dot, x*2, LineDot/8, 1);
            TPPrintRasterLine(dot_print);
        }
        break;
    case 2:     // 180x90
    case 50:
        for(i=0; i<y; i++)
        {
            // memset(dot, 0, sizeof(dot));
            for(k=0; k<x; k++)
            {
                cmd = Getchar();
                if(k < ARRAY_SIZE(dot))
                {
                    dot[k] = cmd;
                }
            }
            while(k < ARRAY_SIZE(dot))
            {
                dot[k++] = 0;
            }
            RasterBitmapPos(dot_print, dot, x, LineDot/8, 1);
            TPPrintRasterLine(dot_print);
            TPPrintRasterLine(dot_print);
        }
        break;
    case 3:     // 90x90
    case 51:
        for(i=0; i<y; i++)
        {
            //memset(dot, 0, sizeof(dot));
            for(k=0; k<x; k++)
            {
                cmd = Getchar();
                if(k < ARRAY_SIZE(dot)/2)
                {
                    dot[k<<1] = bit_2x[cmd >> 4];
                    dot[(k<<1)| 0x01] = bit_2x[cmd & 0x0f];
                }
            }
            while(k < ARRAY_SIZE(dot)/2)
            {
                dot[k<<1] = 0;
                dot[(k<<1) + 0x01] = 0;
                k++;
            }
            RasterBitmapPos(dot_print, dot, 2*x, LineDot/8, 1);
            TPPrintRasterLine(dot_print);
            TPPrintRasterLine(dot_print);
        }
        break;
    }
    //TPStart();
}

//==================================================================================

extern void esc_p_init(void)
{
    uint8_t i;
//----chang
    //dev_debug_printf("--- %s ---- \r\n",__func__);

 //   pt48d_dev_init();
//    PrtSetSpeed(3);

    esc_sts.international_character_set = 0;    // english
    esc_sts.character_code_page = para.character_code_page;

    esc_sts.prt_on = 0;
    esc_sts.larger = 0;
    #ifdef ASCII9X24
    esc_sts.font_en = FONT_B_WIDTH;    // 字体
    #else
    esc_sts.font_en = FONT_A_WIDTH;    // 字体
    #endif
    esc_sts.font_cn = FONT_CN_A_WIDTH;    // 字体
    esc_sts.bold = 0;        // 粗体
    esc_sts.double_strike=0;//重叠打印
    esc_sts.underline = 0;    // 下划线
    esc_sts.revert = 0;        // 反白显示
    esc_sts.rotate = 0;
    esc_sts.start_dot = 0;
    esc_sts.smoothing_mode = 0;    // 平滑模式
    esc_sts.dot_minrow = ARRAY_SIZE(esc_sts.dot[0]);
    memset(esc_sts.dot, 0 ,sizeof(esc_sts.dot));
    for(i=0; i<8; i++)
    {
        esc_sts.tab[i] = 9+8*i;
    }
    esc_sts.linespace = 30;
    esc_sts.charspace = 0;
    esc_sts.align = 0;
    esc_sts.leftspace = 0;
    esc_sts.print_width=LineDot;
    esc_sts.upside_down=0;//倒置


/*#if defined(SUPPORT_BITMAP)
    esc_sts.bitmap.defined = 0;
#endif*/
    esc_sts.barcode_height = 50;
    esc_sts.barcode_width = 2;
    esc_sts.barcode_leftspace = 0;
    esc_sts.barcode_char_pos = 0;//不显示
    esc_sts.barcode_font = FONT_A_HEIGHT;
    esc_sts.userdefine_char = 0;
    esc_sts.asb_mode=0;
#if defined(SUPPORT_USER_CHAR_AB)
    for(i=0; i<ARRAY_SIZE(esc_sts.user_char_a); i++)
    {
        esc_sts.user_char_a[i].defined = 0;
        esc_sts.user_char_b[i].defined = 0;
    }
#endif

    esc_sts.chinese_mode = 1;
#if defined(SUPPORT_CHINESE_USER_CHAR)
    for(i=0; i<ARRAY_SIZE(esc_sts.chinese_user_char); i++)
    {
        esc_sts.chinese_user_char[i].defined = 0;
    }
#endif
    esc_sts.bitmap_flag = 0;

  if(esc_sts.status4 == 0)
  {
      esc_sts.status4=0x12;
  }

  

}
extern void esc_init(void)
{
    pt48d_dev_init();
    esc_p_init();
#if defined(SUPPORT_BITMAP)
    esc_sts.bitmap.defined = 0;
#endif
    Queueinit();

}
extern void TPSelfTest_3(void);
extern void TPSelfTest_4(void);
extern void TPSelfTest_2(void);

#if defined(SUPPORT_QRCode)

#define ALIGN_LEFT      (0X00)
#define ALIGN_MID       (0X01)
#define ALIGN_RIG       (0X02)

//#define doubleqrline
extern void FillBufferDot(uint8_t *p, uint32_t idx, int len, int val)
{
    while (len--)
    {
        if (val)
            p[idx/8] |=  (1<<(7-idx%8));
        else
            p[idx/8] &= ~(1<<(7-idx%8));

        idx ++;
    }
}
char qrstring[41+1];     //QRVERSION = 9 支持最多的字节数552
QRecLevel qrlevel =QR_ECLEVEL_L;
#if defined(doubleqrline)
uint8_t msize = 5;
#else
uint8_t msize = 3;
#endif

static void UpsideDownSetting(uint8_t *dst)
{
    uint8_t temp1, temp2;
    uint16_t k;

    for(k = 0; k < LineDot/2; k++)
    {
        if(dst[k >> 3] & (1 << (7 - (k % 8))))
            temp1 = 1;
        else
            temp1 = 0;

        if(dst[LineDot/8 - 1 - (k >> 3)] & (1 << (k % 8)))
            temp2 = 1;
        else
            temp2 = 0;

        if(temp1)
        {
            dst[LineDot/8 - 1 - (k >> 3)] |= 1 << (k % 8);
        }
        else
        {
            dst[LineDot/8 - 1 - (k >> 3)] &= ~(1 << (k % 8));
        }

        if(temp2)
        {
            dst[k >> 3] |= 1 << (7 - (k % 8));
        }
        else
        {
            dst[k >> 3] &= ~(1 << (7 - (k % 8)));
        }

    }
}

static void QRCodeHandle(uint16_t length,uint8_t cn,uint8_t fn)
{
    #if defined(doubleqrline)
    uint16_t  k;
    #endif
    int16_t  i=0;
    uint8_t model;
    uint16_t  data_num=0, right_margin;
    QRcode *qr;
    static uint16_t len = 0;
    switch (fn)
    {
    case 65:
        if (4 != length) return;
        model=Getchar();
        model=Getchar();
        model++;
        break;
    case 67:
        if (3 != length) return;
        msize = Getchar();
        if ((1>msize) || (8<msize))
            msize = 3;
        break;
    case 69:
        if (3 != length) return;
        switch (Getchar())
        {
        default:
        case '0':
            qrlevel = QR_ECLEVEL_L;
            break;
        case '1':
            qrlevel = QR_ECLEVEL_M;
            break;
        case '2':
            qrlevel = QR_ECLEVEL_Q;
            break;
        case '3':
            qrlevel = QR_ECLEVEL_H;
            break;
        }
        break;
    case 80:
        if (3 >= length) return;
        if (48 != Getchar()) return;
        length -= 3;
        i = 0;
        while (length--)
        {
            if (i>=sizeof(qrstring)) Getchar();
            else
            {
                qrstring[i] = Getchar();
                len = i;
            }
            i++;
        }
        len++;
        //qrstring[i] = '\0';
//        printf("data OK\n");
        //Putchar('A');
        break;
    case 81:
    {
        //uint16_t m;
        if (48 != Getchar()) return;
        if(esc_sts.dot_minrow != ARRAY_SIZE(esc_sts.dot[0]))return;
        //for(m = 0; m < len; m++)
        //    Putchar(qrstring[m]);
        qr = QR_Encode(qrstring, len, qrlevel);//strlen(qrstring)
        if (NULL == qr) return;
        if(esc_sts.leftspace + esc_sts.print_width > LineDot)
        {
            right_margin = LineDot;
        }
        else
        {
            right_margin = esc_sts.leftspace + esc_sts.print_width;
        }
        if(esc_sts.upside_down)
        {
            i = qr->row - 1;
        }
        else
        {
            i = 0;
        }
        if(!qr->row)return;
        for (;;)//i=0; i<qr->row; i++)
        {
            uint16_t j;
            static uint8_t  line[LineDot/8];
            uint16_t prn_width;

            prn_width = esc_sts.print_width;//目前暂无设定打印区域宽度
            memset(line, 0x00, LineDot/8);

            for (j=0; j<qr->col; j++)
            {
                if (qr->data[i*qr->col + j]&0x01)
                {
                    switch (esc_sts.align)
                    {
                    case ALIGN_LEFT:
                        if ( ((esc_sts.leftspace+qr->col*msize)<prn_width)&&(esc_sts.start_dot==0) )
                        {
                            FillBufferDot(line, j*msize +esc_sts.leftspace, msize, 1);
                            break;
                        }
                        if ( ((esc_sts.start_dot+qr->col*msize)<prn_width)&&(esc_sts.start_dot!=0) )
                        {
                            FillBufferDot(line, j*msize +esc_sts.start_dot, msize, 1);
                            break;
                        }
                        break;
                    case ALIGN_MID:
                        if((prn_width - qr->col*msize) > 0)
                            FillBufferDot(line, esc_sts.leftspace + j*msize +(prn_width-qr->col*msize)/2, msize, 1);   //居中打印esc_sts.leftspace
                        break;
                    case ALIGN_RIG:
                        if((prn_width - qr->col*msize) > 0)
                            FillBufferDot(line, j*msize +(right_margin - qr->col*msize), msize, 1);
                        break;
                    default:
                        break;
                    }
                    //FillBufferDot(line, j*msize, msize, 1);
                }
                else
                {
                    switch (esc_sts.align)
                    {
                    case ALIGN_LEFT:
                        if ( ((esc_sts.leftspace+qr->col*msize)<prn_width)&&(esc_sts.start_dot==0) )
                        {
                            FillBufferDot(line, j*msize +esc_sts.leftspace, msize, 0);
                            break;
                        }
                        if ( ((esc_sts.start_dot+qr->col*msize)<prn_width)&&(esc_sts.start_dot!=0) )
                        {
                            FillBufferDot(line, j*msize +esc_sts.start_dot, msize, 0);
                            break;
                        }
                    case ALIGN_MID:
                        if((prn_width - qr->col*msize) > 0)
                            FillBufferDot(line,esc_sts.leftspace + j*msize +(prn_width-qr->col*msize)/2, msize, 0);
                        break;
                    case ALIGN_RIG:
                        if((prn_width - qr->col*msize) > 0)
                            FillBufferDot(line, j*msize +(right_margin -qr->col*msize), msize, 0);
                        break;
                    default:
                        break;
                    }
                    //FillBufferDot(line, j*msize, msize, 0);
                }

            }

            if(esc_sts.upside_down)
            {
                UpsideDownSetting(line);
            }
            for (j=0; j<msize; j++)
            {
                TPPrintLine(line);
            }
            if(esc_sts.upside_down)
            {
                i--;
                if(i < 0)break;
            }
            else
            {
                i++;
                if(i == qr->row)break;
            }
        }
    }
/*
        for (i=0; i<qr->row; i++)
        {
            uint16_t j;
            static uint8_t line[LineDot/8];

            memset(line, 0x00, LineDot/8);
            for (j=0; j<qr->col; j++)
            {
                if (qr->data[i*qr->col + j]&0x01)
                {
                    switch (esc_sts.align)
                    {
                    case ALIGN_LEFT:
                        if ( ((esc_sts.leftspace+qr->col*msize)<LineDot)&&(esc_sts.start_dot==0) )
                        {
                            FillBufferDot(line, j*msize +esc_sts.leftspace, msize, 1);
                            break;
                        }
                        if ( ((esc_sts.start_dot+qr->col*msize)<LineDot)&&(esc_sts.start_dot!=0) )
                        {
                            FillBufferDot(line, j*msize +esc_sts.start_dot, msize, 1);
                            break;
                        }
                        break;
                    case ALIGN_MID:
                        FillBufferDot(line, j*msize +(LineDot-qr->col*msize)/2, msize, 1);   //居中打印esc_sts.leftspace
                        break;
                    case ALIGN_RIG:
                        FillBufferDot(line, j*msize +(LineDot-qr->col*msize), msize, 1);
                        break;
                    default:
                        break;
                    }
                    //FillBufferDot(line, j*msize, msize, 1);
                }
                else
                {
                    switch (esc_sts.align)
                    {
                    case ALIGN_LEFT:
                        if ( ((esc_sts.leftspace+qr->col*msize)<LineDot)&&(esc_sts.start_dot==0) )
                        {
                            FillBufferDot(line, j*msize +esc_sts.leftspace, msize, 0);
                            break;
                        }
                        if ( ((esc_sts.start_dot+qr->col*msize)<LineDot)&&(esc_sts.start_dot!=0) )
                        {
                            FillBufferDot(line, j*msize +esc_sts.start_dot, msize, 0);
                            break;
                        }
                    case ALIGN_MID:
                        FillBufferDot(line, j*msize +(LineDot-qr->col*msize)/2, msize, 0);
                        break;
                    case ALIGN_RIG:
                        FillBufferDot(line, j*msize +(LineDot-qr->col*msize), msize, 0);
                        break;
                    default:
                        break;
                    }
                    //FillBufferDot(line, j*msize, msize, 0);
                }
            }
            #if defined(doubleqrline)
            for (k=0; k<((LineDot/8/2)); k++)
            {
                line[(LineDot/8/2) + k] = line[k];//位带更改后，相应的更改
            }
            #endif
            for (j=0; j<msize; j++)
            {
                TPPrintLine(line);//位带更改后，相应的更改
            }
        }
        */
        break;
    case 82:
        if (48 != Getchar()) return;
        while (1)
        {
            if (i < len)//(qrstring[i]!='\0')
            {
                data_num++;
            }
            else
            {
                break;
            }
            i++;
        }
        Putchar(data_num&0xff);
        Putchar(data_num>>8);
        break;
    }
}
#endif
#define PRINT_QRCODE_LEN_MAX   128
extern void CurrentTestPart(uint8_t part);
extern void Reset_Handler(void);
extern void TPSelfTest_1(void);
extern volatile uint16_t battery_value;
extern bool print_QrcodeData(char *lpsSource, uint8_t RSLevel, uint8_t msize, uint8_t align);

//======================================================================================================
extern void esc_p(void)
{
    uint8_t cmd;
    uint8_t j;
    uint16_t i, k;
//    uint8_t  dot[LineDot/8];
    uint16_t x, y;
//    uint8_t n;

//===============================================================
   

    switch(cmd=Getchar())
    {
       // dev_debug_printf("00 0x%x\r\n", cmd);


            // TODO: TM-T88 (OK)
        case HT:    // 移动打印位置到下一个水平制表位置
        {
            uint8_t i;

            for (i=0; (i<ARRAY_SIZE(esc_sts.tab)) && (esc_sts.tab[i]); i++)
            {
                if (((uint16_t)esc_sts.tab[i] * esc_sts.font_en) >= LineDot)
                {
                    PrintCurrentBuffer(0);
                    break;
                }
                else if (esc_sts.start_dot < (esc_sts.tab[i] * esc_sts.font_en))
                {
                    esc_sts.start_dot = ((uint16_t)esc_sts.tab[i] * esc_sts.font_en);
                    break;
                }
            }
            break;
        }

        // TODO: TM-T88
        case FF:    // 打印并走纸到下页首(Print and return to standard mode (in page mode))
        {
            if (esc_sts.mode == 1)  // page mode
            {
                esc_sts.mode = 0;       // return to standard mode
            }
             PrintCurrentBuffer(0);
            // 走纸到黑标处
            TPFeedToMark(para.max_mark_length*8);   // 250mm
            break;
        }

     case LF:    // line feed
     {
        PrintCurrentBuffer(0);
        break;
     }

     case CR:    // carry return
       {
            break;
        }

    case DC2:
       {
        switch(cmd =Getchar())
        {
        case 0:
            break;
        case '#':
            TPSetSpeed(Getchar());
            break;
        case 'T':
  /*TODO         NVIC_DisableIRQ(UART_IRQn); */
            TPPrintTestPage();
  /*TODO         NVIC_EnableIRQ(UART_IRQn); */
            break;
        case 'U':
           //TPSelfTest();
            break;
        case 'V':
            cmd=Getchar();
            if(para.print_voltage!=cmd)
            {
              para.print_voltage=cmd;
              UpdateParaSector(&para);
            }
            break;
        case 'F':
            #if defined(FONT_DOWNLOAD)
            font_download=1;
            font_download_mode();
            #endif
            break;
        case 'C':
            cmd=Getchar();
            if(para.com_baud!=cmd)
            {
                #if defined(GT30L24M1Z)
                para.com_baud=cmd;
//                UARTInit(config_idx2baud(para.com_baud));//460800
//                NVIC_EnableIRQ(UART_IRQn);
                #else
                para.com_baud=cmd;
                UpdateParaSector(&para);
            //    ResetPrinter();
                #endif
            }
            break;
        case 'D':
         //   config_mode();
            break;
        case 'H':
             break;
       case 'M':
           break;
       case 0x52:
           EraseConfig();
           break;
       case 0x78:
            {
                //cmd = Getchar();
                //if(cmd == 3)
                //    speedtest();
                break;
            }
         }
        break;
       }

    case ESC:        // ESC
        cmd = Getchar();
        switch(cmd)
        {
        case 1:
            TPSelfTest_2();
            break;
//        case 2:
//            TPSelfTest_4();
//            break;
//        case 3:
//            CurrentTestPart(Getchar());
//            break;
//        case 4:
//            printf("%x\n", Reset_Handler);
//            break;
//        case 6:
//            TPSelfTest_3();
//            break;
        case 5:
            TPSelfTest_1();
            break;
        case FF:    // Print data in page mode
        {

            PrintCurrentBuffer(0);
            break;
        }

        case SP:    // 设置字符间距
        {
            esc_sts.charspace = Getchar();
            break;
        }
        case SO:
        {
         //倍宽

            esc_sts.larger = (esc_sts.larger & 0x0f) | 0x10;
            break;
        }
        case DC4://取消倍宽
        {
            esc_sts.larger = (esc_sts.larger & 0x0f) ;
            break;
        }

        case ESC:
        {
            int16_t temp;

            cmd = Getchar();
            if(cmd == 0x40)
            {
                Putchar(battery_value&0xff);
                Putchar(battery_value>>8);
            }
            else if(cmd == 0x50)
            {
                temp = TPHTemperature();
                Putchar(temp&0xff);
                Putchar(temp>>8);
            }
            else if(cmd == 0x60)
            {
                cmd = Getchar();
                if(cmd <= 2)
                {
                    while(TPCheckBusy() || (!IsPrintBufEmpty()));
                    TPSetHeatDots(cmd);
                }
            }
            break;
        }
        case '!':    // 设置字符打印方式
        {
            cmd = Getchar();
            //----chang
            /*
            if(cmd& prt_mode_font)
            {
            esc_sts.font_en = FONT_B_WIDTH;    // 9x24
            esc_sts.font_cn = FONT_CN_B_WIDTH;

            }
            else
            {
            esc_sts.font_en = FONT_A_WIDTH;    // 12x24
            esc_sts.font_cn = FONT_CN_A_WIDTH;

            }
            */
#if 0
            switch(cmd&0x01)
            {
                 case 0:
                    esc_sts.font_en = FONT_A_WIDTH;    // 12x24
                    esc_sts.font_cn = FONT_CN_A_WIDTH;
                    break;
                 case 1:
                    esc_sts.font_en = FONT_B_WIDTH;    // 9x24
                    esc_sts.font_cn = FONT_CN_B_WIDTH;
                    break;
#if 0
                 case 2://位1保留
                     #if 0
                    esc_sts.font_en = FONT_C_WIDTH;    // 9x24
                    esc_sts.font_cn = FONT_CN_A_WIDTH;//没有定义c类汉字----chang
                    #else //恢复到初始设置
                   /* esc_sts.font_en = FONT_A_WIDTH;    // 12x24
                    esc_sts.font_cn = FONT_CN_A_WIDTH;
                    esc_sts.bold = 0;
                    esc_sts.larger = 0;
                    esc_sts.underline = 0;
                    */
                    esc_sts.upside_down = ;
                    #endif
                    break;
                 case 3:
                    break;
#endif
            }
#endif
            if(cmd & prt_mode_font)
            {
                esc_sts.font_en = FONT_B_WIDTH;    // 9x24
                esc_sts.font_cn = FONT_CN_B_WIDTH;
            }
            else
            {
                esc_sts.font_en = FONT_A_WIDTH;    // 12x24
                esc_sts.font_cn = FONT_CN_A_WIDTH;
            }

            if(cmd & prt_mode_revert)
            {
                esc_sts.revert = 1;//on
            }
            else
            {
                esc_sts.revert = 0;//off
            }

            if(cmd & prt_mode_upside_down)
            {
                esc_sts.upside_down = 1;//on
            }
            else
            {
                esc_sts.upside_down = 0;//off
            }

            if(cmd & prt_mode_bold)
            {
                esc_sts.bold = 1;//on
            }
            else
            {
                esc_sts.bold = 0;//off
            }
            if(cmd & prt_mode_double_height)//倍高
            {
                esc_sts.larger = (esc_sts.larger & 0xf0) | 0x01;
            }
            else
            {
                esc_sts.larger = (esc_sts.larger & 0xf0);
            }
            if(cmd & prt_mode_double_weight)//倍宽
            {
                esc_sts.larger = (esc_sts.larger & 0x0f) | 0x10;
            }
            else
            {
                esc_sts.larger = (esc_sts.larger & 0x0f);
            }
            if(cmd & prt_mode_underline)//下划线
            {
                //esc_sts.underline = 1;
                esc_sts.underline = (1 << 7) | 0x01;
            }
            else
            {
                esc_sts.underline = 0;
            }
            break;

        }
        case '%':    // 选择/取消用户自定义字符集
        {
            esc_sts.userdefine_char = Getchar() & 0x01;
            break;
        }
        case '&':    // 定义用户自定义字符

            break;


#if defined(SUPPORT_PICTURE)
        case '*':    // 选择图像模式(Select bit-image mode)

            {
            uint8_t j, k, c, mode, ratio;
            uint8_t height;
            uint16_t i,width;

            mode = Getchar();
            width = Getchar();
            width |= ((uint16_t)Getchar()) << 8;
            memset(esc_sts.font_buf.picture,0,sizeof(esc_sts.font_buf.picture));
            switch (mode)
            {

            case 0:
                height = 1;
                if (width > (LineDot/2))
                {
                    width = (LineDot/2);
                }

                ratio = 2;

                break;
            case 1:
                height = 1;
                if (width > LineDot)
                {
                    width = LineDot;
                }
                ratio = 1;
                break;
            case 32:
                height = 3;
                if (width > (LineDot/2))
                {
                    width = (LineDot/2);
                }

                ratio = 2;
                break;
            case 33:
                height = 3;
                if (width > LineDot)
                {
                    width = LineDot;
                }

                ratio = 1;

                break;
            default:
                break;
                // error parameter
            }

            for(i=0;i<width*height;i++)
            {
                c=Getchar();
                #if 1
                switch(height)
                {
                    case 1:     //8点高度
                    {
                        for(j=0;j<8;j++)       //MSB在前
                        {
                            if(c & (1<<7-j) )
                            {
                                switch(ratio)
                                {
                                    case 1:
                                        for(k=0;k<3;k++)
                                        {
                                            esc_sts.font_buf.picture[i*3+(j*3+k)/8] |= (1<<7-(j*3+k)%8);
                                        }
                                        break;
                                    case 2:
                                        for(k=0;k<3;k++)
                                        {
                                            esc_sts.font_buf.picture[i*2*3+0+(j*3+k)/8] |=(1<<7-(j*3+k)%8);
                                            esc_sts.font_buf.picture[i*2*3+3+(j*3+k)/8] |=(1<<7-(j*3+k)%8);
                                        }
                                        break;
                                    default:
                                        break;
                                }
                            }
                            else
                            {
                                switch(ratio)
                                {
                                  case 1:
                                      for(k=0;k<3;k++)
                                          esc_sts.font_buf.picture[i*3+(j*3+k)/8] &=~ (1<<7-(j*3+k)%8);
                                          break;
                                  case 2:
                                      for(k=0;k<3;k++)
                                      {
                                          esc_sts.font_buf.picture[i*2*3+0+(j*3+k)/8] &=~(1<<7-(j*3+k)%8);
                                          esc_sts.font_buf.picture[i*2*3+3+(j*3+k)/8] &=~(1<<7-(j*3+k)%8);
                                      }
                                      break;
                                  default:
                                      break;
                                }
                            }
                        }

                    }
                         break;
                   case 3:      //24点高度
                    {
                        switch(ratio)
                        {
                            case 1:     //放大1倍
                                esc_sts.font_buf.picture[i/3*3+i%3]=c;
                                break;
                            case 2:
                                esc_sts.font_buf.picture[i/3*3*2+i%3]=c;
                                esc_sts.font_buf.picture[i/3*3*2+i%3+3]=c;     //3表示纵向高度为3个字节
                                break;
                            default:
                                break;
                        }
                    }
                        break;
                   default:
                        break;
                }
                #endif

            }

            DotFillToBuf(esc_sts.font_buf.picture, width * ratio, 24, 0);
            break;
        }
#endif

        case '-':    // 设定/解除英文下划线
            cmd = Getchar();
            switch(cmd)
            {
            case 0: // underline off
            case 48:
                esc_sts.underline &= ~(1 << 7);
                break;
            case 1:// one-dot underline on
            case 49:
                esc_sts.underline = (1 << 7) | 0x01;
                break;
            case 2:// two-dot underline on
            case 50:
                esc_sts.underline = (1 << 7) | 0x02;
                break;
            }
            break;
        case '2':    // 设置字符行间距为……英寸
            esc_sts.linespace = 32;
            break;
        case '3':    // 设置行间距为……点行（……英寸），默认为8
            esc_sts.linespace = Getchar();
            break;
        case '<':    // Moves the print head to the standby position (DOT Printer)
            break;
        case '=':    // 启动或者关闭打印机
            cmd = Getchar();
            switch(cmd & 0x01)
            {
            case 0: // 关闭打印机
                esc_sts.prt_on = 0;
                break;
            case 1: // 启动打印?
                esc_sts.prt_on = 1;
                break;
            }
            break;
#if defined(SUPPORT_USER_CHAR_AB)
        case '?':    // 清除用户自定义字符
            cmd = Getchar();
            if((cmd >= 0x20) && (cmd <= 0x7e))
            {
                esc_sts.user_char_a[cmd - 0x20].defined = 0;
                esc_sts.user_char_b[cmd - 0x20].defined = 0;
            }
            break;
#endif
        case '@':    // 初始化打印机
            // 1. 清除打印缓冲区
            // 2. 恢复默认值
            esc_p_init();
            // 3. 选择字符打印方式
            // 4. 删除用户定义字符
            break;
        case 'B':    // 设定/解除反白打印模式
            esc_sts.revert = Getchar() & 0x01;
            break;
        case 'C':    // 以行为单位设定页长
            {
                    cmd = Getchar();
                    if(esc_sts.mode == 1)
                    {
                        esc_sts.page_H = (uint16_t)cmd;
                    }
                    break;
                }


        case 'D':    // 设定水平定位点/制表位

            memset(esc_sts.tab, 0, sizeof(esc_sts.tab));
            for(i=0; i<sizeof(esc_sts.tab); i++)
            {
                cmd = Getchar();
                if(cmd == 0)    // NULL
                {
                    break;
                }
                else if((i > 0) && (cmd <= esc_sts.tab[i-1]))    // 不是升序
                {
                    break;
                }
                else
                {
                    esc_sts.tab[i] = cmd;
                }
            }
            break;
            
        case 'E':    // 设定/解除粗体打印(Turn emphasized mode on/off)
        {
            esc_sts.bold = Getchar() & 0x01;
            break;
        }
        case 'G':    // 设定/解除重叠打印(Turn on/off double-strike mode)
            esc_sts.double_strike = Getchar() & 0x01;
            break;
        case 'I':    // One time n/8 mm feed
            {
                      uint16_t n;
                      n = Getchar();
                      if (esc_sts.start_dot == 0)
                      {
                          esc_sts.leftspace = (uint16_t)n ;
                          if (esc_sts.leftspace > LineDot)
                          {
                              esc_sts.leftspace = LineDot;
                          }
                      }
                      break;
             }
        case 'J':
         {
            // 打印输出打印缓冲区中的数据，并进纸n个 垂直点距
            uint8_t n;
            n = Getchar();
            if(n<24)
            {
                n = 0;
            }
            else
            {
                n -= 24;
            }
            PrintCurrentBuffer_0(n);
            break;
         }
        case 'K':    // Print and reverse feed
            break;
        case 'L':    // Select page mode
        {
            esc_sts.mode = 1;    // page mode
            break;
        }
        case 'M':    // 选择字符字体
            cmd = Getchar();
            switch(cmd)
            {
            case 0:        // 西文字符（半宽）字体(12×24)
                esc_sts.font_en = FONT_A_WIDTH;
                break;
            case 1:        // 西文字符（半宽）字体(8×16)
                esc_sts.font_en = FONT_B_WIDTH;
                break;
            case 16:    // 简体汉字字符字体24×24
            case 17:    // 简体汉字字符字体16×16
            case 18:    // BIG5汉字字符字体24×24
                esc_sts.font_cn = FONT_CN_A_WIDTH;
                break;
            case 19:    // BIG5汉字字符字体16×16
                esc_sts.font_cn = FONT_CN_B_WIDTH;
                break;
            default:    // unsupport
                break;
            }
            break;
        case 'R':    // Select an international character set
        {
        //----chang
            uint8_t ch;
            ch = Getchar();
            if(ch <= 15)
            {
                esc_sts.international_character_set = ch;
            }
        /*
            switch(Getchar())
            {
            case 0:        // U.S.A.
                break;
            case 1:        // France
                break;
            case 2:        // Germany
                break;
            case 3:        // U.K.
                break;
            case 4:        // Denmark I
                break;
            case 5:        // Sweden
                break;
            case 6:        // Italy
                break;
            case 7:        // Spain I
                break;
            case 8:        // Japan
                break;
            case 9:        // Norway
                break;
            case 10:    // Denmark II
                break;
            case 11:    // Spain II
                break;
            case 12:    // Latin America
                break;
            case 13:    // Korea
                break;
            case 14:    // Slovenia / Croatia
                break;
            case 15:    // China
                break;
            }
            */
            break;
        }
        case 'S':    // Select standard mode
         {
            esc_sts.mode = 1;    // page mode
            break;
         }

        case 'T':    // Select print direction in page mode
            break;
        case 'U':    // Turn unidirectional printing mode on/off
            cmd = Getchar() & 0x01;        // Turns unidirectional printing mode on or off
            break;
        case 'V':    // 设置/解除字符旋转模式
            {
            switch (cmd=Getchar())
            {
            case 0:     // 0 degree
            case 48:
                esc_sts.rotate = 0;
                break;
            case 1:     // 90 degree
            case 49:
                esc_sts.rotate = 1;
                break;
            case 2:     // 180 degree
                esc_sts.rotate = 2;
                break;
            default:     // 270 degree
                break;


            }
            break;
                    }

/*        case 'W':    // 设定/解除倍宽打印(Set printing area in page mode)
            cmd = Getchar();
            switch(cmd)
            {
            case 1:    // 设定倍宽打印
                break;
            case 2:    // 解除倍宽打印
                break;
            }
            break;   */
        case 'a':    // 选择对齐方式(Select justification)
            {
            cmd = Getchar();
            switch (cmd)
            {
            case 0:    // 左对齐
            case 48:
                if (esc_sts.start_dot == 0)
                {
                    esc_sts.align = 0;
                }
                break;
            case 1:    // 居中
            case 49:
                if (esc_sts.start_dot == 0)
                {
                    esc_sts.align = 1;
                }
                break;
            case 2:    // 右对齐
            case 50:
                if (esc_sts.start_dot == 0)
                {
                    esc_sts.align = 2;
                }
                break;
            }
            break;
        }
        case 'c':
            switch(Getchar())
            {
            case '0':
                Getchar();
                break;
            case '1':
                Getchar();
                break;
            case '2':
                Getchar();
                Getchar();
                break;
            case '3':    // Select paper sensor to output paper end signals
                Getchar();
                break;
            case '4':    // Select paper sensor to stop printing
                Getchar();
                break;
            case '5':    // Enable/disable panel buttons
                cmd = Getchar() & (1 << 0);
                break;
            }
            break;
        case 'd':    // 打印打印缓冲区中的数据并进纸n字符行
            // TODO:
            // 打印打印缓冲区中的数据并进纸n字符行
            // 该命令设置打印起始位置为行起点
            {
                uint8_t n;
                // 打印打印缓冲区中的数据并进纸n字符行
                PrintCurrentBuffer(0);
                n = Getchar();      // n
                if(n>0)
                {
                    n -= 1;
                }
                while (n--)
                {
                    BufFillToPrn(0);
                }
                break;
           }

        case 'e':    // Print and reverse feed n lines
            Getchar();        // n
            break;
        case 'm':    //选择切刀模式并半切,486,48d不支持
             break;
        case 'r':    // Select print color
            switch(Getchar())    // n
            {
            case 0:     // Black
            case 48:
                break;
            case 1:     // Red
            case 49:
                break;
            }
            break;
 //----chang
            /*
        case 't':    // Select character code table
            para.com_baud=Getchar();
            UpdateParaSector(&para);
            break;
            */
 #if defined(TM_T88II)||defined(VMP01)
                 // TODO: TM-T88 (OK)
                 case 't':   // Select character code table
                 {
                     uint8_t page;
                     page = Getchar();
                     if(page <= 47)
                     {
                         if(esc_sts.character_code_page != page)
                         {
                             para.character_code_page = page;
                             esc_sts.character_code_page = page;
                             UpdateParaSector(&para);
                         }
                     }
                     break;
                 }
 #endif

        case 'v':    // 向主机传送打印机状态
            #ifdef ESC_v_P_ENABLE
            cmd=Getchar();
            switch(cmd)
            {
                 case 'P':
                     {
                         if(TPPrinterReady())
                         {
                             Putchar(0x00);
                         }
                         else
                         {
                             Putchar(0x04);
                         }
                         break;

                     }
                case 0x1b:
                    {
                        cmd=Getchar();
                        switch(cmd)
                        {
                         case 'v':
                             if(TPPrinterReady())
                             {
                                 Putchar(0x00);
                             }
                             else
                             {
                                 Putchar(0x04);
                             }
                             break;
                        default:
                            break;
                        }
                        break;
                     }
                default:
                 break;
            }
            #else
            if(TPPrinterReady())
               {
                   Putchar(0x00);
               }
               else
               {
                   Putchar(0x04);
               }
            #endif
            break;
        case '{':    // 打印上下倒置(Turns on/off upside-down printing mode)
        {
            if (esc_sts.start_dot == 0)
            {
                esc_sts.upside_down = Getchar() & 0x01;
            }
            else
            {
                Getchar();
            }
            break;
        }
        case '$':    // 设定绝对水平打印位置
        {
            uint16_t n;
            n = (uint16_t)Getchar();
            n |= ((uint16_t)Getchar()) << 8;
            #if 0
            esc_sts.start_dot = n;
            if(esc_sts.start_dot > LineDot)
            {
                esc_sts.start_dot = LineDot;
            }
            #else
            if(n<LineDot)
            {
                esc_sts.start_dot = n;
            }
            #endif
            break;
        }
        case '\\':    // Set relative print position
            {
                uint16_t n;

                n = Getchar();        // nL
                n |= (Getchar() << 8);        // nH
                //n += esc_sts.start_dot;
                if ((esc_sts.start_dot + n) < LineDot)
                {
                    esc_sts.start_dot += n;
                }
                break;
            } 
        case  'p':    //打开钱箱
        {   uint8_t m,t1, t2;
            m = Getchar();
            t1 = Getchar();        // t1、t2 P6开钱箱不用考虑这两个参数？
            t2 = Getchar();        // t2
            dev_misc_cashbox_open();
            break;
        }
        case 'i':     //选择切刀模式并全切,486,48d不支持
            break;
        }
        break;
    case FS:        // FS
        cmd = Getchar();
        switch(cmd)
        {
        case '(':    // Control black mark and roll paper
            break;
        case '&':    // 设定汉字模式(Select multi byte code character mode)
            esc_sts.chinese_mode = 1;
            break;
        case '-':    // Turn underline mode on/off for multi byte code characters
            switch(Getchar())
            {
            case 0:        // off
            case 48:
                break;
            case 1:        // on
            case 49:
                break;
            }
            break;
        case '.':    // 取消汉字字符模式(Cancel multi byte code character mode)
            esc_sts.chinese_mode = 0;
            break;
#if defined(SUPPORT_CHINESE_USER_CHAR)
        case '2':    // 定义用户自定义汉字字符(Define user-defined multi byte code characters)

            break;

#endif
        case '?':    // Cancel user-defined multi byte code characters

             break;



        case 'S':    // Set multi byte code character spacing
            break;
        case 'W':    // Turn quadruple-size mode on/off for multi byte code characters
            break;
        case 'p':    // 打印预存储位图
            // TODO:
        {
            uint8_t bitmap_mode;
            uint8_t n,mode;
            dev_debug_printf("--- %s --- %d ----\r\n", __func__, __LINE__);
            n = Getchar();      // n
            mode = Getchar();
            dev_debug_printf("n:%d, mode:%d\r\n", n, mode);
            #if 1 //defined(SUPPORT_NVBITMAP)
            if(n == 0 || n > 5)
            {
                return;
            }
            bitmap_mode = get_bitmap_mode(bitmap_path[n -1]);
            dev_debug_printf("bitmap_mode:%d \r\n",bitmap_mode);
            
            
            if(bitmap_mode == 0)
            {
                return;
            }
            else if(bitmap_mode == 1)
            {
                print_original_bitmap(bitmap_path[n -1], mode, esc_sts.align);
            }
            else
            {
                Printf_Bitmap(n,mode);
            }
            #else
            n = n;
            mode = mode;
            #endif
            break;
            }

        case 'q':    // Define NV bit image
        {
            uint8_t n;
            uint8_t dat;
            struct
            {
                uint8_t  xL,xH,yL,yH;
            }                    BitmapWH;
            uint16_t k, len;

            uint32_t addr;
            uint8_t *Bitmap_p = NULL, *p = NULL;

            dat = dat;
            n = Getchar();
            addr = bitmap_addr;
            while(addr< ( bitmap_addr + 0x10000))
            {

//                F25L_SectorErase( addr >> 12);
                addr += 0x1000;
            }
            addr = bitmap_addr;

            while(n--)
            {
                BitmapWH.xL = Getchar();//x为宽度，y为高度，送数据时为
                BitmapWH.xH = Getchar();
                BitmapWH.yL = Getchar();
                BitmapWH.yH = Getchar();
                k = ((BitmapWH.xH<<8) + BitmapWH.xL )*((BitmapWH.yH<<8) + BitmapWH.yL )*8 ;
                if(k+4 >0xA000)//单幅位图不可超过40k
                {
                 //   printf("ERROR!!!RETURN!!!\n");
                    break;
                }
                #if defined(SUPPORT_NVBITMAP)
//                F25L_ByteProgram(addr,BitmapWH.xL);
//                F25L_ByteProgram(addr+1,BitmapWH.xH);
//                F25L_ByteProgram(addr+2,BitmapWH.yL);
//                F25L_ByteProgram(addr+3,BitmapWH.yH);
                #endif
                Bitmap_p = k_malloc(k+4);
                if(Bitmap_p == NULL)
                {
                    break;
                }
                Bitmap_p[0] = BitmapWH.xL;
                Bitmap_p[1] = BitmapWH.xH;
                Bitmap_p[2] = BitmapWH.yL;
                Bitmap_p[3] = BitmapWH.yH;
                
                
                p = Bitmap_p;
                p += 4;
                len = k + 4;

                while(k)
                {
                    *p = Getchar();
                /*    #if defined(SUPPORT_NVBITMAP)
                    F25L_ByteProgram(addr++,dat);
                    #else
                    addr++;
                    #endif
                */
                    p++;
                    k--;
                }
                //F25L_write_nv("/mtd2/nvimage.bin", Bitmap_p, len, 0);
                fs_delete_file(bitmap_path[n-1]);
                F25L_write_nv(bitmap_path[n-1], Bitmap_p, len);
                
                k_free(Bitmap_p);
            }
      //      printf("位图下载完成!!!\n");
            break;
        }
        case 'U':    // 按Unicode编码向打印发送数据
            // TODO:
            break;
        case '!':    // 设置字符打印方式
        {
            cmd = Getchar();
           

            if(cmd & (1 << 2))//倍宽
            {
                esc_sts.larger = (esc_sts.larger & 0x0f) | 0x10;
            }
            else
            {
                esc_sts.larger = (esc_sts.larger & 0x0f);
            }

            if(cmd & (1 << 3))//倍高
            {
                esc_sts.larger = (esc_sts.larger & 0xf0) | 0x01;
            }
            else
            {
                esc_sts.larger = (esc_sts.larger & 0xf0);
            }

            if(cmd & (1 << 7))//下划线
            {
                //esc_sts.underline = 1;
                esc_sts.underline = (1 << 7) | 0x01;
            }
            else
            {
                esc_sts.underline = 0;
            }
            break;

        }

        }
        break;
    case GS:        // GS
        cmd = Getchar();
        switch(cmd)
        {
            case 'q':
            {
                uint8_t len = 0;
                uint8_t RSLevel = 0;
                uint8_t msize = 0;
                uint8_t align = 0;
                char  *lpsSource = NULL;
                uint16_t i;

                lpsSource = k_malloc(PRINT_QRCODE_LEN_MAX+1); /*TODO*/
                if(lpsSource == NULL)
                {
                    return;
                }
                len = Getchar();

                RSLevel = Getchar();
                if(RSLevel > 0)
                {
                    RSLevel -= 1;
                }
                msize   = Getchar();
                align   = Getchar();
                if(align > 0)
                {
                    align -= 1;
                }
                
                dev_debug_printf("len = %d, RSLevel = %d, msize = %d, align = %d\r\n", len, RSLevel, msize, align);

                memset(lpsSource, 0, PRINT_QRCODE_LEN_MAX+1);
                for(i = 0; i < len - 3; i++)
                {
                    lpsSource[i] = Getchar();
                }
                lpsSource[i] = '\0';
                if((len < 4) || (len > PRINT_QRCODE_LEN_MAX+4))
                {
                    TRACE_ERR("ERR QR code length out of bounds!\r\n");
                    k_free(lpsSource);
                    lpsSource = NULL;
                    return;
                }
                print_QrcodeData(lpsSource, RSLevel, msize, align);
                k_free(lpsSource);
                lpsSource = NULL;
                break;
            }

#if 0
        case '(':
            switch(Getchar())
            {
            case 'A':    // Execute test print
                Getchar();    // pL
                Getchar();    // pH
                Getchar();    // n
                Getchar();    // m
                break;
            case 'C':    // Edit NV user memory
                break;
            case 'D':    // Enable/disable real-time command
                break;
            case 'E':    // Set user setup commands
                break;
            case 'k':
                {
                    #if defined(SUPPORT_QRCode)
                    uint8_t cn,fn;
                    uint16_t p;
                    p =(uint16_t)Getchar();
                    p |=((uint16_t)Getchar())<<8;
                    cn=Getchar();
                    fn=Getchar();
                    if ((cn!=48)&&(cn!=49))
                    {
                        cn=Getchar();
                        break;
                    }
                    if (cn==49)
                    {
                        QRCodeHandle(p,cn,fn);
                    }
                    #endif
                    break;
                }
            }
            break;
#endif
        case ':':    // Start/end macro definition
            break;
        case '^':    // Execute macro
            break;
        case '$':    // Set absolute vertical print position in page mode
            break;
        case '\\':    // Set relative vertical print position in page mode
            break;
        case '!':    // 放大字符(Select character size)
        {
            uint8_t fa;
            fa=Getchar();
            if (((fa & 0x0f) < FONT_ENLARGE_MAX) && ((fa >> 4) < FONT_ENLARGE_MAX))
            {
                esc_sts.larger = fa;
            }

            break;
        }
        case '\'':    // 打印一水平行上n个线段
        {
            uint8_t  dot[LineDot/8];
            memset(dot, 0, sizeof(dot));
            j = Getchar();    // n segment
            for(; j; j--)
            {
                i = Getchar();
                i |= ((uint16_t)Getchar()) << 8;    // start
                k = Getchar();
                k |= ((uint16_t)Getchar()) << 8;    // stop
                for(; (i<k) && (i<LineDot); i++)
                {
                    dot[i>>3] |= 1 << (7-(i&0x03));
                }
            }
            TPPrintLine(dot);
            break;
        }
        case '"':    // 打印曲线上的文字
            // TODO:
            break;
#if defined(SUPPORT_BITMAP)
        case '*':    // 定义下传位图

            {


            x = Getchar();
            y = Getchar();
            k = (uint16_t)x * y * 8;
            if ((1 <= x) && ((1 <= y) && (y <= 48)) && (k <= sizeof(esc_sts.bitmap.font)))
            {
                esc_sts.bitmap.width = x;
                esc_sts.bitmap.height = y;
                for (i=0; i<k; i++)
                {
                    esc_sts.bitmap.font[i] = Getchar();
                }
                esc_sts.bitmap.defined = 1;
            }
            break;
        }
        case '/':    // 打印下传位图

        {
            cmd = Getchar();
            switch (cmd)
            {
            case 0x00: // 普通
            case 0x30:
            case 0x01: // 倍宽
            case 0x31:
            case 0x02: // 倍高
            case 0x32:
            case 0x03: // 倍宽倍高
            case 0x33:
                if ((esc_sts.start_dot == 0) && esc_sts.bitmap.defined)    // defined
                {
                    DotBufFillToPrn(esc_sts.bitmap.font, esc_sts.bitmap.width*8, esc_sts.bitmap.height, 0, esc_sts.bitmap.height, ((cmd & (1 << 0)) ? 2 : 1), ((cmd & (1 << 1)) ? 2 : 1));
                }
                break;
            }
            break;
        }
#endif
        case 'B':    // Turn white/black reverse printing mode on/off
            esc_sts.revert = Getchar() & 0x01;
            break;
        case 'E':    // set head energizing time
                 TPSetSpeed(Getchar());
            break;
        case 'H':    // Select printing position of HRI characters
            cmd = Getchar();
            switch(cmd)
            {
            case 0:
            case 1:
            case 2:
            case 3:
                // 条码可识读字符的打印位置
                esc_sts.barcode_char_pos = cmd;
                break;
            case 48:
            case 49:
            case 50:
            case 51:
                break;
            }
            break;
        case 'I':    // Transmit printer ID
            switch(Getchar())
            {
            case 2:        // Type of Character (Double byte = 1 Single byte = 0)
                break;
            case 65 :     // Version ID (Firmware)
                Putchar(0x5f);
                Putchar(0x30+VERSION_MAJOR);
                Putchar(0x2e);
                Putchar(0x30);
                Putchar(0x30+VERSION_MINOR);
                Putchar(0x2e);
                Putchar(0x30+VERSION_TEST/10);
                Putchar(0x30+VERSION_TEST%10);
                Putchar(0x20);
                Putchar(0x45);
                Putchar(0x53);
                Putchar(0x43);
                Putchar(0x2f);
                Putchar(0x50);
                Putchar(0x4f);
                Putchar(0x53);
                Putchar(0x00);
                break;
            case 66:    //  Type ID
                break;
            case 67:    // Printer model ID
                break;
            case 80:    // printer model
                break;
            case 81:    // dump all "Flash Command" setting
                break;
            }
            break;
        case 'L':    // 设置左边距
        {
            uint16_t n;
            if(esc_sts.start_dot == 0)
            {
                n = (uint16_t)Getchar();
                n |= ((uint16_t)Getchar()) << 8;
                if(n > 372) //留下一个字符的位置
                {
                    n = 372;
                }
                esc_sts.leftspace = n;
            }
            break;
        }
        case 'P':    // Set horizontal and vertical motion units
            esc_sts.h_motionunit = Getchar();
            esc_sts.v_motionunit = Getchar();
            break;
        case 'V':    // Select cut mode and cut paper
            switch(Getchar())
            {
            case 0:
            case 1:
            case 49:
                break;
            case 66:
                Getchar();        // n
                break;
            }
            break;
        case 'W':    // Set printing area width
            Getchar();        // nL
            Getchar();        // nH
            break;
        case 'a':    // Enable/disable Automatic Status Back (ASB)
            {
              cmd=Getchar();
              if(cmd & prt_mode_asb)
               {
                   esc_sts.asb_mode = 1;//on
                   if(TPPrinterReady())
                       {
                        Putchar(0x00);
                       }
                   else
                     {
                         Putchar(0x04);
                     }
               }
               else
               {
                   esc_sts.asb_mode = 0;//off
               }

              if(cmd&prt_mode_busy)
               {

               }
              else
               {
               }

            }
            break;
        case 'b':    // Turn smoothing mode on/off
            Getchar();
            break;
        case 'f':    // 选择条码识读字符字体
        {
            cmd = Getchar();
            switch (cmd)
            {
            case 0:
            case 48:
                esc_sts.barcode_font = FONT_A_WIDTH;
                break;
            case 1:
            case 49:
                esc_sts.barcode_font = FONT_B_WIDTH;
                break;
            }
            break;
        }
        case 'h':    // Set bar code height
            cmd = Getchar();
            if(cmd >= 1)
            {
                if(cmd > 40)
                {
                    //cmd = 40;
                }
                esc_sts.barcode_height = cmd;
            }
            break;
        case 'k':    // Print bar code
            // TODO:
        {
            uint8_t m;

            m = Getchar();    // m
            switch(m)
            {

            case 2:
            case 67:        // EAN13
                PrintBarcode_EAN13(m);
                break;
            case 4:
            case 69:        // CODE39
                PrintBarcode_CODE39(m);
                break;
            case 5:
              case 70:        // INTERLEAVED 25(ITF)
                  PrintBarcode_ITF(m);
                  break;
            #if defined(SUPPORT_ALL_BARCODE)
            case 0:
              case 65:        // UPC-A
                  PrintBarcode_UPC_A(m);
                  break;
              case 1:
              case 66:        // UPC-E
                  PrintBarcode_UPC_E(m);
                  break;
              case 3:
              case 68:        // EAN8
                  PrintBarcode_EAN8(m);
                  break;
              case 6:
              case 71:        // CODABAR
                  PrintBarcode_CODABAR(m);
                  break;
              case 7:
              case 72:        // CODE93
                  PrintBarcode_CODE93(m);
                  break;
              case 8:
              case 73:        // CODE128
                  PrintBarcode_CODE128(m);
                  break;
            #endif
            }
            break;
         }
/*          {
              uint8_t m;

              m = Getchar();    // m
              switch(m)
              {
              case 0:
              case 65:        // UPC-A
                  PrintBarcode_UPC_A(m);
                  break;
              case 1:
              case 66:        // UPC-E
                  PrintBarcode_UPC_E(m);
                  break;
              case 2:
              case 67:        // EAN13
                  PrintBarcode_EAN13(m);
                  break;
              case 3:
              case 68:        // EAN8
                  PrintBarcode_EAN8(m);
                  break;
              case 4:
              case 69:        // CODE39
                  PrintBarcode_CODE39(m);
                  break;
              case 5:
              case 70:        // INTERLEAVED 25(ITF)
                  PrintBarcode_ITF(m);
                  break;
              case 6:
              case 71:        // CODABAR
                  PrintBarcode_CODABAR(m);
                  break;
              case 7:
              case 72:        // CODE93
                  PrintBarcode_CODE93(m);
                  break;
              case 8:
              case 73:        // CODE128
                  PrintBarcode_CODE128(m);
                  break;
              }
              break;
          }*/
        case 'r':    // Transmit status
            Getchar();  //硬件不支持，读一个值不做处理
            break;
        case 'u':   //1d 75 55 aa
            {
                uint16_t flag;
                flag = (uint16_t)Getchar()<<8;
                flag |= Getchar();
                if (flag == 0x55AA)
                {
                    Putchar('O');
                    Putchar('K');
                    *(unsigned long long *)(0x10000000UL) = RESETFLAG_JMPFROMMAIN;
//                    NVIC_SystemReset();
                }
                break;
            }
    case 'v':    // Print raster bit image
        {
             switch(Getchar())
             {
             case '0':
                 cmd = Getchar();
                 x = Getchar();
                 x |= ((uint16_t)Getchar()) << 8;
                 y = Getchar();
                 y |= ((uint16_t)Getchar()) << 8;
                RasterPrint(cmd, x, y);
                break;
             }
             break;
          }

    case 'w':   // Set bar code width
    {
        cmd = Getchar();
        if((cmd >= 2) && (cmd <= 4))
        {
            esc_sts.barcode_width = cmd;
        }

        break;
    }
    case 'x':
    {
        esc_sts.leftspace=Getchar();
        break;
    }
        }
    break;
case CAN:
        break;
    default:
        {
            //Putchar(cmd);
            //----chang
#if !defined(CHINESE_FONT)||defined (CODEPAGE)
            if((cmd >= 0x20) && (cmd <= 0xff))
            {
                GetEnglishFont(cmd);
                
            }
#else
//////////////////////////////////////////////////////////////////////////////////////////
        if((cmd >= 0x20) && (cmd <= 0x7f))
            {
        
                GetEnglishFont(cmd);
                
            }
#if defined(GB2312)
        else if ((cmd >= 0xa1) && (cmd <= 0xfe))
        {
            uint8_t chs[2];
            chs[0] = cmd;
            chs[1] = Getchar();
        if ((chs[1] >= 0xa1) && (chs[1] <= 0xfe)
          {
                GetChineseFont(chs, CHINESE_FONT_GB2312);
          }
            else
          {
                GetEnglishFont('?');
                GetEnglishFont('?');
          }
        }
#elif defined(GBK) || defined(GB18030)
        else if ((cmd >= 0x81) && (cmd <= 0xfe))
        {
            uint8_t chs[4];
            chs[0] = cmd;
            chs[1] = Getchar();
         
#if defined(GB18030)
            if ((chs[1] >= 0x30) && (chs[1] <= 0x39))
#else
                if (0)
#endif
                {
              
                    chs[2] = Getchar();
                    chs[3] = Getchar();
                    // GB18030定义的4字节扩展
                    if (((chs[2] >= 0x81) && (chs[2] <= 0xfe)) && ((chs[3] >= 0x30) && (chs[3] <= 0x39)))
                    {
                        GetChineseFont(chs, CHINESE_FONT_GB18030);
                    }
                    else
                    {
                        GetEnglishFont('?');
                        GetEnglishFont('?');
                        GetEnglishFont('?');
                        GetEnglishFont('?');
                    }
                }
            // GB13000定义的2字节扩展
                else if ((chs[1] >= 0x40) && (chs[1] <= 0xfe) && (chs[1] != 0x7f))
                {
                    #if defined(GT30L24M1Z)
                    if(((chs[0] == 0xaa) && ((chs[1] <= 0xfe) && (chs[1] >= 0xa1))) || ((chs[0] == 0xab) && ((chs[1] <= 0xc0) && (chs[1] >= 0xa1))))
                    {
                        GetEnglishFontExt(chs);
                    }
                    else
                    {
                        GetChineseFont(chs, CHINESE_FONT_GBK);
                   
                    }
                    #else
                    GetChineseFont(chs, CHINESE_FONT_GBK);
                 
                    #endif
                }
                else
                {
                    GetEnglishFont('?');
                    GetEnglishFont('?');
                
                }
        }
  #endif
#endif

       }

    }
}
//======================================================================================================

