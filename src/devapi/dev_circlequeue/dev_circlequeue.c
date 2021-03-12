/********************************Copyright ( c ) ********************************
**                  DAQU 
**                         
**                        
**
**Created By:        pengxuebin     
** Created Date:    20170316 
** Version:        
** Description:    ���ļ�����ѭ�����в���
****************************************************************************/
#include "devglobal.h"

/****************************************************************************
**Description:        ��ʼ��ѭ�����в���������BUF
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
                    DEVSTATUS_ERR_PARAM_ERR:   ��������
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
**Description:        �ͷ�ѭ������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
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
**Description:        ���ѭ������
**Input parameters:    
**Output parameters: 
**Returned value:
                    0:�ɹ�
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
**Description:        ѭ�������Ƿ���
**Input parameters:    
**Output parameters: 
**Returned value:   
                    0:δ��
                    1:��
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
**Description:        ѭ�������Ƿ��
**Input parameters:    
**Output parameters: 
**Returned value:   
                    0:�ǿ�
                    1:��
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
**Description:        ��ȡѭ��������Ч�ռ�
**Input parameters:    
**Output parameters: 
**Returned value:   
                    0:����
                    >0:��Ч�ռ�(bytes)
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
    //��ȡ���еĿռ�
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
**Description:        ��ȡѭ��������Ч����
**Input parameters:    
**Output parameters: 
**Returned value:   
                    >=0:��Ч���ݸ���(bytes)
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
**Description:        ��ѭ������д����
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=:д������ݸ���
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
    //��ȡ���еĿռ�
    if(in >= out)
    {
        i = buf->m_size-in+out-1;
    }
    else
    {
        i = out-in-1;
    }
    //ȡ�ռ��wlen��Сֵ
    cnt = dev_maths_min(i, wlen);  
    //�������
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
**Description:        ��ѭ�����ж�����
**Input parameters:    
**Output parameters: 
**Returned value:
                    >=:��ȡ�����ݸ���
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
    //��ȡ���е����ݸ���
    i = dev_circlequeue_getdatanum(buf);
    if(in >= out)
    {
        i = in - out;
    }
    else
    {
        i = buf->m_size-out+in;
    }
     
    //ȡ�ռ��wlen��Сֵ
    cnt = dev_maths_min(i, rlen);
    //�������
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



