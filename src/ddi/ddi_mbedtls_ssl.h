
#ifndef __DDI_MBEDTLS_SSL_H__
#define __DDI_MBEDTLS_SSL_H__


#include "ddi_common.h"

#define MBEDTLS_NETWORK_TYPE_GPRS (0)
#define MBEDTLS_NETWORK_TYPE_WIFI (1)
#define MBEDTLS_NETWORK_TYPE_UNKOWN (2)

typedef struct _TRENDIT_CERT_INFO{	
	u8 PK[512];	
	u32 PKLen;	
	s8 subject[128];	
	s8 issuer[128];	
	s8 serial[32];
}TRENDIT_CERT_INFO;

#define DDI_HASHTYPE_1     0
#define DDI_HASHTYPE_256   1

s32 ddi_mbedtls_ssl_open(s32 socket_id);
s32 ddi_mbedtls_ssl_close(void);
s32 ddi_mbedtls_ssl_set_ca_cert(u8* cert, u32 cert_len);
s32 ddi_mbedtls_ssl_set_client_private_key(u8* pk, u32 pk_len);
s32 ddi_mbedtls_ssl_set_client_cert(u8* cert, u32 cert_len);
s32 ddi_mbedtls_ssl_handshake(u8* fail_msg, u32 fail_msg_len);
s32 ddi_mbedtls_ssl_send(const u8* buf, u32 buf_size);
s32 ddi_mbedtls_ssl_recv(unsigned char* buf, u32 buf_size);
s32 ddi_mbedtls_ssl_set_host_name(const char* host_name);
s32 ddi_mbedtls_ioctl(u32 nCmd, u32 lParam, u32 wParam);
s32 ddi_mbedtls_verify_certchain(u8 **certs, u32 certsNum);         
s32 ddi_mbedtls_parse_cert_info(const  u8 *cert, u32 certLen, TRENDIT_CERT_INFO *certInfo);    
s32 ddi_mbedtls_pk_verify_sign(u8 digst_alg, const unsigned char *hash, unsigned char *sig, u8 *PK);
s32 ddi_mbedtls_sk_verify_sign(u8 digst_alg, const unsigned char *hash, unsigned char *sig, u8 *SK);
s32 ddi_mbedtls_pk_sign(u8 digst_alg, const unsigned char *hash, unsigned char *sig, u8 *PK);
s32 ddi_mbedtls_sk_sign(u8 digst_alg, const unsigned char *hash, unsigned char *sig, u8 *SK);


//=====================================================
typedef s32 (*core_ddi_mbedtls_ssl_open)(s32 socket_id);
typedef s32 (*core_ddi_mbedtls_ssl_close)(void);
typedef s32 (*core_ddi_mbedtls_ssl_set_ca_cert)(u8* cert, u32 cert_len);
typedef s32 (*core_ddi_mbedtls_ssl_set_client_private_key)(u8* pk, u32 pk_len);
typedef s32 (*core_ddi_mbedtls_ssl_set_client_cert)(u8* cert, u32 cert_len);
typedef s32 (*core_ddi_mbedtls_ssl_handshake)(u8* fail_msg, u32 fail_msg_len);
typedef s32 (*core_ddi_mbedtls_ssl_send)(const u8* buf, u32 buf_size);
typedef s32 (*core_ddi_mbedtls_ssl_recv)(unsigned char* buf, u32 buf_size);
typedef s32 (*core_ddi_mbedtls_ssl_set_host_name)(const char* host_name);
typedef s32 (*core_ddi_mbedtls_ioctl)(u32 nCmd, u32 lParam, u32 wParam);
typedef s32 (*core_ddi_mbedtls_verify_certchain)(u8 **certs, u32 certsNum);         
typedef s32 (*core_ddi_mbedtls_parse_cert_info)(const  u8 *cert, u32 certLen, TRENDIT_CERT_INFO *certInfo);    
typedef s32 (*core_ddi_mbedtls_pk_verify_sign)(u8 digst_alg, const unsigned char *hash, unsigned char *sig, u8 *PK);
typedef s32 (*core_ddi_mbedtls_sk_verify_sign)(u8 digst_alg, const unsigned char *hash, unsigned char *sig, u8 *SK);
typedef s32 (*core_ddi_mbedtls_pk_sign)(u8 digst_alg, const unsigned char *hash, unsigned char *sig, u8 *PK);
typedef s32 (*core_ddi_mbedtls_sk_sign)(u8 digst_alg, const unsigned char *hash, unsigned char *sig, u8 *SK);

#endif