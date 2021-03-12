#ifndef __WIRELESS_DATA_HANDLE_H
#define __WIRELESS_DATA_HANDLE_H

typedef struct{
    u16 m_rlen;                 //����ָ������
    u16 m_rcnt;                 //���ճ��ȼ���
    u8  m_rbuf[WIRELESSAT_RXBUF_MAX];  //����BUF
    u16 m_rlencnt;              //��¼������ʣ��δ�������ݳ���
    u8 m_socketid;              //������յ�socketid
    volatile u8 m_special;      //bit0   1:����ָ��������������
                                //bit1   1:��������ģʽ(�յ���������Ȼ��з��� >��Ȼ��ʼ���մ�������)
                                //bit2   1:��ʼ������Ready��־
                                //bit3   1:�յ�ָ������
}wireless_parse_t;

//����ģ�鷵�ص����ݶ�Ӧ
typedef struct{
    WIRE_AT_RESULT_E m_wire_at_result;
    s8 m_resp_words[32];
}wire_at_result_info_t;

void dev_wireless_init_parse_data(void);
s32 dev_wireless_data_handle(void);

#endif
