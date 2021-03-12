/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:        pengxuebin     
** Created Date:    20170316 
** Version:        
** Description:    该文件处理循环队列操作
****************************************************************************/

#include "devglobal.h"

#if 0
extern s32 ddi_misc_reboot(void);

void *k_malloc(u32 nbytes)
{
    void *p;

    p = malloc(nbytes);
    if(p == NULL)  // need reset system
    {
    	#ifdef TRENDIT_CORE
//        ddi_misc_reboot();   //申请不到reset
        #endif
    }
    return p;
}


void k_free(void *ap)
{
    free(ap);
}
#else //TRENDIT_CORE

#include "ddi_misc.h"

#define __HEAP_START_ALLOC_MAXSIZE  0x7A000  //0xE0000   896K - 23K
//static volatile kal_mutexid appmalloc_mutex;
static unsigned char trenditmempool[__HEAP_START_ALLOC_MAXSIZE];

char* __HEAP_START = 0;//__section_begin("HEAP");
char* __HEAP_END = 0;//__section_end("HEAP");






#define USED_LINK   1

 
typedef struct ALLOC_HDR
{
    struct
    {
        struct ALLOC_HDR *ptr;
        unsigned int size;
    } s;
    unsigned int align;
    unsigned int pad;
} ALLOC_HDR;

static ALLOC_HDR base;//空闲链表的头结点
static ALLOC_HDR *freep = NULL;

#ifdef USED_LINK
static ALLOC_HDR used;//使用链表的头结点
static ALLOC_HDR *usedp = NULL;
#endif
/********************************************************************/
static void free_a (void *ap)
{
    ALLOC_HDR *bp, *p;

    if(__HEAP_START == NULL)
    {
        return;
    }

    bp = (ALLOC_HDR *)ap - 1;   /* point to block header */

    //uart_printf("free..%08x--",bp);
    //API_Trace(DBG_APP_INFO,"free..%08x--",bp);
    #ifdef USED_LINK
    for(p = &used; ;p = p->s.ptr)
    {
        //uart_printf("\r\n%08x\r\n", p->s.ptr);
        
        if(p->s.ptr == bp)
        {
           p->s.ptr = bp->s.ptr;
           if(p->s.ptr == NULL)
               usedp = p;
               break;
        }

        if(p->s.ptr == NULL)
        {
            //uart_printf("used link can't find this node\r\n");
            //break;
            return;//待释放内存不在已申请链表上，
        }
    }
    #endif
    
    //找到插入点的前一个空闲块跟后一个空闲块
    for (p = freep; !((bp > p) && (bp < p->s.ptr)) ; p = p->s.ptr)
    {
        if ((p >= p->s.ptr) && ((bp > p) || (bp < p->s.ptr)))
        {
            break; /* freed block at start or end of arena */
        }
    }

    if ((bp + bp->s.size) == p->s.ptr)//能跟前一块合并
    {
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    }
    else//
    {
        bp->s.ptr = p->s.ptr;//不能合并，用指针连起
    }

    if ((p + p->s.size) == bp)//能跟后一块合并
    {
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    }
    else//不能合并，用指针连在一起
    {
        p->s.ptr = bp;
    }

    freep = p;
    //API_Trace(DBG_APP_INFO,"ok..\r\n");
    //uart_printf("ok..\r\n");
}

/********************************************************************/
static void *malloc_a (unsigned nbytes)
{
    /* Get addresses for the HEAP start and end */

   
    ALLOC_HDR *p, *prevp;
    unsigned nunits;



    if(nbytes <= 0)//防止被骗
        nbytes = 8;
    
    nunits = ((nbytes + sizeof(ALLOC_HDR)-1) / sizeof(ALLOC_HDR)) + 1;//求出实际要分配的块数

    if ((prevp = freep) == NULL)//空闲链表未建立
    {
        p = (ALLOC_HDR *)__HEAP_START;  //首地址
        p->s.size = ( ((unsigned int)__HEAP_END - (unsigned int)__HEAP_START)   //大小
            / sizeof(ALLOC_HDR) );
        p->s.ptr = &base;
        base.s.ptr = p;     //等于base.s.ptr = (ALLOC_HDR *)__HEAP_START;
        base.s.size = 0;    //如果只剩base说明没内存了
        prevp = freep = &base;

        //建立使用链表
        #ifdef USED_LINK
        used.s.size = 0;
        used.s.ptr = NULL;//说明使用列表为空
        usedp = &used;
        #endif
    }

    for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr)   //轮询链表，寻找合适的块
    {
       
        if (p->s.size >= nunits)
        {
            if (p->s.size == nunits)
            {
                prevp->s.ptr = p->s.ptr;
            }
            else
            {
                p->s.size -= nunits;
                p += p->s.size;     //P 指向将分配出去的空间
                p->s.size = nunits; //记录分配的大小，这里不用设置ptr了，因为被分配出去了
            }
            freep = prevp;  //

            //---添加到使用链表----
            #ifdef USED_LINK
            usedp->s.ptr = p;
            usedp = p;
            usedp->s.ptr = NULL;
            #endif
            //API_Trace(DBG_APP_INFO,"ok %08x\r\n", p);
            //uart_printf("ok %08x\r\n", p);
            return (void *)(p + 1); //减去头结构体才是真正分配的内存
        }

        if (p == freep)
        {
            //uart_printf("err\r\n");
            return NULL;
        }
    }
}
/*
对一个块做标记, 以后需要考虑通过线程来识别
1 core申请
2 APP申请

*/ 

static s32 malloc_set_flag(void *ap, s32 flag, u32 time)
{
    ALLOC_HDR *bp;

    bp = (ALLOC_HDR *)ap - 1;    

    bp->align = flag;
    bp->pad = (s32)time;
}

/*
统计空闲内存总量与块数 检测碎片情况
*/
void malloc_free_count (void)
{
//    #ifndef __EXT_BOOTLOADER__
    u32 HeapFreeSize = 0;
    u32 HeapFreeBlock = 0;
    
    /* Get addresses for the HEAP start and end */
    //char* __HEAP_START = __section_begin("HEAP");
    //char* __HEAP_END = __section_end("HEAP");
    
   
    ALLOC_HDR *p, *prevp;
    unsigned nunits;

    if ((prevp = freep) == NULL)    //第一次分配内存，需要初始化内存分配
    {
        p = (ALLOC_HDR *)__HEAP_START;  //首地址
        p->s.size = ( ((unsigned int)__HEAP_END - (unsigned int)__HEAP_START)   //大小
            / sizeof(ALLOC_HDR) );
        p->s.ptr = &base;
        base.s.ptr = p;
        base.s.size = 0;
        prevp = freep = &base; 
    }

    HeapFreeSize = 0;
    HeapFreeBlock = 0;
    
    for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr)
    {
        HeapFreeSize += p->s.size;
        HeapFreeBlock++;

        if (p == freep)
        {
            dev_debug_printf("--heap, no use size:%d, block:%d\r\n", HeapFreeSize*sizeof(ALLOC_HDR), HeapFreeBlock);
            return;
        }
    }
    
//    #endif
    
}

void malloc_used_count (void)
{
    //#ifndef __EXT_BOOTLOADER__
    ALLOC_HDR *p;
    u32 appusedsize = 0;
    u32 coreusedsize = 0;
    
    u32 cnt = 0;

    #ifdef USED_LINK
    dev_debug_printf("----malloc used ---\r\n");
    for(p = used.s.ptr;;p = p->s.ptr)
    {
        if(p == NULL)
        {
            dev_debug_printf("--- total:%d block,%d byte\r\n---core:%d byte, app:%d byte\r\n", 
                                        cnt,(appusedsize+coreusedsize)*sizeof(ALLOC_HDR), 
                                             coreusedsize*sizeof(ALLOC_HDR), appusedsize*sizeof(ALLOC_HDR)); 
            return;
        }
        else
        {
            if(p->align == 2)
                appusedsize += p->s.size;
            else
                coreusedsize += p->s.size;
                
            cnt++;
            dev_debug_printf("-%d-%d-%d_%d\r\n", cnt, p->align, (p->s.size)*sizeof(ALLOC_HDR), p->pad);
        }
    }
    //#endif
    
    //return;
    //#endif
    
}

u32 appusedsize_bak = 0;
u32 coreusedsize_bak = 0;
u32 totalsize_bak = 0;
/*
检测最大峰值
*/
void malloc_used_check (void)
{
#if 0
    ALLOC_HDR *p;
    u32 appusedsize = 0;
    u32 coreusedsize = 0;
    
    u32 cnt = 0;

    #ifdef USED_LINK
    //sys_uart_printf("----malloc used ---\r\n");
    for(p = used.s.ptr;;p = p->s.ptr)
    {
        if(p == NULL)
        {
            if(appusedsize > appusedsize_bak)
            {
                
                sys_uart_printf("\r\n\r\n---app max:%d byte \r\n\r\n\r\n", appusedsize*sizeof(ALLOC_HDR)); 
                appusedsize_bak = appusedsize;
                
            }

            if(coreusedsize > coreusedsize_bak)
            {
                sys_uart_printf("\r\n\r\n---core max:%d byte \r\n\r\n\r\n", coreusedsize*sizeof(ALLOC_HDR));
                coreusedsize_bak = coreusedsize;                             
            }

            if(appusedsize+coreusedsize > totalsize_bak)
            {
                sys_uart_printf("\r\n\r\n---total max:%d byte \r\n\r\n\r\n", (appusedsize+coreusedsize)*sizeof(ALLOC_HDR));    
                totalsize_bak = appusedsize+coreusedsize;
            }
                                         
            return;
        }
        else
        {
            if(p->align == 2)
                appusedsize += p->s.size;
            else
                coreusedsize += p->s.size;
                
            cnt++;
            //sys_uart_printf("-%d-%d-%d_%d\r\n", cnt, p->align, (p->s.size)*sizeof(ALLOC_HDR), p->pad);
        }
    }
    #endif
#endif
    
    return;
}




void k_free(void *ap)
{

    if(ap == NULL)
        return;

    /*#ifndef __EXT_BOOTLOADER__
    kal_take_mutex(appmalloc_mutex);
    #endif*/
    free_a(ap);
    ap = NULL;
    /*#ifndef __EXT_BOOTLOADER__
    kal_give_mutex(appmalloc_mutex);
    #endif*/
}


void *k_malloc(unsigned nbytes)
{   
    void * p;
    u32 time = 0;

    if(nbytes == 0)
    {
        return NULL;
    }

    if(__HEAP_START == NULL)
    {
         //app_mem.h 的 applib_mem_ap_id_enum
         //#ifdef __EXT_BOOTLOADER__
         __HEAP_START  = (char *)&trenditmempool[0];
        //#else
        //__HEAP_START  = (char *)applib_mem_ap_alloc(APPLIB_MEM_AP_ID_TRENDITAPP,__HEAP_START_ALLOC_MAXSIZE);
        // #endif
        if(__HEAP_START == NULL)
        {
            return NULL;
        }
        __HEAP_END = __HEAP_START + __HEAP_START_ALLOC_MAXSIZE;
        
//        #ifndef __EXT_BOOTLOADER__
//        appmalloc_mutex = kal_create_mutex("appmalloc_mutex");
//        #endif
        
    }

    
    
//    #ifndef __EXT_BOOTLOADER__
 //   kal_take_mutex(appmalloc_mutex);
//    #endif
    p = malloc_a(nbytes);
    
    if(p == NULL)
    {
         //API_Trace(DBG_APP_INFO,"malloc error:%x",nbytes);
    }
    
    if(p == NULL)
    {
        //sys_uart_printf("----core malloc %d失败----\r\n", nbytes);
        malloc_used_count();
        malloc_free_count();
    }
    else
    {
        malloc_set_flag(p, 1, time);
        malloc_used_check();
    }
    
    
//    #ifndef __EXT_BOOTLOADER__    
//    kal_give_mutex(appmalloc_mutex);
//    #endif
    
    return p;
    
}


void *k_calloc(unsigned int n, unsigned int size)
{
    void * p;

    p = k_malloc(n*size);
    if(p != NULL)
    {
        memset((s8 *)p,0,n*size);
    }

    return p;
}



void *k_realloc(void *ap, unsigned int size)
{
    void * allocp;
    ALLOC_HDR *bp, *p;
    u32 nbytes;
    u32 copysize;

    allocp = k_malloc(size);
    if(allocp == NULL)
    {
        return NULL;
    }
    
    if(ap != NULL)
    {
        if(__HEAP_START != NULL)
        {
             bp = (ALLOC_HDR *)ap - 1;   /* point to block header */

            //uart_printf("free..%08x--",bp);
            //API_Trace(DBG_APP_INFO,"free..%08x--",bp);
            #ifdef USED_LINK
            for(p = &used; ;p = p->s.ptr)
            {
                //uart_printf("\r\n%08x\r\n", p->s.ptr);
                if(p->s.ptr == bp)
                {
                    if(p->s.size > 1)
                    {
                        nbytes = (p->s.size - 1)*sizeof(ALLOC_HDR);
                        copysize = nbytes;
                        if(size < copysize)
                        {
                            copysize = size;
                        }
                        
                        memcpy(allocp,ap,copysize);
                        
                    }
                    break;
                }
                
                if(p->s.ptr == NULL)
                {
                    break;
                }
                
                
            }
            #endif
        }

           k_free(ap);
    }
    
    return allocp;
}




void ddi_mem_printfallinfo(u8 type)
{
    malloc_used_count();
    malloc_free_count();
}

//#ifndef __EXT_BOOTLOADER__
/**
 * @brief 获取系统剩余内存
 * @param[out] mem_info 用来填充的内存buff
 * @retval  DEVSTATUS_SUCCESS 获取成功
 * @retval  <0 获取失败
*/
s32 ddi_misc_get_mem_info(mem_info_t *mem_info)
{
    u32 HeapFreeSize = 0;
    u32 HeapFreeBlock = 0;
    s32 ret = DEVSTATUS_ERR_FAIL;
    
    /* Get addresses for the HEAP start and end */
    //char* __HEAP_START = __section_begin("HEAP");
    //char* __HEAP_END = __section_end("HEAP");
    
   
    ALLOC_HDR *p, *prevp;
    unsigned nunits;

    do{
        if(NULL == mem_info)
        {
            ret = DEVSTATUS_ERR_PARAM_ERR;
            break;
        }
        
        if ((prevp = freep) == NULL)    //第一次分配内存，需要初始化内存分配
        {
            p = (ALLOC_HDR *)__HEAP_START;  //首地址
            p->s.size = ( ((unsigned int)__HEAP_END - (unsigned int)__HEAP_START)   //大小
                / sizeof(ALLOC_HDR) );
            p->s.ptr = &base;
            base.s.ptr = p;
            base.s.size = 0;
            prevp = freep = &base; 
        }

        HeapFreeSize = 0;
        HeapFreeBlock = 0;
        
        for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr)
        {
            HeapFreeSize += p->s.size;
            HeapFreeBlock++;

            if (p == freep)
            {
                mem_info->m_remaining_memory = HeapFreeSize*sizeof(ALLOC_HDR);
                mem_info->m_remaining_block = HeapFreeBlock;
                ret = DEVSTATUS_SUCCESS;
                break;
            }
        }
    }while(0);
        
    return ret;
}

#endif

#endif

