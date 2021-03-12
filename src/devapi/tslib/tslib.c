/********************************Copyright ( c ) ***************************
**                  Shenzhen Xinguodu Technology Co., LTD.
**                      http://www.xinguodu.com
**
**
**Created By:       ��ѧ��
**Created Date:     2012-7-10
**Version:          <version >
**Description:  �����������м������TSLIB��ֲ���ģ�������һЩ�޸ģ��Ա���Ӧ��
                ��ϵͳ����
**              �����������̻�����
    ԭʼ����->variance(�����˲�)->Ȩֵƽ��(ƽ��)->У׼(ת��LCD�ϵĵ�)
**modify: ver 1.0   2012.07.11
****************************************************************************/
#include "tslib.h"

#define TSLIB_DEJITTER_DELTA (50)//(100)//dejitter ������ŷ�//�����ж��Ƿ��ǿ�������
#define TSLIB_VARIANCE_DELTA (20)//(30)//�����ж��Ƿ��ǿ����ƶ�


static int tslib_gd = 0;

calibration cal;//������У׼���� cal.x��cal.y�Ǵ�����ֵ��cal.xfb,cal.xfb��LCD����
            //cal.a����������У׼��õ���7����ֵ��
struct tslib_dejitter dejitter_info;
struct tslib_variance variance_info;

static int sqr (int x)
{
    return x * x;
}

//_____________________________У׼���_____________________________________

/****************************************************************************
**Description:          ���У׼�㷨�����ñ�����ǰ���cal��ֵ
**Input parameters:     cal
**Output parameters:    cal.a
**                  
**Returned value:       
**                  
**Created by:           ��ѧ��(2012-6-28)
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
int do_calibration(void)
{
    int j;
    float n, x, y, x2, y2, xy, z, zx, zy;
    float det, det1, det2, det3;
    float scaling = 65536.0;
    
    n = x = y = x2 = y2 = xy = 0;
    for (j = 0; j < 5; j++)
    {
        n += 1.0;
        x += (float)cal.x[j];
        y += (float)cal.y[j];
        x2 += (float)(cal.x[j] * cal.x[j]);
        y2 += (float)(cal.y[j] * cal.y[j]);
        xy += (float)(cal.x[j] * cal.y[j]);
    }
    
    det = n * (x2*y2 - xy*xy) + x * (xy*y - x*y2) + y * (x*xy - y*x2);
    if (det < 0.1 && det > -0.1)
    {
        #ifdef TEST_TP_HXJ
        ///printf("Determinant is too small!%d\n", det);
        #endif
        return 1;
    }
    
    z = zx = zy = 0;
    for (j = 0; j < 5; j++)
    {
        z += (float)cal.xfb[j];
        zx += (float)(cal.xfb[j] * cal.x[j]);
        zy += (float)(cal.xfb[j] * cal.y[j]);
    }
    
    det1 = n * (zx*y2 - xy*zy) + z * (xy*y - x*y2) + y * (x*zy - y*zx);
    det2 = n * (x2*zy - zx*xy) + x * (zx*y - x*zy) + z * (x*xy - y*x2);
    det3 = z * (x2*y2 - xy*xy) + x * (xy*zy - zx*y2) + y * (zx*xy - zy*x2);
    cal.a[0] = (int)((det1 / det) * scaling);
    cal.a[1] = (int)((det2 / det) * scaling);
    cal.a[2] = (int)((det3 / det) * scaling);
    #ifdef TEST_TP_HXJ
    //printf("%10d %10d %10d\n", cal.a[0], cal.a[1], cal.a[2]);
    #endif
    z = zx = zy = 0;
    for (j = 0; j < 5; j++)
    {
        z += (float)cal.yfb[j];
        zx += (float)(cal.yfb[j] * cal.x[j]);
        zy += (float)(cal.yfb[j] * cal.y[j]);
    }
    
    det1 = n * (zx*y2 - xy*zy) + z * (xy*y - x*y2) + y * (x*zy - y*zx);
    det2 = n * (x2*zy - zx*xy) + x * (zx*y - x*zy) + z * (x*xy - y*x2);
    det3 = z * (x2*y2 - xy*xy) + x * (xy*zy - zx*y2) + y * (zx*xy - zy*x2);
    cal.a[3] = (int)((det1 / det) * scaling);
    cal.a[4] = (int)((det2 / det) * scaling);
    cal.a[5] = (int)((det3 / det) * scaling);
    cal.a[6] = (int)scaling;
    return 0;
}
/****************************************************************************
**Description:      ���ݱ����У׼���ݣ�������봥�����Ӧ��LCD����
**Input parameters: 
**Output parameters:    
**                  
**Returned value:       
**                  
**Created by:           ��ѧ��(2012-6-28)
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
int calibrateTP(int tpx, int tpy, int *calx, int *caly)
{
    *calx = (tpx*cal.a[0] + tpy*cal.a[1] + cal.a[2])/cal.a[7];
    *caly = (tpx*cal.a[3] + tpy*cal.a[4] + cal.a[5])/cal.a[7];
    return 0;
}
//_____________________________�˲����_____________________________

//variance_read �����������������ʱ��
//static int variance_read(struct tslib_module_info *info, struct ts_sample *samp, int nr)
//��ѧ�ѣ��޸ĺ����������Ա�����STM32������������������Ĳ���
//����ÿ������һ�����㣬����������0��������1��������������
static int variance_read(struct tslib_variance *info, struct ts_sample *samp, int nr, struct ts_sample *insamp)
{
    struct tslib_variance *var = (struct tslib_variance *)info;
    struct ts_sample cur;
    int count = 0, dist;
    
    int readcount = 0;//��ѧ�ѣ� 2012.07.10 ������¼�ӵײ��ȡ��������

    //while (count < nr)
    while(1)//��ѧ��
    {
        //���N+1����N+2�������N��ľ��볬����ֵ��������ǿ����ƶ�����ʱ�ὫVAR_SUBMITNOISE��ʶ����
        //�������ʶ��Ϊ����һ��ѭ���ж�N+1��N+2֮���Ƿ�Ҳ������ֵ��
        //
        if (var->flags & VAR_SUBMITNOISE) //VAR_SUBMITNOISE��ʶ������
        {
            cur = var->noise;//�����»�ȡ�����㣬���ǽ�noise�е�������Ϊ��ǰ����
            var->flags &= ~VAR_SUBMITNOISE;//���ʶ
        } 
        else 
        {
            #if 0
            if (info->next->ops->read(info->next, &cur, 1) < 1)//����һ���ȡ����
                return count;   
            #else
                if(readcount >= nr)
                    return count;
                
                cur = *insamp;//�ӵײ��ȡ����
                readcount++;
            #endif  
        }
        
        if (cur.pressure == 0) //��ѹ��
        {
            /* Flush the queue immediately when the pen is just
             * released, otherwise the previous layer will
             * get the pen up notification too late. This 
             * will happen if info->next->ops->read() blocks.
             */
            if (var->flags & VAR_PENDOWN) //��һ��pressure��Ϊ0�ŻὫVAR_PENDOWN ��ʶ���������������������
            {
                var->flags |= VAR_SUBMITNOISE;
                var->noise = cur;//�������㱣�浽noise��
                //uart_printf("up\r\n");
            }
            /* Reset the state machine on pen up events. */
            var->flags &= ~(VAR_PENDOWN | VAR_NOISEVALID | VAR_LASTVALID);//��������ʶ
            goto acceptsample;
        } 
        else
            var->flags |= VAR_PENDOWN;//֪ͨ�ʰ���

        if (!(var->flags & VAR_LASTVALID)) 
        {
            //uart_printf("down\r\n");
            var->last = cur;//����ǰֵ����
            var->flags |= VAR_LASTVALID;//��VAR_LASTVALID��ʶ����
            continue;
        }

        if (var->flags & VAR_PENDOWN) //�����ʵ�Ѿ������ж�
        {
            /* Compute the distance between last sample and current */
            dist = sqr (cur.x - var->last.x) +
                   sqr (cur.y - var->last.y);

            if (dist > var->delta) 
            {
                /* Do we suspect the previous sample was a noise? */
                if (var->flags & VAR_NOISEVALID) //��һ���������������
                {
                    //uart_printf("noise 2\r\n");
                    /* Two "noises": it's just a quick pen movement */
                    samp [count++] = var->last = var->noise;
                    var->flags = (var->flags & ~VAR_NOISEVALID) |VAR_SUBMITNOISE;//����������ʶ��������������ʶ
                } 
                else
                {
                    //uart_printf("noise 1\r\n");
                    var->flags |= VAR_NOISEVALID;//��ɿ��������ʶ
                }
                /* The pen jumped too far, maybe it's a noise ... */
                var->noise = cur;
                continue;
            } 
            else
                var->flags &= ~VAR_NOISEVALID;//����������ʶ
        }

acceptsample:
//#ifdef DEBUG
        //printf("VARIANCE----------------> %d %d %d\n",
            //var->last.x, var->last.y, var->last.pressure);
//#endif
        samp [count++] = var->last;
        var->last = cur;
    }

    return count;//��ֲ��STM32������Ӧ�ò������е�
}

void variance_init(struct tslib_variance *var)
{
    var->delta = TSLIB_VARIANCE_DELTA;
    var->flags = 0;
        var->delta = sqr (var->delta);
}

//_____________________________Ȩֵƽ�����_____________________________
//
/* To keep things simple (avoiding division) we ensure that
 * SUM(weight) = power-of-two. Also we must know how to approximate
 * measurements when we have less than NR_SAMPHISTLEN samples.
 */
/*ע�⿴����������ݣ���ά�����ڣ�ǰ��4��ֵ�ĺ����Ƶ������λ����Ϊ1
 *�� ((w[0][0] + w[0][1] + w[0][2] + w[0][3])>>w[0][4]) == 1
 *
 */
static const unsigned char weight [NR_SAMPHISTLEN - 1][NR_SAMPHISTLEN + 1] =
{
    /* The last element is pow2(SUM(0..3)) */
    { 5, 3, 0, 0, 3 },  /* When we have 2 samples ... */
    { 8, 5, 3, 0, 4 },  /* When we have 3 samples ... */
    { 6, 4, 3, 3, 4 },  /* When we have 4 samples ... */
};
/*��2��5������Ȩ��ƽ�������������ڵ�ǰ���ǵڼ����㡣
 *
 *
 *
 */
static void average (struct tslib_dejitter *djt, struct ts_sample *samp)
{
    const unsigned char *w;
    int sn = djt->head;
    int i, x = 0, y = 0;
    unsigned int p = 0;

        w = weight [djt->nr - 2];

    for (i = 0; i < djt->nr; i++) 
    {
        x += djt->hist [sn].x * w [i];
        y += djt->hist [sn].y * w [i];
        p += djt->hist [sn].p * w [i];
        sn = (sn - 1) & (NR_SAMPHISTLEN - 1);//˵�����������ᳬ��5������
    }

    samp->x = x >> w [NR_SAMPHISTLEN];
    samp->y = y >> w [NR_SAMPHISTLEN];
    samp->pressure = p >> w [NR_SAMPHISTLEN];
//#ifdef DEBUG
    //uart_printf("DEJITTER-------> %d %d %d\n", samp->x, samp->y, samp->pressure);
//#endif
}


//static int dejitter_read(struct tslib_module_info *info, struct ts_sample *samp, int nr)
static int dejitter_read(struct tslib_dejitter  *info, struct ts_sample *samp, int nr)
{
       struct tslib_dejitter *djt = (struct tslib_dejitter *)info;
    struct ts_sample *s;
    int count = 0, ret;
    int prev;
    
    //ret = info->next->ops->read(info->next, samp, nr);//����һ��(variance��)����,nr Ӧ����Ҫ��ȡ�ĵ���
    #if 1//��ֲ����STM32��ϵͳ����ֱ�ӽ���ͨ����������
    ret = nr;   

    #endif
    for (s = samp; ret > 0; s++, ret--) //��������
    {
        if (s->pressure == 0) //ѹ��Ϊ0
        {
            //uart_printf("pressure = 0!\r\n");
            /*
             * Pen was released. Reset the state and
             * forget all history events.
             */
            djt->nr = 0;//����Ȩ��ƽ��ģ��Ļ�ѹ����
            samp [count++] = *s;//������ֱ�ӷ���
                        continue;
        }

                /* If the pen moves too fast, reset the backlog. */
        //uart_printf("nr:%d\r\n", djt->nr);        
        if (djt->nr) 
        {
            prev = (djt->head - 1) & (NR_SAMPHISTLEN - 1);//ǰһ������
            
            //uart_printf("prev:%d\r\n", prev);
            
            if (sqr (s->x - djt->hist [prev].x) +
                sqr (s->y - djt->hist [prev].y) > djt->delta) //��ǰ����λ���ڷ�ֵ��˵���ǿ����ƶ�(��֮ǰ��һ�㴦���Ѿ����ɵ㴦��)
            {
//#ifdef DEBUG
                //uart_printf ( "DEJITTER: pen movement exceeds threshold\r\n");
//#endif
                                djt->nr = 0;
            }
        }

        djt->hist[djt->head].x = s->x;
        djt->hist[djt->head].y = s->y;
        djt->hist[djt->head].p = s->pressure;//�����㱣��Ļ�ѹ��������
        if (djt->nr < NR_SAMPHISTLEN)//�Բ���������++��ֱ��+��4,��Ϊ��Ȩ��ƽ����ʱ�����������5����
            djt->nr++;

        /* We'll pass through the very first sample since
         * we can't average it (no history yet).
         */
        if (djt->nr == 1)//������ǵ�һ�β�����û����ʷ���߾ɲ������ݣ�ֱ�Ӹ�ֵ��
        {
            samp [count] = *s;//�������ûʲô�ã���Ϊ���ݱ������Ǳ�����samp�����
        }
        else// ������ǵ�һ�β�������ִ��ƽ����������þ���ƽ����Ĳ���ֵ
        {
            average (djt, samp + count);//��Ȩ��ƽ��
            //samp [count].tv = s->tv;//�����LINUXϵͳ�����ȡ��ʱ��
        }
        count++;

        djt->head = (djt->head + 1) & (NR_SAMPHISTLEN - 1);//����Ĵ�����ѭ�����⣬����ֻ�е�NR_SAMPHISTLEN��:0x01+1,
                                                        //0x03+1,0x07+1,0x0f+1,,,,��Щֵ��ʱ�����������    
        //uart_printf("head:%d\r\n", djt->head);                                                
    }

    return count;
}

//��ʼ��
void dejitter_init(struct tslib_dejitter *djt)
{
    djt->nr = 0;//��ѹ����
    djt->delta = TSLIB_DEJITTER_DELTA;//�ŷ�ֵ
       djt->head = 0;//��һ����ѹֵ������
    djt->delta = sqr (djt->delta);//Ϊʲô��ƽ���
}
/****************************************************************************
**Description:          TSLIB����ӿڣ�����ϵͳ����
**Input parameters:  insamp ����һ������
**Output parameters: outsamp ���ؾ����������������
**                  
**Returned value: ���ص���������������0����1����2��     
**                  
**Created by:           ��ѧ��(2012-7-10)
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
int tslib_deal(struct ts_sample *outsamp, struct ts_sample *insamp)
{
    int ret;
    if(tslib_gd == 0)
    {
        dejitter_init(&dejitter_info);//��ʼ��Ȩ�ؾ�ֵģ��  
        variance_init(&variance_info);//��ʼ���˲�ģ��
    
        tslib_gd = 1;
        
    }
    
    ret = variance_read(&variance_info, outsamp, 1, insamp);
    
    ret = dejitter_read(&dejitter_info, outsamp, ret);//Ȩ��ƽ��
    //uart_printf("ret:%d\r\n", ret);
    return ret;
    //calibrateTP(samp->x, samp->y, &(samp->x), &(samp->y));//У׼,��ΪG101DSҪ������ת����LCD�����BMPͼƬ���꣬
                                                                //���Բ���������У׼ת��
}

