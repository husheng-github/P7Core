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

/****************************************************************************
**Description:        初始化循环队列参数，申请BUF
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
                    DEVSTATUS_ERR_PARAM_ERR:   参数错误
**Created by:        pengxuebin 20170316
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_circlequeue_init(str_CircleQueue *buf, s32 size)
{
    buf->m_size = size;
    buf->m_in = 0;
    buf->m_out = 0;
    buf->m_buf = k_malloc(size);
    if(buf->m_buf == NULL)
    {
        return -1;
    }
    else
    {
        //memset(buf->m_buf, 0, size);
        return 0;
    }
}
/****************************************************************************
**Description:        释放循环队列
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
**Created by:        pengxuebin 20170316
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_circlequeue_free(str_CircleQueue *buf)
{
    if(buf->m_buf!=NULL)
    {
        buf->m_size = 0;
        buf->m_in = 0;
        buf->m_out = 0;
        k_free(buf->m_buf);
        buf->m_buf = NULL;
    }
    return 0;
}
/****************************************************************************
**Description:        清除循环队列
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:成功
**Created by:        pengxuebin 20170316
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_circlequeue_clear(str_CircleQueue *buf)
{
    buf->m_in = 0;
    buf->m_out = 0; 
    //memset(buf->m_buf, 0, size);
    return 0;
}
/****************************************************************************
**Description:        循环队列是否满
**Input parameters:    
**Output parameters: 
**Returned value:   
                    0:未满
                    1:满
**Created by:        pengxuebin 20170316
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_circlequeue_isfull(str_CircleQueue *buf)
{
    if(buf->m_size == 0)
    {
        return 1;
    }
    else if(((buf->m_in+1)== buf->m_out) || ((buf->m_in+1)== (buf->m_out+buf->m_size)))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/****************************************************************************
**Description:        循环队列是否空
**Input parameters:    
**Output parameters: 
**Returned value:   
                    0:非空
                    1:空
**Created by:        pengxuebin 20170316
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
s32 dev_circlequeue_isempty(str_CircleQueue *buf)
{
    if(buf->m_size == 0)
    {
        return 0;
    }
    else if((buf->m_out)==buf->m_in)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/****************************************************************************
**Description:        获取循环队列有效空间
**Input parameters:    
**Output parameters: 
**Returned value:   
                    0:已满
                    >0:有效空间(bytes)
**Created by:        pengxuebin 20170316
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
u16 dev_circlequeue_getspace(str_CircleQueue *buf)
{
    u16 in, out;
    
    if(buf->m_size == 0)
    {
        return 0;
    }
    in = buf->m_in;
    out = buf->m_out;
    //获取队列的空间
    if(in >= out)
    {
        return (buf->m_size-in+out-1);
    }
    else
    {
        return (out-in-1);
    }
  #if 0  
    if(buf->m_in >= buf->m_out)
    {
        return (buf->m_size - buf->m_in + buf->m_out-1);
    }
    else
    {
        return (buf->m_out- buf->m_in-1);
    }
  #endif  
}
/****************************************************************************
**Description:        获取循环队列有效数据
**Input parameters:    
**Output parameters: 
**Returned value:   
                    >=0:有效数据个数(bytes)
**Created by:        pengxuebin 20170316
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
u16 dev_circlequeue_getdatanum(str_CircleQueue *buf)
{
    if(buf->m_size == 0)
    {
        return 0;
    }
    if(buf->m_in >= buf->m_out)
    {
        return (buf->m_in - buf->m_out);
    }
    else
    {
        return (buf->m_size - buf->m_out + buf->m_in);
    }
}
/****************************************************************************
**Description:        向循环队列写数据
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=:写入的数据个数
**Created by:        pengxuebin 20170316
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
u16 dev_circlequeue_write(str_CircleQueue *buf, const u8 *wbuf, u16 wlen)
{ 
    u32 i;
    u32 cnt;
    u16 in, out;
    
    in = buf->m_in;
    out = buf->m_out;
    //获取队列的空间
    if(in >= out)
    {
        i = buf->m_size-in+out-1;
    }
    else
    {
        i = out-in-1;
    }
    //取空间和wlen最小值
    cnt = dev_maths_min(i, wlen);  
    //填充数据
    i = dev_maths_min(cnt, buf->m_size-in);
    memcpy(&buf->m_buf[in], &wbuf[0], i);
    if(cnt>i)
    {
        memcpy(&buf->m_buf[0], &wbuf[i], cnt-i);
        buf->m_in = cnt-i;
    }
    else //if(cnt == i)
    {
        i = in + cnt;
        if(i >= buf->m_size)
        {
            i -= (buf->m_size);
        }
        buf->m_in = i;
    }
    
    return cnt;
}
/****************************************************************************
**Description:        从循环队列读数据
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=:读取的数据个数
**Created by:        pengxuebin 20170316
**----------------------------------------------------------------------------------------------------------------
** Modified by:    
****************************************************************************/
u16 dev_circlequeue_read(str_CircleQueue *buf, u8 *rbuf, u16 rlen)
{ 
    u32 i;
    u32 cnt;
    u16 in, out;
    
    in = buf->m_in;
    out = buf->m_out;
    //获取队列的数据个数
    i = dev_circlequeue_getdatanum(buf);
    if(in >= out)
    {
        i = in - out;
    }
    else
    {
        i = buf->m_size-out+in;
    }
     
    //取空间和wlen最小值
    cnt = dev_maths_min(i, rlen);
    //填充数据
    i = dev_maths_min(cnt, buf->m_size-out);
    memcpy(&rbuf[0], &buf->m_buf[out], i);
    if(cnt>i)
    {
        memcpy(&rbuf[i], &buf->m_buf[0], cnt-i);
        buf->m_out = cnt-i;
    }
    else //if(cnt == i)
    {
        i = out + cnt;
        if(i >= buf->m_size)
        {
            i -= (buf->m_size);
        }
        buf->m_out = i;
    }
    
    return cnt;
}



