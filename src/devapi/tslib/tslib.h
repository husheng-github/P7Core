/********************************Copyright ( c ) ***************************
**                  Shenzhen Xinguodu Technology Co., LTD.
**                      http://www.xinguodu.com
**
**
**Created By:       黄学佳
**Created Date:     2012-7-10
**Version:          <version >
**Description:  
**
****************************************************************************/
#ifndef _TSLIB_H_
#define _TSLIB_H_

//尝试一下 TSLIB里面的5点校准算法，算法参考文档: Tslib中触摸屏校准原理及其实现.pdf
typedef struct {
    int x[5], xfb[5];
    int y[5], yfb[5];
    unsigned int a[7];
} calibration;

extern calibration cal;//触摸屏校准数据

#define NR_SAMPHISTLEN  4


//经过扫描，数据处理后得到一个触摸点的值，放在下面的数据结构中
struct ts_sample
{
        int x;//x 轴的数据
        int y;
        unsigned int pressure;//压力
        unsigned char type;//主要用来指出是下笔还是提笔,或者是触摸中的变化
                //对于原始数据还会表明是可疑点还是不是可疑点
};

struct ts_hist {
    int x;
    int y;
    unsigned int p;
};

struct tslib_dejitter {
    //struct tslib_module_info module;
    int delta;
    int x;
    int y;
    int down;
    int nr;
    int head;
    struct ts_hist hist[NR_SAMPHISTLEN];//积压
};

extern struct tslib_dejitter dejitter_info;

struct tslib_ops {
    int (*read)(struct tslib_module_info *inf, struct ts_sample *samp, int nr);
    int (*fini)(struct tslib_module_info *inf);
};


struct tsdev {
    int fd;
    struct tslib_module_info *list;
    struct tslib_module_info *list_raw; /* points to position in 'list' where raw reads
                           come from.  default is the position of the
                           ts_read_raw module. */
};

struct tslib_module_info {
    struct tsdev *dev;
    struct tslib_module_info *next; /* next module in chain */
    void *handle;           /* dl handle        */
    const struct tslib_ops *ops;
};

#define VAR_PENDOWN         0x00000001//笔按下标识
#define VAR_LASTVALID       0x00000002//前一点是否有效标识，也可以看做下笔标识
#define VAR_NOISEVALID      0x00000004 //上一点是噪声标识
#define VAR_SUBMITNOISE     0x00000008//连续两个可疑样点标识

struct tslib_variance {
    struct tslib_module_info module;
    int delta;
        struct ts_sample last;
        struct ts_sample noise;
    unsigned int flags;

};

extern struct tslib_variance variance_info;
extern struct ts_sample TpScanSourceData[2];//扫描的原始数据

extern int tslib_deal(struct ts_sample *outsamp, struct ts_sample *insamp);

#define TEST_TSLIB 0X01
#undef  TEST_TSLIB

#endif

