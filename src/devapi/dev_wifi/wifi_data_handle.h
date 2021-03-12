#ifndef __WIFI_DATA_HANDLE_H
#define __WIFI_DATA_HANDLE_H

typedef struct{
    u16 m_rlen;                 //����ָ������
    u16 m_rcnt;                 //���ճ��ȼ���
    u8  m_rbuf[WIFIAT_RXBUF_MAX];  //����BUF
    u16 m_rlencnt;              //��¼������ʣ��δ�������ݳ���
    u8 m_socketid;              //������յ�socketid
    volatile u8 m_special;      //bit0   1:����ָ��������������
                                //bit1   1:��������ģʽ(�յ���������Ȼ��з��� >��Ȼ��ʼ���մ�������)
                                //bit2   1:��ʼ������Ready��־
                                //bit3   1:�յ�ָ������
}wifi_parse_t;

//����ģ�鷵�ص����ݶ�Ӧ
typedef struct{
    WIFI_AT_RESULT_E m_wifi_at_result;
    s8 m_resp_words[32];
}wifi_at_result_info_t;

void dev_wifi_set_parse_data(u8 *data, u16 len);
void dev_wifi_init_parse_data(void);
s32 dev_wifi_data_handle(void);

#endif
