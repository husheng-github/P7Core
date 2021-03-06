/********************************Copyright ( c ) ***************************
**                  Shenzhen Xinguodu Technology Co., LTD.
**                      http://www.xinguodu.com
**
**
**Created By:       黄学佳
**Created Date:     2012-7-10
**Version:          <version >
**Description:  触摸屏处理中间件，从TSLIB移植来的，不过有一些修改，以便适应于
                无系统程序
**              触摸处理流程基本是
    原始数据->variance(噪声滤波)->权值平均(平滑)->校准(转成LCD上的点)
**modify: ver 1.0   2012.07.11
****************************************************************************/
#include "tslib.h"

#define TSLIB_DEJITTER_DELTA (50)//(100)//dejitter 插件的门阀//用于判断是否是可疑样点
#define TSLIB_VARIANCE_DELTA (20)//(30)//用来判断是否是快速移动


static int tslib_gd = 0;

calibration cal;//触摸屏校准数据 cal.x跟cal.y是触摸屏值，cal.xfb,cal.xfb是LCD坐标
            //cal.a数据里面是校准后得到的7个数值，
struct tslib_dejitter dejitter_info;
struct tslib_variance variance_info;

static int sqr (int x)
{
    return x * x;
}

//_____________________________校准插件_____________________________________

/****************************************************************************
**Description:          五点校准算法，调用本函数前请对cal赋值
**Input parameters:     cal
**Output parameters:    cal.a
**                  
**Returned value:       
**                  
**Created by:           黄学佳(2012-6-28)
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
**Description:      根据保存的校准数据，算出输入触摸点对应的LCD坐标
**Input parameters: 
**Output parameters:    
**                  
**Returned value:       
**                  
**Created by:           黄学佳(2012-6-28)
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
//_____________________________滤波插件_____________________________

//variance_read 函数遇到可疑样点的时候，
//static int variance_read(struct tslib_module_info *info, struct ts_sample *samp, int nr)
//黄学佳，修改函数参数，以便用于STM32，增加用于输入样点的参数
//建议每次输入一个样点，输出样点会是0个，或者1个，或者两个，
static int variance_read(struct tslib_variance *info, struct ts_sample *samp, int nr, struct ts_sample *insamp)
{
    struct tslib_variance *var = (struct tslib_variance *)info;
    struct ts_sample cur;
    int count = 0, dist;
    
    int readcount = 0;//黄学佳， 2012.07.10 用来记录从底层获取的样点数

    //while (count < nr)
    while(1)//黄学佳
    {
        //如果N+1，跟N+2两个点跟N点的距离超出阀值，则可能是快速移动，这时会将VAR_SUBMITNOISE标识置起，
        //置这个标识是为了下一个循环判断N+1跟N+2之间是否也超出阀值，
        //
        if (var->flags & VAR_SUBMITNOISE) //VAR_SUBMITNOISE标识被置起，
        {
            cur = var->noise;//不重新获取新样点，而是将noise中的样点作为当前样点
            var->flags &= ~VAR_SUBMITNOISE;//清标识
        } 
        else 
        {
            #if 0
            if (info->next->ops->read(info->next, &cur, 1) < 1)//从上一层获取样点
                return count;   
            #else
                if(readcount >= nr)
                    return count;
                
                cur = *insamp;//从底层获取样点
                readcount++;
            #endif  
        }
        
        if (cur.pressure == 0) //无压力
        {
            /* Flush the queue immediately when the pen is just
             * released, otherwise the previous layer will
             * get the pen up notification too late. This 
             * will happen if info->next->ops->read() blocks.
             */
            if (var->flags & VAR_PENDOWN) //上一点pressure不为0才会将VAR_PENDOWN 标识置起，因此来到这里就是提笔
            {
                var->flags |= VAR_SUBMITNOISE;
                var->noise = cur;//将提笔这点保存到noise中
                //uart_printf("up\r\n");
            }
            /* Reset the state machine on pen up events. */
            var->flags &= ~(VAR_PENDOWN | VAR_NOISEVALID | VAR_LASTVALID);//清其他标识
            goto acceptsample;
        } 
        else
            var->flags |= VAR_PENDOWN;//通知笔按下

        if (!(var->flags & VAR_LASTVALID)) 
        {
            //uart_printf("down\r\n");
            var->last = cur;//将当前值保存
            var->flags |= VAR_LASTVALID;//将VAR_LASTVALID标识置上
            continue;
        }

        if (var->flags & VAR_PENDOWN) //这个其实已经不用判断
        {
            /* Compute the distance between last sample and current */
            dist = sqr (cur.x - var->last.x) +
                   sqr (cur.y - var->last.y);

            if (dist > var->delta) 
            {
                /* Do we suspect the previous sample was a noise? */
                if (var->flags & VAR_NOISEVALID) //上一个样点可能是噪声
                {
                    //uart_printf("noise 2\r\n");
                    /* Two "noises": it's just a quick pen movement */
                    samp [count++] = var->last = var->noise;
                    var->flags = (var->flags & ~VAR_NOISEVALID) |VAR_SUBMITNOISE;//清可疑样点标识，设检查可疑样点标识
                } 
                else
                {
                    //uart_printf("noise 1\r\n");
                    var->flags |= VAR_NOISEVALID;//设成可疑样点标识
                }
                /* The pen jumped too far, maybe it's a noise ... */
                var->noise = cur;
                continue;
            } 
            else
                var->flags &= ~VAR_NOISEVALID;//清可疑样点标识
        }

acceptsample:
//#ifdef DEBUG
        //printf("VARIANCE----------------> %d %d %d\n",
            //var->last.x, var->last.y, var->last.pressure);
//#endif
        samp [count++] = var->last;
        var->last = cur;
    }

    return count;//移植到STM32后，这里应该不会运行到
}

void variance_init(struct tslib_variance *var)
{
    var->delta = TSLIB_VARIANCE_DELTA;
    var->flags = 0;
        var->delta = sqr (var->delta);
}

//_____________________________权值平均插件_____________________________
//
/* To keep things simple (avoiding division) we ensure that
 * SUM(weight) = power-of-two. Also we must know how to approximate
 * measurements when we have less than NR_SAMPHISTLEN samples.
 */
/*注意看这个表格的数据，二维数据内，前面4个值的和右移第五个数位，则为1
 *即 ((w[0][0] + w[0][1] + w[0][2] + w[0][3])>>w[0][4]) == 1
 *
 */
static const unsigned char weight [NR_SAMPHISTLEN - 1][NR_SAMPHISTLEN + 1] =
{
    /* The last element is pow2(SUM(0..3)) */
    { 5, 3, 0, 0, 3 },  /* When we have 2 samples ... */
    { 8, 5, 3, 0, 4 },  /* When we have 3 samples ... */
    { 6, 4, 3, 3, 4 },  /* When we have 4 samples ... */
};
/*对2到5个点做权重平均，点数决定于当前点是第几个点。
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
        sn = (sn - 1) & (NR_SAMPHISTLEN - 1);//说明数据链表不会超过5个数据
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
    
    //ret = info->next->ops->read(info->next, samp, nr);//从下一层(variance层)读点,nr 应该是要读取的点数
    #if 1//移植用于STM32无系统程序，直接将点通过参数传入
    ret = nr;   

    #endif
    for (s = samp; ret > 0; s++, ret--) //读到数据
    {
        if (s->pressure == 0) //压力为0
        {
            //uart_printf("pressure = 0!\r\n");
            /*
             * Pen was released. Reset the state and
             * forget all history events.
             */
            djt->nr = 0;//清零权重平均模块的积压计数
            samp [count++] = *s;//将数据直接返回
                        continue;
        }

                /* If the pen moves too fast, reset the backlog. */
        //uart_printf("nr:%d\r\n", djt->nr);        
        if (djt->nr) 
        {
            prev = (djt->head - 1) & (NR_SAMPHISTLEN - 1);//前一点索引
            
            //uart_printf("prev:%d\r\n", prev);
            
            if (sqr (s->x - djt->hist [prev].x) +
                sqr (s->y - djt->hist [prev].y) > djt->delta) //与前点移位大于阀值，说明是快速移动(在之前的一层处理，已经将飞点处理)
            {
//#ifdef DEBUG
                //uart_printf ( "DEJITTER: pen movement exceeds threshold\r\n");
//#endif
                                djt->nr = 0;
            }
        }

        djt->hist[djt->head].x = s->x;
        djt->hist[djt->head].y = s->y;
        djt->hist[djt->head].p = s->pressure;//将样点保存的积压缓冲里面
        if (djt->nr < NR_SAMPHISTLEN)//对采样样点数++，直到+到4,因为做权重平均的时候我们最多用5个点
            djt->nr++;

        /* We'll pass through the very first sample since
         * we can't average it (no history yet).
         */
        if (djt->nr == 1)//如果这是第一次采样，没有历史或者旧采样数据，直接赋值。
        {
            samp [count] = *s;//这里可能没什么用，因为数据本来就是保存在samp里面的
        }
        else// 如果不是第一次采样，就执行平均函数，求得经过平均后的采样值
        {
            average (djt, samp + count);//做权重平均
            //samp [count].tv = s->tv;//这个是LINUX系统里面获取的时间
        }
        count++;

        djt->head = (djt->head + 1) & (NR_SAMPHISTLEN - 1);//巧妙的处理了循环问题，不过只有当NR_SAMPHISTLEN是:0x01+1,
                                                        //0x03+1,0x07+1,0x0f+1,,,,这些值的时候才能这样做    
        //uart_printf("head:%d\r\n", djt->head);                                                
    }

    return count;
}

//初始化
void dejitter_init(struct tslib_dejitter *djt)
{
    djt->nr = 0;//积压数量
    djt->delta = TSLIB_DEJITTER_DELTA;//门阀值
       djt->head = 0;//第一个积压值的索引
    djt->delta = sqr (djt->delta);//为什么做平方?
}
/****************************************************************************
**Description:          TSLIB处理接口，供无系统调用
**Input parameters:  insamp 传入一个样点
**Output parameters: outsamp 返回经过处理的样点数据
**                  
**Returned value: 返回的样点数数可能是0个，1个，2个     
**                  
**Created by:           黄学佳(2012-7-10)
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
int tslib_deal(struct ts_sample *outsamp, struct ts_sample *insamp)
{
    int ret;
    if(tslib_gd == 0)
    {
        dejitter_init(&dejitter_info);//初始化权重均值模块  
        variance_init(&variance_info);//初始化滤波模块
    
        tslib_gd = 1;
        
    }
    
    ret = variance_read(&variance_info, outsamp, 1, insamp);
    
    ret = dejitter_read(&dejitter_info, outsamp, ret);//权重平均
    //uart_printf("ret:%d\r\n", ret);
    return ret;
    //calibrateTP(samp->x, samp->y, &(samp->x), &(samp->y));//校准,因为G101DS要将样点转换成LCD坐标跟BMP图片坐标，
                                                                //所以不再这里做校准转换
}

