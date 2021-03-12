#ifndef __DRV_BT_H
#define __DRV_BT_H


#ifdef DEBUG_BT_EN
#define BT_DEBUG(format,...)    dev_debug_printf("%s(%d):"format, __func__, __LINE__, ##__VA_ARGS__)
#define BT_DEBUGHEX             dev_debug_printformat
#else
#define BT_DEBUG(...) 
#define BT_DEBUGHEX(...)
#endif

#define BT_HCI_PACKET_CMD      0x01
#define BT_HCI_PACHET_EVENT    0x02

//YC1021  CMD����
#define HCI_CMD_SET_BT_ADDR       0x00    //����BT3.0 ��ַ
#define HCI_CMD_SET_BLE_ADDR      0x01    //����BLE ��ַ
#define HCI_CMD_SET_VISIBILITY    0x02    //���ÿɷ��ֺ͹㲥
#define HCI_CMD_SET_BT_NAME       0x03    //����BT3.0 ����
#define HCI_CMD_SET_BLE_NAME      0x04    //����BLE ����
#define HCI_CMD_SEND_SPP_DATA     0x05    //����BT3.0��SPP������
#define HCI_CMD_SEND_BLE_DATA     0x09    //����BLE ����
#define HCI_CMD_STATUS_REQUEST    0x0B    //��������״̬
#define HCI_CMD_SET_PAIRING_MODE  0x0C    //�������ģʽ
#define HCI_CMD_SET_PINCODE       0x0D    //���������
#define HCI_CMD_SET_UART_FLOW     0x0E    //����UART ����
#define HCI_CMD_SET_UART_BAUD     0x0F    //����UART ������
#define HCI_CMD_VERSION_REQUEST   0x10    //��ѯģ��̼��汾
#define HCI_CMD_BT_DISCONNECT     0x11    //�Ͽ�BT3.0 ����
#define HCI_CMD_BLE_DISCONNECT    0x12    //�Ͽ�BLE ����
#define HCI_CMD_SET_COD           0x15    //����COD
#define HCI_CMD_SET_NVRAM         0x26    //�·�NV ����
#define HCI_CMD_ENTER_SLEEP_MODE  0x27    //����˯��ģʽ
#define HCI_CMD_CONFIRM_GKEY      0x28    //Numeric Comparison ��Է�ʽ�ж���Կ�ıȽ�
#define HCI_CMD_SET_ADV_DATA      0x2A    //����ADV ����
#define HCI_CMD_POWER_REQ         0x2B    //��ѯģ���Դ��ѹ
#define HCI_CMD_POWER_SET         0x2C    //��ȡ��Դ��ѹ���ܿ���
#define HCI_CMD_PASSKEY_ENTRY     0x30    //����PASSKEY ��Է�ʽ�ж���Կ�ıȽ�
#define HCI_CMD_SET_GPIO          0x31    //����GPIO
#define HCI_CMD_READ_GPIO         0x32    //��ȡGPIO ����

//YC1021 EVENT
#define HCI_EVENT_BT_CONNECTED      0x00    //BT3.0 ���ӽ���
#define HCI_EVENT_BLE_CONNECTED     0x02    //BLE ���ӽ���
#define HCI_EVENT_BT_DISCONNECTED   0x03    //BT3.0 ���ӶϿ�
#define HCI_EVENT_BLE_DISCONNECTED  0x05    //BLE ���ӶϿ�
#define HCI_EVENT_CMD_COMPLETE      0x06    //���������
#define HCI_EVENT_SPP_DATA_RECEIVED 0x07    //���յ�BT3.0 ���ݣ�SPP��
#define HCI_EVENT_BLE_DATA_RECEIVED 0x08    //���յ�BLE ����
#define HCI_EVENT_I_AM_READY        0x09    //ģ��׼����
#define HCI_EVENT_STAUS_RESPONSE    0x0A    //״̬�ظ�
#define HCI_EVENT_NVRAM_CHANGED     0x0D    //�ϴ�NVRAM ����
#define HCI_EVENT_UART_EXCEPTION    0x0F    //HCI ����ʽ����
#define HCI_EVENT_GKEY              0x0E    //����Numeric Comparison ��Է�ʽ�в�������Կ
#define HCI_EVENT_GET_PASSKEY       0x10    //PASSKEY ��Է�ʽ��֪ͨMCU ������Կ



//=====================================================
s32 drv_bt_param_set(u8 btmod, u8 *btname, u8 *btmac, u8 *nvram);
s32 drv_bt_param_get(u8 *btmod, u8 *btname, u8 *btmac, u8 *nvram);
void drv_bt_init(void);
void drv_bt_rst(void);
void drv_bt_wakeup_ctl(u8 flg);
s32 drv_bt_boot_swrst(void);
//s32 drv_bt_boot_setbaud(u32 baud);
s32 drv_bt_boot_patch(void);
void drv_bt_task_process(void);
void drv_bt_task_ctl_dealflg(u8 flg);
s32 drv_bt_param_set(u8 btmod, u8 *btname, u8 *btmac, u8 *nvram);
s32 drv_bt_param_get(u8 *btmod, u8 *btname, u8 *btmac, u8 *nvram);
s32 drv_bt_hci_cmd_set_bt_addr(u8 *mac);
s32 drv_bt_hci_cmd_set_ble_addr(u8 *mac);
s32 drv_bt_hci_cmd_set_visibility(u8 bt30visi, u8 bt30connect, u8 bleADV);
s32 drv_bt_hci_cmd_set_bt_name(u8 *name);
s32 drv_bt_hci_cmd_set_ble_name(u8 *name);
//s32 drv_bt_hci_cmd_send_spp_data(u8 *wbuf, u8 wlen);
//s32 drv_bt_hci_cmd_send_ble_data(u8 *attrib, u8 *wbuf, u8 wlen);
s32 drv_bt_hci_cmd_status_request(u8 *status);
s32 drv_bt_hci_cmd_set_pairing_mode(u8 mod);
s32 drv_bt_hci_cmd_set_pairing_mode(u8 mod);
s32 drv_bt_hci_cmd_set_pincode(u8 *pincode, u8 pincodelen);
s32 drv_bt_hci_cmd_set_uart_flow(u8 mod);
s32 drv_bt_hci_cmd_set_uart_baud(u32 baud);
s32 drv_bt_hci_cmd_version_request(u8 *ver);
s32 drv_bt_hci_cmd_bt_disconnect(void);
s32 drv_bt_hci_cmd_ble_disconnect(void);
s32 drv_bt_hci_cmd_set_cod(u8 *cod);
s32 drv_bt_hci_cmd_set_nvram(u8 *nvram);
s32 drv_bt_hci_cmd_sleep_mode(void);
s32 drv_bt_hci_cmd_set_adv_data(u8 *advdata, u8 advdatalen);
s32 drv_bt_hci_cmd_power_req(u8 *vol);
s32 drv_bt_hci_cmd_power_set(u8 mod);
s32 drv_bt_hci_cmd_confirm_gkey(u8 flg);
s32 drv_bt_hci_cmd_passkey_entry(u8 *passkey, u8 passkeylen);
s32 drv_bt_hci_cmd_set_gpio(u8 mod, u8 gpio, u8 value);
s32 drv_bt_hci_cmd_read_gpio(u8 gpio, u8 *val);
s32 drv_bt_send_data(u8 *wbuf, u16 wlen);
s32 drv_bt_read_data(u8 *rbuf, u16 rlen);
s32 drv_bt_disconnect(void);

#endif
