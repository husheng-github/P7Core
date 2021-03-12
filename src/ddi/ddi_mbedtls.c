#include "ddi_mbedtls_ssl.h"

#ifdef DDI_MBEDTLS_ENABLE

s32 ddi_mbedtls_ssl_open(s32 socket_id)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_ssl_close(void)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_ssl_set_ca_cert(u8* cert, u32 cert_len)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_ssl_set_client_private_key(u8* pk, u32 pk_len)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_ssl_set_client_cert(u8* cert, u32 cert_len)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_ssl_handshake(u8* fail_msg, u32 fail_msg_len)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_ssl_send(const u8* buf, u32 buf_size)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_ssl_recv(unsigned char* buf, u32 buf_size)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_ssl_set_host_name(const char* host_name)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	return DDI_ENODEV;
}

s32 ddi_mbedtls_verify_certchain(u8 **certs, u32 certsNum)
{
	return DDI_ENODEV;
}         
s32 ddi_mbedtls_parse_cert_info(const  u8 *cert, u32 certLen, SDK_CERT_INFO *certInfo)
{
	return DDI_ENODEV;
}    
s32 ddi_mbedtls_pk_verify_sign(u8 digst_alg, const unsigned char *hash, unsigned char *sig, u8 *PK)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_sk_verify_sign(u8 digst_alg, const unsigned char *hash, unsigned char *sig, u8 *SK)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_pk_sign(u8 digst_alg, const unsigned char *hash, unsigned char *sig, u8 *PK)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_sk_sign(u8 digst_alg, const unsigned char *hash, unsigned char *sig, u8 *SK)
{
	return DDI_ENODEV;
}
#else

s32 ddi_mbedtls_ssl_open(s32 socket_id)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_ssl_close(void)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_ssl_set_ca_cert(u8* cert, u32 cert_len)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_ssl_set_client_private_key(u8* pk, u32 pk_len)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_ssl_set_client_cert(u8* cert, u32 cert_len)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_ssl_handshake(u8* fail_msg, u32 fail_msg_len)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_ssl_send(const u8* buf, u32 buf_size)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_ssl_recv(unsigned char* buf, u32 buf_size)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_ssl_set_host_name(const char* host_name)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_ioctl(u32 nCmd, u32 lParam, u32 wParam)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_verify_certchain(u8 **certs, u32 certsNum)
{
	return DDI_ENODEV;
}         
s32 ddi_mbedtls_parse_cert_info(const  u8 *cert, u32 certLen, TRENDIT_CERT_INFO *certInfo)
{
	return DDI_ENODEV;
}    
s32 ddi_mbedtls_pk_verify_sign(u8 digst_alg, const unsigned char *hash, unsigned char *sig, u8 *PK)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_sk_verify_sign(u8 digst_alg, const unsigned char *hash, unsigned char *sig, u8 *SK)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_pk_sign(u8 digst_alg, const unsigned char *hash, unsigned char *sig, u8 *PK)
{
	return DDI_ENODEV;
}
s32 ddi_mbedtls_sk_sign(u8 digst_alg, const unsigned char *hash, unsigned char *sig, u8 *SK)
{
	return DDI_ENODEV;
}
#endif
