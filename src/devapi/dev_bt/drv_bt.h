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

//YC1021  CMD命令
#define HCI_CMD_SET_BT_ADDR       0x00    //设置BT3.0 地址
#define HCI_CMD_SET_BLE_ADDR      0x01    //设置BLE 地址
#define HCI_CMD_SET_VISIBILITY    0x02    //设置可发现和广播
#define HCI_CMD_SET_BT_NAME       0x03    //设置BT3.0 名称
#define HCI_CMD_SET_BLE_NAME      0x04    //设置BLE 名称
#define HCI_CMD_SEND_SPP_DATA     0x05    //发送BT3.0（SPP）数据
#define HCI_CMD_SEND_BLE_DATA     0x09    //发送BLE 数据
#define HCI_CMD_STATUS_REQUEST    0x0B    //请求蓝牙状态
#define HCI_CMD_SET_PAIRING_MODE  0x0C    //设置配对模式
#define HCI_CMD_SET_PINCODE       0x0D    //设置配对码
#define HCI_CMD_SET_UART_FLOW     0x0E    //设置UART 流控
#define HCI_CMD_SET_UART_BAUD     0x0F    //设置UART 波特率
#define HCI_CMD_VERSION_REQUEST   0x10    //查询模块固件版本
#define HCI_CMD_BT_DISCONNECT     0x11    //断开BT3.0 连接
#define HCI_CMD_BLE_DISCONNECT    0x12    //断开BLE 连接
#define HCI_CMD_SET_COD           0x15    //设置COD
#define HCI_CMD_SET_NVRAM         0x26    //下发NV 数据
#define HCI_CMD_ENTER_SLEEP_MODE  0x27    //进入睡眠模式
#define HCI_CMD_CONFIRM_GKEY      0x28    //Numeric Comparison 配对方式中对密钥的比较
#define HCI_CMD_SET_ADV_DATA      0x2A    //设置ADV 数据
#define HCI_CMD_POWER_REQ         0x2B    //查询模块电源电压
#define HCI_CMD_POWER_SET         0x2C    //读取电源电压功能开关
#define HCI_CMD_PASSKEY_ENTRY     0x30    //用于PASSKEY 配对方式中对密钥的比较
#define HCI_CMD_SET_GPIO          0x31    //设置GPIO
#define HCI_CMD_READ_GPIO         0x32    //读取GPIO 设置

//YC1021 EVENT
#define HCI_EVENT_BT_CONNECTED      0x00    //BT3.0 连接建立
#define HCI_EVENT_BLE_CONNECTED     0x02    //BLE 连接建立
#define HCI_EVENT_BT_DISCONNECTED   0x03    //BT3.0 连接断开
#define HCI_EVENT_BLE_DISCONNECTED  0x05    //BLE 连接断开
#define HCI_EVENT_CMD_COMPLETE      0x06    //命令已完成
#define HCI_EVENT_SPP_DATA_RECEIVED 0x07    //接收到BT3.0 数据（SPP）
#define HCI_EVENT_BLE_DATA_RECEIVED 0x08    //接收到BLE 数据
#define HCI_EVENT_I_AM_READY        0x09    //模块准备好
#define HCI_EVENT_STAUS_RESPONSE    0x0A    //状态回复
#define HCI_EVENT_NVRAM_CHANGED     0x0D    //上传NVRAM 数据
#define HCI_EVENT_UART_EXCEPTION    0x0F    //HCI 包格式错误
#define HCI_EVENT_GKEY              0x0E    //发送Numeric Comparison 配对方式中产生的密钥
#define HCI_EVENT_GET_PASSKEY       0x10    //PASSKEY 配对方式中通知MCU 返回密钥



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
