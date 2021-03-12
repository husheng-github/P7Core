#ifndef __DEV_CIRCLEQUEUE_H
#define __DEV_CIRCLEQUEUE_H

typedef struct _str_CircleQueue
{
    u8 *m_buf;      //循环BUF指针
    volatile u16 m_in;       //起始地址
    volatile u16 m_out;      //结束地址
    u16 m_size;     //循环BUF长度
    u16 m_RUF;      //保留
}str_CircleQueue;

s32 dev_circlequeue_init(str_CircleQueue *buf, s32 size);
s32 dev_circlequeue_free(str_CircleQueue *buf);
s32 dev_circlequeue_clear(str_CircleQueue *buf);
s32 dev_circlequeue_isfull(str_CircleQueue *buf);
s32 dev_circlequeue_isempty(str_CircleQueue *buf);
u16 dev_circlequeue_getspace(str_CircleQueue *buf);
u16 dev_circlequeue_getdatanum(str_CircleQueue *buf);
u16 dev_circlequeue_write(str_CircleQueue *buf, const u8 *wbuf, u16 wlen);
u16 dev_circlequeue_read(str_CircleQueue *buf, u8 *rbuf, u16 rlen);

#endif
