#ifndef __WIFI_DATA_HANDLE_H
#define __WIFI_DATA_HANDLE_H

typedef struct{
    u16 m_rlen;                 //接收指定长度
    u16 m_rcnt;                 //接收长度计算
    u8  m_rbuf[WIFIAT_RXBUF_MAX];  //接收BUF
    u16 m_rlencnt;              //记录该命令剩余未接收内容长度
    u8 m_socketid;              //保存接收的socketid
    volatile u8 m_special;      //bit0   1:接收指定长度数据数据
                                //bit1   1:发送数据模式(收到此命令后先换行返回 >，然后开始接收串口数据)
                                //bit2   1:初始化接收Ready标志
                                //bit3   1:收到指定数据
}wifi_parse_t;

//无线模块返回的内容对应
typedef struct{
    WIFI_AT_RESULT_E m_wifi_at_result;
    s8 m_resp_words[32];
}wifi_at_result_info_t;

void dev_wifi_set_parse_data(u8 *data, u16 len);
void dev_wifi_init_parse_data(void);
s32 dev_wifi_data_handle(void);

#endif
