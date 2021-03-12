#ifndef __DEV_CIRCLEQUEUE_H
#define __DEV_CIRCLEQUEUE_H

typedef struct _str_CircleQueue
{
    u8 *m_buf;      //ѭ��BUFָ��
    volatile u16 m_in;       //��ʼ��ַ
    volatile u16 m_out;      //������ַ
    u16 m_size;     //ѭ��BUF����
    u16 m_RUF;      //����
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
