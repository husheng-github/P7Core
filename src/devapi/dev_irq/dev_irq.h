#ifndef __DEV_IRQ_H
#define __DEV_IRQ_H

#define IRQn_MAX      51    //(<IRQn_MAX)

typedef struct _str_irq_handler{
    void    *m_data;
    void    (*m_func)(void *data);
    u32      m_param;                 //对于MH1902,保存触发模式
}str_irq_handler_t;


void dev_irq_default_isr(void *data);
#endif

