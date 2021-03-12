/********************************Copyright ( c ) ***************************
**                  Shenzhen Xinguodu Technology Co., LTD.
**                      http://www.xinguodu.com
**
**
**Created By:       »ÆÑ§¼Ñ
**Created Date:     2012-7-10
**Version:          <version >
**Description:  ´¥ÃşÆÁ´¦ÀíÖĞ¼ä¼ş£¬´ÓTSLIBÒÆÖ²À´µÄ£¬²»¹ıÓĞÒ»Ğ©ĞŞ¸Ä£¬ÒÔ±ãÊÊÓ¦ÓÚ
                ÎŞÏµÍ³³ÌĞò
**              ´¥Ãş´¦ÀíÁ÷³Ì»ù±¾ÊÇ
    Ô­Ê¼Êı¾İ->variance(ÔëÉùÂË²¨)->È¨ÖµÆ½¾ù(Æ½»¬)->Ğ£×¼(×ª³ÉLCDÉÏµÄµã)
**modify: ver 1.0   2012.07.11
****************************************************************************/
#include "tslib.h"

#define TSLIB_DEJITTER_DELTA (50)//(100)//dejitter ²å¼şµÄÃÅ·§//ÓÃÓÚÅĞ¶ÏÊÇ·ñÊÇ¿ÉÒÉÑùµã
#define TSLIB_VARIANCE_DELTA (20)//(30)//ÓÃÀ´ÅĞ¶ÏÊÇ·ñÊÇ¿ìËÙÒÆ¶¯


static int tslib_gd = 0;

calibration cal;//´¥ÃşÆÁĞ£×¼Êı¾İ cal.x¸úcal.yÊÇ´¥ÃşÆÁÖµ£¬cal.xfb,cal.xfbÊÇLCD×ø±ê
            //cal.aÊı¾İÀïÃæÊÇĞ£×¼ºóµÃµ½µÄ7¸öÊıÖµ£¬
struct tslib_dejitter dejitter_info;
struct tslib_variance variance_info;

static int sqr (int x)
{
    return x * x;
}

//_____________________________Ğ£×¼²å¼ş_____________________________________

/****************************************************************************
**Description:          ÎåµãĞ£×¼Ëã·¨£¬µ÷ÓÃ±¾º¯ÊıÇ°Çë¶Ôcal¸³Öµ
**Input parameters:     cal
**Output parameters:    cal.a
**                  
**Returned value:       
**                  
**Created by:           »ÆÑ§¼Ñ(2012-6-28)
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
**Description:      ¸ù¾İ±£´æµÄĞ£×¼Êı¾İ£¬Ëã³öÊäÈë´¥Ãşµã¶ÔÓ¦µÄLCD×ø±ê
**Input parameters: 
**Output parameters:    
**                  
**Returned value:       
**                  
**Created by:           »ÆÑ§¼Ñ(2012-6-28)
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
//_____________________________ÂË²¨²å¼ş_____________________________

//variance_read º¯ÊıÓöµ½¿ÉÒÉÑùµãµÄÊ±ºò£¬
//static int variance_read(struct tslib_module_info *info, struct ts_sample *samp, int nr)
//»ÆÑ§¼Ñ£¬ĞŞ¸Äº¯Êı²ÎÊı£¬ÒÔ±ãÓÃÓÚSTM32£¬Ôö¼ÓÓÃÓÚÊäÈëÑùµãµÄ²ÎÊı
//½¨ÒéÃ¿´ÎÊäÈëÒ»¸öÑùµã£¬Êä³öÑùµã»áÊÇ0¸ö£¬»òÕß1¸ö£¬»òÕßÁ½¸ö£¬
static int variance_read(struct tslib_variance *info, struct ts_sample *samp, int nr, struct ts_sample *insamp)
{
    struct tslib_variance *var = (struct tslib_variance *)info;
    struct ts_sample cur;
    int count = 0, dist;
    
    int readcount = 0;//»ÆÑ§¼Ñ£¬ 2012.07.10 ÓÃÀ´¼ÇÂ¼´Óµ×²ã»ñÈ¡µÄÑùµãÊı

    //while (count < nr)
    while(1)//»ÆÑ§¼Ñ
    {
        //Èç¹ûN+1£¬¸úN+2Á½¸öµã¸úNµãµÄ¾àÀë³¬³ö·§Öµ£¬Ôò¿ÉÄÜÊÇ¿ìËÙÒÆ¶¯£¬ÕâÊ±»á½«VAR_SUBMITNOISE±êÊ¶ÖÃÆğ£¬
        //ÖÃÕâ¸ö±êÊ¶ÊÇÎªÁËÏÂÒ»¸öÑ­»·ÅĞ¶ÏN+1¸úN+2Ö®¼äÊÇ·ñÒ²³¬³ö·§Öµ£¬
        //
        if (var->flags & VAR_SUBMITNOISE) //VAR_SUBMITNOISE±êÊ¶±»ÖÃÆğ£¬
        {
            cur = var->noise;//²»ÖØĞÂ»ñÈ¡ĞÂÑùµã£¬¶øÊÇ½«noiseÖĞµÄÑùµã×÷Îªµ±Ç°Ñùµã
            var->flags &= ~VAR_SUBMITNOISE;//Çå±êÊ¶
        } 
        else 
        {
            #if 0
            if (info->next->ops->read(info->next, &cur, 1) < 1)//´ÓÉÏÒ»²ã»ñÈ¡Ñùµã
                return count;   
            #else
                if(readcount >= nr)
                    return count;
                
                cur = *insamp;//´Óµ×²ã»ñÈ¡Ñùµã
                readcount++;
            #endif  
        }
        
        if (cur.pressure == 0) //ÎŞÑ¹Á¦
        {
            /* Flush the queue immediately when the pen is just
             * released, otherwise the previous layer will
             * get the pen up notification too late. This 
             * will happen if info->next->ops->read() blocks.
             */
            if (var->flags & VAR_PENDOWN) //ÉÏÒ»µãpressure²»Îª0²Å»á½«VAR_PENDOWN ±êÊ¶ÖÃÆğ£¬Òò´ËÀ´µ½ÕâÀï¾ÍÊÇÌá±Ê
            {
                var->flags |= VAR_SUBMITNOISE;
                var->noise = cur;//½«Ìá±ÊÕâµã±£´æµ½noiseÖĞ
                //uart_printf("up\r\n");
            }
            /* Reset the state machine on pen up events. */
            var->flags &= ~(VAR_PENDOWN | VAR_NOISEVALID | VAR_LASTVALID);//ÇåÆäËû±êÊ¶
            goto acceptsample;
        } 
        else
            var->flags |= VAR_PENDOWN;//Í¨Öª±Ê°´ÏÂ

        if (!(var->flags & VAR_LASTVALID)) 
        {
            //uart_printf("down\r\n");
            var->last = cur;//½«µ±Ç°Öµ±£´æ
            var->flags |= VAR_LASTVALID;//½«VAR_LASTVALID±êÊ¶ÖÃÉÏ
            continue;
        }

        if (var->flags & VAR_PENDOWN) //Õâ¸öÆäÊµÒÑ¾­²»ÓÃÅĞ¶Ï
        {
            /* Compute the distance between last sample and current */
            dist = sqr (cur.x - var->last.x) +
                   sqr (cur.y - var->last.y);

            if (dist > var->delta) 
            {
                /* Do we suspect the previous sample was a noise? */
                if (var->flags & VAR_NOISEVALID) //ÉÏÒ»¸öÑùµã¿ÉÄÜÊÇÔëÉù
                {
                    //uart_printf("noise 2\r\n");
                    /* Two "noises": it's just a quick pen movement */
                    samp [count++] = var->last = var->noise;
                    var->flags = (var->flags & ~VAR_NOISEVALID) |VAR_SUBMITNOISE;//Çå¿ÉÒÉÑùµã±êÊ¶£¬Éè¼ì²é¿ÉÒÉÑùµã±êÊ¶
                } 
                else
                {
                    //uart_printf("noise 1\r\n");
                    var->flags |= VAR_NOISEVALID;//Éè³É¿ÉÒÉÑùµã±êÊ¶
                }
                /* The pen jumped too far, maybe it's a noise ... */
                var->noise = cur;
                continue;
            } 
            else
                var->flags &= ~VAR_NOISEVALID;//Çå¿ÉÒÉÑùµã±êÊ¶
        }

acceptsample:
//#ifdef DEBUG
        //printf("VARIANCE----------------> %d %d %d\n",
            //var->last.x, var->last.y, var->last.pressure);
//#endif
        samp [count++] = var->last;
        var->last = cur;
    }

    return count;//ÒÆÖ²µ½STM32ºó£¬ÕâÀïÓ¦¸Ã²»»áÔËĞĞµ½
}

void variance_init(struct tslib_variance *var)
{
    var->delta = TSLIB_VARIANCE_DELTA;
    var->flags = 0;
        var->delta = sqr (var->delta);
}

//_____________________________È¨ÖµÆ½¾ù²å¼ş_____________________________
//
/* To keep things simple (avoiding division) we ensure that
 * SUM(weight) = power-of-two. Also we must know how to approximate
 * measurements when we have less than NR_SAMPHISTLEN samples.
 */
/*×¢Òâ¿´Õâ¸ö±í¸ñµÄÊı¾İ£¬¶şÎ¬Êı¾İÄÚ£¬Ç°Ãæ4¸öÖµµÄºÍÓÒÒÆµÚÎå¸öÊıÎ»£¬ÔòÎª1
 *¼´ ((w[0][0] + w[0][1] + w[0][2] + w[0][3])>>w[0][4]) == 1
 *
 */
static const unsigned char weight [NR_SAMPHISTLEN - 1][NR_SAMPHISTLEN + 1] =
{
    /* The last element is pow2(SUM(0..3)) */
    { 5, 3, 0, 0, 3 },  /* When we have 2 samples ... */
    { 8, 5, 3, 0, 4 },  /* When we have 3 samples ... */
    { 6, 4, 3, 3, 4 },  /* When we have 4 samples ... */
};
/*¶Ô2µ½5¸öµã×öÈ¨ÖØÆ½¾ù£¬µãÊı¾ö¶¨ÓÚµ±Ç°µãÊÇµÚ¼¸¸öµã¡£
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
        sn = (sn - 1) & (NR_SAMPHISTLEN - 1);//ËµÃ÷Êı¾İÁ´±í²»»á³¬¹ı5¸öÊı¾İ
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
    
    //ret = info->next->ops->read(info->next, samp, nr);//´ÓÏÂÒ»²ã(variance²ã)¶Áµã,nr Ó¦¸ÃÊÇÒª¶ÁÈ¡µÄµãÊı
    #if 1//ÒÆÖ²ÓÃÓÚSTM32ÎŞÏµÍ³³ÌĞò£¬Ö±½Ó½«µãÍ¨¹ı²ÎÊı´«Èë
    ret = nr;   

    #endif
    for (s = samp; ret > 0; s++, ret--) //¶Áµ½Êı¾İ
    {
        if (s->pressure == 0) //Ñ¹Á¦Îª0
        {
            //uart_printf("pressure = 0!\r\n");
            /*
             * Pen was released. Reset the state and
             * forget all history events.
             */
            djt->nr = 0;//ÇåÁãÈ¨ÖØÆ½¾ùÄ£¿éµÄ»ıÑ¹¼ÆÊı
            samp [count++] = *s;//½«Êı¾İÖ±½Ó·µ»Ø
                        continue;
        }

                /* If the pen moves too fast, reset the backlog. */
        //uart_printf("nr:%d\r\n", djt->nr);        
        if (djt->nr) 
        {
            prev = (djt->head - 1) & (NR_SAMPHISTLEN - 1);//Ç°Ò»µãË÷Òı
            
            //uart_printf("prev:%d\r\n", prev);
            
            if (sqr (s->x - djt->hist [prev].x) +
                sqr (s->y - djt->hist [prev].y) > djt->delta) //ÓëÇ°µãÒÆÎ»´óÓÚ·§Öµ£¬ËµÃ÷ÊÇ¿ìËÙÒÆ¶¯(ÔÚÖ®Ç°µÄÒ»²ã´¦Àí£¬ÒÑ¾­½«·Éµã´¦Àí)
            {
//#ifdef DEBUG
                //uart_printf ( "DEJITTER: pen movement exceeds threshold\r\n");
//#endif
                                djt->nr = 0;
            }
        }

        djt->hist[djt->head].x = s->x;
        djt->hist[djt->head].y = s->y;
        djt->hist[djt->head].p = s->pressure;//½«Ñùµã±£´æµÄ»ıÑ¹»º³åÀïÃæ
        if (djt->nr < NR_SAMPHISTLEN)//¶Ô²ÉÑùÑùµãÊı++£¬Ö±µ½+µ½4,ÒòÎª×öÈ¨ÖØÆ½¾ùµÄÊ±ºòÎÒÃÇ×î¶àÓÃ5¸öµã
            djt->nr++;

        /* We'll pass through the very first sample since
         * we can't average it (no history yet).
         */
        if (djt->nr == 1)//Èç¹ûÕâÊÇµÚÒ»´Î²ÉÑù£¬Ã»ÓĞÀúÊ·»òÕß¾É²ÉÑùÊı¾İ£¬Ö±½Ó¸³Öµ¡£
        {
            samp [count] = *s;//ÕâÀï¿ÉÄÜÃ»Ê²Ã´ÓÃ£¬ÒòÎªÊı¾İ±¾À´¾ÍÊÇ±£´æÔÚsampÀïÃæµÄ
        }
        else// Èç¹û²»ÊÇµÚÒ»´Î²ÉÑù£¬¾ÍÖ´ĞĞÆ½¾ùº¯Êı£¬ÇóµÃ¾­¹ıÆ½¾ùºóµÄ²ÉÑùÖµ
        {
            average (djt, samp + count);//×öÈ¨ÖØÆ½¾ù
            //samp [count].tv = s->tv;//Õâ¸öÊÇLINUXÏµÍ³ÀïÃæ»ñÈ¡µÄÊ±¼ä
        }
        count++;

        djt->head = (djt->head + 1) & (NR_SAMPHISTLEN - 1);//ÇÉÃîµÄ´¦ÀíÁËÑ­»·ÎÊÌâ£¬²»¹ıÖ»ÓĞµ±NR_SAMPHISTLENÊÇ:0x01+1,
                                                        //0x03+1,0x07+1,0x0f+1,,,,ÕâĞ©ÖµµÄÊ±ºò²ÅÄÜÕâÑù×ö    
        //uart_printf("head:%d\r\n", djt->head);                                                
    }

    return count;
}

//³õÊ¼»¯
void dejitter_init(struct tslib_dejitter *djt)
{
    djt->nr = 0;//»ıÑ¹ÊıÁ¿
    djt->delta = TSLIB_DEJITTER_DELTA;//ÃÅ·§Öµ
       djt->head = 0;//µÚÒ»¸ö»ıÑ¹ÖµµÄË÷Òı
    djt->delta = sqr (djt->delta);//ÎªÊ²Ã´×öÆ½·½å
}
/****************************************************************************
**Description:          TSLIB´¦Àí½Ó¿Ú£¬¹©ÎŞÏµÍ³µ÷ÓÃ
**Input parameters:  insamp ´«ÈëÒ»¸öÑùµã
**Output parameters: outsamp ·µ»Ø¾­¹ı´¦ÀíµÄÑùµãÊı¾İ
**                  
**Returned value: ·µ»ØµÄÑùµãÊıÊı¿ÉÄÜÊÇ0¸ö£¬1¸ö£¬2¸ö     
**                  
**Created by:           »ÆÑ§¼Ñ(2012-7-10)
**--------------------------------------------------------------------------
**Modified by:      
**Modified by:      
****************************************************************************/
int tslib_deal(struct ts_sample *outsamp, struct ts_sample *insamp)
{
    int ret;
    if(tslib_gd == 0)
    {
        dejitter_init(&dejitter_info);//³õÊ¼»¯È¨ÖØ¾ùÖµÄ£¿é  
        variance_init(&variance_info);//³õÊ¼»¯ÂË²¨Ä£¿é
    
        tslib_gd = 1;
        
    }
    
    ret = variance_read(&variance_info, outsamp, 1, insamp);
    
    ret = dejitter_read(&dejitter_info, outsamp, ret);//È¨ÖØÆ½¾ù
    //uart_printf("ret:%d\r\n", ret);
    return ret;
    //calibrateTP(samp->x, samp->y, &(samp->x), &(samp->y));//Ğ£×¼,ÒòÎªG101DSÒª½«Ñùµã×ª»»³ÉLCD×ø±ê¸úBMPÍ¼Æ¬×ø±ê£¬
                                                                //ËùÒÔ²»ÔÙÕâÀï×öĞ£×¼×ª»»
}

