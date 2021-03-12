/********************************Copyright ( c ) ***************************
**                  Shenzhen Xinguodu Technology Co., LTD.
**                      http://www.xinguodu.com
**
**
**Created By:       ��ѧ��
**Created Date:     2012-7-10
**Version:          <version >
**Description:  
**
****************************************************************************/
#ifndef _TSLIB_H_
#define _TSLIB_H_

//����һ�� TSLIB�����5��У׼�㷨���㷨�ο��ĵ�: Tslib�д�����У׼ԭ����ʵ��.pdf
typedef struct {
    int x[5], xfb[5];
    int y[5], yfb[5];
    unsigned int a[7];
} calibration;

extern calibration cal;//������У׼����

#define NR_SAMPHISTLEN  4


//����ɨ�裬���ݴ����õ�һ���������ֵ��������������ݽṹ��
struct ts_sample
{
        int x;//x �������
        int y;
        unsigned int pressure;//ѹ��
        unsigned char type;//��Ҫ����ָ�����±ʻ������,�����Ǵ����еı仯
                //����ԭʼ���ݻ�������ǿ��ɵ㻹�ǲ��ǿ��ɵ�
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
    struct ts_hist hist[NR_SAMPHISTLEN];//��ѹ
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

#define VAR_PENDOWN         0x00000001//�ʰ��±�ʶ
#define VAR_LASTVALID       0x00000002//ǰһ���Ƿ���Ч��ʶ��Ҳ���Կ����±ʱ�ʶ
#define VAR_NOISEVALID      0x00000004 //��һ����������ʶ
#define VAR_SUBMITNOISE     0x00000008//�����������������ʶ

struct tslib_variance {
    struct tslib_module_info module;
    int delta;
        struct ts_sample last;
        struct ts_sample noise;
    unsigned int flags;

};

extern struct tslib_variance variance_info;
extern struct ts_sample TpScanSourceData[2];//ɨ���ԭʼ����

extern int tslib_deal(struct ts_sample *outsamp, struct ts_sample *insamp);

#define TEST_TSLIB 0X01
#undef  TEST_TSLIB

#endif

