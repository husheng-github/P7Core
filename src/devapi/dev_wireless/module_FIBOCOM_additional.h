#ifndef __MODULE_FIBOCOM_ADDITIONAL_H
#define __MODULE_FIBOCOM_ADDITIONAL_H

s32 G500_bt_open(u8 type, AT_API_GET_TYPE_E at_api_get_type);
s32 G500_bt_close(AT_API_GET_TYPE_E at_api_get_type);
s32 G500_bt_get_status(AT_API_GET_TYPE_E at_api_get_type);
s32 G500_bt_get_ver(u8 *ver_buf, s32 buf_size, AT_API_GET_TYPE_E at_api_get_type);
s32 G500_bt_set_param(bt_param_t *param, AT_API_GET_TYPE_E at_api_get_type);
s32 G500_bt_get_param(bt_param_t *param, AT_API_GET_TYPE_E at_api_get_type);
s32 G500_bt_socket_create(s32 socket_id, SOCKET_TYPE type, u8 *param, u16 port, AT_API_GET_TYPE_E at_api_get_type);
s32 G500_bt_tcp_connect_report_handle(s8 *buff);
s32 G500_bt_socket_send(s32 socket_id, u8 *wbuf, s32 wlen, AT_API_GET_TYPE_E at_api_get_type);
s32 G500_bt_socket_close(s32 socket_id, AT_API_GET_TYPE_E at_api_get_type);

#endif
